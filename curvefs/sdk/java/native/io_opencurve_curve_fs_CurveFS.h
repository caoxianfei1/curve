/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class io_opencurve_curve_fs_CurveFS */

#ifndef _Included_io_opencurve_curve_fs_CurveFS
#define _Included_io_opencurve_curve_fs_CurveFS
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     io_opencurve_curve_fs_CurveFS
 * Method:    nativeCurveFScreate
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_io_opencurve_curve_fs_CurveFS_nativeCurveFScreate
  (JNIEnv *, jclass);

/*
 * Class:     io_opencurve_curve_fs_CurveFS
 * Method:    nativeCurveFSMount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveFS_nativeCurveFSMount
  (JNIEnv *, jclass, jlong);

/*
 * Class:     io_opencurve_curve_fs_CurveFS
 * Method:    nativeCurveFSMkdir
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveFS_nativeCurveFSMkdir
  (JNIEnv *, jclass, jlong, jstring, jint);

/*
 * Class:     io_opencurve_curve_fs_CurveFS
 * Method:    nativeCurveFSRmdir
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_opencurve_curve_fs_CurveFS_nativeCurveFSRmdir
  (JNIEnv *, jclass, jlong, jstring);

#ifdef __cplusplus
}
#endif
#endif