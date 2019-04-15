package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfig extends JsonInfoBase {
  private JsonInfoConfigEntry config = new JsonInfoConfigEntry();

  /**
   * @return the config response
   */
  public JsonInfoConfigEntry getConfig() {
    return this.config;
  }

  /**
   * @param config the config response to set
   */
  @JsonProperty("config")
  public void setConfig(final JsonInfoConfigEntry config) {
    if (config == null) {
      this.config = new JsonInfoConfigEntry();
    } else {
      this.config = config;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.config.hashCode();
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
    final JsonInfoConfig other = (JsonInfoConfig) obj;

    if (!this.config.equals(other.config)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoCOnfig [config=");
    builder.append(this.config);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}