package org.olsr.v1.info.proxy.api;

import java.io.IOException;

import org.osgi.annotation.versioning.ProviderType;

/**
 * The interface of a component that can retrieve information from an OLSRd v1 txtinfo plugin or jsoninfo plugin.
 */
@ProviderType
public interface OlsrdInfoProxy {
  /**
   * Send a command to the OLSRd v1 txtinfo plugin and return the output.
   *
   * @param command The command to send
   * @return The output, line by line in an ordered list (never null). Any HTTP headers will be removed from the raw
   *         output of the OLSRd v1 txtinfo plugin.
   * @throws IOException When the command is null or empty, or when reading from the OLSRd v1 txtinfo plugin failed
   */
  InfoResult getTxtInfo(String command) throws IOException;

  /**
   * Send a command to the OLSRd v1 jsoninfo plugin and return the output.
   *
   * @param command The command to send
   * @return The output, as a single line (never null). Any HTTP headers will be removed from the raw output of the
   *         OLSRd v1 jsoninfo plugin.
   * @throws IOException When the command is null or empty, or when reading from the OLSRd v1 jsoninfo plugin failed
   */
  InfoResult getJsonInfo(String command) throws IOException;
}