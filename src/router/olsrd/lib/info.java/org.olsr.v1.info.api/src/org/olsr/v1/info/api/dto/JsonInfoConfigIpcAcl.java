package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * An IPC ACL entry in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigIpcAcl implements Comparable<JsonInfoConfigIpcAcl> {
  private boolean host      = false;
  private String  ipAddress = "";
  private int     genmask   = 0;

  /**
   * @return the host status
   */
  public boolean getHost() {
    return this.host;
  }

  /**
   * @param host the host status to set
   */
  @JsonProperty("host")
  public void setHost(final boolean host) {
    this.host = host;
  }

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
   * @return the genmask
   */
  public int getGenmask() {
    return this.genmask;
  }

  /**
   * @param genmask the genmask to set
   */
  @JsonProperty("genmask")
  public void setGenmask(final int genmask) {
    this.genmask = genmask;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.host ? 1231 : 1237);
    result = (prime * result) + this.ipAddress.hashCode();
    result = (prime * result) + this.genmask;
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

    return (this.compareTo((JsonInfoConfigIpcAcl) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigIpcAcl other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.ipAddress.compareTo(other.ipAddress);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.genmask, other.genmask);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(other.host, this.host);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigIpcAcl [host=");
    builder.append(this.host);
    builder.append(", ipAddress=");
    builder.append(this.ipAddress);
    builder.append(", genmask=");
    builder.append(this.genmask);
    builder.append("]");
    return builder.toString();
  }
}