package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.Willingness;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Willingness configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigWillingness implements Comparable<JsonInfoConfigWillingness> {
  private Willingness willingness    = Willingness.UNKNOWN;
  private boolean     auto           = false;
  private double      updateInterval = 0.0;

  /**
   * @return the willingness
   */
  public Willingness getWillingness() {
    return this.willingness;
  }

  /**
   * @param willingness the willingness to set
   */
  @JsonProperty("willingness")
  public void setWillingness(final int willingness) {
    this.willingness = Willingness.fromValue(willingness);
  }

  /**
   * @return the willingness auto mode
   */
  public boolean getAuto() {
    return this.auto;
  }

  /**
   * @param auto the willingness auto mode to set
   */
  @JsonProperty("auto")
  public void setAuto(final boolean auto) {
    this.auto = auto;
  }

  /**
   * @return the update interval
   */
  public double getUpdateInterval() {
    return this.updateInterval;
  }

  /**
   * @param updateInterval the update interval to set
   */
  @JsonProperty("updateInterval")
  public void setUpdateInterval(final double updateInterval) {
    this.updateInterval = updateInterval;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.willingness.getValue();
    result = (prime * result) + (this.auto ? 1231 : 1237);
    final long temp = Double.doubleToLongBits(this.updateInterval);
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

    return (this.compareTo((JsonInfoConfigWillingness) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigWillingness other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.willingness.getValue(), other.willingness.getValue());
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.updateInterval, other.updateInterval);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(other.auto, this.auto);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigWillingness [willingness=");
    builder.append(this.willingness);
    builder.append(", auto=");
    builder.append(this.auto);
    builder.append(", updateInterval=");
    builder.append(this.updateInterval);
    builder.append("]");
    return builder.toString();
  }
}