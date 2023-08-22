/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class io_opencurve_curve_fs_CurveMount */

#ifndef _Included_io_opencurve_curve_fs_CurveMount
#define _Included_io_opencurve_curve_fs_CurveMount
#ifdef __cplusplus
extern "C" {
#endif
#undef io_opencurve_curve_fs_CurveMount_O_RDONLY
#define io_opencurve_curve_fs_CurveMount_O_RDONLY 1L
#undef io_opencurve_curve_fs_CurveMount_O_RDWR
#define io_opencurve_curve_fs_CurveMount_O_RDWR 2L
#undef io_opencurve_curve_fs_CurveMount_O_APPEND
#define io_opencurve_curve_fs_CurveMount_O_APPEND 4L
#undef io_opencurve_curve_fs_CurveMount_O_CREAT
#define io_opencurve_curve_fs_CurveMount_O_CREAT 8L
#undef io_opencurve_curve_fs_CurveMount_O_TRUNC
#define io_opencurve_curve_fs_CurveMount_O_TRUNC 16L
#undef io_opencurve_curve_fs_CurveMount_O_EXCL
#define io_opencurve_curve_fs_CurveMount_O_EXCL 32L
#undef io_opencurve_curve_fs_CurveMount_O_WRONLY
#define io_opencurve_curve_fs_CurveMount_O_WRONLY 64L
#undef io_opencurve_curve_fs_CurveMount_O_DIRECTORY
#define io_opencurve_curve_fs_CurveMount_O_DIRECTORY 128L
#undef io_opencurve_curve_fs_CurveMount_SEEK_SET
#define io_opencurve_curve_fs_CurveMount_SEEK_SET 1L
#undef io_opencurve_curve_fs_CurveMount_SEEK_CUR
#define io_opencurve_curve_fs_CurveMount_SEEK_CUR 2L
#undef io_opencurve_curve_fs_CurveMount_SEEK_END
#define io_opencurve_curve_fs_CurveMount_SEEK_END 3L
#undef io_opencurve_curve_fs_CurveMount_SETATTR_MODE
#define io_opencurve_curve_fs_CurveMount_SETATTR_MODE 1L
#undef io_opencurve_curve_fs_CurveMount_SETATTR_UID
#define io_opencurve_curve_fs_CurveMount_SETATTR_UID 2L
#undef io_opencurve_curve_fs_CurveMount_SETATTR_GID
#define io_opencurve_curve_fs_CurveMount_SETATTR_GID 4L
#undef io_opencurve_curve_fs_CurveMount_SETATTR_MTIME
#define io_opencurve_curve_fs_CurveMount_SETATTR_MTIME 8L
#undef io_opencurve_curve_fs_CurveMount_SETATTR_ATIME
#define io_opencurve_curve_fs_CurveMount_SETATTR_ATIME 16L
/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSCreate
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSCreate
  (JNIEnv *, jobject);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSRelease
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSRelease
  (JNIEnv *, jobject);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSConfSet
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSConfSet
  (JNIEnv *, jclass, jlong, jstring, jstring);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSMount
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSMount
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSUmount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSUmount
  (JNIEnv *, jclass, jlong);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSMkDirs
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSMkDirs
  (JNIEnv *, jclass, jlong, jstring, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSRmDir
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSRmDir
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSListDir
 * Signature: (JLjava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSListDir
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSOpen
 * Signature: (JLjava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSOpen
  (JNIEnv *, jclass, jlong, jstring, jint, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSLSeek
 * Signature: (JIJI)J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSLSeek
  (JNIEnv *, jclass, jlong, jint, jlong, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativieCurveFSRead
 * Signature: (JI[BJJ)J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveMount_nativieCurveFSRead
  (JNIEnv *, jclass, jlong, jint, jbyteArray, jlong, jlong);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativieCurveFSWrite
 * Signature: (JI[BJJ)J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveMount_nativieCurveFSWrite
  (JNIEnv *, jclass, jlong, jint, jbyteArray, jlong, jlong);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSFSync
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSFSync
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSClose
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSClose
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSUnlink
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSUnlink
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSStatFS
 * Signature: (JLjava/lang/String;Lio/opencurve/curve/fs/CurveStatVFS;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSStatFS
  (JNIEnv *, jclass, jlong, jstring, jobject);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSLstat
 * Signature: (JLjava/lang/String;Lio/opencurve/curve/fs/CurveStat;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSLstat
  (JNIEnv *, jclass, jlong, jstring, jobject);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSFStat
 * Signature: (JILio/opencurve/curve/fs/CurveStat;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSFStat
  (JNIEnv *, jclass, jlong, jint, jobject);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSSetAttr
 * Signature: (JLjava/lang/String;Lio/opencurve/curve/fs/CurveStat;I)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSSetAttr
  (JNIEnv *, jclass, jlong, jstring, jobject, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSChmod
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSChmod
  (JNIEnv *, jclass, jlong, jstring, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveMount
 * Method:    nativeCurveFSRename
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveMount_nativeCurveFSRename
  (JNIEnv *, jclass, jlong, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
