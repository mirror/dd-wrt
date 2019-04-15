package org.olsr.v1.info.api.dto;

import java.net.InetAddress;
import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A neighbor entry in the {@link InfoCommand#TWOHOP} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoTwoHopEntry extends JsonInfoNeighborsEntryBase {
  private final List<InetAddress> twoHopNeighbors = new LinkedList<>();

  /**
   * @return the 2-hop neighbors
   */
  public List<InetAddress> getTwoHopNeighbors() {
    return this.twoHopNeighbors;
  }

  /**
   * @param twoHopNeighbors the 2-hop neighbors to set
   */
  @JsonProperty("twoHopNeighbors")
  public void setTwoHopNeighbors(final List<InetAddress> twoHopNeighbors) {
    this.twoHopNeighbors.clear();
    if (twoHopNeighbors != null) {
      this.twoHopNeighbors.addAll(twoHopNeighbors);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.twoHopNeighbors.hashCode();
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
    final JsonInfoTwoHopEntry other = (JsonInfoTwoHopEntry) obj;

    if (!this.twoHopNeighbors.equals(other.twoHopNeighbors)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoNeighborsEntry [twoHopNeighbors=");
    builder.append(this.twoHopNeighbors);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}