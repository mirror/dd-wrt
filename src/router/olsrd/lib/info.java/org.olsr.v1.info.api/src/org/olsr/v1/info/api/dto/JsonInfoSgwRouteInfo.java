package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A smart-gateway route info entry in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgwRouteInfo implements Comparable<JsonInfoSgwRouteInfo> {
  private boolean active         = false;
  private int     family         = 0;
  private long    rtTable        = 0;
  private long    flags          = 0;
  private int     scope          = 0;
  private int     ifIndex        = 0;
  private int     metric         = 0;
  private int     protocol       = 0;
  private boolean srcSet         = false;
  private boolean gwSet          = false;
  private boolean dstSet         = false;
  private boolean delSimilar     = false;
  private boolean blackhole      = false;
  private String  srcStore       = "";
  private String  gwStore        = "";
  private String  dstStore       = "";
  private int     dstStoreLength = 0;

  /**
   * @return the active status
   */
  public boolean getActive() {
    return this.active;
  }

  /**
   * @param active the active status to set
   */
  @JsonProperty("active")
  public void setActive(final boolean active) {
    this.active = active;
  }

  /**
   * @return the protocol family
   */
  public int getFamily() {
    return this.family;
  }

  /**
   * @param family the protocol family to set
   */
  @JsonProperty("family")
  public void setFamily(final int family) {
    this.family = family;
  }

  /**
   * @return the routing table
   */
  public long getRtTable() {
    return this.rtTable;
  }

  /**
   * @param rtTable the routing table to set
   */
  @JsonProperty("rtTable")
  public void setRtTable(final long rtTable) {
    this.rtTable = rtTable;
  }

  /**
   * @return the flags
   */
  public long getFlags() {
    return this.flags;
  }

  /**
   * @param flags the flags to set
   */
  @JsonProperty("flags")
  public void setFlags(final long flags) {
    this.flags = flags;
  }

  /**
   * @return the scope
   */
  public int getScope() {
    return this.scope;
  }

  /**
   * @param scope the scope to set
   */
  @JsonProperty("scope")
  public void setScope(final int scope) {
    this.scope = scope;
  }

  /**
   * @return the interface index
   */
  public int getIfIndex() {
    return this.ifIndex;
  }

  /**
   * @param ifIndex the interface index to set
   */
  @JsonProperty("ifIndex")
  public void setIfIndex(final int ifIndex) {
    this.ifIndex = ifIndex;
  }

  /**
   * @return the metric
   */
  public int getMetric() {
    return this.metric;
  }

  /**
   * @param metric the metric to set
   */
  @JsonProperty("metric")
  public void setMetric(final int metric) {
    this.metric = metric;
  }

  /**
   * @return the protocol
   */
  public int getProtocol() {
    return this.protocol;
  }

  /**
   * @param protocol the protocol to set
   */
  @JsonProperty("protocol")
  public void setProtocol(final int protocol) {
    this.protocol = protocol;
  }

  /**
   * @return the source-is-set status
   */
  public boolean getSrcSet() {
    return this.srcSet;
  }

  /**
   * @param srcSet the source-is-set status to set
   */
  @JsonProperty("srcSet")
  public void setSrcSet(final boolean srcSet) {
    this.srcSet = srcSet;
  }

  /**
   * @return the gateway-is-set status
   */
  public boolean getGwSet() {
    return this.gwSet;
  }

  /**
   * @param gwSet the gateway-is-set status to set
   */
  @JsonProperty("gwSet")
  public void setGwSet(final boolean gwSet) {
    this.gwSet = gwSet;
  }

  /**
   * @return the destination-is-set status
   */
  public boolean getDstSet() {
    return this.dstSet;
  }

  /**
   * @param dstSet the destination-is-set status to set
   */
  @JsonProperty("dstSet")
  public void setDstSet(final boolean dstSet) {
    this.dstSet = dstSet;
  }

  /**
   * @return the delete-similar status
   */
  public boolean getDelSimilar() {
    return this.delSimilar;
  }

  /**
   * @param delSimilar the delete-similar status to set
   */
  @JsonProperty("delSimilar")
  public void setDelSimilar(final boolean delSimilar) {
    this.delSimilar = delSimilar;
  }

  /**
   * @return the blackhole status
   */
  public boolean getBlackhole() {
    return this.blackhole;
  }

  /**
   * @param blackhole the blackhole status to set
   */
  @JsonProperty("blackhole")
  public void setBlackhole(final boolean blackhole) {
    this.blackhole = blackhole;
  }

  /**
   * @return the source IP address
   */
  public String getSrcStore() {
    return this.srcStore;
  }

  /**
   * @param srcStore the source IP address to set
   */
  @JsonProperty("srcStore")
  public void setSrcStore(final InetAddress srcStore) {
    if (srcStore == null) {
      this.srcStore = "";
    } else {
      this.srcStore = srcStore.getHostAddress();
    }
  }

  /**
   * @return the gateway IP address
   */
  public String getGwStore() {
    return this.gwStore;
  }

  /**
   * @param gwStore the gateway IP address to set
   */
  @JsonProperty("gwStore")
  public void setGwStore(final InetAddress gwStore) {
    if (gwStore == null) {
      this.gwStore = "";
    } else {
      this.gwStore = gwStore.getHostAddress();
    }
  }

  /**
   * @return the destination IP address
   */
  public String getDstStore() {
    return this.dstStore;
  }

  /**
   * @param dstStore the destination IP address to set
   */
  @JsonProperty("dstStore")
  public void setDstStore(final InetAddress dstStore) {
    if (dstStore == null) {
      this.dstStore = "";
    } else {
      this.dstStore = dstStore.getHostAddress();
    }
  }

  /**
   * @return the destination prefix length
   */
  public int getDstStoreLength() {
    return this.dstStoreLength;
  }

  /**
   * @param dstStoreLength the destination prefix length to set
   */
  @JsonProperty("dstStoreLength")
  public void setDstStoreLength(final int dstStoreLength) {
    this.dstStoreLength = dstStoreLength;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.active ? 1231 : 1237);
    result = (prime * result) + this.family;
    result = (prime * result) + (int) (this.rtTable ^ (this.rtTable >>> 32));
    result = (prime * result) + (int) (this.flags ^ (this.flags >>> 32));
    result = (prime * result) + this.scope;
    result = (prime * result) + this.ifIndex;
    result = (prime * result) + this.metric;
    result = (prime * result) + this.protocol;
    result = (prime * result) + (this.srcSet ? 1231 : 1237);
    result = (prime * result) + (this.gwSet ? 1231 : 1237);
    result = (prime * result) + (this.dstSet ? 1231 : 1237);
    result = (prime * result) + (this.delSimilar ? 1231 : 1237);
    result = (prime * result) + (this.blackhole ? 1231 : 1237);
    result = (prime * result) + this.srcStore.hashCode();
    result = (prime * result) + this.gwStore.hashCode();
    result = (prime * result) + this.dstStore.hashCode();
    result = (prime * result) + this.dstStoreLength;
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

    return (this.compareTo((JsonInfoSgwRouteInfo) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoSgwRouteInfo other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Boolean.compare(other.active, this.active);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.family, other.family);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.rtTable, other.rtTable);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.flags, other.flags);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.scope, other.scope);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.ifIndex, other.ifIndex);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.metric, other.metric);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.protocol, other.protocol);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.srcSet, other.srcSet);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.gwSet, other.gwSet);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.dstSet, other.dstSet);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.delSimilar, other.delSimilar);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.blackhole, other.blackhole);
    if (result != 0) {
      return result;
    }

    result = this.srcStore.compareTo(other.srcStore);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.gwStore.compareTo(other.gwStore);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.dstStore.compareTo(other.dstStore);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.dstStoreLength, other.dstStoreLength);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoSgwRouteInfo [active=");
    builder.append(this.active);
    builder.append(", family=");
    builder.append(this.family);
    builder.append(", rtTable=");
    builder.append(this.rtTable);
    builder.append(", flags=");
    builder.append(this.flags);
    builder.append(", scope=");
    builder.append(this.scope);
    builder.append(", ifIndex=");
    builder.append(this.ifIndex);
    builder.append(", metric=");
    builder.append(this.metric);
    builder.append(", protocol=");
    builder.append(this.protocol);
    builder.append(", srcSet=");
    builder.append(this.srcSet);
    builder.append(", gwSet=");
    builder.append(this.gwSet);
    builder.append(", dstSet=");
    builder.append(this.dstSet);
    builder.append(", delSimilar=");
    builder.append(this.delSimilar);
    builder.append(", blackhole=");
    builder.append(this.blackhole);
    builder.append(", srcStore=");
    builder.append(this.srcStore);
    builder.append(", gwStore=");
    builder.append(this.gwStore);
    builder.append(", dstStore=");
    builder.append(this.dstStore);
    builder.append(", dstStoreLength=");
    builder.append(this.dstStoreLength);
    builder.append("]");
    return builder.toString();
  }
}