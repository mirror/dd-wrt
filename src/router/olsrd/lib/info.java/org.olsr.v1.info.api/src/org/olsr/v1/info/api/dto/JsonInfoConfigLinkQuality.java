package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Link quality configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigLinkQuality implements Comparable<JsonInfoConfigLinkQuality> {
  private int     level     = 0;
  private boolean fishEye   = false;
  private double  aging     = 0.0;
  private String  algorithm = "";

  /**
   * @return the level
   */
  public int getLevel() {
    return this.level;
  }

  /**
   * @param level the level to set
   */
  @JsonProperty("level")
  public void setLevel(final int level) {
    this.level = level;
  }

  /**
   * @return the fish-eye mode
   */
  public boolean getFishEye() {
    return this.fishEye;
  }

  /**
   * @param fishEye the fish-eye mode to set
   */
  @JsonProperty("fishEye")
  public void setFishEye(final boolean fishEye) {
    this.fishEye = fishEye;
  }

  /**
   * @return the aging
   */
  public double getAging() {
    return this.aging;
  }

  /**
   * @param aging the aging to set
   */
  @JsonProperty("aging")
  public void setAging(final double aging) {
    this.aging = aging;
  }

  /**
   * @return the algorithm
   */
  public String getAlgorithm() {
    return this.algorithm;
  }

  /**
   * @param algorithm the algorithm to set
   */
  @JsonProperty("algorithm")
  public void setAlgorithm(final String algorithm) {
    if (algorithm == null) {
      this.algorithm = "";
    } else {
      this.algorithm = algorithm;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.level;
    result = (prime * result) + (this.fishEye ? 1231 : 1237);
    final long temp = Double.doubleToLongBits(this.aging);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.algorithm.hashCode();
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

    return (this.compareTo((JsonInfoConfigLinkQuality) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigLinkQuality other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.algorithm.compareTo(other.algorithm);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.level, other.level);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.aging, other.aging);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.fishEye, other.fishEye);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigLinkQuality [level=");
    builder.append(this.level);
    builder.append(", fishEye=");
    builder.append(this.fishEye);
    builder.append(", aging=");
    builder.append(this.aging);
    builder.append(", algorithm=");
    builder.append(this.algorithm);
    builder.append("]");
    return builder.toString();
  }
}