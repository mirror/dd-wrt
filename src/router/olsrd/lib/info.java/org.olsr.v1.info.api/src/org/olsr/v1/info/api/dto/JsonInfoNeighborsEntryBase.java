package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.Willingness;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A neighbor entry in the {@link InfoCommand#NEIGHBORS} jsoninfo OLSRd plugin response, and the base class for 2-hop
 * entries in the {@link InfoCommand#TWOHOP} jsoninfo OLSRd plugin response.
 */
@ProviderType
public class JsonInfoNeighborsEntryBase {
  private String      ipAddress               = "";
  private boolean     symmetric               = false;
  private Willingness willingness             = Willingness.UNKNOWN;
  private boolean     isMultiPointRelay       = false;
  private boolean     wasMultiPointRelay      = false;
  private boolean     multiPointRelaySelector = false;
  private boolean     skip                    = false;
  private int         neighbor2nocov          = 0;
  private int         linkcount               = 0;
  private int         twoHopNeighborCount     = 0;

  /**
   * @return the IP address of the neighbor
   */
  public String getIpAddress() {
    return this.ipAddress;
  }

  /**
   * @param ipAddress the IP address of the neighbor to set
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
   * @return true when the neighbor is marked as symmetric
   */
  public boolean getSymmetric() {
    return this.symmetric;
  }

  /**
   * @param symmetric true to mark the neighbor as symmetric
   */
  @JsonProperty("symmetric")
  public void setSymmetric(final boolean symmetric) {
    this.symmetric = symmetric;
  }

  /**
   * @return the willingness of the neighbor
   */
  public Willingness getWillingness() {
    return this.willingness;
  }

  /**
   * @param willingness the willingness of the neighbor to set
   */
  @JsonProperty("willingness")
  public void setWillingness(final int willingness) {
    this.willingness = Willingness.fromValue(willingness);
  }

  /**
   * @return true when the neighbor is marked as a multi-point relay
   */
  public boolean getMultiPointRelay() {
    return this.isMultiPointRelay;
  }

  /**
   * @param isMultiPointRelay true to mark the neighbor as a multi-point relay
   */
  @JsonProperty("isMultiPointRelay")
  public void setIsMultiPointRelay(final boolean isMultiPointRelay) {
    this.isMultiPointRelay = isMultiPointRelay;
  }

  /**
   * @return true when the neighbor is marked as previously being a multi-point relay
   */
  public boolean wasMultiPointRelay() {
    return this.wasMultiPointRelay;
  }

  /**
   * @param wasMultiPointRelay true to mark the neighbor as previously being a multi-point relay
   */
  @JsonProperty("wasMultiPointRelay")
  public void setWasMultiPointRelay(final boolean wasMultiPointRelay) {
    this.wasMultiPointRelay = wasMultiPointRelay;
  }

  /**
   * @return true when the neighbor is marked as a multi-point relay selector
   */
  public boolean getMultiPointRelaySelector() {
    return this.multiPointRelaySelector;
  }

  /**
   * @param multiPointRelaySelector true to mark the neighbor as a multi-point relay selector
   */
  @JsonProperty("multiPointRelaySelector")
  public void setMultiPointRelaySelector(final boolean multiPointRelaySelector) {
    this.multiPointRelaySelector = multiPointRelaySelector;
  }

  /**
   * @return true when the neighbor is marked as skipped
   */
  public boolean getSkip() {
    return this.skip;
  }

  /**
   * @param skip true to mark the neighbor as skipped
   */
  @JsonProperty("skip")
  public void setSkip(final boolean skip) {
    this.skip = skip;
  }

  /**
   * @return true when the neighbor is marked as neighbor2nocov
   */
  public int getNeighbor2nocov() {
    return this.neighbor2nocov;
  }

  /**
   * @param neighbor2nocov true to mark the neighbor as neighbor2nocov
   */
  @JsonProperty("neighbor2nocov")
  public void setNeighbor2nocov(final int neighbor2nocov) {
    this.neighbor2nocov = neighbor2nocov;
  }

  /**
   * @return the link count of the neighbor
   */
  public int getLinkcount() {
    return this.linkcount;
  }

  /**
   * @param linkcount the link count of the neighbor to set
   */
  @JsonProperty("linkcount")
  public void setLinkcount(final int linkcount) {
    this.linkcount = linkcount;
  }

  /**
   * @return the 2-hop neighbors count
   */
  public int getTwoHopNeighborCount() {
    return this.twoHopNeighborCount;
  }

  /**
   * @param twoHopNeighborCount the 2-hop neighbors count to set
   */
  @JsonProperty("twoHopNeighborCount")
  public void setTwoHopNeighborCount(final int twoHopNeighborCount) {
    this.twoHopNeighborCount = twoHopNeighborCount;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.ipAddress.hashCode();
    result = (prime * result) + (this.symmetric ? 1231 : 1237);
    result = (prime * result) + this.willingness.getValue();
    result = (prime * result) + (this.isMultiPointRelay ? 1231 : 1237);
    result = (prime * result) + (this.wasMultiPointRelay ? 1231 : 1237);
    result = (prime * result) + (this.multiPointRelaySelector ? 1231 : 1237);
    result = (prime * result) + (this.skip ? 1231 : 1237);
    result = (prime * result) + this.neighbor2nocov;
    result = (prime * result) + this.linkcount;
    result = (prime * result) + this.twoHopNeighborCount;
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

    return (this.compareTo((JsonInfoNeighborsEntryBase) other) == 0);
  }

  public int compareTo(final JsonInfoNeighborsEntryBase other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.ipAddress.compareTo(other.ipAddress);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Boolean.compare(this.symmetric, other.symmetric);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.willingness.getValue(), other.willingness.getValue());
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.isMultiPointRelay, other.isMultiPointRelay);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.wasMultiPointRelay, other.wasMultiPointRelay);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.multiPointRelaySelector, other.multiPointRelaySelector);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.skip, other.skip);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.neighbor2nocov, other.neighbor2nocov);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.linkcount, other.linkcount);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.twoHopNeighborCount, other.twoHopNeighborCount);
    if (result != 0) {
      return result;
    }
    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoNeighborsEntryBase [ipAddress=");
    builder.append(this.ipAddress);
    builder.append(", symmetric=");
    builder.append(this.symmetric);
    builder.append(", willingness=");
    builder.append(this.willingness);
    builder.append(", isMultiPointRelay=");
    builder.append(this.isMultiPointRelay);
    builder.append(", wasMultiPointRelay=");
    builder.append(this.wasMultiPointRelay);
    builder.append(", multiPointRelaySelector=");
    builder.append(this.multiPointRelaySelector);
    builder.append(", skip=");
    builder.append(this.skip);
    builder.append(", neighbor2nocov=");
    builder.append(this.neighbor2nocov);
    builder.append(", linkcount=");
    builder.append(this.linkcount);
    builder.append(", twoHopNeighborCount=");
    builder.append(this.twoHopNeighborCount);
    builder.append("]");
    return builder.toString();
  }
}