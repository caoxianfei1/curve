package io.opencurve.curve.fs.hadoop;

import io.opencurve.curve.fs.flink.CurveFileSystemFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.AbstractFileSystem;
import org.apache.hadoop.fs.DelegateToFileSystem;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

/**
 * The CurveFS implementation of AbstractFileSystem.
 * This impl delegates to the old FileSystem
 */
public class CurveFS extends DelegateToFileSystem {
  /**
   * This constructor has the signature needed by
   * {@link AbstractFileSystem#createFileSystem(URI, Configuration)}.
   *
   * @param theUri which must be that of localFs
   * @param conf
   * @throws IOException
   * @throws URISyntaxException
   */
  CurveFS(final URI theUri, final Configuration conf) throws IOException,
    URISyntaxException {
    super(theUri, new CurveFileSystem(conf), conf, CurveFileSystemFactory.SCHEME, true);
  }
}
