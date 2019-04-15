package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#MID} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoMid extends JsonInfoBase {
  private final List<JsonInfoMidEntry> mid = new LinkedList<>();

  /**
   * @return the mid response
   */
  public List<JsonInfoMidEntry> getMid() {
    return this.mid;
  }

  /**
   * @param mid the mid response to set
   */
  @JsonProperty("mid")
  public void setMid(final List<JsonInfoMidEntry> mid) {
    this.mid.clear();
    if (mid != null) {
      this.mid.addAll(mid);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.mid.hashCode();
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
    final JsonInfoMid other = (JsonInfoMid) obj;

    if (!this.mid.equals(other.mid)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoMid [mid=");
    builder.append(this.mid);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}