package org.olsr.v1.info.api.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public class JsonInfoPudPositionEntryDate implements Comparable<JsonInfoPudPositionEntryDate> {
  private int year  = 0;
  private int month = 0;
  private int day   = 0;

  /**
   * @return the year
   */
  public int getYear() {
    return this.year;
  }

  /**
   * @param year the year to set
   */
  @JsonProperty("year")
  public void setYear(final int year) {
    this.year = year;
  }

  /**
   * @return the month of the year
   */
  public int getMonth() {
    return this.month;
  }

  /**
   * @param month the month of the year to set
   */
  @JsonProperty("month")
  public void setMonth(final int month) {
    this.month = month;
  }

  /**
   * @return the day of the month
   */
  public int getDay() {
    return this.day;
  }

  /**
   * @param day the day of the month to set
   */
  @JsonProperty("day")
  public void setDay(final int day) {
    this.day = day;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.year;
    result = (prime * result) + this.month;
    result = (prime * result) + this.day;
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

    return (this.compareTo((JsonInfoPudPositionEntryDate) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoPudPositionEntryDate other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.year, other.year);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.month, other.month);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.day, other.day);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPositionEntryDate [year=");
    builder.append(this.year);
    builder.append(", month=");
    builder.append(this.month);
    builder.append(", day=");
    builder.append(this.day);
    builder.append("]");
    return builder.toString();
  }
}