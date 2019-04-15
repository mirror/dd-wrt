package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The {@link InfoCommand#ALL} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoStartup extends JsonInfoBase {
  private JsonInfoVersionEntry             version = new JsonInfoVersionEntry();
  private final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
  private JsonInfoConfigEntry              config  = new JsonInfoConfigEntry();

  /**
   * @return the version response
   */
  public JsonInfoVersionEntry getVersion() {
    return this.version;
  }

  /**
   * @param version the version response to set
   */
  @JsonProperty("version")
  public void setVersion(final JsonInfoVersionEntry version) {
    if (version == null) {
      this.version = new JsonInfoVersionEntry();
    } else {
      this.version = version;
    }
  }

  /**
   * @return the plugins response
   */
  public List<JsonInfoPluginsEntry> getPlugins() {
    return this.plugins;
  }

  /**
   * @param plugins the plugins response to set
   */
  @JsonProperty("plugins")
  public void setPlugins(final List<JsonInfoPluginsEntry> plugins) {
    this.plugins.clear();
    if (plugins != null) {
      this.plugins.addAll(plugins);
    }
  }

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
    result = (prime * result) + this.version.hashCode();
    result = (prime * result) + this.plugins.hashCode();
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
    final JsonInfoStartup other = (JsonInfoStartup) obj;

    if (!this.version.equals(other.version)) {
      return false;
    }
    if (!this.plugins.equals(other.plugins)) {
      return false;
    }
    if (!this.config.equals(other.config)) {
      return false;
    }

    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoAll [version=");
    builder.append(this.version);
    builder.append(", plugins=");
    builder.append(this.plugins);
    builder.append(", config=");
    builder.append(this.config);
    builder.append(",");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}