package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * FIB configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigFib implements Comparable<JsonInfoConfigFib> {
  private String metric        = "";
  private String metricDefault = "";

  /**
   * @return the FIB metric
   */
  public String getMetric() {
    return this.metric;
  }

  /**
   * @param metric the FIB metric to set
   */
  @JsonProperty("metric")
  public void setMetric(final String metric) {
    if (metric == null) {
      this.metric = "";
    } else {
      this.metric = metric;
    }
  }

  /**
   * @return the FIB metric default
   */
  public String getMetricDefault() {
    return this.metricDefault;
  }

  /**
   * @param metricDefault the FIB metric default to set
   */
  @JsonProperty("metricDefault")
  public void setMetricDefault(final String metricDefault) {
    if (metricDefault == null) {
      this.metricDefault = "";
    } else {
      this.metricDefault = metricDefault;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.metric.hashCode();
    result = (prime * result) + this.metricDefault.hashCode();
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

    return (this.compareTo((JsonInfoConfigFib) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigFib other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.metric.compareTo(other.metric);
    if (result != 0) {
      return result;
    }

    result = this.metricDefault.compareTo(other.metricDefault);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigFib [metric=");
    builder.append(this.metric);
    builder.append(", metricDefault=");
    builder.append(this.metricDefault);
    builder.append("]");
    return builder.toString();
  }
}