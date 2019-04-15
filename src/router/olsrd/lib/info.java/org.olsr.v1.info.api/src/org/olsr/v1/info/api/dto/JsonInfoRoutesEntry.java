package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.OlsrdConstants;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A route entry in the {@link InfoCommand#ROUTES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoRoutesEntry implements Comparable<JsonInfoRoutesEntry> {
  private String destination             = "";
  private int    destinationPrefixLength = 0;
  private String gateway                 = "";
  private long   metric                  = 0;
  private double etx                     = Double.POSITIVE_INFINITY;
  private double rtpMetricCost           = Double.POSITIVE_INFINITY;
  private String networkInterface        = "";

  /**
   * @return the destination IP address
   */
  public String getDestination() {
    return this.destination;
  }

  /**
   * @param destination the destination IP address to set
   */
  @JsonProperty("destination")
  public void setDestination(final InetAddress destination) {
    if (destination == null) {
      this.destination = "";
    } else {
      this.destination = destination.getHostAddress();
    }
  }

  /**
   * @return the destination prefix length
   */
  public int getDestinationPrefixLength() {
    return this.destinationPrefixLength;
  }

  /**
   * @param destinationPrefixLength the destination prefix length to set
   */
  @JsonProperty("genmask")
  public void setDestinationPrefixLength(final int destinationPrefixLength) {
    this.destinationPrefixLength = destinationPrefixLength;
  }

  /**
   * @return the gateway IP address
   */
  public String getGateway() {
    return this.gateway;
  }

  /**
   * @param gateway the gateway IP address to set
   */
  @JsonProperty("gateway")
  public void setGateway(final InetAddress gateway) {
    if (gateway == null) {
      this.gateway = "";
    } else {
      this.gateway = gateway.getHostAddress();
    }
  }

  /**
   * @return the metric
   */
  public long getMetric() {
    return this.metric;
  }

  /**
   * @param metric the metric to set
   */
  @JsonProperty("metric")
  public void setMetric(final long metric) {
    this.metric = metric;
  }

  /**
   * @return the ETX
   */
  public double getEtx() {
    return this.etx;
  }

  /**
   * @param etx the ETX to set
   */
  @JsonProperty("etx")
  public void setEtx(final double etx) {
    if (Double.compare(etx, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.etx = Double.POSITIVE_INFINITY;
    } else {
      this.etx = etx;
    }
  }

  /**
   * @return the rtp metric cost
   */
  public double getRtpMetricCost() {
    return this.rtpMetricCost;
  }

  /**
   * @param rtpMetricCost the rtp metric cost to set
   */
  @JsonProperty("rtpMetricCost")
  public void setRtpMetricCost(final double rtpMetricCost) {
    if (Double.compare(rtpMetricCost, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.rtpMetricCost = Double.POSITIVE_INFINITY;
    } else {
      this.rtpMetricCost = rtpMetricCost;
    }
  }

  /**
   * @return the network interface
   */
  public String getNetworkInterface() {
    return this.networkInterface;
  }

  /**
   * @param networkInterface the network interface to set
   */
  @JsonProperty("networkInterface")
  public void setNetworkInterface(final String networkInterface) {
    if (networkInterface == null) {
      this.networkInterface = "";
    } else {
      this.networkInterface = networkInterface;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.destination.hashCode();
    result = (prime * result) + this.destinationPrefixLength;
    result = (prime * result) + this.gateway.hashCode();
    result = (prime * result) + (int) (this.metric ^ (this.metric >>> 32));
    long temp = Double.doubleToLongBits(this.etx);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.rtpMetricCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.networkInterface.hashCode();
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

    return (this.compareTo((JsonInfoRoutesEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoRoutesEntry other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.gateway.compareTo(other.gateway);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.destination.compareTo(other.destination);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.destinationPrefixLength, other.destinationPrefixLength);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.metric, other.metric);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.etx, other.etx);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.rtpMetricCost, other.rtpMetricCost);
    if (result != 0) {
      return result;
    }

    result = this.networkInterface.compareTo(other.networkInterface);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoRoutesEntry [destination=");
    builder.append(this.destination);
    builder.append(", destinationPrefixLength=");
    builder.append(this.destinationPrefixLength);
    builder.append(", gateway=");
    builder.append(this.gateway);
    builder.append(", metric=");
    builder.append(this.metric);
    builder.append(", etx=");
    builder.append(this.etx);
    builder.append(", rtpMetricCost=");
    builder.append(this.rtpMetricCost);
    builder.append(", networkInterface=");
    builder.append(this.networkInterface);
    builder.append("]");
    return builder.toString();
  }
}