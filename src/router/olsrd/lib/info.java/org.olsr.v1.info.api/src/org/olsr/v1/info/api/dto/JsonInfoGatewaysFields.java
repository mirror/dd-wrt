package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The fields of the {@link InfoCommand#GATEWAYS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoGatewaysFields {
  private final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
  private final Set<JsonInfoGatewaysEntry> ipv6 = new TreeSet<>();

  /**
   * @return the ipv4 gateway entries
   */
  public Set<JsonInfoGatewaysEntry> getIpv4() {
    return this.ipv4;
  }

  /**
   * @param ipv4 the ipv4 gateway entries to set
   */
  @JsonProperty("ipv4")
  public void setIpv4(final Set<JsonInfoGatewaysEntry> ipv4) {
    this.ipv4.clear();
    if (ipv4 != null) {
      this.ipv4.addAll(ipv4);
    }
  }

  /**
   * @return the ipv6 gateway entries
   */
  public Set<JsonInfoGatewaysEntry> getIpv6() {
    return this.ipv6;
  }

  /**
   * @param ipv6 the ipv6 gateway entries to set
   */
  @JsonProperty("ipv6")
  public void setIpv6(final Set<JsonInfoGatewaysEntry> ipv6) {
    this.ipv6.clear();
    if (ipv6 != null) {
      this.ipv6.addAll(ipv6);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.ipv4.hashCode();
    result = (prime * result) + this.ipv6.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (this.getClass() != obj.getClass()) {
      return false;
    }
    final JsonInfoGatewaysFields other = (JsonInfoGatewaysFields) obj;
    if (!this.ipv4.equals(other.ipv4)) {
      return false;
    }
    if (!this.ipv6.equals(other.ipv6)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoGatewaysFields [ipv4=");
    builder.append(this.ipv4);
    builder.append(", ipv6=");
    builder.append(this.ipv6);
    builder.append("]");
    return builder.toString();
  }
}