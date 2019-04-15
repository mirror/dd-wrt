package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Hysteresis configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigHysteresis implements Comparable<JsonInfoConfigHysteresis> {
  private boolean enabled       = false;
  private double  scaling       = 0.0;
  private double  thresholdLow  = 0.0;
  private double  thresholdHigh = 0.0;

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
   * @return the scaling
   */
  public double getScaling() {
    return this.scaling;
  }

  /**
   * @param scaling the scaling to set
   */
  @JsonProperty("scaling")
  public void setScaling(final double scaling) {
    this.scaling = scaling;
  }

  /**
   * @return the low threshold
   */
  public double getThresholdLow() {
    return this.thresholdLow;
  }

  /**
   * @param thresholdLow the low threshold to set
   */
  @JsonProperty("thresholdLow")
  public void setThresholdLow(final double thresholdLow) {
    this.thresholdLow = thresholdLow;
  }

  /**
   * @return the high threshold
   */
  public double getThresholdHigh() {
    return this.thresholdHigh;
  }

  /**
   * @param thresholdHigh the high threshold to set
   */
  @JsonProperty("thresholdHigh")
  public void setThresholdHigh(final double thresholdHigh) {
    this.thresholdHigh = thresholdHigh;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    long temp;
    result = (prime * result) + (this.enabled ? 1231 : 1237);
    temp = Double.doubleToLongBits(this.scaling);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.thresholdLow);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.thresholdHigh);
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

    return (this.compareTo((JsonInfoConfigHysteresis) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigHysteresis other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Boolean.compare(other.enabled, this.enabled);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.scaling, other.scaling);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.thresholdLow, other.thresholdLow);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.thresholdHigh, other.thresholdHigh);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigHysteresis [enabled=");
    builder.append(this.enabled);
    builder.append(", scaling=");
    builder.append(this.scaling);
    builder.append(", thresholdLow=");
    builder.append(this.thresholdLow);
    builder.append(", thresholdHigh=");
    builder.append(this.thresholdHigh);
    builder.append("]");
    return builder.toString();
  }
}