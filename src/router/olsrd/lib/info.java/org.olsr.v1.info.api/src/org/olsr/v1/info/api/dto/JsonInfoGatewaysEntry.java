package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

/**
 * A gateway entry in the {@link InfoCommand#GATEWAYS} jsoninfo OLSRd plugin response, and the base class for
 * smart-gateway entries in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response.
 */
@ProviderType
public class JsonInfoGatewaysEntry extends JsonInfoGatewaysEntryBase implements Comparable<JsonInfoGatewaysEntry> {

  @Override
  public int compareTo(final JsonInfoGatewaysEntry other) {
    return super.compareTo(other);
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoGatewaysEntry [");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}