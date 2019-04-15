package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#GATEWAYS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoGateways extends JsonInfoBase {
  private JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();

  /**
   * @return the gateways response
   */
  public JsonInfoGatewaysFields getGateways() {
    return this.gateways;
  }

  /**
   * @param gateways the gateways response to set
   */
  @JsonProperty("gateways")
  public void setGateways(final JsonInfoGatewaysFields gateways) {
    if (gateways == null) {
      this.gateways = new JsonInfoGatewaysFields();
    } else {
      this.gateways = gateways;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.gateways.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (!super.equals(obj)) {
      return false;
    }
    /* class comparison is already done in super.equals() */
    final JsonInfoGateways other = (JsonInfoGateways) obj;

    if (!this.gateways.equals(other.gateways)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoGateways [gateways=");
    builder.append(this.gateways);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}