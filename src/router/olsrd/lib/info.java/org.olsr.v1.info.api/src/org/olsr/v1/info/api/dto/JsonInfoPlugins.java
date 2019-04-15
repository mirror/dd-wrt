package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#PLUGINS} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoPlugins extends JsonInfoBase {
  private final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();

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

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.plugins.hashCode();
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
    final JsonInfoPlugins other = (JsonInfoPlugins) obj;

    if (!this.plugins.equals(other.plugins)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPlugins [plugins=");
    builder.append(this.plugins);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}