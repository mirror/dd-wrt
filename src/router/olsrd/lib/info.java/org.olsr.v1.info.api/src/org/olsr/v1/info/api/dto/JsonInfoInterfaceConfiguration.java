package org.olsr.v1.info.api.dto;

import java.net.InetAddress;
import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * An interface configuration entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoInterfaceConfiguration {
  private String                                   ipv4Broadcast               = "";
  private String                                   ipv6Multicast               = "";
  private String                                   ipv4Source                  = "";
  private String                                   ipv6Source                  = "";
  private int                                      ipv6SourcePrefixLength      = 0;
  private String                                   mode                        = "";
  private int                                      weightValue                 = 0;
  private boolean                                  weightFixed                 = false;
  private JsonInfoMessageParameters                hello                       = new JsonInfoMessageParameters();
  private JsonInfoMessageParameters                tc                          = new JsonInfoMessageParameters();
  private JsonInfoMessageParameters                mid                         = new JsonInfoMessageParameters();
  private JsonInfoMessageParameters                hna                         = new JsonInfoMessageParameters();
  private final Set<JsonInfoLinkQualityMultiplier> linkQualityMultipliers      = new TreeSet<>();
  private int                                      linkQualityMultipliersCount = 0;
  private boolean                                  autoDetectChanges           = false;

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
   * @return the ipv4 source IP address
   */
  public String getIpv4Source() {
    return this.ipv4Source;
  }

  /**
   * @param ipv4Source the ipv4 source IP address to set
   */
  @JsonProperty("ipv4Source")
  public void setIpv4Source(final InetAddress ipv4Source) {
    if (ipv4Source == null) {
      this.ipv4Source = "";
    } else {
      this.ipv4Source = ipv4Source.getHostAddress();
    }
  }

  /**
   * @return the ipv6 source IP address
   */
  public String getIpv6Source() {
    return this.ipv6Source;
  }

  /**
   * @param ipv6Source the ipv6 source IP address to set
   */
  @JsonProperty("ipv6Source")
  public void setIpv6Source(final InetAddress ipv6Source) {
    if (ipv6Source == null) {
      this.ipv6Source = "";
    } else {
      this.ipv6Source = ipv6Source.getHostAddress();
    }
  }

  /**
   * @return the ipv6 source address prefix length
   */
  public int getIpv6SourcePrefixLength() {
    return this.ipv6SourcePrefixLength;
  }

  /**
   * @param ipv6SourcePrefixLength the ipv6 source address prefix length to set
   */
  @JsonProperty("ipv6SourcePrefixLength")
  public void setIpv6SourcePrefixLength(final int ipv6SourcePrefixLength) {
    this.ipv6SourcePrefixLength = ipv6SourcePrefixLength;
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
   * @return the weight value
   */
  public int getWeightValue() {
    return this.weightValue;
  }

  /**
   * @param weightValue the weight value to set
   */
  @JsonProperty("weightValue")
  public void setWeightValue(final int weightValue) {
    this.weightValue = weightValue;
  }

  /**
   * @return the weight fixed setting
   */
  public boolean getWeightFixed() {
    return this.weightFixed;
  }

  /**
   * @param weightFixed the weight fixed setting to set
   */
  @JsonProperty("weightFixed")
  public void setWeightFixed(final boolean weightFixed) {
    this.weightFixed = weightFixed;
  }

  /**
   * @return the hello message parameters
   */
  public JsonInfoMessageParameters getHello() {
    return this.hello;
  }

  /**
   * @param hello the hello message parameters to set
   */
  @JsonProperty("hello")
  public void setHello(final JsonInfoMessageParameters hello) {
    if (hello == null) {
      this.hello = new JsonInfoMessageParameters();
    } else {
      this.hello = hello;
    }
  }

  /**
   * @return the tc message parameters
   */
  public JsonInfoMessageParameters getTc() {
    return this.tc;
  }

  /**
   * @param tc the tc message parameters to set
   */
  @JsonProperty("tc")
  public void setTc(final JsonInfoMessageParameters tc) {
    if (tc == null) {
      this.tc = new JsonInfoMessageParameters();
    } else {
      this.tc = tc;
    }
  }

  /**
   * @return the mid message parameters
   */
  public JsonInfoMessageParameters getMid() {
    return this.mid;
  }

  /**
   * @param mid the mid message parameters to set
   */
  @JsonProperty("mid")
  public void setMid(final JsonInfoMessageParameters mid) {
    if (mid == null) {
      this.mid = new JsonInfoMessageParameters();
    } else {
      this.mid = mid;
    }
  }

  /**
   * @return the hna message parameters
   */
  public JsonInfoMessageParameters getHna() {
    return this.hna;
  }

  /**
   * @param hna the hna message parameters to set
   */
  @JsonProperty("hna")
  public void setHna(final JsonInfoMessageParameters hna) {
    if (hna == null) {
      this.hna = new JsonInfoMessageParameters();
    } else {
      this.hna = hna;
    }
  }

  /**
   * @return the link quality multipliers
   */
  public Set<JsonInfoLinkQualityMultiplier> getLinkQualityMultipliers() {
    return this.linkQualityMultipliers;
  }

  /**
   * @param linkQualityMultipliers the link quality multipliers to set
   */
  @JsonProperty("linkQualityMultipliers")
  public void setLinkQualityMultipliers(final Set<JsonInfoLinkQualityMultiplier> linkQualityMultipliers) {
    this.linkQualityMultipliers.clear();
    if (linkQualityMultipliers != null) {
      this.linkQualityMultipliers.addAll(linkQualityMultipliers);
    }
  }

  /**
   * @return the link quality multipliers count
   */
  public int getLinkQualityMultipliersCount() {
    return this.linkQualityMultipliersCount;
  }

  /**
   * @param linkQualityMultipliersCount the link quality multipliers count to set
   */
  @JsonProperty("linkQualityMultipliersCount")
  public void setLinkQualityMultipliersCount(final int linkQualityMultipliersCount) {
    this.linkQualityMultipliersCount = linkQualityMultipliersCount;
  }

  /**
   * @return the auto detect changes state
   */
  public boolean getAutoDetectChanges() {
    return this.autoDetectChanges;
  }

  /**
   * @param autoDetectChanges the auto detect changes state to set
   */
  @JsonProperty("autoDetectChanges")
  public void setAutoDetectChanges(final boolean autoDetectChanges) {
    this.autoDetectChanges = autoDetectChanges;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.ipv4Broadcast.hashCode();
    result = (prime * result) + this.ipv6Multicast.hashCode();
    result = (prime * result) + this.ipv4Source.hashCode();
    result = (prime * result) + this.ipv6Source.hashCode();
    result = (prime * result) + this.ipv6SourcePrefixLength;
    result = (prime * result) + this.mode.hashCode();
    result = (prime * result) + this.weightValue;
    result = (prime * result) + (this.weightFixed ? 1231 : 1237);
    result = (prime * result) + this.hello.hashCode();
    result = (prime * result) + this.tc.hashCode();
    result = (prime * result) + this.mid.hashCode();
    result = (prime * result) + this.hna.hashCode();
    result = (prime * result) + this.linkQualityMultipliers.hashCode();
    result = (prime * result) + this.linkQualityMultipliersCount;
    result = (prime * result) + (this.autoDetectChanges ? 1231 : 1237);
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
    final JsonInfoInterfaceConfiguration other = (JsonInfoInterfaceConfiguration) obj;

    if (!this.ipv4Broadcast.equals(other.ipv4Broadcast)) {
      return false;
    }

    if (!this.ipv6Multicast.equals(other.ipv6Multicast)) {
      return false;
    }

    if (!this.ipv4Source.equals(other.ipv4Source)) {
      return false;
    }

    if (!this.ipv6Source.equals(other.ipv6Source)) {
      return false;
    }

    if (this.ipv6SourcePrefixLength != other.ipv6SourcePrefixLength) {
      return false;
    }

    if (!this.mode.equals(other.mode)) {
      return false;
    }

    if (this.weightValue != other.weightValue) {
      return false;
    }

    if (this.weightFixed != other.weightFixed) {
      return false;
    }

    if (!this.hello.equals(other.hello)) {
      return false;
    }

    if (!this.tc.equals(other.tc)) {
      return false;
    }

    if (!this.mid.equals(other.mid)) {
      return false;
    }

    if (!this.hna.equals(other.hna)) {
      return false;
    }

    if (!this.linkQualityMultipliers.equals(other.linkQualityMultipliers)) {
      return false;
    }

    if (this.linkQualityMultipliersCount != other.linkQualityMultipliersCount) {
      return false;
    }

    if (this.autoDetectChanges != other.autoDetectChanges) {
      return false;
    }

    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoInterfaceConfiguration [ipv4Broadcast=");
    builder.append(this.ipv4Broadcast);
    builder.append(", ipv6Multicast=");
    builder.append(this.ipv6Multicast);
    builder.append(", ipv4Source=");
    builder.append(this.ipv4Source);
    builder.append(", ipv6Source=");
    builder.append(this.ipv6Source);
    builder.append(", ipv6SourcePrefixLength=");
    builder.append(this.ipv6SourcePrefixLength);
    builder.append(", mode=");
    builder.append(this.mode);
    builder.append(", weightValue=");
    builder.append(this.weightValue);
    builder.append(", weightFixed=");
    builder.append(this.weightFixed);
    builder.append(", hello=");
    builder.append(this.hello);
    builder.append(", tc=");
    builder.append(this.tc);
    builder.append(", mid=");
    builder.append(this.mid);
    builder.append(", hna=");
    builder.append(this.hna);
    builder.append(", linkQualityMultipliers=");
    builder.append(this.linkQualityMultipliers);
    builder.append(", linkQualityMultipliersCount=");
    builder.append(this.linkQualityMultipliersCount);
    builder.append(", autoDetectChanges=");
    builder.append(this.autoDetectChanges);
    builder.append("]");
    return builder.toString();
  }
}