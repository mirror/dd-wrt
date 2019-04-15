package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

/**
 * A neighbor entry in the {@link InfoCommand#NEIGHBORS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoNeighborsEntry extends JsonInfoNeighborsEntryBase implements Comparable<JsonInfoNeighborsEntry> {
  @Override
  public int hashCode() {
    return super.hashCode();
  }

  @Override
  public boolean equals(final Object other) {
    return super.equals(other);
  }

  @Override
  public int compareTo(final JsonInfoNeighborsEntry other) {
    return super.compareTo(other);
  }

  @Override
  public String toString() {
    return super.toString();
  }
}