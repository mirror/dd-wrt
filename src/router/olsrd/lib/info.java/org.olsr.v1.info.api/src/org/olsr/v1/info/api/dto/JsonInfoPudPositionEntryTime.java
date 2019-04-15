package org.olsr.v1.info.api.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public class JsonInfoPudPositionEntryTime implements Comparable<JsonInfoPudPositionEntryTime> {
  private int hour   = 0;
  private int minute = 0;
  private int second = 0;
  private int hsec   = 0;

  /**
   * @return the hour of the day
   */
  public int getHour() {
    return this.hour;
  }

  /**
   * @param hour the hour of the day to set
   */
  @JsonProperty("hour")
  public void setHour(final int hour) {
    this.hour = hour;
  }

  /**
   * @return the minute of the hour
   */
  public int getMinute() {
    return this.minute;
  }

  /**
   * @param minute the minute of the hour to set
   */
  @JsonProperty("minute")
  public void setMinute(final int minute) {
    this.minute = minute;
  }

  /**
   * @return the second of the minute
   */
  public int getSecond() {
    return this.second;
  }

  /**
   * @param second the second of the minute to set
   */
  @JsonProperty("second")
  public void setSecond(final int second) {
    this.second = second;
  }

  /**
   * @return the 1/100 second of the second
   */
  public int getHsec() {
    return this.hsec;
  }

  /**
   * @param hsec the 1/100 second of the second to set
   */
  @JsonProperty("hsec")
  public void setHsec(final int hsec) {
    this.hsec = hsec;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.hour;
    result = (prime * result) + this.minute;
    result = (prime * result) + this.second;
    result = (prime * result) + this.hsec;
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

    return (this.compareTo((JsonInfoPudPositionEntryTime) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoPudPositionEntryTime other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.hour, other.hour);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.minute, other.minute);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.second, other.second);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.hsec, other.hsec);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPositionEntryTime [hour=");
    builder.append(this.hour);
    builder.append(", minute=");
    builder.append(this.minute);
    builder.append(", second=");
    builder.append(this.second);
    builder.append(", hsec=");
    builder.append(this.hsec);
    builder.append("]");
    return builder.toString();
  }
}