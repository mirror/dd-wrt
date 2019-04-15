package org.olsr.v1.info.api.contants;

import org.osgi.annotation.versioning.ProviderType;

/**
 * OLSRd constants
 */
@ProviderType
public class OlsrdConstants {
  public static final long ROUTE_COST_BROKEN = 0xffffffffL;
  public static final long LINK_COST_BROKEN  = (1L << 22);
}