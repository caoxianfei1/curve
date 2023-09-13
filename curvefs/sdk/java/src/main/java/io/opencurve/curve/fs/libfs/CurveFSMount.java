package io.opencurve.curve.fs.libfs;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Scanner;
import java.util.stream.Collectors;
import java.util.Map;
import java.util.HashMap;

public class CurveFSMount {
    // init
    private native long nativeCurveFSCreate();
    private native void nativeCurveFSRelease(long instancePtr);
    private static native void nativeCurveFSConfSet(long instancePtr, String key, String value);
    private static native int nativeCurveFSMount(long instancePtr, String fsname);
    private static native int nativeCurveFSUmount(long instancePtr);
    // dir*
    private static native int nativeCurveFSMkDirs(long instancePtr, String path, int mode);
    private static native int nativeCurveFSRmDir(long instancePtr, String path);
    private static native String[] nativeCurveFSListDir(long instancePtr, String path);
    // file*
    private static native int nativeCurveFSOpen(long instancePtr, String path, int flags, int mode);
    private static native long nativeCurveFSLSeek(long instancePtr, int fd, long offset, int whence);
    private static native long nativieCurveFSRead(long instancePtr, int fd, byte[] buffer, long size, long offset);
    private static native long nativieCurveFSWrite(long instancePtr, int fd, byte[] buffer, long size, long offset);
    private static native int nativeCurveFSFSync(long instancePtr, int fd);
    private static native int nativeCurveFSClose(long instancePtr, int fd);
    private static native int nativeCurveFSUnlink(long instancePtr, String path);
    // others
    private static native int nativeCurveFSStatFS(long instancePtr, CurveFSStatVFS statvfs);
    private static native int nativeCurveFSLstat(long instancePtr, String path, CurveFSStat stat);
    private static native int nativeCurveFSFStat(long instancePtr, int fd, CurveFSStat stat);
    private static native int nativeCurveFSSetAttr(long instancePtr, String path, CurveFSStat stat, int mask);
    private static native int nativeCurveFSChmod(long instancePtr, String path, int mode);    
    private static native int nativeCurveFSChown(long instancePtr, String path, int uid, int gid);
    private static native int nativeCurveFSRename(long instancePtr, String src, String dst);

    /*
     * Flags for open().
     *
     * Must be synchronized with JNI if changed.
     */
    public static final int O_RDONLY = 1;
    public static final int O_RDWR = 2;
    public static final int O_APPEND = 4;
    public static final int O_CREAT = 8;
    public static final int O_TRUNC = 16;
    public static final int O_EXCL = 32;
    public static final int O_WRONLY = 64;
    public static final int O_DIRECTORY = 128;

    /*
     * Whence flags for seek().
     *
     * Must be synchronized with JNI if changed.
     */
    public static final int SEEK_SET = 0;
    public static final int SEEK_CUR = 1;
    public static final int SEEK_END = 2;

    /*
     * Attribute flags for setattr().
     *
     * Must be synchronized with JNI if changed.
     */
    public static final int SETATTR_MODE = 1;
    public static final int SETATTR_UID = 2;
    public static final int SETATTR_GID = 4;
    public static final int SETATTR_MTIME = 8;
    public static final int SETATTR_ATIME = 16;

    private static final String CURVEFS_DEBUG_ENV_VAR = "CURVEFS_DEBUG";
    private static final String CLASS_NAME = "io.opencurve.curve.fs.CurveFSMount";
    private static final String PERMISSSION_UID = "vfs.permission.uid";    
    private static final String PERMISSSION_GIDS = "vfs.permission.gids";
    private static final String PERMISSSION_UMASK = "vfs.permission.umask";

    private long instancePtr;
    private boolean initialized = false;
    
    private Mapping mapping;
    private String user;
    private String group;
    private String superUser;
    private String superGroup;
    private short umask;

    private int uid;
    private int gid;
    private ArrayList<Integer> gids;

    private boolean isSuperUser(String pUser, ArrayList<String> pGroups) {
        if (pUser == this.superUser) {
            return true;
        }
        for (int i = 0; i < pGroups.size(); i++) {
            if (pGroups.get(i) == this.superGroup) {
                return true;
            }
        }
        return false;
    }

    private int lookupUid(String pUser) throws NoSuchAlgorithmException, IOException {
        if (pUser == this.superUser) {
            return 0;
        }
        return mapping.lookupUser(pUser);
    }

    private int lookupGid(String pGroup) throws NoSuchAlgorithmException, IOException {
        if (pGroup == this.superGroup) {
            return 0;
        }
        return mapping.lookupGroup(pGroup);
    }

    private String uid2name(int pUid) throws IOException {
        String name = this.superUser;
        if (pUid > 0) {
            name = mapping.lookupUserID(pUid);
        }
        return name;
    }

    private String gid2name(int pGid) throws IOException {
        String name = this.superGroup;
        if (pGid > 0) {
            name = mapping.lookupGroupID(pGid);
        }
        return name;
    }

    private ArrayList<Integer> lookupGids(ArrayList<String> pGroups) throws NoSuchAlgorithmException, IOException {
        ArrayList<Integer> tmpGids = new ArrayList<>();
        for (int i = 0; i < pGroups.size(); i++) {
            tmpGids.add(lookupGid(pGroups.get(i)));
        }
        return tmpGids;
    }

    private void setVFSPermission() {
        String gids = String.join(",", 
            this.gids.stream().map(String::valueOf).collect(Collectors.toList()));
        confSet(PERMISSSION_UID, String.valueOf(this.uid));
        confSet(PERMISSSION_GIDS, gids);
        confSet(PERMISSSION_UMASK, String.valueOf(this.umask));
    }

    private static void accessLog(String name, String... args) {
        String value = System.getenv(CURVEFS_DEBUG_ENV_VAR);
        if (!Boolean.valueOf(value)) {
            return;
        }

        String params = String.join(",", args);
        String message = String.format("%s.%s(%s)", CLASS_NAME, name, params);
        System.out.println(message);
    }

    static {
        accessLog("loadLibrary");
        try {
            CurveFSNativeLoader.getInstance().loadLibrary();
        } catch(Exception e) {}
    }

    protected void finalize() throws Throwable {
        accessLog("finalize");
        if (initialized) {
            nativeCurveFSUmount(instancePtr);
            nativeCurveFSRelease(instancePtr);
        }
    }

    public CurveFSMount() {
        accessLog("CurveMount");
        instancePtr = nativeCurveFSCreate();
        initialized = true;
    }

    public void confSet(String key, String value) {
        accessLog("confSet", key, value);
        nativeCurveFSConfSet(instancePtr, key, value);
    }

    public void mount(String fsname, String fstype, Object option) {
        accessLog("mount");
        nativeCurveFSMount(instancePtr, fsname);
    }

    public void umount() {
        accessLog("umount");
        nativeCurveFSUmount(instancePtr);
    }

    public void shutdown() throws IOException {
        accessLog("shutdown");
    }

    // directory*
    public void mkdirs(String path, int mode) throws IOException {
        accessLog("mkdirs", path.toString());
        nativeCurveFSMkDirs(instancePtr, path, mode);
    }

    public void rmdir(String path) throws IOException {
        accessLog("rmdir", path.toString());
        nativeCurveFSRmDir(instancePtr, path);
    }

    public String[] listdir(String path) throws IOException {
        accessLog("listdir", path.toString());
        return nativeCurveFSListDir(instancePtr, path);
    }

    // file*
    public int open(String path, int flags, int mode) throws IOException {
        accessLog("open", path.toString());
        return nativeCurveFSOpen(instancePtr, path, flags, mode);
    }

    public long lseek(int fd, long offset, int whence) throws IOException {
        accessLog("lseek", String.valueOf(fd), String.valueOf(offset), String.valueOf(whence));
        return nativeCurveFSLSeek(instancePtr, fd, offset, whence);
    }

    public int read(int fd, byte[] buf, long size, long offset) throws IOException {
        accessLog("read", String.valueOf(fd), String.valueOf(size), String.valueOf(size));
        long rc = nativieCurveFSRead(instancePtr, fd, buf, size, offset);
        return (int) rc;
    }

    public int write(int fd, byte[] buf, long size, long offset) throws IOException {
        accessLog("write", String.valueOf(fd), String.valueOf(size), String.valueOf(size));
        long rc = nativieCurveFSWrite(instancePtr, fd, buf, size, offset);
        return (int) rc;
    }

    public void fsync(int fd) throws IOException {
        accessLog("fsync", String.valueOf(fd));
        nativeCurveFSFSync(instancePtr, fd);
    }

    public void close(int fd) throws IOException {
        accessLog("close", String.valueOf(fd));
        nativeCurveFSClose(instancePtr, fd);
    }

    public void unlink(String path) throws IOException {
        accessLog("unlink", path.toString());
        nativeCurveFSUnlink(instancePtr, path);
    }

    // others
    public void statfs(String path, CurveFSStatVFS statvfs) throws IOException {
        accessLog("statfs", path.toString());
        nativeCurveFSStatFS(instancePtr, statvfs);
    }

    public void lstat(String path, CurveFSStat stat) throws IOException {
        accessLog("lstat", path.toString());
        nativeCurveFSLstat(instancePtr, path, stat);
        stat.owner = uid2name(stat.uid);
        stat.group = gid2name(stat.gid);
    }

    public void fstat(int fd, CurveFSStat stat) throws IOException {
        accessLog("fstat",  String.valueOf(fd));
        nativeCurveFSFStat(instancePtr, fd, stat);
    }

    public void setattr(String path, CurveFSStat stat, int mask) throws IOException {
        accessLog("setattr", path.toString());
        nativeCurveFSSetAttr(instancePtr, path, stat, mask);
    }

    public void chmod(String path, int mode) throws IOException {
        accessLog("chmod", path.toString());
        nativeCurveFSChmod(instancePtr, path, mode);
    }

    public void chown(String path, String user, String group) throws IOException, NoSuchAlgorithmException {
        accessLog("chown", path.toString(), user.toString(), group.toString());
        nativeCurveFSChown(instancePtr, path, lookupUid(user), lookupGid(group));
    }

    public void rename(String src, String dst) throws IOException {
        accessLog("rename", src.toString(), dst.toString());
        nativeCurveFSRename(instancePtr, src, dst);
    }

    public void setGuids(String name, String pUser, String pGrouping, String pSuperUser, String pSuperGroup, short pUmask) throws IOException, NoSuchAlgorithmException {
        accessLog("setGuids", name, pUser, pGrouping, pSuperUser, pSuperGroup, String.valueOf(pUmask));
        this.user = pUser;
        this.group = pGrouping;
        this.superUser = pSuperUser;
        this.superGroup = pSuperGroup;
        this.umask = pUmask;
        this.gids = new ArrayList<>();

        this.mapping = new Mapping(name);
        String[] groupsArr = pGrouping.split(",");
        ArrayList<String> groups = new ArrayList<>(Arrays.asList(groupsArr));
        if (isSuperUser(pUser, groups)) {
            this.uid = 0;
            this.gid = 0;
            this.gids.add(0);
        } else {
            this.uid = lookupUid(pUser);
            this.gids = lookupGids(groups);
            if (this.gids.size() > 0) {
                this.gid = this.gids.get(0);
            }
        }

        setVFSPermission();
    }

    public void updateGuids(String uidStr, String grouping) throws NoSuchAlgorithmException, IOException {
        Map<String, Integer> uids = new HashMap<String, Integer>();        
        Map<String, Integer> groupIds = new HashMap<String, Integer>();
        ArrayList<String> groups = new ArrayList<>();

        if (!uidStr.isEmpty() && !"".equals(uidStr.trim())) {
            Scanner scanner = new Scanner(uidStr);
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                String[] tokens = line.split(":");
                if (tokens.length < 2) {
                    continue;
                }
                String username = tokens[0];
                String uid = tokens[1];
                uids.put(username, Integer.parseInt(uid));
            }
            scanner.close();
        }
        
        if (!grouping.isEmpty() && !"".equals(grouping.trim())) {
            Scanner scanner = new Scanner(grouping);
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                String[] tokens = line.split(":");
                if (tokens.length < 2) {
                    continue;
                }
                String groupname = tokens[0];
                String gid = tokens[1];
                groupIds.put(groupname, Integer.parseInt(gid));
                if (tokens.length > 2) {
                    String[] users = tokens[3].split(",");
                    for (int i = 0; i < users.length; i++) {
                        if (user == users[i]) {
                            groups.add(groupname);
                        }
                    }
                }
            }
            scanner.close();
        }
        mapping.update(uids, groupIds, false);
        this.uid = lookupUid(this.user);
        if (groups.size() > 0) {
            this.gids = lookupGids(groups);
        }

        setVFSPermission();
    }
}
