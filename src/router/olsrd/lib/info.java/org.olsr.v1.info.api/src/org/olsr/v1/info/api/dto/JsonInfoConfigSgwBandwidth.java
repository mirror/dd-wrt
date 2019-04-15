package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Smart-gateway bandwidth configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigSgwBandwidth implements Comparable<JsonInfoConfigSgwBandwidth> {
  private long uplinkKbps   = 0;
  private long downlinkKbps = 0;

  /**
   * @return the uplink bandwidth in Kbps
   */
  public long getUplinkKbps() {
    return this.uplinkKbps;
  }

  /**
   * @param uplinkKbps the uplink bandwidth in Kbps to set
   */
  @JsonProperty("uplinkKbps")
  public void setUplinkKbps(final long uplinkKbps) {
    this.uplinkKbps = uplinkKbps;
  }

  /**
   * @return the downlink bandwidth in Kbps
   */
  public long getDownlinkKbps() {
    return this.downlinkKbps;
  }

  /**
   * @param downlinkKbps the downlink bandwidth in Kbps to set
   */
  @JsonProperty("downlinkKbps")
  public void setDownlinkKbps(final long downlinkKbps) {
    this.downlinkKbps = downlinkKbps;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (int) (this.uplinkKbps ^ (this.uplinkKbps >>> 32));
    result = (prime * result) + (int) (this.downlinkKbps ^ (this.downlinkKbps >>> 32));
    return result;
  }

  @Override
  public boolean equals(final Object other) {
    if (this == other) {
      return true;
    }
    if (other == null) {
      return false;
    }
    if (this.getClass() != other.getClass()) {
      return false;
    }

    return (this.compareTo((JsonInfoConfigSgwBandwidth) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigSgwBandwidth other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Long.compare(this.uplinkKbps, other.uplinkKbps);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.downlinkKbps, other.downlinkKbps);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigSgwBandwidth [uplinkKbps=");
    builder.append(this.uplinkKbps);
    builder.append(", downlinkKbps=");
    builder.append(this.downlinkKbps);
    builder.append("]");
    return builder.toString();
  }
}