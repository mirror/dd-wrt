package org.olsr.v1.info.api.dto;

import java.util.Map;
import java.util.TreeMap;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A plugin entry in the {@link InfoCommand#PLUGINS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoPluginsEntry {
  private String                    plugin     = "";
  private final Map<String, String> parameters = new TreeMap<>();

  /**
   * @return the plugin library
   */
  public String getPlugin() {
    return this.plugin;
  }

  /**
   * @param plugin the plugin library to set
   */
  @JsonProperty("plugin")
  public void setPlugin(final String plugin) {
    if (plugin == null) {
      this.plugin = "";
    } else {
      this.plugin = plugin;
    }
  }

  /**
   * @return the plugin parameters
   */
  public Map<String, String> getParameters() {
    return this.parameters;
  }

  /**
   * @param parameters the plugin parameters to set
   */
  @JsonProperty("parameters")
  public void setParameters(final Map<String, String> parameters) {
    this.parameters.clear();
    if (parameters != null) {
      this.parameters.putAll(parameters);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.plugin.hashCode();
    result = (prime * result) + this.parameters.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (this.getClass() != obj.getClass()) {
      return false;
    }
    final JsonInfoPluginsEntry other = (JsonInfoPluginsEntry) obj;
    if (!this.plugin.equals(other.plugin)) {
      return false;
    }
    if (!this.parameters.equals(other.parameters)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPluginsEntry [plugin=");
    builder.append(this.plugin);
    builder.append(", parameters=");
    builder.append(this.parameters);
    builder.append("]");
    return builder.toString();
  }
}