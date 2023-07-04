package io.opencurve.curve.fs.libfs;

public class CurveFSNativeLoader {
    private static boolean initialized = false;
    private static final CurveFSNativeLoader instance = new CurveFSNativeLoader();

    private static final String LIBRARY_NAME = "curvefs_jni";
    private static final String JNI_PATH_ENV_VAR = "CURVEFS_JNI_PATH";

    private CurveFSNativeLoader() {}

    public static CurveFSNativeLoader getInstance() {
        return instance;
    }

    public synchronized void loadLibrary() {
        if (initialized) {
            return;
        }

        String path = System.getenv(JNI_PATH_ENV_VAR);
        if (path != null) {
            System.load(path);
        } else {
            System.loadLibrary(LIBRARY_NAME);
        }
        initialized = true;
    }
}

