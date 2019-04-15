
package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A link quality multiplier entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoLinkQualityMultiplier implements Comparable<JsonInfoLinkQualityMultiplier> {
  private String route      = "";
  private double multiplier = 0.0;

  /**
   * @return the route IP address
   */
  public String getRoute() {
    return this.route;
  }

  /**
   * @param route the route IP address to set
   */
  @JsonProperty("route")
  public void setRoute(final InetAddress route) {
    if (route == null) {
      this.route = "";
    } else {
      this.route = route.getHostAddress();
    }
  }

  /**
   * @return the validity time
   */
  public double getMultiplier() {
    return this.multiplier;
  }

  /**
   * @param multiplier the validity time to set
   */
  @JsonProperty("multiplier")
  public void setMultiplier(final double multiplier) {
    this.multiplier = multiplier;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.route.hashCode();
    final long temp = Double.doubleToLongBits(this.multiplier);
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

    return (this.compareTo((JsonInfoLinkQualityMultiplier) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoLinkQualityMultiplier other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.route.compareTo(other.route);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = Double.compare(this.multiplier, other.multiplier);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoLinkQualityMultiplier [route=");
    builder.append(this.route);
    builder.append(", multiplier=");
    builder.append(this.multiplier);
    builder.append("]");
    return builder.toString();
  }
}