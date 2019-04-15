package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The container of the {@link InfoCommand#VERSION} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoVersion extends JsonInfoBase implements Comparable<JsonInfoVersion> {
  private JsonInfoVersionEntry version = new JsonInfoVersionEntry();

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

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.version.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object other) {
    if (this == other) {
      return true;
    }
    if (!super.equals(other)) {
      return false;
    }
    /* class comparison is already done in super.equals() */

    return (this.compareTo((JsonInfoVersion) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoVersion other) {
    if (other == null) {
      return -1;
    }

    int result = super.compareTo(other);
    if (result != 0) {
      return result;
    }

    result = this.version.compareTo(other.version);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoVersion [version=");
    builder.append(this.version);
    builder.append(", ");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}