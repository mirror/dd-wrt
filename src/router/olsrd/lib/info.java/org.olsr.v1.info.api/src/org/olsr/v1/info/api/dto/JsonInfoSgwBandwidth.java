package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.contants.OlsrdConstants;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A smart-gateway bandwidth entry in the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgwBandwidth implements Comparable<JsonInfoSgwBandwidth> {
  private boolean requireNetwork = true;
  private boolean requireGateway = true;
  private long    egressUk       = 0;
  private long    egressDk       = 0;
  private double  pathCost       = Double.POSITIVE_INFINITY;
  private String  network        = "";
  private int     networkLength  = 0;
  private String  gateway        = "";
  private boolean networkSet     = false;
  private boolean gatewaySet     = false;
  private double  costs          = Double.POSITIVE_INFINITY;

  /**
   * @return the require network status
   */
  public boolean getRequireNetwork() {
    return this.requireNetwork;
  }

  /**
   * @param requireNetwork the require network status to set
   */
  @JsonProperty("requireNetwork")
  public void setRequireNetwork(final boolean requireNetwork) {
    this.requireNetwork = requireNetwork;
  }

  /**
   * @return the require gateway status
   */
  public boolean getRequireGateway() {
    return this.requireGateway;
  }

  /**
   * @param requireGateway the require gateway status to set
   */
  @JsonProperty("requireGateway")
  public void setRequireGateway(final boolean requireGateway) {
    this.requireGateway = requireGateway;
  }

  /**
   * @return the egress uplink bandwidth (kbps)
   */
  public long getEgressUk() {
    return this.egressUk;
  }

  /**
   * @param egressUk the egress uplink bandwidth (kbps) to set
   */
  @JsonProperty("egressUk")
  public void setEgressUk(final long egressUk) {
    this.egressUk = egressUk;
  }

  /**
   * @return the egress downlink bandwidth (kbps)
   */
  public long getEgressDk() {
    return this.egressDk;
  }

  /**
   * @param egressDk the egress downlink bandwidth (kbps) to set
   */
  @JsonProperty("egressDk")
  public void setEgressDk(final long egressDk) {
    this.egressDk = egressDk;
  }

  /**
   * @return the path cost
   */
  public double getPathCost() {
    return this.pathCost;
  }

  /**
   * @param pathCost the path cost to set
   */
  @JsonProperty("pathCost")
  public void setPathCost(final double pathCost) {
    if (Double.compare(pathCost, OlsrdConstants.ROUTE_COST_BROKEN) >= 0) {
      this.pathCost = Double.POSITIVE_INFINITY;
    } else {
      this.pathCost = pathCost;
    }
  }

  /**
   * @return the network IP address
   */
  public String getNetwork() {
    return this.network;
  }

  /**
   * @param network the network IP address to set
   */
  @JsonProperty("network")
  public void setNetwork(final InetAddress network) {
    if (network == null) {
      this.network = "";
    } else {
      this.network = network.getHostAddress();
    }
  }

  /**
   * @return the network prefix length
   */
  public int getNetworkLength() {
    return this.networkLength;
  }

  /**
   * @param networkLength the network prefix length to set
   */
  @JsonProperty("networkLength")
  public void setNetworkLength(final int networkLength) {
    this.networkLength = networkLength;
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
   * @return the network-is-set status
   */
  public boolean getNetworkSet() {
    return this.networkSet;
  }

  /**
   * @param networkSet the network-is-set status to set
   */
  @JsonProperty("networkSet")
  public void setNetworkSet(final boolean networkSet) {
    this.networkSet = networkSet;
  }

  /**
   * @return the gateway-is-set status
   */
  public boolean getGatewaySet() {
    return this.gatewaySet;
  }

  /**
   * @param gatewaySet the gateway-is-set status to set
   */
  @JsonProperty("gatewaySet")
  public void setGatewaySet(final boolean gatewaySet) {
    this.gatewaySet = gatewaySet;
  }

  /**
   * @return the costs
   */
  public double getCosts() {
    return this.costs;
  }

  /**
   * @param costs the costs to set
   */
  @JsonProperty("costs")
  public void setCosts(final double costs) {
    if (Double.compare(costs, Long.MAX_VALUE) >= 0) {
      this.costs = Double.POSITIVE_INFINITY;
    } else {
      this.costs = costs;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (this.requireNetwork ? 1231 : 1237);
    result = (prime * result) + (this.requireGateway ? 1231 : 1237);
    result = (prime * result) + (int) (this.egressUk ^ (this.egressUk >>> 32));
    result = (prime * result) + (int) (this.egressDk ^ (this.egressDk >>> 32));
    long temp = Double.doubleToLongBits(this.pathCost);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.network.hashCode();
    result = (prime * result) + this.networkLength;
    result = (prime * result) + this.gateway.hashCode();
    result = (prime * result) + (this.networkSet ? 1231 : 1237);
    result = (prime * result) + (this.gatewaySet ? 1231 : 1237);
    temp = Double.doubleToLongBits(this.costs);
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

    return (this.compareTo((JsonInfoSgwBandwidth) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoSgwBandwidth other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Double.compare(this.costs, other.costs);
    if (result != 0) {
      return result;
    }

    result = this.gateway.compareTo(other.gateway);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.network.compareTo(other.network);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Integer.compare(this.networkLength, other.networkLength);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.egressUk, other.egressUk);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.egressDk, other.egressDk);
    if (result != 0) {
      return result;
    }

    result = Double.compare(this.pathCost, other.pathCost);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.requireNetwork, other.requireNetwork);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.requireGateway, other.requireGateway);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.networkSet, other.networkSet);
    if (result != 0) {
      return result;
    }

    result = Boolean.compare(this.gatewaySet, other.gatewaySet);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoSgwBandwidth [requireNetwork=");
    builder.append(this.requireNetwork);
    builder.append(", requireGateway=");
    builder.append(this.requireGateway);
    builder.append(", egressUk=");
    builder.append(this.egressUk);
    builder.append(", egressDk=");
    builder.append(this.egressDk);
    builder.append(", pathCost=");
    builder.append(this.pathCost);
    builder.append(", network=");
    builder.append(this.network);
    builder.append(", networkLength=");
    builder.append(this.networkLength);
    builder.append(", gateway=");
    builder.append(this.gateway);
    builder.append(", networkSet=");
    builder.append(this.networkSet);
    builder.append(", gatewaySet=");
    builder.append(this.gatewaySet);
    builder.append(", costs=");
    builder.append(this.costs);
    builder.append("]");
    return builder.toString();
  }
}