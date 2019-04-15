package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Smart-gateway prefix configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigSgwPrefix implements Comparable<JsonInfoConfigSgwPrefix> {
  private String prefix = "";
  private int    length = 0;

  /**
   * @return the prefix IP address
   */
  public String getPrefix() {
    return this.prefix;
  }

  /**
   * @param prefix the prefix IP address to set
   */
  @JsonProperty("prefix")
  public void setPrefix(final InetAddress prefix) {
    if (prefix == null) {
      this.prefix = "";
    } else {
      this.prefix = prefix.getHostAddress();
    }
  }

  /**
   * @return the prefix length
   */
  public int getLength() {
    return this.length;
  }

  /**
   * @param length the prefix length to set
   */
  @JsonProperty("length")
  public void setLength(final int length) {
    this.length = length;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.prefix.hashCode();
    result = (prime * result) + this.length;
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

    return (this.compareTo((JsonInfoConfigSgwPrefix) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigSgwPrefix other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.prefix.compareTo(other.prefix);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Long.compare(this.length, other.length);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigSgwPrefix [prefix=");
    builder.append(this.prefix);
    builder.append(", length=");
    builder.append(this.length);
    builder.append("]");
    return builder.toString();
  }
}