#include <vector>
#include <utility>

#include "absl/cleanup/cleanup.h"
#include "curvefs/sdk/libcurvefs/libcurvefs.h"
#include "curvefs/sdk/java/native/io_opencurve_curve_fs_CurveMount.h"

/* Cached field IDs for io.opencurve.curve.fs.CurveStat */
static jfieldID curvestat_mode_fid;
static jfieldID curvestat_uid_fid;
static jfieldID curvestat_gid_fid;
static jfieldID curvestat_size_fid;
static jfieldID curvestat_blksize_fid;
static jfieldID curvestat_blocks_fid;
static jfieldID curvestat_a_time_fid;
static jfieldID curvestat_m_time_fid;
static jfieldID curvestat_is_file_fid;
static jfieldID curvestat_is_directory_fid;
static jfieldID curvestat_is_symlink_fid;

/* Cached field IDs for io.opencurve.curve.fs.CurveStatVFS */
static jfieldID curvestatvfs_bsize_fid;
static jfieldID curvestatvfs_frsize_fid;
static jfieldID curvestatvfs_blocks_fid;
static jfieldID curvestatvfs_bavail_fid;
static jfieldID curvestatvfs_files_fid;
static jfieldID curvestatvfs_fsid_fid;
static jfieldID curvestatvfs_namemax_fid;

/*
 * Setup cached field IDs
 */
static void setup_field_ids(JNIEnv* env) {
    jclass curvestat_cls;
    jclass curvestatvfs_cls;

/*
 * Get a fieldID from a class with a specific type
 *
 * clz: jclass
 * field: field in clz
 * type: integer, long, etc..
 *
 * This macro assumes some naming convention that is used
 * only in this file:
 *
 *   GETFID(curvestat, mode, I) gets translated into
 *     curvestat_mode_fid = env->GetFieldID(curvestat_cls, "mode", "I");
 */
#define GETFID(clz, field, type) do { \
    clz ## _ ## field ## _fid = env->GetFieldID(clz ## _cls, #field, #type); \
    if (!clz ## _ ## field ## _fid) \
        return; \
    } while (0)

    /* Cache CurveStat fields */

    curvestat_cls = env->FindClass("io/opencurve/curve/fs/CurveStat");
    if (!curvestat_cls) {
        return;
    }

    GETFID(curvestat, mode, I);
    GETFID(curvestat, uid, I);
    GETFID(curvestat, gid, I);
    GETFID(curvestat, size, J);
    GETFID(curvestat, blksize, J);
    GETFID(curvestat, blocks, J);
    GETFID(curvestat, a_time, J);
    GETFID(curvestat, m_time, J);
    GETFID(curvestat, is_file, Z);
    GETFID(curvestat, is_directory, Z);
    GETFID(curvestat, is_symlink, Z);

    /* Cache CurveStatVFS fields */

    curvestatvfs_cls = env->FindClass("io/opencurve/curve/fs/CurveStatVFS");
    if (!curvestatvfs_cls) {
        return;
    }

    GETFID(curvestatvfs, bsize, J);
    GETFID(curvestatvfs, frsize, J);
    GETFID(curvestatvfs, blocks, J);
    GETFID(curvestatvfs, bavail, J);
    GETFID(curvestatvfs, files, J);
    GETFID(curvestatvfs, fsid, J);
    GETFID(curvestatvfs, namemax, J);

#undef GETFID
}

static void fill_curvestat(JNIEnv* env,
                           jobject j_curvestat,
                           struct stat* stat, 
                           jstring owner,
                           jstring group) {
    env->SetIntField(j_curvestat, curvestat_mode_fid, stat->st_mode);
    env->SetIntField(j_curvestat, curvestat_uid_fid, stat->st_uid);
    env->SetIntField(j_curvestat, curvestat_gid_fid, stat->st_gid);
    env->SetLongField(j_curvestat, curvestat_size_fid, stat->st_size);
    env->SetLongField(j_curvestat, curvestat_blksize_fid, stat->st_blksize);
    env->SetLongField(j_curvestat, curvestat_blocks_fid, stat->st_blocks);

    const char *c_owner = env->GetStringUTFChars(owner, NULL);
    const char* c_group = env->GetStringUTFChars(group, NULL);

    jclass curvestat_class = env->GetObjectClass(j_curvestat);
    jfieldID owner_fid = env->GetFieldID(curvestat_class, "owner", "Ljava/lang/String;");
    jfieldID group_fid = env->GetFieldID(curvestat_class, "group", "Ljava/lang/String;");
    env->SetObjectField(j_curvestat, owner_fid, env->NewStringUTF(c_owner));
    env->SetObjectField(j_curvestat, group_fid, env->NewStringUTF(c_group));

    env->ReleaseStringUTFChars(owner, c_owner);
    env->ReleaseStringUTFChars(group, c_group);
    // mtime
    uint64_t time = stat->st_mtim.tv_sec;
    time *= 1000;
    time += stat->st_mtim.tv_nsec / 1000000;
    env->SetLongField(j_curvestat, curvestat_m_time_fid, time);

    // atime
    time = stat->st_atim.tv_sec;
    time *= 1000;
    time += stat->st_atim.tv_nsec / 1000000;
    env->SetLongField(j_curvestat, curvestat_a_time_fid, time);

    env->SetBooleanField(j_curvestat, curvestat_is_file_fid,
        S_ISREG(stat->st_mode) ? JNI_TRUE : JNI_FALSE);

    env->SetBooleanField(j_curvestat, curvestat_is_directory_fid,
        S_ISDIR(stat->st_mode) ? JNI_TRUE : JNI_FALSE);

    env->SetBooleanField(j_curvestat, curvestat_is_symlink_fid,
        S_ISLNK(stat->st_mode) ? JNI_TRUE : JNI_FALSE);
}

static void fill_curvestatvfs(JNIEnv* env,
                              jobject j_curvestatvfs,
                              struct statvfs st) {
    env->SetLongField(j_curvestatvfs, curvestatvfs_bsize_fid, st.f_bsize);
    env->SetLongField(j_curvestatvfs, curvestatvfs_frsize_fid, st.f_frsize);
    env->SetLongField(j_curvestatvfs, curvestatvfs_blocks_fid, st.f_blocks);
    env->SetLongField(j_curvestatvfs, curvestatvfs_bavail_fid, st.f_bavail);
    env->SetLongField(j_curvestatvfs, curvestatvfs_files_fid, st.f_files);
    env->SetLongField(j_curvestatvfs, curvestatvfs_fsid_fid, st.f_fsid);
    env->SetLongField(j_curvestatvfs, curvestatvfs_namemax_fid, st.f_namemax);
}

/* Map io_opencurve_curve_fs_CurveMount_O_* open flags to values in libc */
static inline uint32_t fixup_open_flags(jint jflags) {
    uint32_t flags = 0;

#define FIXUP_OPEN_FLAG(name) \
    if (jflags & io_opencurve_curve_fs_CurveMount_##name) \
        flags |= name;

    FIXUP_OPEN_FLAG(O_RDONLY)
    FIXUP_OPEN_FLAG(O_RDWR)
    FIXUP_OPEN_FLAG(O_APPEND)
    FIXUP_OPEN_FLAG(O_CREAT)
    FIXUP_OPEN_FLAG(O_TRUNC)
    FIXUP_OPEN_FLAG(O_EXCL)
    FIXUP_OPEN_FLAG(O_WRONLY)
    FIXUP_OPEN_FLAG(O_DIRECTORY)

#undef FIXUP_OPEN_FLAG

    return flags;
}

// FIXME: use fuse define
#define CURVEFS_SETATTR_MODE       (1 << 0)
#define CURVEFS_SETATTR_UID        (1 << 1)
#define CURVEFS_SETATTR_GID        (1 << 2)
#define CURVEFS_SETATTR_SIZE       (1 << 3)
#define CURVEFS_SETATTR_ATIME      (1 << 4)
#define CURVEFS_SETATTR_MTIME      (1 << 5)
#define CURVEFS_SETATTR_ATIME_NOW  (1 << 7)
#define CURVEFS_SETATTR_MTIME_NOW  (1 << 8)
#define CURVEFS_SETATTR_CTIME      (1 << 10)

/* Map JAVA_SETATTR_* to values in curve lib */
static inline int fixup_attr_mask(jint jmask) {
    int mask = 0;

#define FIXUP_ATTR_MASK(name) \
    if (jmask & io_opencurve_curve_fs_CurveMount_##name) \
        mask |= CURVEFS_##name;

    FIXUP_ATTR_MASK(SETATTR_MODE)
    FIXUP_ATTR_MASK(SETATTR_UID)
    FIXUP_ATTR_MASK(SETATTR_GID)
    FIXUP_ATTR_MASK(SETATTR_MTIME)
    FIXUP_ATTR_MASK(SETATTR_ATIME)

#undef FIXUP_ATTR_MASK
    return mask;
}

/*
 * Exception throwing helper. Adapted from Apache Hadoop header
 * org_apache_hadoop.h by adding the do {} while (0) construct.
 */
#define THROW(env, exception_name, message) \
    do { \
        jclass ecls = env->FindClass(exception_name); \
        if (ecls) { \
            int ret = env->ThrowNew(ecls, message); \
            if (ret < 0) { \
                printf("(CurveFS) Fatal Error\n"); \
            } \
            env->DeleteLocalRef(ecls); \
        } \
    } while (0)

static void handle_error(JNIEnv* env, int rc) {
    switch (rc) {
        case ENOENT:
            THROW(env, "java/io/FileNotFoundException", "");
            return;
        default:
            break;
    }

    THROW(env, "java/io/IOException", strerror(rc));
}

// nativeCurveFSCreate: curvefs_create
JNIEXPORT jlong
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSCreate
    (JNIEnv* env, jobject) {
    setup_field_ids(env);
    uintptr_t instance = curvefs_create();
    return reinterpret_cast<uint64_t>(instance);
}

// TODO: ------------------- delete it
static char *rand_string(char *str, size_t size)
{
    srand(time(NULL));
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

char* rand_string_alloc(size_t size)
{
     char *s = static_cast<char*>(malloc(size + 1));
     if (s) {
         rand_string(s, size);
     }
     return s;
}

// TODO: ------------------- delete it

// FIXME:
// nativeCurveFSMount: curvefs_mount
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSMount
    (JNIEnv* env, jclass clz, jlong j_instance) {
    auto instance = j_instance;
    curvefs_conf_set(instance, "s3.ak", "curve");
    curvefs_conf_set(instance, "s3.sk", "Netease@2023");
    curvefs_conf_set(instance, "s3.endpoint", "59.111.93.76:9000");
    curvefs_conf_set(instance, "s3.bucket_name", "hadoop-fs");
    curvefs_conf_set(instance, "mdsOpt.rpcRetryOpt.addrs", "59.111.93.76:6700,59.111.93.77:6700,59.111.93.78:6700");
    curvefs_conf_set(instance, "fs.accessLogging", "true");
    curvefs_conf_set(instance, "vfs.entryCache.lruSize", "0");
    curvefs_conf_set(instance, "vfs.attrCache.lruSize", "0");
    curvefs_conf_set(instance, "client.loglevel", "6");
    int rc = curvefs_mount(instance, "test-001", rand_string_alloc(20));
    return rc;
}

// nativeSetGuids: curvefs_set_guids
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeSetGuids
  (JNIEnv *env, jclass, jlong j_instance, jstring j_name, jstring j_user, jstring j_grouping, jstring j_superUser, jstring j_superGroup, jshort j_umask) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* name = env->GetStringUTFChars(j_name, NULL);
    const char* user = env->GetStringUTFChars(j_user, NULL);
    const char* grouping = env->GetStringUTFChars(j_grouping, NULL);
    const char* superUser = env->GetStringUTFChars(j_superUser, NULL);
    const char* superGroup = env->GetStringUTFChars(j_superGroup, NULL);
    uint16_t umask = static_cast<uint16_t>(j_umask);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_name, name);
        env->ReleaseStringUTFChars(j_user, user);
        env->ReleaseStringUTFChars(j_grouping, grouping);
        env->ReleaseStringUTFChars(j_superUser, superUser);
        env->ReleaseStringUTFChars(j_superGroup, superGroup);
    });

    int rc = curvefs_set_guids(instance, name, user, grouping, superUser, superGroup, umask);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeUpdateGuids: curvefs_update_guids
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeUpdateGuids
  (JNIEnv *env, jclass, jlong j_instance, jstring j_uids, jstring j_grouping) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* uids = env->GetStringUTFChars(j_uids, NULL);
    const char* grouping = env->GetStringUTFChars(j_grouping, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_uids, uids);
        env->ReleaseStringUTFChars(j_grouping, grouping);
    });
    auto rc = curvefs_update_guids(instance, uids, grouping);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return 0;
}

// nativeCurveFSMkDirs: curvefs_mkdir
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSMkDirs
  (JNIEnv* env, jclass, jlong j_instance, jstring j_path, jint j_mode) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    uint16_t mode = static_cast<uint16_t>(j_mode);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    // curvefs_mkdir
    int rc = curvefs_mkdir(instance, path, mode);
    if (rc == EEXIST) {
        rc = 0;
    } else if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSRmDir: curvefs_rmdir
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSRmDir
    (JNIEnv* env, jclass, jlong j_instance, jstring j_path) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    // curvefs_rmdir
    int rc = curvefs_rmdir(instance, path);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSOpen: curvefs_open
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSOpen
    (JNIEnv* env, jclass,
     jlong j_instance, jstring j_path, jint j_flags, jint j_mode) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    uint32_t flags = fixup_open_flags(j_flags);
    uint16_t mode = static_cast<uint16_t>(j_mode);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    // curvefs_open
    int fd = curvefs_open(instance, path, flags, mode);
    if (fd < 0) {
        handle_error(env, fd);
    }
    return fd;
}

// nativieCurveFSRead: curvefs_read
JNIEXPORT jlong
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativieCurveFSRead
    (JNIEnv* env, jclass, jlong j_instance, jint j_fd,
     jbyteArray j_buffer, jlong j_size, jlong j_offset) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);
    jbyte* c_buffer = env->GetByteArrayElements(j_buffer, NULL);
    char* buffer = reinterpret_cast<char*>(c_buffer);
    size_t count = static_cast<size_t>(j_size);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseByteArrayElements(j_buffer, c_buffer, JNI_ABORT);
    });

    ssize_t n = curvefs_read(instance, fd, buffer, count);
    if (n < 0) {
        handle_error(env, n);
    }

    return static_cast<jlong>(n);
}

// nativieCurveFSWrite: curvefs_write
JNIEXPORT jlong
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativieCurveFSWrite
    (JNIEnv* env, jclass, jlong j_instance, jint j_fd,
     jbyteArray j_buffer, jlong j_size, jlong j_offset) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);
    size_t count = static_cast<size_t>(j_size);
    jbyte* c_buffer = env->GetByteArrayElements(j_buffer, NULL);
    char* buffer = reinterpret_cast<char*>(c_buffer);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseByteArrayElements(j_buffer, c_buffer, JNI_ABORT);
    });

    ssize_t n = curvefs_write(instance, fd, buffer, count);
    if (n < 0) {
        handle_error(env, n);
    }
    return static_cast<jlong>(n);
}

// nativeCurveFSFSync: curvefs_fsync
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSFSync
    (JNIEnv* env, jclass, jlong j_instance, jint j_fd) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);

    // curvefs_fsync
    int rc = curvefs_fsync(instance, fd);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSClose: curvefs_close
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSClose
    (JNIEnv* env, jclass, jlong j_instance, jint j_fd) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);

    // curvefs_close
    int rc = curvefs_close(instance, fd);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSUnlink: curvefs_unlink
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSUnlink
    (JNIEnv* env, jclass, jlong j_instance, jstring j_path) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    int rc = curvefs_unlink(instance, path);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

//  nativeSetOwner: curvefs_setowner
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeSetOwner
  (JNIEnv *env, jclass, jlong j_instance, jstring j_path, jstring j_user, jstring j_group) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    const char* user = env->GetStringUTFChars(j_user, NULL);
    const char* group = env->GetStringUTFChars(j_group, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
        env->ReleaseStringUTFChars(j_user, user);
        env->ReleaseStringUTFChars(j_group, group);
    });
    int rc = curvefs_setowner(instance, path, user, group);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSFStat: curvefs_fstat
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSFStat
    (JNIEnv* env, jclass, jlong j_instance, jint j_fd, jobject j_curvestat) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);

    // curvefs_fstat
    struct stat stat;
    auto rc = curvefs_fstat(instance, fd, &stat);
    if (rc != 0) {
        handle_error(env, rc);
        return rc;
    }

    // get owner & group
    std::string owner = curvefs_lookup_owner(instance, stat.st_uid);
    std::string group = curvefs_lookup_group(instance, stat.st_gid);
    jstring jowner = env->NewStringUTF(owner.c_str());
    jstring jgroup = env->NewStringUTF(group.c_str());
    fill_curvestat(env, j_curvestat, &stat, jowner, jowner);
    return rc;
}

// nativeCurveFSStatFS: curvefs_statfs
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSStatFS
    (JNIEnv* env, jclass,
     jlong j_instance, jstring j_path, jobject j_curvestatvfs) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);

    struct statvfs statvfs;
    int rc = curvefs_statfs(instance, "/", &statvfs);
    if (rc != 0) {
        handle_error(env, rc);
        return rc;
    }

    fill_curvestatvfs(env, j_curvestatvfs, statvfs);
    return rc;
}

// nativeCurveFSLstat: curvefs_lstat
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSLstat
    (JNIEnv* env, jclass,
     jlong j_instance, jstring j_path, jobject j_curvestat) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    // curvefs_lstat
    struct stat stat;
    auto rc = curvefs_lstat(instance, path, &stat);
    if (rc != 0) {
        handle_error(env, rc);
        return rc;
    }

    // get owner & group
    std::string owner = curvefs_lookup_owner(instance, stat.st_uid);
    std::string group = curvefs_lookup_group(instance, stat.st_gid);
    jstring jowner = env->NewStringUTF(owner.c_str());
    jstring jgroup = env->NewStringUTF(group.c_str());
    
    fill_curvestat(env, j_curvestat, &stat, jowner, jgroup);
    return rc;
}

// nativeCurveFSListDir: curvefs_opendir/curvefs_readdir/curvefs_closedir
JNIEXPORT jobjectArray
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSListDir
    (JNIEnv* env, jclass, jlong j_instance, jstring j_path) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    // curvefs_opendir
    dir_stream_t dir_stream;
    auto rc = curvefs_opendir(instance, path, &dir_stream);
    if (rc != 0) {
        handle_error(env, rc);
        return NULL;
    }

    // curvefs_readdir
    std::vector<dirent_t> dirents;
    dirent_t dirent;
    for ( ;; ) {
        ssize_t n = curvefs_readdir(instance, &dir_stream, &dirent);
        if (n < 0) {
            handle_error(env, rc);
            return NULL;
        } else if (n == 0) {
            break;
        }
        dirents.push_back(dirent);
    }

    // closedir
    rc = curvefs_closedir(instance, &dir_stream);
    if (rc != 0) {
        handle_error(env, rc);
        return NULL;
    }

    // convert to
    jobjectArray j_names = env->NewObjectArray(
        dirents.size(), env->FindClass("java/lang/String"), NULL);

    for (int i = 0; i < dirents.size(); i++) {
        jstring j_name = env->NewStringUTF(dirents[i].name);
        env->SetObjectArrayElement(j_names, i, j_name);
        env->DeleteLocalRef(j_name);
    }
    return j_names;
}

// nativeCurveFSSetAttr: curvefs_setattr
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSSetAttr
    (JNIEnv* env, jclass,
     jlong j_instance, jstring j_path, jobject j_curvestat, jint j_mask) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    int to_set = fixup_attr_mask(j_mask);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    struct stat stat;
    memset(&stat, 0, sizeof(stat));
    stat.st_mode = env->GetIntField(j_curvestat, curvestat_mode_fid);
    stat.st_uid = env->GetIntField(j_curvestat, curvestat_uid_fid);
    stat.st_gid = env->GetIntField(j_curvestat, curvestat_gid_fid);
    uint64_t mtime_msec = env->GetLongField(j_curvestat, curvestat_m_time_fid);
    uint64_t atime_msec = env->GetLongField(j_curvestat, curvestat_a_time_fid);
    stat.st_mtim.tv_sec = mtime_msec / 1000;
    stat.st_mtim.tv_nsec = (mtime_msec % 1000) * 1000000;
    stat.st_atim.tv_sec = atime_msec / 1000;
    stat.st_atim.tv_nsec = (atime_msec % 1000) * 1000000;

    int rc = curvefs_setattr(instance, path, &stat, to_set);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSChmod: curvefs_chmod
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSChmod
    (JNIEnv* env, jclass, jlong j_instance, jstring j_path, jint j_mode) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    uint16_t mode = static_cast<uint16_t>(j_mode);
    const char* path = env->GetStringUTFChars(j_path, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_path, path);
    });

    int rc = curvefs_chmod(instance, path, mode);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSRename: curvefs_rename
JNIEXPORT jint
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSRename
    (JNIEnv* env, jclass, jlong j_instance, jstring j_src, jstring j_dst) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    const char* src = env->GetStringUTFChars(j_src, NULL);
    const char* dst = env->GetStringUTFChars(j_dst, NULL);
    auto defer = absl::MakeCleanup([&]() {
        env->ReleaseStringUTFChars(j_src, src);
        env->ReleaseStringUTFChars(j_dst, dst);
    });

    int rc = curvefs_rename(instance, src, dst);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}

// nativeCurveFSLSeek: curvefs_lseek
JNIEXPORT jlong
JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSLSeek
    (JNIEnv* env, jclass,
     jlong j_instance, jint j_fd, jlong j_offset, jint j_whence) {
    uintptr_t instance = static_cast<uintptr_t>(j_instance);
    int fd = static_cast<int>(j_fd);
    uint64_t offset = static_cast<uint64_t>(j_offset);

    int whence;
    switch (j_whence) {
    case io_opencurve_curve_fs_CurveMount_SEEK_SET:
        whence = SEEK_SET;
        break;
    case io_opencurve_curve_fs_CurveMount_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case io_opencurve_curve_fs_CurveMount_SEEK_END:
        whence = SEEK_END;
        break;
    default:
        return -1;
    }

    int rc = curvefs_lseek(instance, fd, offset, whence);
    if (rc != 0) {
        handle_error(env, rc);
    }
    return rc;
}
