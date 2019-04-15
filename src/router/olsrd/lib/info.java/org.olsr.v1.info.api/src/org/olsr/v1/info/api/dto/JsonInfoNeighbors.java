package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#NEIGHBORS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoNeighbors extends JsonInfoBase {
  private final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();

  /**
   * @return the neighbors response
   */
  public Set<JsonInfoNeighborsEntry> getNeighbors() {
    return this.neighbors;
  }

  /**
   * @param neighbors the neighbors response to set
   */
  @JsonProperty("neighbors")
  public void setNeighbors(final Set<JsonInfoNeighborsEntry> neighbors) {
    this.neighbors.clear();
    if (neighbors != null) {
      this.neighbors.addAll(neighbors);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.neighbors.hashCode();
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
    final JsonInfoNeighbors other = (JsonInfoNeighbors) obj;

    if (!this.neighbors.equals(other.neighbors)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoNeighbors [neighbors=");
    builder.append(this.neighbors);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}