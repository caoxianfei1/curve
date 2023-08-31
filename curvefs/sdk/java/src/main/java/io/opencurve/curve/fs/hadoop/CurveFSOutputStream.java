package io.opencurve.curve.fs.hadoop;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import io.opencurve.curve.fs.libfs.CurveFSMount;

import java.io.IOException;
import java.io.OutputStream;

/**
 * <p>
 * An {@link OutputStream} for a CurveFileSystem and corresponding
 * Curve instance.
 *
 * TODO:
 *  - When libcurvefs-jni supports ByteBuffer interface we can get rid of the
 *  use of the buffer here to reduce memory copies and just use buffers in
 *  libcurvefs. Currently it might be useful to reduce JNI crossings, but not
 *  much more.
 */
public class CurveFSOutputStream extends OutputStream {
  private static final Log LOG = LogFactory.getLog(CurveFSOutputStream.class);
  private boolean closed;

  private CurveFSProto curve;

  private int fileHandle;

  private byte[] buffer;
  private int bufUsed = 0;

  /**
   * Construct the CurveOutputStream.
   * @param conf The FileSystem configuration.
   * @param fh The Curve filehandle to connect to.
   */
  public CurveFSOutputStream(Configuration conf, CurveFSProto curvefs,
      int fh, int bufferSize) {
    curve = curvefs;
    fileHandle = fh;
    closed = false;
    buffer = new byte[1<<21];
  }

  /**
   * Close the Curve file handle if close() wasn't explicitly called.
   */
  protected void finalize() throws Throwable {
    try {
      if (!closed) {
        close();
      }
    } finally {
      super.finalize();
    }
  }

  /**
   * Ensure that the stream is opened.
   */
  private synchronized void checkOpen() throws IOException {
    if (closed)
      throw new IOException("operation on closed stream (fd=" + fileHandle + ")");
  }

  /**
   * Get the current position in the file.
   * @return The file offset in bytes.
   */
  public synchronized long getPos() throws IOException {
    checkOpen();
    return curve.lseek(fileHandle, 0, CurveFSMount.SEEK_CUR);
  }

  @Override
  public synchronized void write(int b) throws IOException {
    byte buf[] = new byte[1];
    buf[0] = (byte) b;
    write(buf, 0, 1);
  }

  @Override
  public synchronized void write(byte buf[], int off, int len) throws IOException {
    checkOpen();

    while (len > 0) {
      int remaining = Math.min(len, buffer.length - bufUsed);
      System.arraycopy(buf, off, buffer, bufUsed, remaining);

      bufUsed += remaining;
      off += remaining;
      len -= remaining;

      if (buffer.length == bufUsed)
        flushBuffer();
    }
  }

  /*
   * Moves data from the buffer into libcurvefs.
   */
  private synchronized void flushBuffer() throws IOException {
    if (bufUsed == 0)
      return;

    while (bufUsed > 0) {
      int ret = curve.write(fileHandle, buffer, bufUsed, -1);
      if (ret < 0)
        throw new IOException("curve.write: ret=" + ret);

      if (ret == bufUsed) {
        bufUsed = 0;
        return;
      }

      assert(ret > 0);
      assert(ret < bufUsed);

      /*
       * TODO: handle a partial write by shifting the remainder of the data in
       * the buffer back to the beginning and retrying the write. It would
       * probably be better to use a ByteBuffer 'view' here, and I believe
       * using a ByteBuffer has some other performance benefits but we'll
       * likely need to update the libcurvefs-jni implementation.
       */
      int remaining = bufUsed - ret;
      System.arraycopy(buffer, ret, buffer, 0, remaining);
      bufUsed -= ret;
    }

    assert(bufUsed == 0);
  }

  @Override
  public synchronized void flush() throws IOException {
    checkOpen();
    flushBuffer(); // buffer -> libcurvefs
    curve.fsync(fileHandle); // libcurvefs -> cluster
  }

  @Override
  public synchronized void close() throws IOException {
    checkOpen();
    flush();
    curve.close(fileHandle);
    closed = true;
  }
}
