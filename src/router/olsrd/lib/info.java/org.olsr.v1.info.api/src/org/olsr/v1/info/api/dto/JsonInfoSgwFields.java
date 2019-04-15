package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The fields of the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgwFields {
  private final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
  private final Set<JsonInfoSgwEntry>       ipv4   = new TreeSet<>();
  private final Set<JsonInfoSgwEntry>       ipv6   = new TreeSet<>();

  /**
   * @return the egress smart-gateway entries
   */
  public Set<JsonInfoSgwEgressEntry> getEgress() {
    return this.egress;
  }

  /**
   * @param egress the egress smart-gateway entries to set
   */
  @JsonProperty("egress")
  public void setEgress(final Set<JsonInfoSgwEgressEntry> egress) {
    this.egress.clear();
    if (egress != null) {
      this.egress.addAll(egress);
    }
  }

  /**
   * @return the ipv4 smart-gateway entries
   */
  public Set<JsonInfoSgwEntry> getIpv4() {
    return this.ipv4;
  }

  /**
   * @param ipv4 the ipv4 smart-gateway entries to set
   */
  @JsonProperty("ipv4")
  public void setIpv4(final Set<JsonInfoSgwEntry> ipv4) {
    this.ipv4.clear();
    if (ipv4 != null) {
      this.ipv4.addAll(ipv4);
    }
  }

  /**
   * @return the ipv6 smart-gateway entries
   */
  public Set<JsonInfoSgwEntry> getIpv6() {
    return this.ipv6;
  }

  /**
   * @param ipv6 the ipv6 smart-gateway entries to set
   */
  @JsonProperty("ipv6")
  public void setIpv6(final Set<JsonInfoSgwEntry> ipv6) {
    this.ipv6.clear();
    if (ipv6 != null) {
      this.ipv6.addAll(ipv6);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.egress.hashCode();
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
    final JsonInfoSgwFields other = (JsonInfoSgwFields) obj;
    if (!this.egress.equals(other.egress)) {
      return false;
    }
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
    builder.append("JsonInfoSgwFields [egress=");
    builder.append(this.egress);
    builder.append(", ipv4=");
    builder.append(this.ipv4);
    builder.append(", ipv6=");
    builder.append(this.ipv6);
    builder.append("]");
    return builder.toString();
  }
}