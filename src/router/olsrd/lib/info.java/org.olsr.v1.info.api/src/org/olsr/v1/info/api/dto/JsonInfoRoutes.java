package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#ROUTES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoRoutes extends JsonInfoBase {
  private final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();

  /**
   * @return the routes response
   */
  public Set<JsonInfoRoutesEntry> getRoutes() {
    return this.routes;
  }

  /**
   * @param routes the routes response to set
   */
  @JsonProperty("routes")
  public void setRoutes(final Set<JsonInfoRoutesEntry> routes) {
    this.routes.clear();
    if (routes != null) {
      this.routes.addAll(routes);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.routes.hashCode();
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
    final JsonInfoRoutes other = (JsonInfoRoutes) obj;

    if (!this.routes.equals(other.routes)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoRoutes [routes=");
    builder.append(this.routes);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}