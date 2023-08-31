// -*- mode:Java; tab-width:2; c-basic-offset:2; indent-tabs-mode:t -*-

package io.opencurve.curve.fs.hadoop;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;

import io.opencurve.curve.fs.libfs.CurveFSMount;
import io.opencurve.curve.fs.libfs.CurveFSStat;
import io.opencurve.curve.fs.libfs.CurveFSStatVFS;

import java.io.IOException;
import java.net.InetAddress;
import java.net.URI;

abstract class CurveFSProto {

  abstract void initialize(URI uri, Configuration conf) throws IOException;
  abstract int __open(Path path, int flags, int mode) throws IOException;
  abstract int open(Path path, int flags, int mode) throws IOException;
//  abstract int open(Path path, int flags, int mode, int stripe_unit,
//      int stripe_count, int object_size, String data_pool) throws IOException;
  abstract void fstat(int fd, CurveFSStat stat) throws IOException;
  abstract void lstat(Path path, CurveFSStat stat) throws IOException;
  abstract void statfs(Path path, CurveFSStatVFS stat) throws IOException;
  abstract void unlink(Path path) throws IOException;
  abstract void rmdir(Path path) throws IOException;
  abstract String[] listdir(Path path) throws IOException;
  abstract void setattr(Path path, CurveFSStat stat, int mask) throws IOException;
  abstract void chmod(Path path, int mode) throws IOException;
  abstract long lseek(int fd, long offset, int whence) throws IOException;
  abstract void close(int fd) throws IOException;
  abstract void shutdown() throws IOException;
  abstract void rename(Path src, Path dst) throws IOException;
  abstract short getDefaultReplication();
  abstract short get_file_replication(Path path) throws IOException;
  abstract int write(int fd, byte[] buf, long size, long offset) throws IOException;
  abstract int read(int fd, byte[] buf, long size, long offset) throws IOException;
  abstract void mkdirs(Path path, int mode) throws IOException;
  abstract void fsync(int fd) throws IOException;
}
