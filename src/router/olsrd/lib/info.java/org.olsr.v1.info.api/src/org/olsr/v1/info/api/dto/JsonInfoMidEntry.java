package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A mid entry in the {@link InfoCommand#MID} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoMidEntry {
  private JsonInfoTimedIpAddress            main    = new JsonInfoTimedIpAddress();
  private final Set<JsonInfoTimedIpAddress> aliases = new TreeSet<>();

  /**
   * @return the main IP address and its validity time
   */
  public JsonInfoTimedIpAddress getMain() {
    return this.main;
  }

  /**
   * @param main the main IP address and its validity time to set
   */
  @JsonProperty("main")
  public void setMain(final JsonInfoTimedIpAddress main) {
    if (main == null) {
      this.main = new JsonInfoTimedIpAddress();
    } else {
      this.main = main;
    }
  }

  /**
   * @return the aliases (IP addresses and their validity times) of main
   */
  public Set<JsonInfoTimedIpAddress> getAliases() {
    return this.aliases;
  }

  /**
   * @param aliases the aliases (IP addresses and their validity times) of main to set
   */
  @JsonProperty("aliases")
  public void setAliases(final Set<JsonInfoTimedIpAddress> aliases) {
    this.aliases.clear();
    if (aliases != null) {
      this.aliases.addAll(aliases);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.main.hashCode();
    result = (prime * result) + this.aliases.hashCode();
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
    final JsonInfoMidEntry other = (JsonInfoMidEntry) obj;
    if (!this.main.equals(other.main)) {
      return false;
    }
    if (!this.aliases.equals(other.aliases)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoMidEntry [main=");
    builder.append(this.main);
    builder.append(", aliases=");
    builder.append(this.aliases);
    builder.append("]");
    return builder.toString();
  }
}