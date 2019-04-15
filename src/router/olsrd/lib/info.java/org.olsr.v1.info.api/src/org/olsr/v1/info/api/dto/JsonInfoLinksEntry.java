package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.OlsrdConstants;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A link entry in the {@link InfoCommand#LINKS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoLinksEntry implements Comparable<JsonInfoLinksEntry> {
  private String  localIP             = "";
  private String  remoteIP            = "";
  private String  olsrInterface       = "";
  private String  ifName              = "";
  private long    validityTime        = 0;
  private long    symmetryTime        = 0;
  private long    asymmetryTime       = 0;
  private long    vtime               = 0;
  private String  currentLinkStatus   = "";
  private String  previousLinkStatus  = "";
  private double  hysteresis          = 0.0;
  private boolean pending             = false;
  private long    lostLinkTime        = 0;
  private long    helloTime           = 0;
  private long    lastHelloTime       = 0;
  private boolean seqnoValid          = false;
  private int     seqno               = 0;
  private long    lossHelloInterval   = 0;
  private long    lossTime            = 0;
  private long    lossMultiplier      = 0;
  private double  linkCost            = Double.POSITIVE_INFINITY;
  private double  linkQuality         = 0.0;
  private double  neighborLinkQuality = 0.0;

  /**
   * @return the local IP address
   */
  public String getLocalIP() {
    return this.localIP;
  }

  /**
   * @param localIP the local IP address to set
   */
  @JsonProperty("localIP")
  public void setLocalIP(final InetAddress localIP) {
    if (localIP == null) {
      this.localIP = "";
    } else {
      this.localIP = localIP.getHostAddress();
    }
  }

  /**
   * @return the remote IP address
   */
  public String getRemoteIP() {
    return this.remoteIP;
  }

  /**
   * @param remoteIP the remote IP address to set
   */
  @JsonProperty("remoteIP")
  public void setRemoteIP(final InetAddress remoteIP) {
    if (remoteIP == null) {
      this.remoteIP = "";
    } else {
      this.remoteIP = remoteIP.getHostAddress();
    }
  }

  /**
   * @return the olsr interface
   */
  public String getOlsrInterface() {
    return this.olsrInterface;
  }

  /**
   * @param olsrInterface the olsr interface to set
   */
  @JsonProperty("olsrInterface")
  public void setOlsrInterface(final String olsrInterface) {
    if (olsrInterface == null) {
      this.olsrInterface = "";
    } else {
      this.olsrInterface = olsrInterface;
    }
  }

  /**
   * @return the interface name
   */
  public String getIfName() {
    return this.ifName;
  }

  /**
   * @param ifName the interface name to set
   */
  @JsonProperty("ifName")
  public void setIfName(final String ifName) {
    if (ifName == null) {
      this.ifName = "";
    } else {
      this.ifName = ifName;
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
   * @return the symmetry time
   */
  public long getSymmetryTime() {
    return this.symmetryTime;
  }

  /**
   * @param symmetryTime the symmetry time to set
   */
  @JsonProperty("symmetryTime")
  public void setSymmetryTime(final long symmetryTime) {
    this.symmetryTime = symmetryTime;
  }

  /**
   * @return the asymmetry time
   */
  public long getAsymmetryTime() {
    return this.asymmetryTime;
  }

  /**
   * @param asymmetryTime the asymmetry time to set
   */
  @JsonProperty("asymmetryTime")
  public void setAsymmetryTime(final long asymmetryTime) {
    this.asymmetryTime = asymmetryTime;
  }

  /**
   * @return the vtime
   */
  public long getVtime() {
    return this.vtime;
  }

  /**
   * @param vtime the vtime to set
   */
  @JsonProperty("vtime")
  public void setVtime(final long vtime) {
    this.vtime = vtime;
  }

  /**
   * @return the current link status
   */
  public String getCurrentLinkStatus() {
    return this.currentLinkStatus;
  }

  /**
   * @param currentLinkStatus the current link status to set
   */
  @JsonProperty("currentLinkStatus")
  public void setCurrentLinkStatus(final String currentLinkStatus) {
    if (currentLinkStatus == null) {
      this.currentLinkStatus = "";
    } else {
      this.currentLinkStatus = currentLinkStatus;
    }
  }

  /**
   * @return the previous link status
   */
  public String getPreviousLinkStatus() {
    return this.previousLinkStatus;
  }

  /**
   * @param previousLinkStatus the previous link status to set
   */
  @JsonProperty("previousLinkStatus")
  public void setPreviousLinkStatus(final String previousLinkStatus) {
    if (previousLinkStatus == null) {
      this.previousLinkStatus = "";
    } else {
      this.previousLinkStatus = previousLinkStatus;
    }
  }

  /**
   * @return the hysteresis
   */
  public double getHysteresis() {
    return this.hysteresis;
  }

  /**
   * @param hysteresis the hysteresis to set
   */
  @JsonProperty("hysteresis")
  public void setHysteresis(final double hysteresis) {
    this.hysteresis = hysteresis;
  }

  /**
   * @return the pending status
   */
  public boolean getPending() {
    return this.pending;
  }

  /**
   * @param pending the pending status to set
   */
  @JsonProperty("pending")
  public void setPending(final boolean pending) {
    this.pending = pending;
  }

  /**
   * @return the lost link time
   */
  public long getLostLinkTime() {
    return this.lostLinkTime;
  }

  /**
   * @param lostLinkTime the lost link time to set
   */
  @JsonProperty("lostLinkTime")
  public void setLostLinkTime(final long lostLinkTime) {
    this.lostLinkTime = lostLinkTime;
  }

  /**
   * @return the hello time
   */
  public long getHelloTime() {
    return this.helloTime;
  }

  /**
   * @param helloTime the hello time to set
   */
  @JsonProperty("helloTime")
  public void setHelloTime(final long helloTime) {
    this.helloTime = helloTime;
  }

  /**
   * @return the last hello time
   */
  public long getLastHelloTime() {
    return this.lastHelloTime;
  }

  /**
   * @param lastHelloTime the last hello time to set
   */
  @JsonProperty("lastHelloTime")
  public void setLastHelloTime(final long lastHelloTime) {
    this.lastHelloTime = lastHelloTime;
  }

  /**
   * @return the seqno valid status
   */
  public boolean getSeqnoValid() {
    return this.seqnoValid;
  }

  /**
   * @param seqnoValid the seqno valid status to set
   */
  @JsonProperty("seqnoValid")
  public void setSeqnoValid(final boolean seqnoValid) {
    this.seqnoValid = seqnoValid;
  }

  /**
   * @return the seqno
   */
  public int getSeqno() {
    return this.seqno;
  }

  /**
   * @param seqno the seqno to set
   */
  @JsonProperty("seqno")
  public void setSeqno(final int seqno) {
    this.seqno = seqno;
  }

  /**
   * @return the loss hello interval
   */
  public long getLossHelloInterval() {
    return this.lossHelloInterval;
  }

  /**
   * @param lossHelloInterval the loss hello interval to set
   */
  @JsonProperty("lossHelloInterval")
  public void setLossHelloInterval(final long lossHelloInterval) {
    this.lossHelloInterval = lossHelloInterval;
  }

  /**
   * @return the loss time
   */
  public long getLossTime() {
    return this.lossTime;
  }

  /**
   * @param lossTime the loss time to set
   */
  @JsonProperty("lossTime")
  public void setLossTime(final long lossTime) {
    this.lossTime = lossTime;
  }

  /**
   * @return the loss multiplier
   */
  public long getLossMultiplier() {
    return this.lossMultiplier;
  }

  /**
   * @param lossMultiplier the loss multiplier to set
   */
  @JsonProperty("lossMultiplier")
  public void setLossMultiplier(final long lossMultiplier) {
    this.lossMultiplier = lossMultiplier;
  }

  /**
   * @return the link cost
   */
  public double getLinkCost() {
    return this.linkCost;
  }

  /**
   * @param linkCost the link cost to set
   */
  @JsonProperty("linkCost")
  public void setLinkCost(final double linkCost) {
    if (Double.compare(linkCost, OlsrdConstants.LINK_COST_BROKEN) >= 0) {
      this.linkCost = Double.POSITIVE_INFINITY;
    } else {
      this.linkCost = linkCost;
    }
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
    result = (prime * result) + this.localIP.hashCode();
    result = (prime * result) + this.remoteIP.hashCode();
    result = (prime * result) + this.olsrInterface.hashCode();
    result = (prime * result) + this.ifName.hashCode();
    result = (prime * result) + (int) (this.validityTime ^ (this.validityTime >>> 32));
    result = (prime * result) + (int) (this.symmetryTime ^ (this.symmetryTime >>> 32));
    result = (prime * result) + (int) (this.asymmetryTime ^ (this.asymmetryTime >>> 32));
    result = (prime * result) + (int) (this.vtime ^ (this.vtime >>> 32));
    result = (prime * result) + this.currentLinkStatus.hashCode();
    result = (prime * result) + this.previousLinkStatus.hashCode();
    long temp = Double.doubleToLongBits(this.hysteresis);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (this.pending ? 1231 : 1237);
    result = (prime * result) + (int) (this.lostLinkTime ^ (this.lostLinkTime >>> 32));
    result = (prime * result) + (int) (this.helloTime ^ (this.helloTime >>> 32));
    result = (prime * result) + (int) (this.lastHelloTime ^ (this.lastHelloTime >>> 32));
    result = (prime * result) + (this.seqnoValid ? 1231 : 1237);
    result = (prime * result) + this.seqno;
    result = (prime * result) + (int) (this.lossHelloInterval ^ (this.lossHelloInterval >>> 32));
    result = (prime * result) + (int) (this.lossTime ^ (this.lossTime >>> 32));
    result = (prime * result) + (int) (this.lossMultiplier ^ (this.lossMultiplier >>> 32));
    temp = Double.doubleToLongBits(this.linkCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
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

    return (this.compareTo((JsonInfoLinksEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoLinksEntry other) {
    if (other == null) {
      return -1;
    }

    int result;

    /* olsrInterface */

    result = this.olsrInterface.compareTo(other.olsrInterface);
    if (result != 0) {
      return result;
    }

    /* ifName */

    result = this.ifName.compareTo(other.ifName);
    if (result != 0) {
      return result;
    }

    /* localIP */

    result = this.localIP.compareTo(other.localIP);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    /* remoteIP */

    result = this.remoteIP.compareTo(other.remoteIP);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    /* currentLinkStatus */

    result = this.currentLinkStatus.compareTo(other.currentLinkStatus);
    if (result != 0) {
      return result;
    }

    /* previousLinkStatus */

    result = this.previousLinkStatus.compareTo(other.previousLinkStatus);
    if (result != 0) {
      return result;
    }

    /* validityTime */

    result = Long.compare(this.validityTime, other.validityTime);
    if (result != 0) {
      return result;
    }

    /* symmetryTime */

    result = Long.compare(this.symmetryTime, other.symmetryTime);
    if (result != 0) {
      return result;
    }

    /* asymmetryTime; */

    result = Long.compare(this.asymmetryTime, other.asymmetryTime);
    if (result != 0) {
      return result;
    }

    /* vtime; */

    result = Long.compare(this.vtime, other.vtime);
    if (result != 0) {
      return result;
    }

    /* hysteresis */

    result = Double.compare(this.hysteresis, other.hysteresis);
    if (result != 0) {
      return result;
    }

    /* pending */

    result = Boolean.compare(this.pending, other.pending);
    if (result != 0) {
      return result;
    }

    /* lostLinkTime */

    result = Long.compare(this.lostLinkTime, other.lostLinkTime);
    if (result != 0) {
      return result;
    }

    /* helloTime */

    result = Long.compare(this.helloTime, other.helloTime);
    if (result != 0) {
      return result;
    }

    /* lastHelloTime */

    result = Long.compare(this.lastHelloTime, other.lastHelloTime);
    if (result != 0) {
      return result;
    }

    /* seqnoValid */

    result = Boolean.compare(this.seqnoValid, other.seqnoValid);
    if (result != 0) {
      return result;
    }

    /* seqno */

    result = Integer.compare(this.seqno, other.seqno);
    if (result != 0) {
      return result;
    }

    /* lossHelloInterval */

    result = Long.compare(this.lossHelloInterval, other.lossHelloInterval);
    if (result != 0) {
      return result;
    }

    /* lossTime */

    result = Long.compare(this.lossTime, other.lossTime);
    if (result != 0) {
      return result;
    }

    /* lossMultiplier */

    result = Long.compare(this.lossMultiplier, other.lossMultiplier);
    if (result != 0) {
      return result;
    }

    /* linkCost */

    result = Double.compare(this.linkCost, other.linkCost);
    if (result != 0) {
      return result;
    }

    /* linkQuality */

    result = Double.compare(this.linkQuality, other.linkQuality);
    if (result != 0) {
      return result;
    }

    /* neighborLinkQuality */

    result = Double.compare(this.neighborLinkQuality, other.neighborLinkQuality);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoLinksEntry [localIP=");
    builder.append(this.localIP);
    builder.append(", remoteIP=");
    builder.append(this.remoteIP);
    builder.append(", olsrInterface=");
    builder.append(this.olsrInterface);
    builder.append(", ifName=");
    builder.append(this.ifName);
    builder.append(", validityTime=");
    builder.append(this.validityTime);
    builder.append(", symmetryTime=");
    builder.append(this.symmetryTime);
    builder.append(", asymmetryTime=");
    builder.append(this.asymmetryTime);
    builder.append(", vtime=");
    builder.append(this.vtime);
    builder.append(", currentLinkStatus=");
    builder.append(this.currentLinkStatus);
    builder.append(", previousLinkStatus=");
    builder.append(this.previousLinkStatus);
    builder.append(", hysteresis=");
    builder.append(this.hysteresis);
    builder.append(", pending=");
    builder.append(this.pending);
    builder.append(", lostLinkTime=");
    builder.append(this.lostLinkTime);
    builder.append(", helloTime=");
    builder.append(this.helloTime);
    builder.append(", lastHelloTime=");
    builder.append(this.lastHelloTime);
    builder.append(", seqnoValid=");
    builder.append(this.seqnoValid);
    builder.append(", seqno=");
    builder.append(this.seqno);
    builder.append(", lossHelloInterval=");
    builder.append(this.lossHelloInterval);
    builder.append(", lossTime=");
    builder.append(this.lossTime);
    builder.append(", lossMultiplier=");
    builder.append(this.lossMultiplier);
    builder.append(", linkCost=");
    builder.append(this.linkCost);
    builder.append(", linkQuality=");
    builder.append(this.linkQuality);
    builder.append(", neighborLinkQuality=");
    builder.append(this.neighborLinkQuality);
    builder.append("]");
    return builder.toString();
  }
}