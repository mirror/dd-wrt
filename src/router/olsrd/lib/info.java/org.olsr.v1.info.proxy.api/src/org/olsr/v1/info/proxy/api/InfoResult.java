package org.olsr.v1.info.proxy.api;

import java.net.HttpURLConnection;
import java.util.LinkedList;
import java.util.List;

import org.osgi.annotation.versioning.ProviderType;

/**
 * The result of information retrieval from an OLSRd v1 txtinfo plugin or jsoninfo plugin.
 */
@ProviderType
public class InfoResult {
  /** The HTTP status code of the request */
  public int          status = HttpURLConnection.HTTP_OK;

  /** The output of the request */
  public List<String> output = new LinkedList<>();
}