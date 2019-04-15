package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#SMART_GATEWAY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoSgw extends JsonInfoBase {
  private JsonInfoSgwFields sgw = new JsonInfoSgwFields();

  /**
   * @return the smart-gateways response
   */
  public JsonInfoSgwFields getSgw() {
    return this.sgw;
  }

  /**
   * @param sgw the smart-gateways response to set
   */
  @JsonProperty("sgw")
  public void setSgw(final JsonInfoSgwFields sgw) {
    if (sgw == null) {
      this.sgw = new JsonInfoSgwFields();
    } else {
      this.sgw = sgw;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.sgw.hashCode();
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
    final JsonInfoSgw other = (JsonInfoSgw) obj;

    if (!this.sgw.equals(other.sgw)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoSgw [sgw=");
    builder.append(this.sgw);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}