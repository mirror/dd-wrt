package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Smart-gateway configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigSgw {
  private boolean                           enabled                  = false;
  private boolean                           alwaysRemoveServerTunnel = false;
  private boolean                           allowNAT                 = false;
  private boolean                           uplinkNAT                = false;
  private int                               useCount                 = 0;
  private int                               takeDownPercentage       = 0;
  private String                            instanceId               = "";
  private String                            policyRoutingScript      = "";
  private JsonInfoConfigSgwEgress           egress                   = new JsonInfoConfigSgwEgress();
  private String                            statusFile               = "";
  private long                              tablesOffset             = 0;
  private long                              rulesOffset              = 0;
  private long                              period                   = 0;
  private int                               stableCount              = 0;
  private int                               threshold                = 0;
  private JsonInfoConfigSgwCostsCalculation costsCalculation         = new JsonInfoConfigSgwCostsCalculation();
  private long                              maxCostMaxEtx            = 0;
  private String                            uplink                   = "";
  private JsonInfoConfigSgwBandwidth        bandwidth                = new JsonInfoConfigSgwBandwidth();
  private JsonInfoConfigSgwPrefix           prefix                   = new JsonInfoConfigSgwPrefix();

  /**
   * @return the enabled status
   */
  public boolean getEnabled() {
    return this.enabled;
  }

  /**
   * @param enabled the enabled status to set
   */
  @JsonProperty("enabled")
  public void setEnabled(final boolean enabled) {
    this.enabled = enabled;
  }

  /**
   * @return the always remove server tunnel status
   */
  public boolean getAlwaysRemoveServerTunnel() {
    return this.alwaysRemoveServerTunnel;
  }

  /**
   * @param alwaysRemoveServerTunnel the always remove server tunnel status to set
   */
  @JsonProperty("alwaysRemoveServerTunnel")
  public void setAlwaysRemoveServerTunnel(final boolean alwaysRemoveServerTunnel) {
    this.alwaysRemoveServerTunnel = alwaysRemoveServerTunnel;
  }

  /**
   * @return the allow NAT status
   */
  public boolean getAllowNAT() {
    return this.allowNAT;
  }

  /**
   * @param allowNAT the allow NAT status to set
   */
  @JsonProperty("allowNAT")
  public void setAllowNAT(final boolean allowNAT) {
    this.allowNAT = allowNAT;
  }

  /**
   * @return the uplink NAT status
   */
  public boolean getUplinkNAT() {
    return this.uplinkNAT;
  }

  /**
   * @param uplinkNAT the uplink NAT status to set
   */
  @JsonProperty("uplinkNAT")
  public void setUplinkNAT(final boolean uplinkNAT) {
    this.uplinkNAT = uplinkNAT;
  }

  /**
   * @return the use count
   */
  public int getUseCount() {
    return this.useCount;
  }

  /**
   * @param useCount the use count to set
   */
  @JsonProperty("useCount")
  public void setUseCount(final int useCount) {
    this.useCount = useCount;
  }

  /**
   * @return the take-down percentage
   */
  public int getTakeDownPercentage() {
    return this.takeDownPercentage;
  }

  /**
   * @param takeDownPercentage the take-down percentage to set
   */
  @JsonProperty("takeDownPercentage")
  public void setTakeDownPercentage(final int takeDownPercentage) {
    this.takeDownPercentage = takeDownPercentage;
  }

  /**
   * @return the instance Id
   */
  public String getInstanceId() {
    return this.instanceId;
  }

  /**
   * @param instanceId the instance Id to set
   */
  @JsonProperty("instanceId")
  public void setInstanceId(final String instanceId) {
    if (instanceId == null) {
      this.instanceId = "";
    } else {
      this.instanceId = instanceId;
    }
  }

  /**
   * @return the policy routing script
   */
  public String getPolicyRoutingScript() {
    return this.policyRoutingScript;
  }

  /**
   * @param policyRoutingScript the policy routing script to set
   */
  @JsonProperty("policyRoutingScript")
  public void setPolicyRoutingScript(final String policyRoutingScript) {
    if (policyRoutingScript == null) {
      this.policyRoutingScript = "";
    } else {
      this.policyRoutingScript = policyRoutingScript;
    }
  }

  /**
   * @return the egress configuration
   */
  public JsonInfoConfigSgwEgress getEgress() {
    return this.egress;
  }

  /**
   * @param egress the egress configuration to set
   */
  @JsonProperty("egress")
  public void setEgress(final JsonInfoConfigSgwEgress egress) {
    if (egress == null) {
      this.egress = new JsonInfoConfigSgwEgress();
    } else {
      this.egress = egress;
    }
  }

  /**
   * @return the status file
   */
  public String getStatusFile() {
    return this.statusFile;
  }

  /**
   * @param statusFile the status file to set
   */
  @JsonProperty("statusFile")
  public void setStatusFile(final String statusFile) {
    if (statusFile == null) {
      this.statusFile = "";
    } else {
      this.statusFile = statusFile;
    }
  }

  /**
   * @return the tables offset
   */
  public long getTablesOffset() {
    return this.tablesOffset;
  }

  /**
   * @param tablesOffset the tables offset to set
   */
  @JsonProperty("tablesOffset")
  public void setTablesOffset(final long tablesOffset) {
    this.tablesOffset = tablesOffset;
  }

  /**
   * @return the rules offset
   */
  public long getRulesOffset() {
    return this.rulesOffset;
  }

  /**
   * @param rulesOffset the rules offset to set
   */
  @JsonProperty("rulesOffset")
  public void setRulesOffset(final long rulesOffset) {
    this.rulesOffset = rulesOffset;
  }

  /**
   * @return the period
   */
  public long getPeriod() {
    return this.period;
  }

  /**
   * @param period the period to set
   */
  @JsonProperty("period")
  public void setPeriod(final long period) {
    this.period = period;
  }

  /**
   * @return the stable count
   */
  public int getStableCount() {
    return this.stableCount;
  }

  /**
   * @param stableCount the stable count to set
   */
  @JsonProperty("stableCount")
  public void setStableCount(final int stableCount) {
    this.stableCount = stableCount;
  }

  /**
   * @return the threshold
   */
  public int getThreshold() {
    return this.threshold;
  }

  /**
   * @param threshold the threshold to set
   */
  @JsonProperty("threshold")
  public void setThreshold(final int threshold) {
    this.threshold = threshold;
  }

  /**
   * @return the costs calculation configuration
   */
  public JsonInfoConfigSgwCostsCalculation getCostsCalculation() {
    return this.costsCalculation;
  }

  /**
   * @param costsCalculation the costs calculation configuration to set
   */
  @JsonProperty("costsCalculation")
  public void setCostsCalculation(final JsonInfoConfigSgwCostsCalculation costsCalculation) {
    if (costsCalculation == null) {
      this.costsCalculation = new JsonInfoConfigSgwCostsCalculation();
    } else {
      this.costsCalculation = costsCalculation;
    }
  }

  /**
   * @return the maximum-cost-maximum-ETX setting
   */
  public long getMaxCostMaxEtx() {
    return this.maxCostMaxEtx;
  }

  /**
   * @param maxCostMaxEtx the maximum-cost-maximum-ETX setting to set
   */
  @JsonProperty("maxCostMaxEtx")
  public void setMaxCostMaxEtx(final long maxCostMaxEtx) {
    this.maxCostMaxEtx = maxCostMaxEtx;
  }

  /**
   * @return the uplink mode
   */
  public String getUplink() {
    return this.uplink;
  }

  /**
   * @param uplink the uplink mode to set
   */
  @JsonProperty("uplink")
  public void setUplink(final String uplink) {
    if (uplink == null) {
      this.uplink = "";
    } else {
      this.uplink = uplink;
    }
  }

  /**
   * @return the bandwidth
   */
  public JsonInfoConfigSgwBandwidth getBandwidth() {
    return this.bandwidth;
  }

  /**
   * @param bandwidth the bandwidth to set
   */
  @JsonProperty("bandwidth")
  public void setBandwidth(final JsonInfoConfigSgwBandwidth bandwidth) {
    if (bandwidth == null) {
      this.bandwidth = new JsonInfoConfigSgwBandwidth();
    } else {
      this.bandwidth = bandwidth;
    }
  }

  /**
   * @return the prefix
   */
  public JsonInfoConfigSgwPrefix getPrefix() {
    return this.prefix;
  }

  /**
   * @param prefix the prefix to set
   */
  @JsonProperty("prefix")
  public void setPrefix(final JsonInfoConfigSgwPrefix prefix) {
    if (prefix == null) {
      this.prefix = new JsonInfoConfigSgwPrefix();
    } else {
      this.prefix = prefix;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.enabled ? 1231 : 1237);
    result = (prime * result) + (this.alwaysRemoveServerTunnel ? 1231 : 1237);
    result = (prime * result) + (this.allowNAT ? 1231 : 1237);
    result = (prime * result) + (this.uplinkNAT ? 1231 : 1237);
    result = (prime * result) + this.useCount;
    result = (prime * result) + this.takeDownPercentage;
    result = (prime * result) + this.instanceId.hashCode();
    result = (prime * result) + this.policyRoutingScript.hashCode();
    result = (prime * result) + this.egress.hashCode();
    result = (prime * result) + this.statusFile.hashCode();
    result = (prime * result) + (int) (this.tablesOffset ^ (this.tablesOffset >>> 32));
    result = (prime * result) + (int) (this.rulesOffset ^ (this.rulesOffset >>> 32));
    result = (prime * result) + (int) (this.period ^ (this.period >>> 32));
    result = (prime * result) + this.stableCount;
    result = (prime * result) + this.threshold;
    result = (prime * result) + this.costsCalculation.hashCode();
    result = (prime * result) + (int) (this.maxCostMaxEtx ^ (this.maxCostMaxEtx >>> 32));
    result = (prime * result) + this.uplink.hashCode();
    result = (prime * result) + this.bandwidth.hashCode();
    result = (prime * result) + this.prefix.hashCode();
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
    final JsonInfoConfigSgw other = (JsonInfoConfigSgw) obj;
    if (this.enabled != other.enabled) {
      return false;
    }
    if (this.alwaysRemoveServerTunnel != other.alwaysRemoveServerTunnel) {
      return false;
    }
    if (this.allowNAT != other.allowNAT) {
      return false;
    }
    if (this.uplinkNAT != other.uplinkNAT) {
      return false;
    }
    if (this.useCount != other.useCount) {
      return false;
    }
    if (this.takeDownPercentage != other.takeDownPercentage) {
      return false;
    }
    if (!this.instanceId.equals(other.instanceId)) {
      return false;
    }
    if (!this.policyRoutingScript.equals(other.policyRoutingScript)) {
      return false;
    }
    if (!this.egress.equals(other.egress)) {
      return false;
    }
    if (!this.statusFile.equals(other.statusFile)) {
      return false;
    }
    if (this.tablesOffset != other.tablesOffset) {
      return false;
    }
    if (this.rulesOffset != other.rulesOffset) {
      return false;
    }
    if (this.period != other.period) {
      return false;
    }
    if (this.stableCount != other.stableCount) {
      return false;
    }
    if (this.threshold != other.threshold) {
      return false;
    }
    if (!this.costsCalculation.equals(other.costsCalculation)) {
      return false;
    }
    if (this.maxCostMaxEtx != other.maxCostMaxEtx) {
      return false;
    }
    if (!this.uplink.equals(other.uplink)) {
      return false;
    }
    if (!this.bandwidth.equals(other.bandwidth)) {
      return false;
    }
    if (!this.prefix.equals(other.prefix)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigSgw [enabled=");
    builder.append(this.enabled);
    builder.append(", alwaysRemoveServerTunnel=");
    builder.append(this.alwaysRemoveServerTunnel);
    builder.append(", allowNAT=");
    builder.append(this.allowNAT);
    builder.append(", uplinkNAT=");
    builder.append(this.uplinkNAT);
    builder.append(", useCount=");
    builder.append(this.useCount);
    builder.append(", takeDownPercentage=");
    builder.append(this.takeDownPercentage);
    builder.append(", instanceId=");
    builder.append(this.instanceId);
    builder.append(", policyRoutingScript=");
    builder.append(this.policyRoutingScript);
    builder.append(", egress=");
    builder.append(this.egress);
    builder.append(", statusFile=");
    builder.append(this.statusFile);
    builder.append(", tablesOffset=");
    builder.append(this.tablesOffset);
    builder.append(", rulesOffset=");
    builder.append(this.rulesOffset);
    builder.append(", period=");
    builder.append(this.period);
    builder.append(", stableCount=");
    builder.append(this.stableCount);
    builder.append(", threshold=");
    builder.append(this.threshold);
    builder.append(", costsCalculation=");
    builder.append(this.costsCalculation);
    builder.append(", maxCostMaxEtx=");
    builder.append(this.maxCostMaxEtx);
    builder.append(", uplink=");
    builder.append(this.uplink);
    builder.append(", bandwidth=");
    builder.append(this.bandwidth);
    builder.append(", prefix=");
    builder.append(this.prefix);
    builder.append("]");
    return builder.toString();
  }
}