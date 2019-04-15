package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoInterfaces extends JsonInfoBase {
  private final List<JsonInfoInterfacesEntry> interfaces = new LinkedList<>();

  /**
   * @return the interfaces response
   */
  public List<JsonInfoInterfacesEntry> getInterfaces() {
    return this.interfaces;
  }

  /**
   * @param interfaces the interfaces response to set
   */
  @JsonProperty("interfaces")
  public void setInterfaces(final List<JsonInfoInterfacesEntry> interfaces) {
    this.interfaces.clear();
    if (interfaces != null) {
      this.interfaces.addAll(interfaces);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.interfaces.hashCode();
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
    final JsonInfoInterfaces other = (JsonInfoInterfaces) obj;

    if (!this.interfaces.equals(other.interfaces)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoInterfaces [interfaces=");
    builder.append(this.interfaces);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}