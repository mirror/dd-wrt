package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#TOPOLOGY} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoTopology extends JsonInfoBase {
  private final Set<JsonInfoTopologyEntry> topology = new TreeSet<>();

  /**
   * @return the topology response
   */
  public Set<JsonInfoTopologyEntry> getTopology() {
    return this.topology;
  }

  /**
   * @param topology the topology response to set
   */
  @JsonProperty("topology")
  public void setTopology(final Set<JsonInfoTopologyEntry> topology) {
    this.topology.clear();
    if (topology != null) {
      this.topology.addAll(topology);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.topology.hashCode();
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
    final JsonInfoTopology other = (JsonInfoTopology) obj;

    if (!this.topology.equals(other.topology)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoTopology [topology=");
    builder.append(this.topology);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}