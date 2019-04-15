package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.OlsrdConstants;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The base class of a gateway entry in the {@link InfoCommand#GATEWAYS} jsoninfo OLSRd plugin response and of a
 * smart-gateway entry in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response.
 */
@ProviderType
public class JsonInfoGatewaysEntryBase {
  private boolean selected    = false;
  private boolean selectable  = false;
  private String  originator  = "";
  private String  prefix      = "";
  private int     prefixLen   = 0;
  private long    uplink      = 0;
  private long    downlink    = 0;
  private double  cost        = Double.POSITIVE_INFINITY;
  private boolean ipv4        = false;
  private boolean ipv4Nat     = false;
  private boolean ipv6        = false;
  private long    expireTime  = 0;
  private long    cleanupTime = 0;
  private double  pathCost    = Double.POSITIVE_INFINITY;
  private int     hops        = 0;

  /**
   * @return true when the gateway is marked as selected
   */
  public boolean getSelected() {
    return this.selected;
  }

  /**
   * @param selected true to mark the gateway as selected
   */
  @JsonProperty("selected")
  public void setSelected(final boolean selected) {
    this.selected = selected;
  }

  /**
   * @return true when the gateway is marked as selectable
   */
  public boolean getSelectable() {
    return this.selectable;
  }

  /**
   * @param selectable true to mark the gateway as selectable
   */
  @JsonProperty("selectable")
  public void setSelectable(final boolean selectable) {
    this.selectable = selectable;
  }

  /**
   * @return the originator IP address
   */
  public String getOriginator() {
    return this.originator;
  }

  /**
   * @param originator the originator IP address to set
   */
  @JsonProperty("originator")
  public void setOriginator(final InetAddress originator) {
    if (originator == null) {
      this.originator = "";
    } else {
      this.originator = originator.getHostAddress();
    }
  }

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
  public int getPrefixLen() {
    return this.prefixLen;
  }

  /**
   * @param prefixLen the prefix length to set
   */
  @JsonProperty("prefixLen")
  public void setPrefixLen(final int prefixLen) {
    this.prefixLen = prefixLen;
  }

  /**
   * @return the uplink bandwidth (kbps)
   */
  public long getUplink() {
    return this.uplink;
  }

  /**
   * @param uplink the uplink bandwidth (kbps) to set
   */
  @JsonProperty("uplink")
  public void setUplink(final long uplink) {
    this.uplink = uplink;
  }

  /**
   * @return the downlink bandwidth (kbps)
   */
  public long getDownlink() {
    return this.downlink;
  }

  /**
   * @param downlink the downlink bandwidth (kbps) to set
   */
  @JsonProperty("downlink")
  public void setDownlink(final long downlink) {
    this.downlink = downlink;
  }

  /**
   * @return the gateway cost
   */
  public double getCost() {
    return this.cost;
  }

  /**
   * @param cost the gateway cost to set
   */
  @JsonProperty("cost")
  public void setCost(final double cost) {
    if (Double.compare(cost, Long.MAX_VALUE) >= 0) {
      this.cost = Double.POSITIVE_INFINITY;
    } else {
      this.cost = cost;
    }
  }

  /**
   * @return true when the gateway is marked as advertising ipv4
   */
  public boolean getIpv4() {
    return this.ipv4;
  }

  /**
   * @param ipv4 true to mark the gateway as advertising ipv4
   */
  @JsonProperty("IPv4")
  public void setIpv4(final boolean ipv4) {
    this.ipv4 = ipv4;
  }

  /**
   * @return true when the gateway is marked as advertising ipv4Nat
   */
  public boolean getIpv4Nat() {
    return this.ipv4Nat;
  }

  /**
   * @param ipv4Nat true to mark the gateway as advertising ipv4Nat
   */
  @JsonProperty("IPv4-NAT")
  public void setIpv4Nat(final boolean ipv4Nat) {
    this.ipv4Nat = ipv4Nat;
  }

  /**
   * @return true when the gateway is marked as advertising ipv6
   */
  public boolean getIpv6() {
    return this.ipv6;
  }

  /**
   * @param ipv6 true to mark the gateway as advertising ipv6
   */
  @JsonProperty("IPv6")
  public void setIpv6(final boolean ipv6) {
    this.ipv6 = ipv6;
  }

  /**
   * @return the expire time
   */
  public long getExpireTime() {
    return this.expireTime;
  }

  /**
   * @param expireTime the expireT tme to set
   */
  @JsonProperty("expireTime")
  public void setExpireTime(final long expireTime) {
    this.expireTime = expireTime;
  }

  /**
   * @return the cleanup time
   */
  public long getCleanupTime() {
    return this.cleanupTime;
  }

  /**
   * @param cleanupTime the cleanup time to set
   */
  @JsonProperty("cleanupTime")
  public void setCleanupTime(final long cleanupTime) {
    this.cleanupTime = cleanupTime;
  }

  /**
   * @return the path costs
   */
  public double getPathCost() {
    return this.pathCost;
  }

  /**
   * @param pathCost the path costs to set
   */
  @JsonProperty("pathcost")
  public void setPathCost(final double pathCost) {
    if (Double.compare(pathCost, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.pathCost = Double.POSITIVE_INFINITY;
    } else {
      this.pathCost = pathCost;
    }
  }

  /**
   * @return the hops
   */
  public int getHops() {
    return this.hops;
  }

  /**
   * @param hops the hops to set
   */
  @JsonProperty("hops")
  public void setHops(final int hops) {
    this.hops = hops;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.selected ? 1231 : 1237);
    result = (prime * result) + (this.selectable ? 1231 : 1237);
    result = (prime * result) + this.originator.hashCode();
    result = (prime * result) + this.prefix.hashCode();
    result = (prime * result) + this.prefixLen;
    result = (prime * result) + (int) (this.uplink ^ (this.uplink >>> 32));
    result = (prime * result) + (int) (this.downlink ^ (this.downlink >>> 32));
    long temp = Double.doubleToLongBits(this.cost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (this.ipv4 ? 1231 : 1237);
    result = (prime * result) + (this.ipv4Nat ? 1231 : 1237);
    result = (prime * result) + (this.ipv6 ? 1231 : 1237);
    result = (prime * result) + (int) (this.expireTime ^ (this.expireTime >>> 32));
    result = (prime * result) + (int) (this.cleanupTime ^ (this.cleanupTime >>> 32));
    temp = Double.doubleToLongBits(this.pathCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.hops;
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

    return (this.compareTo((JsonInfoGatewaysEntryBase) other) == 0);
  }

  public int compareTo(final JsonInfoGatewaysEntryBase other) {
    if (other == null) {
      return -1;
    }

    int result = Boolean.compare(this.selected, other.selected);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.selectable, other.selectable);
    if (result != 0) {
      return result;
    }

    result = this.originator.compareTo(other.originator);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.prefix.compareTo(other.prefix);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.prefixLen, other.prefixLen);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.uplink, other.uplink);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.downlink, other.downlink);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.cost, other.cost);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.ipv4, other.ipv4);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.ipv4Nat, other.ipv4Nat);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.ipv6, other.ipv6);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.expireTime, other.expireTime);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.cleanupTime, other.cleanupTime);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.pathCost, other.pathCost);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.hops, other.hops);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoGatewaysEntry [selected=");
    builder.append(this.selected);
    builder.append(", selectable=");
    builder.append(this.selectable);
    builder.append(", originator=");
    builder.append(this.originator);
    builder.append(", prefix=");
    builder.append(this.prefix);
    builder.append(", prefixLen=");
    builder.append(this.prefixLen);
    builder.append(", uplink=");
    builder.append(this.uplink);
    builder.append(", downlink=");
    builder.append(this.downlink);
    builder.append(", cost=");
    builder.append(this.cost);
    builder.append(", ipv4=");
    builder.append(this.ipv4);
    builder.append(", ipv4Nat=");
    builder.append(this.ipv4Nat);
    builder.append(", ipv6=");
    builder.append(this.ipv6);
    builder.append(", expireTime=");
    builder.append(this.expireTime);
    builder.append(", cleanupTime=");
    builder.append(this.cleanupTime);
    builder.append(", pathCost=");
    builder.append(this.pathCost);
    builder.append(", hops=");
    builder.append(this.hops);
    builder.append("]");
    return builder.toString();
  }
}