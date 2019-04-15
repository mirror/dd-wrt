package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * An alias in the {@link InfoCommand#MID} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoTimedIpAddress implements Comparable<JsonInfoTimedIpAddress> {
  private String ipAddress    = "";
  private long   validityTime = 0;

  /**
   * @return the IP address
   */
  public String getIpAddress() {
    return this.ipAddress;
  }

  /**
   * @param ipAddress the IP address to set
   */
  @JsonProperty("ipAddress")
  public void setIpAddress(final InetAddress ipAddress) {
    if (ipAddress == null) {
      this.ipAddress = "";
    } else {
      this.ipAddress = ipAddress.getHostAddress();
    }
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
    result = (prime * result) + this.ipAddress.hashCode();
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

    return (this.compareTo((JsonInfoTimedIpAddress) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoTimedIpAddress other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.ipAddress.compareTo(other.ipAddress);
    if (result != 0) {
      return CompareUtils.clip(result);
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
    builder.append("JsonInfoTimedIpAddress [ipAddress=");
    builder.append(this.ipAddress);
    builder.append(", validityTime=");
    builder.append(this.validityTime);
    builder.append("]");
    return builder.toString();
  }
}