package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#PUD_POSITION} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoPudPosition extends JsonInfoBase {
  private JsonInfoPudPositionEntry pudPosition = new JsonInfoPudPositionEntry();

  /**
   * @return the pud position response
   */
  public JsonInfoPudPositionEntry getPudPosition() {
    return this.pudPosition;
  }

  /**
   * @param pudPosition the pud position response to set
   */
  @JsonProperty("pudPosition")
  public void setPudPosition(final JsonInfoPudPositionEntry pudPosition) {
    if (pudPosition == null) {
      this.pudPosition = new JsonInfoPudPositionEntry();
    } else {
      this.pudPosition = pudPosition;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.pudPosition.hashCode();
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
    final JsonInfoPudPosition other = (JsonInfoPudPosition) obj;

    if (!this.pudPosition.equals(other.pudPosition)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPosition [pudPosition=");
    builder.append(this.pudPosition);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}