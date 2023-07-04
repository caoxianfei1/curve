// -*- mode:Java; tab-width:2; c-basic-offset:2; indent-tabs-mode:t -*-

package io.opencurve.curve.fs.hadoop;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.*;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.util.Progressable;

import io.opencurve.curve.fs.libfs.CurveFSMount;
import io.opencurve.curve.fs.libfs.CurveFSStat;
import io.opencurve.curve.fs.libfs.CurveFSStatVFS;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.URI;
import java.util.ArrayList;


/**
 * Known Issues:
 * <p>
 * 1. Per-file replication and block size are ignored.
 */
public class CurveFileSystem extends FileSystem {
    private static final Log LOG = LogFactory.getLog(CurveFileSystem.class);
    private URI uri;
    private Path workingDir;
    private CurveFsProto curve = null;

    /**
     * Create a new CurveFileSystem.
     */
    public CurveFileSystem() {
    }

    /**
     * Create a new CurveFileSystem.
     */
    public CurveFileSystem(Configuration conf) {
        setConf(conf);
    }

    /**
     * Create an absolute path using the working directory.
     */
    private Path makeAbsolute(Path path) {
        if (path.isAbsolute()) {
            return path;
        }
        return new Path(workingDir, path);
    }

    public URI getUri() {
        return uri;
    }

    @Override
    public void initialize(URI uri, Configuration conf) throws IOException {
        super.initialize(uri, conf);
        if (curve == null) {
            curve = new CurveTalker(conf, LOG);
        }
        curve.initialize(uri, conf);
        setConf(conf);
        this.uri = URI.create(uri.getScheme() + "://" + uri.getAuthority());
        this.workingDir = getHomeDirectory();
    }

    /**
     * Open a Curve file and attach the file handle to an FSDataInputStream.
     *
     * @param path       The file to open
     * @param bufferSize Curve does internal buffering; but you can buffer in
     *                   the Java code too if you like.
     * @return FSDataInputStream reading from the given path.
     * @throws IOException if the path DNE or is a
     *                     directory, or there is an error getting data to set up the FSDataInputStream.
     */
    public FSDataInputStream open(Path path, int bufferSize) throws IOException {
        path = makeAbsolute(path);

        // throws filenotfoundexception if path is a directory
        int fd = curve.open(path, CurveFSMount.O_RDONLY, 0);

        /* get file size */
        CurveFSStat stat = new CurveFSStat();
        curve.fstat(fd, stat);

        CurveInputStream istream = new CurveInputStream(getConf(), curve, fd,
                stat.size, bufferSize);
        return new FSDataInputStream(istream);
    }

    /**
     * Close down the CurveFileSystem. Runs the base-class close method
     * and then kills the Curve client itself.
     */
    @Override
    public void close() throws IOException {
        super.close(); // this method does stuff, make sure it's run!
        curve.shutdown();
    }

    /**
     * Get an FSDataOutputStream to append onto a file.
     *
     * @param path       The File you want to append onto
     * @param bufferSize Curve does internal buffering but you can buffer in the Java code as well if you like.
     * @param progress   The Progressable to report progress to.
     *                   Reporting is limited but exists.
     * @return An FSDataOutputStream that connects to the file on Curve.
     * @throws IOException If the file cannot be found or appended to.
     */
    public FSDataOutputStream append(Path path, int bufferSize,
                                     Progressable progress) throws IOException {
        path = makeAbsolute(path);

        if (progress != null) {
            progress.progress();
        }

        int fd = curve.open(path, CurveFSMount.O_WRONLY | CurveFSMount.O_APPEND, 0);

        if (progress != null) {
            progress.progress();
        }

        CurveOutputStream ostream = new CurveOutputStream(getConf(), curve, fd, bufferSize);
        return new FSDataOutputStream(ostream, statistics);
    }

    public Path getWorkingDirectory() {
        return workingDir;
    }

    @Override
    public void setWorkingDirectory(Path dir) {
        workingDir = makeAbsolute(dir);
    }

    /**
     * Create a directory and any nonexistent parents. Any portion
     * of the directory tree can exist without error.
     *
     * @param path  The directory path to create
     * @param perms The permissions to apply to the created directories.
     * @return true if successful, false otherwise
     * @throws IOException if the path is a child of a file.
     */
    @Override
    public boolean mkdirs(Path path, FsPermission perms) throws IOException {
        path = makeAbsolute(path);
        curve.mkdirs(path, (int) perms.toShort());
        return true;
    }

    /**
     * Create a directory and any nonexistent parents. Any portion
     * of the directory tree can exist without error.
     * Apply umask from conf
     *
     * @param f The directory path to create
     * @return true if successful, false otherwise
     * @throws IOException if the path is a child of a file.
     */
    @Override
    public boolean mkdirs(Path f) throws IOException {
        return mkdirs(f, FsPermission.getDirDefault().applyUMask(FsPermission.getUMask(getConf())));
    }

    /**
     * Get stat information on a file. This does not fill owner or group, as
     * Curve's support for these is a bit different than HDFS'.
     *
     * @param path The path to stat.
     * @return FileStatus object containing the stat information.
     * @throws FileNotFoundException if the path could not be resolved.
     */
    public FileStatus getFileStatus(Path path) throws IOException {
        path = makeAbsolute(path);

        CurveFSStat stat = new CurveFSStat();
        curve.lstat(path, stat);

        FileStatus status = new FileStatus(stat.size, stat.isDir(),
                curve.get_file_replication(path), stat.blksize, stat.m_time,
                stat.a_time, new FsPermission((short) stat.mode),
                System.getProperty("user.name"), System.getProperty("group.name"), path.makeQualified(this));

        return status;
    }

    /**
     * Get the FileStatus for each listing in a directory.
     *
     * @param path The directory to get listings from.
     * @return FileStatus[] containing one FileStatus for each directory listing;
     * null if path does not exist.
     */
    public FileStatus[] listStatus(Path path) throws IOException {
        path = makeAbsolute(path);

        if (isFile(path))
            return new FileStatus[]{getFileStatus(path)};

        String[] dirlist = curve.listdir(path);
        if (dirlist != null) {
            FileStatus[] status = new FileStatus[dirlist.length];
            for (int i = 0; i < status.length; i++) {
                status[i] = getFileStatus(new Path(path, dirlist[i]));
            }
            return status;
        } else {
            throw new FileNotFoundException("File " + path + " does not exist.");
        }
    }

    @Override
    public void setPermission(Path path, FsPermission permission) throws IOException {
        path = makeAbsolute(path);
        curve.chmod(path, permission.toShort());
    }

    @Override
    public void setTimes(Path path, long mtime, long atime) throws IOException {
        path = makeAbsolute(path);

        CurveFSStat stat = new CurveFSStat();
        int mask = 0;

        if (mtime != -1) {
            mask |= CurveFSMount.SETATTR_MTIME;
            stat.m_time = mtime;
        }

        if (atime != -1) {
            mask |= CurveFSMount.SETATTR_ATIME;
            stat.a_time = atime;
        }

        curve.setattr(path, stat, mask);
    }

    /**
     * Create a new file and open an FSDataOutputStream that's connected to it.
     *
     * @param path        The file to create.
     * @param permission  The permissions to apply to the file.
     * @param overwrite   If true, overwrite any existing file with
     *                    this name; otherwise don't.
     * @param bufferSize  Curve does internal buffering, but you can buffer
     *                    in the Java code too if you like.
     * @param replication Replication factor. See documentation on the
     *                    "curve.data.pools" configuration option.
     * @param blockSize   Ignored by Curve. You can set client-wide block sizes
     *                    via the fs.curve.blockSize param if you like.
     * @param progress    A Progressable to report back to.
     *                    Reporting is limited but exists.
     * @return An FSDataOutputStream pointing to the created file.
     * @throws IOException if the path is an
     *                     existing directory, or the path exists but overwrite is false, or there is a
     *                     failure in attempting to open for append with Curve.
     */
    public FSDataOutputStream create(Path path, FsPermission permission,
                                     boolean overwrite, int bufferSize, short replication, long blockSize,
                                     Progressable progress) throws IOException {

        path = makeAbsolute(path);

        boolean exists = exists(path);

        if (progress != null) {
            progress.progress();
        }

        int flags = CurveFSMount.O_WRONLY | CurveFSMount.O_CREAT;

        if (exists) {
            if (overwrite)
                flags |= CurveFSMount.O_TRUNC;
            else
                throw new FileAlreadyExistsException();
        } else {
            Path parent = path.getParent();
            if (parent != null)
                if (!mkdirs(parent))
                    throw new IOException("mkdirs failed for " + parent.toString());
        }

        if (progress != null) {
            progress.progress();
        }

        int fd = curve.open(path, flags, (int) permission.toShort());

        if (progress != null) {
            progress.progress();
        }

        OutputStream ostream = new CurveOutputStream(getConf(), curve, fd,
                bufferSize);
        return new FSDataOutputStream(ostream, statistics);
    }

    /**
     * Opens an FSDataOutputStream at the indicated Path with write-progress
     * reporting. Same as create(), except fails if parent directory doesn't
     * already exist.
     *
     * @param path        the file name to open
     * @param permission
     * @param overwrite   if a file with this name already exists, then if true,
     *                    the file will be overwritten, and if false an error will be thrown.
     * @param bufferSize  the size of the buffer to be used.
     * @param replication required block replication for the file.
     * @param blockSize
     * @param progress
     * @throws IOException
     * @see #setPermission(Path, FsPermission)
     * @deprecated API only for 0.20-append
     */
    @Deprecated
    public FSDataOutputStream createNonRecursive(Path path, FsPermission permission,
                                                 boolean overwrite,
                                                 int bufferSize, short replication, long blockSize,
                                                 Progressable progress) throws IOException {
        path = makeAbsolute(path);

        Path parent = path.getParent();

        if (parent != null) {
            CurveFSStat stat = new CurveFSStat();
            curve.lstat(parent, stat); // handles FileNotFoundException case
            if (stat.isFile())
                throw new FileAlreadyExistsException(parent.toString());
        }

        return this.create(path, permission, overwrite,
                bufferSize, replication, blockSize, progress);
    }

    /**
     * Rename a file or directory.
     *
     * @param src The current path of the file/directory
     * @param dst The new name for the path.
     * @return true if the rename succeeded, false otherwise.
     */
    @Override
    public boolean rename(Path src, Path dst) throws IOException {
        src = makeAbsolute(src);
        dst = makeAbsolute(dst);

        try {
            CurveFSStat stat = new CurveFSStat();
            curve.lstat(dst, stat);
            if (stat.isDir())
                return rename(src, new Path(dst, src.getName()));
            return false;
        } catch (FileNotFoundException e) {
        }

        try {
            curve.rename(src, dst);
        } catch (FileNotFoundException e) {
            throw e;
        } catch (Exception e) {
            return false;
        }

        return true;
    }

//    /**
//     * Get a BlockLocation object for each block in a file.
//     *
//     * @param file  A FileStatus object corresponding to the file you want locations for.
//     * @param start The offset of the first part of the file you are interested in.
//     * @param len   The amount of the file past the offset you are interested in.
//     * @return A BlockLocation[] where each object corresponds to a block within
//     * the given range.
//     */
//    @Override
//    public BlockLocation[] getFileBlockLocations(FileStatus file, long start, long len) throws IOException {
//        Path abs_path = makeAbsolute(file.getPath());
//
//        int fh = curve.open(abs_path, CurveFSMount.O_RDONLY, 0);
//        if (fh < 0) {
//            LOG.error("getFileBlockLocations:got error " + fh + ", exiting and returning null!");
//            return null;
//        }
//
//        ArrayList<BlockLocation> blocks = new ArrayList<BlockLocation>();
//
//        long curPos = start;
//        long endOff = curPos + len;
//        do {
//            CurveFileExtent extent = curve.get_file_extent(fh, curPos);
//
//            int[] osds = extent.getOSDs();
//            String[] names = new String[osds.length];
//            String[] hosts = new String[osds.length];
//            String[] racks = new String[osds.length];
//
//            for (int i = 0; i < osds.length; i++) {
//                InetAddress addr = curve.get_osd_address(osds[i]);
//                names[i] = addr.getHostAddress();
//
//                /*
//                 * Grab the hostname and rack from the crush hierarchy. Current we
//                 * hard code the item types. For a more general treatment, we'll need
//                 * a new configuration option that allows users to map their custom
//                 * crush types to hosts and topology.
//                 */
//                Bucket[] path = curve.get_osd_crush_location(osds[i]);
//                for (Bucket bucket : path) {
//                    String type = bucket.getType();
//                    if (type.compareTo("host") == 0)
//                        hosts[i] = bucket.getName();
//                    else if (type.compareTo("rack") == 0)
//                        racks[i] = bucket.getName();
//                }
//            }
//
//            blocks.add(new BlockLocation(names, hosts, racks,
//                    extent.getOffset(), extent.getLength()));
//
//            curPos += extent.getLength();
//        } while (curPos < endOff);
//
//        curve.close(fh);
//
//        BlockLocation[] locations = new BlockLocation[blocks.size()];
//        locations = blocks.toArray(locations);
//
//        return locations;
//    }

    @Deprecated
    public boolean delete(Path path) throws IOException {
        return delete(path, false);
    }

    public boolean delete(Path path, boolean recursive) throws IOException {
        path = makeAbsolute(path);

        /* path exists? */
        FileStatus status;
        try {
            status = getFileStatus(path);
        } catch (FileNotFoundException e) {
            return false;
        }

        /* we're done if its a file */
        if (status.isFile()) {
            curve.unlink(path);
            return true;
        }

        /* get directory contents */
        FileStatus[] dirlist = listStatus(path);
        if (dirlist == null)
            return false;

        if (!recursive && dirlist.length > 0)
            throw new IOException("Directory " + path.toString() + "is not empty.");

        for (FileStatus fs : dirlist) {
            if (!delete(fs.getPath(), recursive))
                return false;
        }

        curve.rmdir(path);
        return true;
    }

    @Override
    public short getDefaultReplication() {
        return curve.getDefaultReplication();
    }

    @Override
    public long getDefaultBlockSize() {
        return super.getDefaultBlockSize();
    }

    @Override
    public FsStatus getStatus(Path p) throws IOException {
        CurveFSStatVFS stat = new CurveFSStatVFS();
        curve.statfs(p, stat);

        FsStatus status = new FsStatus(stat.bsize * stat.blocks,
                stat.bsize * (stat.blocks - stat.bavail),
                stat.bsize * stat.bavail);
        return status;
    }

    @Override
    protected int getDefaultPort() {
        return super.getDefaultPort();
    }

    @Override
    public String getCanonicalServiceName() {
        return null; // Does not support Token
    }
}
