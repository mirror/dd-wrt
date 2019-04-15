
package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * An OLSR interface entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoOlsrInterface implements Comparable<JsonInfoOlsrInterface> {
  private boolean              up                    = false;
  private String               ipv4Address           = "";
  private String               ipv4Netmask           = "";
  private String               ipv4Broadcast         = "";
  private String               mode                  = "";
  private String               ipv6Address           = "";
  private String               ipv6Multicast         = "";
  private String               ipAddress             = "";
  private boolean              emulatedInterface     = false;
  private int                  olsrSocket            = -1;
  private int                  sendSocket            = -1;
  private int                  metric                = 0;
  private int                  mtu                   = 0;
  private int                  flags                 = 0;
  private int                  index                 = -1;
  private boolean              wireless              = false;
  private String               name                  = "";
  private int                  seqNum                = 0;
  private JsonInfoMessageTimes messageTimes          = new JsonInfoMessageTimes();
  private boolean              icmpRedirectBackup    = false;
  private boolean              spoofFilterBackup     = false;
  private long                 helloEmissionInterval = 0;
  private JsonInfoMessageTimes validityTimes         = new JsonInfoMessageTimes();
  private long                 forwardingTimeout     = 0;
  private long                 sgwZeroBwTimeout      = 0;
  private int                  ttlIndex              = 0;
  private boolean              immediateSendTc       = false;

  /**
   * @return the up
   */
  public boolean getUp() {
    return this.up;
  }

  /**
   * @param up the up to set
   */
  @JsonProperty("up")
  public void setUp(final boolean up) {
    this.up = up;
  }

  /**
   * @return the ipv4 IP address
   */
  public String getIpv4Address() {
    return this.ipv4Address;
  }

  /**
   * @param ipv4Address the ipv4 IP address to set
   */
  @JsonProperty("ipv4Address")
  public void setIpv4Address(final InetAddress ipv4Address) {
    if (ipv4Address == null) {
      this.ipv4Address = "";
    } else {
      this.ipv4Address = ipv4Address.getHostAddress();
    }
  }

  /**
   * @return the ipv4 netmask IP address
   */
  public String getIpv4Netmask() {
    return this.ipv4Netmask;
  }

  /**
   * @param ipv4Netmask the ipv4 netmask IP address to set
   */
  @JsonProperty("ipv4Netmask")
  public void setIpv4Netmask(final InetAddress ipv4Netmask) {
    if (ipv4Netmask == null) {
      this.ipv4Netmask = "";
    } else {
      this.ipv4Netmask = ipv4Netmask.getHostAddress();
    }
  }

  /**
   * @return the ipv4 broadcast IP address
   */
  public String getIpv4Broadcast() {
    return this.ipv4Broadcast;
  }

  /**
   * @param ipv4Broadcast the ipv4 broadcast IP address to set
   */
  @JsonProperty("ipv4Broadcast")
  public void setIpv4Broadcast(final InetAddress ipv4Broadcast) {
    if (ipv4Broadcast == null) {
      this.ipv4Broadcast = "";
    } else {
      this.ipv4Broadcast = ipv4Broadcast.getHostAddress();
    }
  }

  /**
   * @return the mode
   */
  public String getMode() {
    return this.mode;
  }

  /**
   * @param mode the mode to set
   */
  @JsonProperty("mode")
  public void setMode(final String mode) {
    if (mode == null) {
      this.mode = "";
    } else {
      this.mode = mode;
    }
  }

  /**
   * @return the ipv6 IP address
   */
  public String getIpv6Address() {
    return this.ipv6Address;
  }

  /**
   * @param ipv6Address the ipv6 IP address to set
   */
  @JsonProperty("ipv6Address")
  public void setIpv6Address(final InetAddress ipv6Address) {
    if (ipv6Address == null) {
      this.ipv6Address = "";
    } else {
      this.ipv6Address = ipv6Address.getHostAddress();
    }
  }

  /**
   * @return the ipv6 multicast IP address
   */
  public String getIpv6Multicast() {
    return this.ipv6Multicast;
  }

  /**
   * @param ipv6Multicast the ipv6 multicast IP address to set
   */
  @JsonProperty("ipv6Multicast")
  public void setIpv6Multicast(final InetAddress ipv6Multicast) {
    if (ipv6Multicast == null) {
      this.ipv6Multicast = "";
    } else {
      this.ipv6Multicast = ipv6Multicast.getHostAddress();
    }
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
   * @return the emulatedInterface
   */
  public boolean getEmulatedInterface() {
    return this.emulatedInterface;
  }

  /**
   * @param emulatedInterface the emulatedInterface to set
   */
  @JsonProperty("emulatedInterface")
  public void setEmulatedInterface(final boolean emulatedInterface) {
    this.emulatedInterface = emulatedInterface;
  }

  /**
   * @return the olsrSocket
   */
  public int getOlsrSocket() {
    return this.olsrSocket;
  }

  /**
   * @param olsrSocket the olsrSocket to set
   */
  @JsonProperty("olsrSocket")
  public void setOlsrSocket(final int olsrSocket) {
    this.olsrSocket = olsrSocket;
  }

  /**
   * @return the sendSocket
   */
  public int getSendSocket() {
    return this.sendSocket;
  }

  /**
   * @param sendSocket the sendSocket to set
   */
  @JsonProperty("sendSocket")
  public void setSendSocket(final int sendSocket) {
    this.sendSocket = sendSocket;
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
   * @return the mtu
   */
  public int getMtu() {
    return this.mtu;
  }

  /**
   * @param mtu the mtu to set
   */
  @JsonProperty("mtu")
  public void setMtu(final int mtu) {
    this.mtu = mtu;
  }

  /**
   * @return the flags
   */
  public int getFlags() {
    return this.flags;
  }

  /**
   * @param flags the flags to set
   */
  @JsonProperty("flags")
  public void setFlags(final int flags) {
    this.flags = flags;
  }

  /**
   * @return the index
   */
  public int getIndex() {
    return this.index;
  }

  /**
   * @param index the index to set
   */
  @JsonProperty("index")
  public void setIndex(final int index) {
    this.index = index;
  }

  /**
   * @return the wireless
   */
  public boolean getWireless() {
    return this.wireless;
  }

  /**
   * @param wireless the wireless to set
   */
  @JsonProperty("wireless")
  public void setWireless(final boolean wireless) {
    this.wireless = wireless;
  }

  /**
   * @return the name
   */
  public String getName() {
    return this.name;
  }

  /**
   * @param name the name to set
   */
  @JsonProperty("name")
  public void setName(final String name) {
    if (name == null) {
      this.name = "";
    } else {
      this.name = name;
    }
  }

  /**
   * @return the seqNum
   */
  public int getSeqNum() {
    return this.seqNum;
  }

  /**
   * @param seqNum the seqNum to set
   */
  @JsonProperty("seqNum")
  public void setSeqNum(final int seqNum) {
    this.seqNum = seqNum;
  }

  /**
   * @return the messageTimes
   */
  public JsonInfoMessageTimes getMessageTimes() {
    return this.messageTimes;
  }

  /**
   * @param messageTimes the messageTimes to set
   */
  @JsonProperty("messageTimes")
  public void setMessageTimes(final JsonInfoMessageTimes messageTimes) {
    if (messageTimes == null) {
      this.messageTimes = new JsonInfoMessageTimes();
    } else {
      this.messageTimes = messageTimes;
    }
  }

  /**
   * @return the icmpRedirectBackup
   */
  public boolean getIcmpRedirectBackup() {
    return this.icmpRedirectBackup;
  }

  /**
   * @param icmpRedirectBackup the icmpRedirectBackup to set
   */
  @JsonProperty("icmpRedirectBackup")
  public void setIcmpRedirectBackup(final boolean icmpRedirectBackup) {
    this.icmpRedirectBackup = icmpRedirectBackup;
  }

  /**
   * @return the spoofFilterBackup
   */
  public boolean getSpoofFilterBackup() {
    return this.spoofFilterBackup;
  }

  /**
   * @param spoofFilterBackup the spoofFilterBackup to set
   */
  @JsonProperty("spoofFilterBackup")
  public void setSpoofFilterBackup(final boolean spoofFilterBackup) {
    this.spoofFilterBackup = spoofFilterBackup;
  }

  /**
   * @return the helloEmissionInterval
   */
  public long getHelloEmissionInterval() {
    return this.helloEmissionInterval;
  }

  /**
   * @param helloEmissionInterval the helloEmissionInterval to set
   */
  @JsonProperty("helloEmissionInterval")
  public void setHelloEmissionInterval(final long helloEmissionInterval) {
    this.helloEmissionInterval = helloEmissionInterval;
  }

  /**
   * @return the validityTimes
   */
  public JsonInfoMessageTimes getValidityTimes() {
    return this.validityTimes;
  }

  /**
   * @param validityTimes the validityTimes to set
   */
  @JsonProperty("validityTimes")
  public void setValidityTimes(final JsonInfoMessageTimes validityTimes) {
    if (validityTimes == null) {
      this.validityTimes = new JsonInfoMessageTimes();
    } else {
      this.validityTimes = validityTimes;
    }
  }

  /**
   * @return the forwardingTimeout
   */
  public long getForwardingTimeout() {
    return this.forwardingTimeout;
  }

  /**
   * @param forwardingTimeout the forwardingTimeout to set
   */
  @JsonProperty("forwardingTimeout")
  public void setForwardingTimeout(final long forwardingTimeout) {
    this.forwardingTimeout = forwardingTimeout;
  }

  /**
   * @return the sgwZeroBwTimeout
   */
  public long getSgwZeroBwTimeout() {
    return this.sgwZeroBwTimeout;
  }

  /**
   * @param sgwZeroBwTimeout the sgwZeroBwTimeout to set
   */
  @JsonProperty("sgwZeroBwTimeout")
  public void setSgwZeroBwTimeout(final long sgwZeroBwTimeout) {
    this.sgwZeroBwTimeout = sgwZeroBwTimeout;
  }

  /**
   * @return the ttlIndex
   */
  public int getTtlIndex() {
    return this.ttlIndex;
  }

  /**
   * @param ttlIndex the ttlIndex to set
   */
  @JsonProperty("ttlIndex")
  public void setTtlIndex(final int ttlIndex) {
    this.ttlIndex = ttlIndex;
  }

  /**
   * @return the immediateSendTc
   */
  public boolean getImmediateSendTc() {
    return this.immediateSendTc;
  }

  /**
   * @param immediateSendTc the immediateSendTc to set
   */
  @JsonProperty("immediateSendTc")
  public void setImmediateSendTc(final boolean immediateSendTc) {
    this.immediateSendTc = immediateSendTc;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.up ? 1231 : 1237);
    result = (prime * result) + this.ipv4Address.hashCode();
    result = (prime * result) + this.ipv4Netmask.hashCode();
    result = (prime * result) + this.ipv4Broadcast.hashCode();
    result = (prime * result) + this.mode.hashCode();
    result = (prime * result) + this.ipv6Address.hashCode();
    result = (prime * result) + this.ipv6Multicast.hashCode();
    result = (prime * result) + this.ipAddress.hashCode();
    result = (prime * result) + (this.emulatedInterface ? 1231 : 1237);
    result = (prime * result) + this.olsrSocket;
    result = (prime * result) + this.sendSocket;
    result = (prime * result) + this.metric;
    result = (prime * result) + this.mtu;
    result = (prime * result) + this.flags;
    result = (prime * result) + this.index;
    result = (prime * result) + (this.wireless ? 1231 : 1237);
    result = (prime * result) + this.name.hashCode();
    result = (prime * result) + this.seqNum;
    result = (prime * result) + this.messageTimes.hashCode();
    result = (prime * result) + (this.icmpRedirectBackup ? 1231 : 1237);
    result = (prime * result) + (this.spoofFilterBackup ? 1231 : 1237);
    result = (prime * result) + (int) (this.helloEmissionInterval ^ (this.helloEmissionInterval >>> 32));
    result = (prime * result) + this.validityTimes.hashCode();
    result = (prime * result) + (int) (this.forwardingTimeout ^ (this.forwardingTimeout >>> 32));
    result = (prime * result) + (int) (this.sgwZeroBwTimeout ^ (this.sgwZeroBwTimeout >>> 32));
    result = (prime * result) + this.ttlIndex;
    result = (prime * result) + (this.immediateSendTc ? 1231 : 1237);
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

    return (this.compareTo((JsonInfoOlsrInterface) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoOlsrInterface other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Boolean.compare(other.up, this.up);
    if (result != 0) {
      return result;
    }

    result = this.name.compareTo(other.name);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.mode.compareTo(other.mode);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipAddress.compareTo(other.ipAddress);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipv4Address.compareTo(other.ipv4Address);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipv4Netmask.compareTo(other.ipv4Netmask);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipv4Broadcast.compareTo(other.ipv4Broadcast);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipv6Address.compareTo(other.ipv6Address);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.ipv6Multicast.compareTo(other.ipv6Multicast);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Boolean.compare(this.emulatedInterface, other.emulatedInterface);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.olsrSocket, other.olsrSocket);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.sendSocket, other.sendSocket);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.metric, other.metric);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.mtu, other.mtu);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.flags, other.flags);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.index, other.index);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.wireless, other.wireless);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.seqNum, other.seqNum);
    if (result != 0) {
      return result;
    }

    result = this.messageTimes.compareTo(other.messageTimes);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.icmpRedirectBackup, other.icmpRedirectBackup);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.spoofFilterBackup, other.spoofFilterBackup);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.helloEmissionInterval, other.helloEmissionInterval);
    if (result != 0) {
      return result;
    }

    result = this.validityTimes.compareTo(other.validityTimes);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.forwardingTimeout, other.forwardingTimeout);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.sgwZeroBwTimeout, other.sgwZeroBwTimeout);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.ttlIndex, other.ttlIndex);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.immediateSendTc, other.immediateSendTc);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoOlsrInterface [up=");
    builder.append(this.up);
    builder.append(", ipv4Address=");
    builder.append(this.ipv4Address);
    builder.append(", ipv4Netmask=");
    builder.append(this.ipv4Netmask);
    builder.append(", ipv4Broadcast=");
    builder.append(this.ipv4Broadcast);
    builder.append(", mode=");
    builder.append(this.mode);
    builder.append(", ipv6Address=");
    builder.append(this.ipv6Address);
    builder.append(", ipv6Multicast=");
    builder.append(this.ipv6Multicast);
    builder.append(", ipAddress=");
    builder.append(this.ipAddress);
    builder.append(", emulatedInterface=");
    builder.append(this.emulatedInterface);
    builder.append(", olsrSocket=");
    builder.append(this.olsrSocket);
    builder.append(", sendSocket=");
    builder.append(this.sendSocket);
    builder.append(", metric=");
    builder.append(this.metric);
    builder.append(", mtu=");
    builder.append(this.mtu);
    builder.append(", flags=");
    builder.append(this.flags);
    builder.append(", index=");
    builder.append(this.index);
    builder.append(", wireless=");
    builder.append(this.wireless);
    builder.append(", name=");
    builder.append(this.name);
    builder.append(", seqNum=");
    builder.append(this.seqNum);
    builder.append(", messageTimes=");
    builder.append(this.messageTimes);
    builder.append(", icmpRedirectBackup=");
    builder.append(this.icmpRedirectBackup);
    builder.append(", spoofFilterBackup=");
    builder.append(this.spoofFilterBackup);
    builder.append(", helloEmissionInterval=");
    builder.append(this.helloEmissionInterval);
    builder.append(", validityTimes=");
    builder.append(this.validityTimes);
    builder.append(", forwardingTimeout=");
    builder.append(this.forwardingTimeout);
    builder.append(", sgwZeroBwTimeout=");
    builder.append(this.sgwZeroBwTimeout);
    builder.append(", ttlIndex=");
    builder.append(this.ttlIndex);
    builder.append(", immediateSendTc=");
    builder.append(this.immediateSendTc);
    builder.append("]");
    return builder.toString();
  }
}