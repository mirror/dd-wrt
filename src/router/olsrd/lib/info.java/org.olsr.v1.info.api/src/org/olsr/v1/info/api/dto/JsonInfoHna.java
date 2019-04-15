package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#HNA} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoHna extends JsonInfoBase {
  private final Set<JsonInfoHnaEntry> hna = new TreeSet<>();

  /**
   * @return the HNA response
   */
  public Set<JsonInfoHnaEntry> getHna() {
    return this.hna;
  }

  /**
   * @param hna the HNA response to set
   */
  @JsonProperty("hna")
  public void setHna(final Set<JsonInfoHnaEntry> hna) {
    this.hna.clear();
    if (hna != null) {
      this.hna.addAll(hna);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.hna.hashCode();
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
    final JsonInfoHna other = (JsonInfoHna) obj;

    if (!this.hna.equals(other.hna)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoHna [hna=");
    builder.append(this.hna);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}