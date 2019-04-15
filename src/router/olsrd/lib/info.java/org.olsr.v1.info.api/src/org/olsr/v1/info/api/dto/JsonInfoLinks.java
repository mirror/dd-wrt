package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#LINKS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoLinks extends JsonInfoBase {
  private final Set<JsonInfoLinksEntry> links = new TreeSet<>();

  /**
   * @return the links response
   */
  public Set<JsonInfoLinksEntry> getLinks() {
    return this.links;
  }

  /**
   * @param links the links response to set
   */
  @JsonProperty("links")
  public void setLinks(final Set<JsonInfoLinksEntry> links) {
    this.links.clear();
    if (links != null) {
      this.links.addAll(links);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.links.hashCode();
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
    final JsonInfoLinks other = (JsonInfoLinks) obj;

    if (!this.links.equals(other.links)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoLinks [links=");
    builder.append(this.links);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}