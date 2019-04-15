package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Routing table configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigRtTable implements Comparable<JsonInfoConfigRtTable> {
  private int main                = 0;
  private int default_            = 0;
  private int tunnel              = 0;
  private int priority            = 0;
  private int tunnelPriority      = 0;
  private int defaultOlsrPriority = 0;
  private int defaultPriority     = 0;

  /**
   * @return the main table
   */
  public int getMain() {
    return this.main;
  }

  /**
   * @param main the main table to set
   */
  @JsonProperty("main")
  public void setMain(final int main) {
    this.main = main;
  }

  /**
   * @return the default table
   */
  public int getDefault() {
    return this.default_;
  }

  /**
   * @param dflt the default table to set
   */
  @JsonProperty("default")
  public void setDefault(final int dflt) {
    this.default_ = dflt;
  }

  /**
   * @return the tunnel table
   */
  public int getTunnel() {
    return this.tunnel;
  }

  /**
   * @param tunnel the tunnel table to set
   */
  @JsonProperty("tunnel")
  public void setTunnel(final int tunnel) {
    this.tunnel = tunnel;
  }

  /**
   * @return the priority table
   */
  public int getPriority() {
    return this.priority;
  }

  /**
   * @param priority the priority table to set
   */
  @JsonProperty("priority")
  public void setPriority(final int priority) {
    this.priority = priority;
  }

  /**
   * @return the tunnel priority table
   */
  public int getTunnelPriority() {
    return this.tunnelPriority;
  }

  /**
   * @param tunnelPriority the tunnel priority table to set
   */
  @JsonProperty("tunnelPriority")
  public void setTunnelPriority(final int tunnelPriority) {
    this.tunnelPriority = tunnelPriority;
  }

  /**
   * @return the default OLSR priority table
   */
  public int getDefaultOlsrPriority() {
    return this.defaultOlsrPriority;
  }

  /**
   * @param defaultOlsrPriority the default OLSR priority table to set
   */
  @JsonProperty("defaultOlsrPriority")
  public void setDefaultOlsrPriority(final int defaultOlsrPriority) {
    this.defaultOlsrPriority = defaultOlsrPriority;
  }

  /**
   * @return the default priority table
   */
  public int getDefaultPriority() {
    return this.defaultPriority;
  }

  /**
   * @param defaultPriority the default priority table to set
   */
  @JsonProperty("defaultPriority")
  public void setDefaultPriority(final int defaultPriority) {
    this.defaultPriority = defaultPriority;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.main;
    result = (prime * result) + this.default_;
    result = (prime * result) + this.tunnel;
    result = (prime * result) + this.priority;
    result = (prime * result) + this.tunnelPriority;
    result = (prime * result) + this.defaultOlsrPriority;
    result = (prime * result) + this.defaultPriority;
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

    return (this.compareTo((JsonInfoConfigRtTable) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoConfigRtTable other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Integer.compare(this.main, other.main);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.tunnel, other.tunnel);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.priority, other.priority);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.tunnelPriority, other.tunnelPriority);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.default_, other.default_);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.defaultOlsrPriority, other.defaultOlsrPriority);
    if (result != 0) {
      return result;
    }

    result = Integer.compare(this.defaultPriority, other.defaultPriority);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigRtTable [main=");
    builder.append(this.main);
    builder.append(", default_=");
    builder.append(this.default_);
    builder.append(", tunnel=");
    builder.append(this.tunnel);
    builder.append(", priority=");
    builder.append(this.priority);
    builder.append(", tunnelPriority=");
    builder.append(this.tunnelPriority);
    builder.append(", defaultOlsrPriority=");
    builder.append(this.defaultOlsrPriority);
    builder.append(", defaultPriority=");
    builder.append(this.defaultPriority);
    builder.append("]");
    return builder.toString();
  }
}