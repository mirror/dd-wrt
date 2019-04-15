package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A smart-gateway entry in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgwEntry extends JsonInfoGatewaysEntryBase implements Comparable<JsonInfoSgwEntry> {
  private String destination  = "";
  private String tunnel       = "";
  private int    tableNr      = 0;
  private int    ruleNr       = 0;
  private int    bypassRuleNr = 0;

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
   * @return the tunnel name
   */
  public String getTunnel() {
    return this.tunnel;
  }

  /**
   * @param tunnel the tunnel name to set
   */
  @JsonProperty("tunnel")
  public void setTunnel(final String tunnel) {
    if (tunnel == null) {
      this.tunnel = "";
    } else {
      this.tunnel = tunnel;
    }
  }

  /**
   * @return the routing table number
   */
  public int getTableNr() {
    return this.tableNr;
  }

  /**
   * @param tableNr the routing table number to set
   */
  @JsonProperty("tableNr")
  public void setTableNr(final int tableNr) {
    this.tableNr = tableNr;
  }

  /**
   * @return the iptables rule number
   */
  public int getRuleNr() {
    return this.ruleNr;
  }

  /**
   * @param ruleNr the iptables rule number to set
   */
  @JsonProperty("ruleNr")
  public void setRuleNr(final int ruleNr) {
    this.ruleNr = ruleNr;
  }

  /**
   * @return the iptables bypass rule number
   */
  public int getBypassRuleNr() {
    return this.bypassRuleNr;
  }

  /**
   * @param bypassRuleNr the iptables bypass rule number to set
   */
  @JsonProperty("bypassRuleNr")
  public void setBypassRuleNr(final int bypassRuleNr) {
    this.bypassRuleNr = bypassRuleNr;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.destination.hashCode();
    result = (prime * result) + this.tunnel.hashCode();
    result = (prime * result) + this.tableNr;
    result = (prime * result) + this.ruleNr;
    result = (prime * result) + this.bypassRuleNr;
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

    return (this.compareTo((JsonInfoSgwEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoSgwEntry other) {
    if (other == null) {
      return -1;
    }

    int result = super.compareTo(other);
    if (result != 0) {
      return result;
    }

    result = this.tunnel.compareTo(other.tunnel);
    if (result != 0) {
      return result;
    }

    result = this.destination.compareTo(other.destination);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.tableNr, other.tableNr);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.ruleNr, other.ruleNr);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.bypassRuleNr, other.bypassRuleNr);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoSgwEntry [destination=");
    builder.append(this.destination);
    builder.append(", tunnel=");
    builder.append(this.tunnel);
    builder.append(", tableNr=");
    builder.append(this.tableNr);
    builder.append(", ruleNr=");
    builder.append(this.ruleNr);
    builder.append(", bypassRuleNr=");
    builder.append(this.bypassRuleNr);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}