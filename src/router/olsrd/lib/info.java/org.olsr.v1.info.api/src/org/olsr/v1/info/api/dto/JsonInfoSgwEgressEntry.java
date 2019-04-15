package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A smart-gateway egress entry in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgwEgressEntry implements Comparable<JsonInfoSgwEgressEntry> {
  private boolean              selected            = false;
  private String               name                = "";
  private int                  ifIndex             = 0;
  private int                  tableNr             = 0;
  private int                  ruleNr              = 0;
  private int                  bypassRuleNr        = 0;
  private boolean              upPrevious          = false;
  private boolean              upCurrent           = false;
  private boolean              upChanged           = false;
  private JsonInfoSgwBandwidth bwPrevious          = new JsonInfoSgwBandwidth();
  private JsonInfoSgwBandwidth bwCurrent           = new JsonInfoSgwBandwidth();
  private boolean              bwCostsChanged      = false;
  private boolean              bwNetworkChanged    = false;
  private boolean              bwGatewayChanged    = false;
  private boolean              bwChanged           = false;
  private JsonInfoSgwRouteInfo networkRouteCurrent = new JsonInfoSgwRouteInfo();
  private JsonInfoSgwRouteInfo egressRouteCurrent  = new JsonInfoSgwRouteInfo();
  private boolean              inEgressFile        = false;

  /**
   * @return the selected status
   */
  public boolean getSelected() {
    return this.selected;
  }

  /**
   * @param selected the selected status to set
   */
  @JsonProperty("selected")
  public void setSelected(final boolean selected) {
    this.selected = selected;
  }

  /**
   * @return the name of the egress interface
   */
  public String getName() {
    return this.name;
  }

  /**
   * @param name the name of the egress interface to set
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
   * @return the table number
   */
  public int getTableNr() {
    return this.tableNr;
  }

  /**
   * @param tableNr the table number to set
   */
  @JsonProperty("tableNr")
  public void setTableNr(final int tableNr) {
    this.tableNr = tableNr;
  }

  /**
   * @return the rule number
   */
  public int getRuleNr() {
    return this.ruleNr;
  }

  /**
   * @param ruleNr the rule number to set
   */
  @JsonProperty("ruleNr")
  public void setRuleNr(final int ruleNr) {
    this.ruleNr = ruleNr;
  }

  /**
   * @return the bypass rule number
   */
  public int getBypassRuleNr() {
    return this.bypassRuleNr;
  }

  /**
   * @param bypassRuleNr the bypass rule number to set
   */
  @JsonProperty("bypassRuleNr")
  public void setBypassRuleNr(final int bypassRuleNr) {
    this.bypassRuleNr = bypassRuleNr;
  }

  /**
   * @return the previous up status
   */
  public boolean getUpPrevious() {
    return this.upPrevious;
  }

  /**
   * @param upPrevious the previous up status to set
   */
  @JsonProperty("upPrevious")
  public void setUpPrevious(final boolean upPrevious) {
    this.upPrevious = upPrevious;
  }

  /**
   * @return the current up status
   */
  public boolean getUpCurrent() {
    return this.upCurrent;
  }

  /**
   * @param upCurrent the current up status to set
   */
  @JsonProperty("upCurrent")
  public void setUpCurrent(final boolean upCurrent) {
    this.upCurrent = upCurrent;
  }

  /**
   * @return the changed status for up
   */
  public boolean getUpChanged() {
    return this.upChanged;
  }

  /**
   * @param upChanged the changed status for up to set
   */
  @JsonProperty("upChanged")
  public void setUpChanged(final boolean upChanged) {
    this.upChanged = upChanged;
  }

  /**
   * @return the previous bandwidth
   */
  public JsonInfoSgwBandwidth getBwPrevious() {
    return this.bwPrevious;
  }

  /**
   * @param bwPrevious the previous bandwidth to set
   */
  @JsonProperty("bwPrevious")
  public void setBwPrevious(final JsonInfoSgwBandwidth bwPrevious) {
    if (bwPrevious == null) {
      this.bwPrevious = new JsonInfoSgwBandwidth();
    } else {
      this.bwPrevious = bwPrevious;
    }
  }

  /**
   * @return the current bandwidth
   */
  public JsonInfoSgwBandwidth getBwCurrent() {
    return this.bwCurrent;
  }

  /**
   * @param bwCurrent the current bandwidth to set
   */
  @JsonProperty("bwCurrent")
  public void setBwCurrent(final JsonInfoSgwBandwidth bwCurrent) {
    if (bwCurrent == null) {
      this.bwCurrent = new JsonInfoSgwBandwidth();
    } else {
      this.bwCurrent = bwCurrent;
    }
  }

  /**
   * @return the bandwidth costs changed status
   */
  public boolean getBwCostsChanged() {
    return this.bwCostsChanged;
  }

  /**
   * @param bwCostsChanged the bandwidth costs changed status to set
   */
  @JsonProperty("bwCostsChanged")
  public void setBwCostsChanged(final boolean bwCostsChanged) {
    this.bwCostsChanged = bwCostsChanged;
  }

  /**
   * @return the bandwidth network changed status
   */
  public boolean getBwNetworkChanged() {
    return this.bwNetworkChanged;
  }

  /**
   * @param bwNetworkChanged the bandwidth network changed status to set
   */
  @JsonProperty("bwNetworkChanged")
  public void setBwNetworkChanged(final boolean bwNetworkChanged) {
    this.bwNetworkChanged = bwNetworkChanged;
  }

  /**
   * @return the bandwidth gateway changed status
   */
  public boolean getBwGatewayChanged() {
    return this.bwGatewayChanged;
  }

  /**
   * @param bwGatewayChanged the bandwidth gateway changed status to set
   */
  @JsonProperty("bwGatewayChanged")
  public void setBwGatewayChanged(final boolean bwGatewayChanged) {
    this.bwGatewayChanged = bwGatewayChanged;
  }

  /**
   * @return the bandwidth changed status
   */
  public boolean getBwChanged() {
    return this.bwChanged;
  }

  /**
   * @param bwChanged the bandwidth changed status to set
   */
  @JsonProperty("bwChanged")
  public void setBwChanged(final boolean bwChanged) {
    this.bwChanged = bwChanged;
  }

  /**
   * @return the current network route
   */
  public JsonInfoSgwRouteInfo getNetworkRouteCurrent() {
    return this.networkRouteCurrent;
  }

  /**
   * @param networkRouteCurrent the current network route to set
   */
  @JsonProperty("networkRouteCurrent")
  public void setNetworkRouteCurrent(final JsonInfoSgwRouteInfo networkRouteCurrent) {
    if (networkRouteCurrent == null) {
      this.networkRouteCurrent = new JsonInfoSgwRouteInfo();
    } else {
      this.networkRouteCurrent = networkRouteCurrent;
    }
  }

  /**
   * @return the current egress route
   */
  public JsonInfoSgwRouteInfo getEgressRouteCurrent() {
    return this.egressRouteCurrent;
  }

  /**
   * @param egressRouteCurrent the current egress route to set
   */
  @JsonProperty("egressRouteCurrent")
  public void setEgressRouteCurrent(final JsonInfoSgwRouteInfo egressRouteCurrent) {
    if (egressRouteCurrent == null) {
      this.egressRouteCurrent = new JsonInfoSgwRouteInfo();
    } else {
      this.egressRouteCurrent = egressRouteCurrent;
    }
  }

  /**
   * @return the interface-is-in-the-egress-file status
   */
  public boolean getInEgressFile() {
    return this.inEgressFile;
  }

  /**
   * @param inEgressFile the interface-is-in-the-egress-file status to set
   */
  @JsonProperty("inEgressFile")
  public void setInEgressFile(final boolean inEgressFile) {
    this.inEgressFile = inEgressFile;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.selected ? 1231 : 1237);
    result = (prime * result) + this.name.hashCode();
    result = (prime * result) + this.ifIndex;
    result = (prime * result) + this.tableNr;
    result = (prime * result) + this.ruleNr;
    result = (prime * result) + this.bypassRuleNr;
    result = (prime * result) + (this.upPrevious ? 1231 : 1237);
    result = (prime * result) + (this.upCurrent ? 1231 : 1237);
    result = (prime * result) + (this.upChanged ? 1231 : 1237);
    result = (prime * result) + this.bwPrevious.hashCode();
    result = (prime * result) + this.bwCurrent.hashCode();
    result = (prime * result) + (this.bwCostsChanged ? 1231 : 1237);
    result = (prime * result) + (this.bwNetworkChanged ? 1231 : 1237);
    result = (prime * result) + (this.bwGatewayChanged ? 1231 : 1237);
    result = (prime * result) + (this.bwChanged ? 1231 : 1237);
    result = (prime * result) + this.networkRouteCurrent.hashCode();
    result = (prime * result) + this.egressRouteCurrent.hashCode();
    result = (prime * result) + (this.inEgressFile ? 1231 : 1237);
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

    return (this.compareTo((JsonInfoSgwEgressEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoSgwEgressEntry other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Boolean.compare(other.selected, this.selected);
    if (result != 0) {
      return result;
    }

    result = this.name.compareTo(other.name);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Long.compare(this.tableNr, other.tableNr);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.ruleNr, other.ruleNr);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.bypassRuleNr, other.bypassRuleNr);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.ifIndex, other.ifIndex);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.upPrevious, other.upPrevious);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.upCurrent, other.upCurrent);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.upChanged, other.upChanged);
    if (result != 0) {
      return result;
    }

    result = this.bwPrevious.compareTo(other.bwPrevious);
    if (result != 0) {
      return result;
    }

    result = this.bwCurrent.compareTo(other.bwCurrent);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.bwCostsChanged, other.bwCostsChanged);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.bwNetworkChanged, other.bwNetworkChanged);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.bwGatewayChanged, other.bwGatewayChanged);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.bwChanged, other.bwChanged);
    if (result != 0) {
      return result;
    }

    result = this.networkRouteCurrent.compareTo(other.networkRouteCurrent);
    if (result != 0) {
      return result;
    }

    result = this.egressRouteCurrent.compareTo(other.egressRouteCurrent);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.inEgressFile, other.inEgressFile);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoSgwEgressEntry [selected=");
    builder.append(this.selected);
    builder.append(", name=");
    builder.append(this.name);
    builder.append(", ifIndex=");
    builder.append(this.ifIndex);
    builder.append(", tableNr=");
    builder.append(this.tableNr);
    builder.append(", ruleNr=");
    builder.append(this.ruleNr);
    builder.append(", bypassRuleNr=");
    builder.append(this.bypassRuleNr);
    builder.append(", upPrevious=");
    builder.append(this.upPrevious);
    builder.append(", upCurrent=");
    builder.append(this.upCurrent);
    builder.append(", upChanged=");
    builder.append(this.upChanged);
    builder.append(", bwPrevious=");
    builder.append(this.bwPrevious);
    builder.append(", bwCurrent=");
    builder.append(this.bwCurrent);
    builder.append(", bwCostsChanged=");
    builder.append(this.bwCostsChanged);
    builder.append(", bwNetworkChanged=");
    builder.append(this.bwNetworkChanged);
    builder.append(", bwGatewayChanged=");
    builder.append(this.bwGatewayChanged);
    builder.append(", bwChanged=");
    builder.append(this.bwChanged);
    builder.append(", networkRouteCurrent=");
    builder.append(this.networkRouteCurrent);
    builder.append(", egressRouteCurrent=");
    builder.append(this.egressRouteCurrent);
    builder.append(", inEgressFile=");
    builder.append(this.inEgressFile);
    builder.append("]");
    return builder.toString();
  }
}