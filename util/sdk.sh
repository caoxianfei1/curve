#!/usr/bin/env bash

set -e

g_hadoop_prefix="/home/${USER}/.local/hadoop"
g_hadoop_lib="${g_hadoop_prefix}/share/hadoop/common/lib"
g_hadoop_etc="${g_hadoop_prefix}/etc/hadoop/core-site.xml"
g_libcurvefs_jni="/curve/bazel-bin/curvefs/sdk/java/native/libcurvefs_jni.so"
g_curvefs_hadoop_jar="/curve/curvefs/sdk/java/target/curvefs-hadoop-1.0-SNAPSHOT.jar"

# build libcurvefs_jni.so
(
    cd /curve/curvefs/sdk/java/src/main/java &&
    javac -h /curve/curvefs/sdk/java/native/ io/opencurve/curve/fs/libfs/CurveFSMount.java
)
(
    cd /curve &&
    bazel build --compilation_mode=dbg --config=gcc7-later //curvefs/sdk/java/native:curvefs_jni
)
(
    sudo rm -rf /usr/lib/libcurvefs_jni.so &&
    sudo cp "$(realpath ${g_libcurvefs_jni})" /usr/lib/libcurvefs_jni.so
)

# build curvefs-hadoop
(
    cd /curve/curvefs/sdk/java &&
    mvn package
)

# setup hadoop
(
    cd "${g_hadoop_lib}" &&
    rm -f curvefs-hadoop-1.0-SNAPSHOT.jar &&
    ln -s "${g_curvefs_hadoop_jar}" curvefs-hadoop-1.0-SNAPSHOT.jar
)

# output
g_output=/curve/curvefs/sdk/output
rm -rf ${g_output}
mkdir -p ${g_output}
(
    cd "${g_output}" &&
    cp "${g_hadoop_etc}" . &&
    cp "${g_curvefs_hadoop_jar}" . &&
    cp "$(realpath ${g_libcurvefs_jni})" .
)

echo -e "\nBuild SDK success :"
echo "-------------------"
ls -l ${g_output}/*
