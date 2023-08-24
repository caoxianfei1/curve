// -*- mode:Java; tab-width:2; c-basic-offset:2; indent-tabs-mode:t -*-

package io.opencurve.curve.fs.hadoop;

import org.apache.commons.logging.Log;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;

import io.opencurve.curve.fs.CurveMount;
import io.opencurve.curve.fs.libfs.CurveFSMount;
import io.opencurve.curve.fs.libfs.CurveFSStat;
import io.opencurve.curve.fs.libfs.CurveFSStatVFS;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URI;
import java.util.Map;

class CurveFSTalker extends CurveFSProto {

  private CurveFSMount mount;

  public CurveFSTalker(Configuration conf, Log log) {
    mount = null;
  }

  private String pathString(Path path) {
		if (null == path) {
			return "/";
		}
    return path.toUri().getPath();
  }

  void initialize(URI uri, Configuration conf) throws IOException {
    mount = new CurveFSMount();
    String fsname = "";

    // get config from conf with curvefs prefix
    String curvefsPrefix = "curvefs";
    Map<String,String> confMap= conf.getValByRegex("^"+curvefsPrefix + "\\..*");
    for (Map.Entry<String,String> entry : confMap.entrySet()) {
      String key = entry.getKey();
      String value = entry.getValue();
      if (key.equals(curvefsPrefix + ".name")) {
        fsname = value;
      }
      mount.confSet(key.substring(curvefsPrefix.length() + 1), value);
    }

    if(fsname.isEmpty()) {
      throw new IOException("curvefs.name is not set");
    }

    mount.mount(fsname, null, null);
  }

  /*
   * Open a file. Allows directories to be opened. used internally to get the
   * pool name. Hadoop doesn't allow directories to be opened, and that is
   * handled below.
   */
  int __open(Path path, int flags, int mode) throws IOException {
    return mount.open(pathString(path), flags, mode);
  }

  /*
   * Open a file. Curve will not complain if we open a directory, but this
   * isn't something that Hadoop expects and we should throw an exception in
   * this case.
   */
  int open(Path path, int flags, int mode) throws IOException {
    int fd = mount.open(pathString(path), flags, mode);
    CurveFSStat stat = new CurveFSStat();
    fstat(fd, stat);
    if (stat.isDir()) {
      mount.close(fd);
      throw new FileNotFoundException();
    }
    return fd;
  }

  void fstat(int fd, CurveFSStat stat) throws IOException {
    mount.fstat(fd, stat);
  }

  void lstat(Path path, CurveFSStat stat) throws IOException {
      mount.lstat(pathString(path), stat);
  }

  void statfs(Path path, CurveFSStatVFS stat) throws IOException {
    mount.statfs(pathString(path), stat);
  }

  void rmdir(Path path) throws IOException {
    mount.rmdir(pathString(path));
  }

  void unlink(Path path) throws IOException {
    mount.unlink(pathString(path));
  }

  void rename(Path src, Path dst) throws IOException {
    mount.rename(pathString(src), pathString(dst));
  }

  String[] listdir(Path path) throws IOException {
    CurveFSStat stat = new CurveFSStat();
    try {
      mount.lstat(pathString(path), stat);
    } catch (FileNotFoundException e) {
      return null;
    }
    if (!stat.isDir())
      return null;
    return mount.listdir(pathString(path));
  }

  void mkdirs(Path path, int mode) throws IOException {
    mount.mkdirs(pathString(path), mode);
  }

  void close(int fd) throws IOException {
    mount.close(fd);
  }

  void chmod(Path path, int mode) throws IOException {
    mount.chmod(pathString(path), mode);
  }

  void shutdown() throws IOException {
    if (null != mount)
      mount.umount();
    mount = null;
  }

  short getDefaultReplication() {
    return 1;
  }

  short get_file_replication(Path path) throws IOException {
    CurveFSStat stat = new CurveFSStat();
    mount.lstat(pathString(path), stat);
    int replication = 1;
    if (stat.isFile()) {
      int fd = mount.open(pathString(path), CurveFSMount.O_WRONLY, 0);
      //replication = mount.get_file_replication(fd);
      mount.close(fd);
    }
    return (short)replication;
  }

  void setattr(Path path, CurveFSStat stat, int mask) throws IOException {
    mount.setattr(pathString(path), stat, mask);
  }

  void fsync(int fd) throws IOException {
    mount.fsync(fd);
  }

  long lseek(int fd, long offset, int whence) throws IOException {
    return mount.lseek(fd, offset, whence);
  }

  int write(int fd, byte[] buf, long size, long offset) throws IOException {
    return (int)mount.write(fd, buf, size, offset);
  }

  int read(int fd, byte[] buf, long size, long offset) throws IOException {
    return (int)mount.read(fd, buf, size, offset);
  }

  int setguids(String name, String user, String grouping, String superUser, String superGroup, short umask) {
    int rc = (int)mount.setguids(name, user, grouping, superUser, superGroup, umask);
    return rc;
  }

  int updateguids(String uids, String grouping) {
    return (int)mount.updateguids(uids, grouping);
  }

  int setowner(Path path, String username, String groupname) throws IOException {
    return mount.setowner(pathString(path), username, groupname);
  }
}
