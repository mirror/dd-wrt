package org.olsr.v1.info.api.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public class JsonInfoPudPositionEntrySatellite implements Comparable<JsonInfoPudPositionEntrySatellite> {
  private int id        = 0;
  private int elevation = 0;
  private int azimuth   = 0;
  private int signal    = 0;

  /**
   * @return the id
   */
  public int getId() {
    return this.id;
  }

  /**
   * @param id the id to set
   */
  @JsonProperty("id")
  public void setId(final int id) {
    this.id = id;
  }

  /**
   * @return the elevation
   */
  public int getElevation() {
    return this.elevation;
  }

  /**
   * @param elevation the elevation to set
   */
  @JsonProperty("elevation")
  public void setElevation(final int elevation) {
    this.elevation = elevation;
  }

  /**
   * @return the azimuth
   */
  public int getAzimuth() {
    return this.azimuth;
  }

  /**
   * @param azimuth the azimuth to set
   */
  @JsonProperty("azimuth")
  public void setAzimuth(final int azimuth) {
    this.azimuth = azimuth;
  }

  /**
   * @return the signal (dB)
   */
  public int getSignal() {
    return this.signal;
  }

  /**
   * @param signal the signal (dB) to set
   */
  @JsonProperty("signal")
  public void setSignal(final int signal) {
    this.signal = signal;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.id;
    result = (prime * result) + this.elevation;
    result = (prime * result) + this.azimuth;
    result = (prime * result) + this.signal;
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

    return (this.compareTo((JsonInfoPudPositionEntrySatellite) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoPudPositionEntrySatellite other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.id, other.id);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.elevation, other.elevation);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.azimuth, other.azimuth);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.signal, other.signal);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPositionEntrySatellite [id=");
    builder.append(this.id);
    builder.append(", elevation=");
    builder.append(this.elevation);
    builder.append(", azimuth=");
    builder.append(this.azimuth);
    builder.append(", signal=");
    builder.append(this.signal);
    builder.append("]");
    return builder.toString();
  }
}