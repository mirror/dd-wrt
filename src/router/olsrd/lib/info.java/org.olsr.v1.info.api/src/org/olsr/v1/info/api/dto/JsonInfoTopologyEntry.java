package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.OlsrdConstants;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A topology entry in the {@link InfoCommand#TOPOLOGY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoTopologyEntry implements Comparable<JsonInfoTopologyEntry> {
  private String  lastHopIP           = "";
  private double  pathCost            = Double.POSITIVE_INFINITY;
  private long    validityTime        = 0;
  private long    refCount            = 0;
  private int     msgSeq              = 0;
  private int     msgHops             = 0;
  private int     hops                = 0;
  private int     ansn                = 0;
  private int     tcIgnored           = 0;
  private int     errSeq              = 0;
  private boolean errSeqValid         = false;
  private String  destinationIP       = "";
  private double  tcEdgeCost          = Double.POSITIVE_INFINITY;
  private int     ansnEdge            = 0;
  private double  linkQuality         = 0.0;
  private double  neighborLinkQuality = 0.0;

  /**
   * @return the last-hop IP address
   */
  public String getLastHopIP() {
    return this.lastHopIP;
  }

  /**
   * @param lastHopIP the last-hop IP address to set
   */
  @JsonProperty("lastHopIP")
  public void setLastHopIP(final InetAddress lastHopIP) {
    if (lastHopIP == null) {
      this.lastHopIP = "";
    } else {
      this.lastHopIP = lastHopIP.getHostAddress();
    }
  }

  /**
   * @return the path cost
   */
  public double getPathCost() {
    return this.pathCost;
  }

  /**
   * @param pathCost the path cost to set
   */
  @JsonProperty("pathCost")
  public void setPathCost(final double pathCost) {
    if (Double.compare(pathCost, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.pathCost = Double.POSITIVE_INFINITY;
    } else {
      this.pathCost = pathCost;
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

  /**
   * @return the reference count
   */
  public long getRefCount() {
    return this.refCount;
  }

  /**
   * @param refCount the reference count to set
   */
  @JsonProperty("refCount")
  public void setRefCount(final long refCount) {
    this.refCount = refCount;
  }

  /**
   * @return the message sequence number
   */
  public int getMsgSeq() {
    return this.msgSeq;
  }

  /**
   * @param msgSeq the message sequence number to set
   */
  @JsonProperty("msgSeq")
  public void setMsgSeq(final int msgSeq) {
    this.msgSeq = msgSeq;
  }

  /**
   * @return the message hop count
   */
  public int getMsgHops() {
    return this.msgHops;
  }

  /**
   * @param msgHops the message hop count to set
   */
  @JsonProperty("msgHops")
  public void setMsgHops(final int msgHops) {
    this.msgHops = msgHops;
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

  /**
   * @return the ansn
   */
  public int getAnsn() {
    return this.ansn;
  }

  /**
   * @param ansn the ansn to set
   */
  @JsonProperty("ansn")
  public void setAnsn(final int ansn) {
    this.ansn = ansn;
  }

  /**
   * @return the number of ignored TC messages
   */
  public int getTcIgnored() {
    return this.tcIgnored;
  }

  /**
   * @param tcIgnored the number of ignored TC messages to set
   */
  @JsonProperty("tcIgnored")
  public void setTcIgnored(final int tcIgnored) {
    this.tcIgnored = tcIgnored;
  }

  /**
   * @return the message sequence number of an error
   */
  public int getErrSeq() {
    return this.errSeq;
  }

  /**
   * @param errSeq the message sequence number of an error to set
   */
  @JsonProperty("errSeq")
  public void setErrSeq(final int errSeq) {
    this.errSeq = errSeq;
  }

  /**
   * @return the validity of the message sequence number of an error
   */
  public boolean getErrSeqValid() {
    return this.errSeqValid;
  }

  /**
   * @param errSeqValid the validity of the message sequence number of an error to set
   */
  @JsonProperty("errSeqValid")
  public void setErrSeqValid(final boolean errSeqValid) {
    this.errSeqValid = errSeqValid;
  }

  /**
   * @return the destination IP address
   */
  public String getDestinationIP() {
    return this.destinationIP;
  }

  /**
   * @param destinationIP the destination IP address to set
   */
  @JsonProperty("destinationIP")
  public void setDestinationIP(final InetAddress destinationIP) {
    if (destinationIP == null) {
      this.destinationIP = "";
    } else {
      this.destinationIP = destinationIP.getHostAddress();
    }
  }

  /**
   * @return the TC edge cost
   */
  public double getTcEdgeCost() {
    return this.tcEdgeCost;
  }

  /**
   * @param tcEdgeCost the TC edge cost to set
   */
  @JsonProperty("tcEdgeCost")
  public void setTcEdgeCost(final double tcEdgeCost) {
    if (Double.compare(tcEdgeCost, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.tcEdgeCost = Double.POSITIVE_INFINITY;
    } else {
      this.tcEdgeCost = tcEdgeCost;
    }
  }

  /**
   * @return the ansn edge
   */
  public int getAnsnEdge() {
    return this.ansnEdge;
  }

  /**
   * @param ansnEdge the ansn edge to set
   */
  @JsonProperty("ansnEdge")
  public void setAnsnEdge(final int ansnEdge) {
    this.ansnEdge = ansnEdge;
  }

  /**
   * @return the link quality
   */
  public double getLinkQuality() {
    return this.linkQuality;
  }

  /**
   * @param linkQuality the link quality to set
   */
  @JsonProperty("linkQuality")
  public void setLinkQuality(final double linkQuality) {
    this.linkQuality = linkQuality;
  }

  /**
   * @return the neighbor link quality
   */
  public double getNeighborLinkQuality() {
    return this.neighborLinkQuality;
  }

  /**
   * @param neighborLinkQuality the neighbor link quality to set
   */
  @JsonProperty("neighborLinkQuality")
  public void setNeighborLinkQuality(final double neighborLinkQuality) {
    this.neighborLinkQuality = neighborLinkQuality;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    long temp;
    result = (prime * result) + this.lastHopIP.hashCode();
    temp = Double.doubleToLongBits(this.pathCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (int) (this.validityTime ^ (this.validityTime >>> 32));
    result = (prime * result) + (int) (this.refCount ^ (this.refCount >>> 32));
    result = (prime * result) + this.msgSeq;
    result = (prime * result) + this.msgHops;
    result = (prime * result) + this.hops;
    result = (prime * result) + this.ansn;
    result = (prime * result) + this.tcIgnored;
    result = (prime * result) + this.errSeq;
    result = (prime * result) + (this.errSeqValid ? 1231 : 1237);
    result = (prime * result) + this.destinationIP.hashCode();
    temp = Double.doubleToLongBits(this.tcEdgeCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.ansnEdge;
    temp = Double.doubleToLongBits(this.linkQuality);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.neighborLinkQuality);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
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

    return (this.compareTo((JsonInfoTopologyEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoTopologyEntry other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.lastHopIP.compareTo(other.lastHopIP);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Double.compare(this.pathCost, other.pathCost);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.validityTime, other.validityTime);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.refCount, other.refCount);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.msgSeq, other.msgSeq);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.msgHops, other.msgHops);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.hops, other.hops);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.ansn, other.ansn);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.tcIgnored, other.tcIgnored);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.errSeq, other.errSeq);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.errSeqValid, other.errSeqValid);
    if (result != 0) {
      return result;
    }

    result = this.destinationIP.compareTo(other.destinationIP);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Double.compare(this.tcEdgeCost, other.tcEdgeCost);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.ansnEdge, other.ansnEdge);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.linkQuality, other.linkQuality);
    if (result != 0) {
      return result;
    }
    result = Double.compare(this.neighborLinkQuality, other.neighborLinkQuality);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoTopologyEntry [lastHopIP=");
    builder.append(this.lastHopIP);
    builder.append(", pathCost=");
    builder.append(this.pathCost);
    builder.append(", validityTime=");
    builder.append(this.validityTime);
    builder.append(", refCount=");
    builder.append(this.refCount);
    builder.append(", msgSeq=");
    builder.append(this.msgSeq);
    builder.append(", msgHops=");
    builder.append(this.msgHops);
    builder.append(", hops=");
    builder.append(this.hops);
    builder.append(", ansn=");
    builder.append(this.ansn);
    builder.append(", tcIgnored=");
    builder.append(this.tcIgnored);
    builder.append(", errSeq=");
    builder.append(this.errSeq);
    builder.append(", errSeqValid=");
    builder.append(this.errSeqValid);
    builder.append(", destinationIP=");
    builder.append(this.destinationIP);
    builder.append(", tcEdgeCost=");
    builder.append(this.tcEdgeCost);
    builder.append(", ansnEdge=");
    builder.append(this.ansnEdge);
    builder.append(", linkQuality=");
    builder.append(this.linkQuality);
    builder.append(", neighborLinkQuality=");
    builder.append(this.neighborLinkQuality);
    builder.append("]");
    return builder.toString();
  }
}