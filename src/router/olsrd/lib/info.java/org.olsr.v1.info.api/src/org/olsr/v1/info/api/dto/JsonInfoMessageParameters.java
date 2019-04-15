
package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A message parameters entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoMessageParameters implements Comparable<JsonInfoMessageParameters> {
  private double emissionInterval = 0.0;
  private double validityTime     = 0.0;

  /**
   * @return the emission interval
   */
  public double getEmissionInterval() {
    return this.emissionInterval;
  }

  /**
   * @param emissionInterval the emission interval to set
   */
  @JsonProperty("emissionInterval")
  public void setEmissionInterval(final double emissionInterval) {
    this.emissionInterval = emissionInterval;
  }

  /**
   * @return the validity time
   */
  public double getValidityTime() {
    return this.validityTime;
  }

  /**
   * @param validityTime the validity time to set
   */
  @JsonProperty("validityTime")
  public void setValidityTime(final double validityTime) {
    this.validityTime = validityTime;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    long temp;
    temp = Double.doubleToLongBits(this.emissionInterval);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.validityTime);
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

    return (this.compareTo((JsonInfoMessageParameters) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoMessageParameters other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Double.compare(this.emissionInterval, other.emissionInterval);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.validityTime, other.validityTime);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoMessageParameters [emissionInterval=");
    builder.append(this.emissionInterval);
    builder.append(", validityTime=");
    builder.append(this.validityTime);
    builder.append("]");
    return builder.toString();
  }
}