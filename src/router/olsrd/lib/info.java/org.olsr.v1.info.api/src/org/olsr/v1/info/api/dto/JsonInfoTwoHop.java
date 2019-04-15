package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#TWOHOP} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoTwoHop extends JsonInfoBase {
  private final List<JsonInfoTwoHopEntry> twohop = new LinkedList<>();

  /**
   * @return the 2-hop response
   */
  public List<JsonInfoTwoHopEntry> getTwoHop() {
    return this.twohop;
  }

  /**
   * @param twohop the 2-hop response to set
   */
  @JsonProperty("2hop")
  public void setTwoHop(final List<JsonInfoTwoHopEntry> twohop) {
    this.twohop.clear();
    if (twohop != null) {
      this.twohop.addAll(twohop);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.twohop.hashCode();
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
    final JsonInfoTwoHop other = (JsonInfoTwoHop) obj;

    if (!this.twohop.equals(other.twohop)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoTwoHop [twohop=");
    builder.append(this.twohop);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}