package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A HNA entry in the {@link InfoCommand#HNA} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoHnaEntry implements Comparable<JsonInfoHnaEntry> {
  private String gateway                 = "";
  private String destination             = "";
  private int    destinationPrefixLength = 0;
  private long   validityTime            = 0;

  /**
   * @return the gateway IP address
   */
  public String getGateway() {
    return this.gateway;
  }

  /**
   * @param gateway the gateway IP address to set
   */
  @JsonProperty("gateway")
  public void setGateway(final InetAddress gateway) {
    if (gateway == null) {
      this.gateway = "";
    } else {
      this.gateway = gateway.getHostAddress();
    }
  }

  /**
   * @return the destination IP address
   */
  public String getDestination() {
    return this.destination;
  }

  /**
   * @param destination the destination IP address to set
   */
  @JsonProperty("destination")
  public void setDestination(final InetAddress destination) {
    if (destination == null) {
      this.destination = "";
    } else {
      this.destination = destination.getHostAddress();
    }
  }

  /**
   * @return the destination prefix length
   */
  public int getDestinationPrefixLength() {
    return this.destinationPrefixLength;
  }

  /**
   * @param destinationPrefixLength the destination prefix length to set
   */
  @JsonProperty("genmask")
  public void setDestinationPrefixLength(final int destinationPrefixLength) {
    this.destinationPrefixLength = destinationPrefixLength;
  }

  /**
   * @return the validity time
   */
  public long getValidityTime() {
    return this.validityTime;
  }

  /**
   * @param validityTime the validity time to set
   */
  @JsonProperty("validityTime")
  public void setValidityTime(final long validityTime) {
    this.validityTime = validityTime;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.gateway.hashCode();
    result = (prime * result) + this.destination.hashCode();
    result = (prime * result) + this.destinationPrefixLength;
    result = (prime * result) + (int) (this.validityTime ^ (this.validityTime >>> 32));
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

    return (this.compareTo((JsonInfoHnaEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoHnaEntry other) {
    if (other == null) {
      return -1;
    }

    int result = this.gateway.compareTo(other.gateway);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.destination.compareTo(other.destination);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.destinationPrefixLength, other.destinationPrefixLength);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.validityTime, other.validityTime);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoHnaEntry [gateway=");
    builder.append(this.gateway);
    builder.append(", destination=");
    builder.append(this.destination);
    builder.append(", destinationPrefixLength=");
    builder.append(this.destinationPrefixLength);
    builder.append(", validityTime=");
    builder.append(this.validityTime);
    builder.append("]");
    return builder.toString();
  }
}