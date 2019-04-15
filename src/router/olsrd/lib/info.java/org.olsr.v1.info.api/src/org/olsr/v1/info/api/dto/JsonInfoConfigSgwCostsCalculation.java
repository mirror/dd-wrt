package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Smart-gateway costs calculation configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigSgwCostsCalculation implements Comparable<JsonInfoConfigSgwCostsCalculation> {
  private int  exitLinkUp   = 0;
  private int  exitLinkDown = 0;
  private int  etx          = 0;
  private long dividerEtx   = 0;

  /**
   * @return the exitlink-up multiplier
   */
  public int getExitLinkUp() {
    return this.exitLinkUp;
  }

  /**
   * @param exitLinkUp the exitlink-up multiplier to set
   */
  @JsonProperty("exitLinkUp")
  public void setExitLinkUp(final int exitLinkUp) {
    this.exitLinkUp = exitLinkUp;
  }

  /**
   * @return the exitlink-down multiplier
   */
  public int getExitLinkDown() {
    return this.exitLinkDown;
  }

  /**
   * @param exitLinkDown the exitlink-down multiplier to set
   */
  @JsonProperty("exitLinkDown")
  public void setExitLinkDown(final int exitLinkDown) {
    this.exitLinkDown = exitLinkDown;
  }

  /**
   * @return the ETX multiplier
   */
  public int getEtx() {
    return this.etx;
  }

  /**
   * @param etx the ETX multiplier to set
   */
  @JsonProperty("etx")
  public void setEtx(final int etx) {
    this.etx = etx;
  }

  /**
   * @return the ETX divider
   */
  public long getDividerEtx() {
    return this.dividerEtx;
  }

  /**
   * @param dividerEtx the ETX divider to set
   */
  @JsonProperty("dividerEtx")
  public void setDividerEtx(final long dividerEtx) {
    this.dividerEtx = dividerEtx;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.exitLinkUp;
    result = (prime * result) + this.exitLinkDown;
    result = (prime * result) + this.etx;
    result = (prime * result) + (int) (this.dividerEtx ^ (this.dividerEtx >>> 32));
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

    return (this.compareTo((JsonInfoConfigSgwCostsCalculation) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigSgwCostsCalculation other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.exitLinkUp, other.exitLinkUp);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.exitLinkDown, other.exitLinkDown);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.etx, other.etx);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.dividerEtx, other.dividerEtx);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigSgwCostsCalculation [exitLinkUp=");
    builder.append(this.exitLinkUp);
    builder.append(", exitLinkDown=");
    builder.append(this.exitLinkDown);
    builder.append(", etx=");
    builder.append(this.etx);
    builder.append(", dividerEtx=");
    builder.append(this.dividerEtx);
    builder.append("]");
    return builder.toString();
  }
}