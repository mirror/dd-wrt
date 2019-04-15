package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A version entry in the {@link InfoCommand#VERSION} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoVersionEntry implements Comparable<JsonInfoVersionEntry> {
  private String version        = "";
  private String gitDescriptor  = "";
  private String gitSha         = "";
  private String releaseVersion = "";
  private String sourceHash     = "";

  /**
   * @return the full version
   */
  public String getVersion() {
    return this.version;
  }

  /**
   * @param version the full version to set
   */
  @JsonProperty("version")
  public void setVersion(final String version) {
    if (version == null) {
      this.version = "";
    } else {
      this.version = version;
    }
  }

  /**
   * @return the git descriptor
   */
  public String getGitDescriptor() {
    return this.gitDescriptor;
  }

  /**
   * @param gitDescriptor the git descriptor to set
   */
  @JsonProperty("gitDescriptor")
  public void setGitDescriptor(final String gitDescriptor) {
    if (gitDescriptor == null) {
      this.gitDescriptor = "";
    } else {
      this.gitDescriptor = gitDescriptor;
    }
  }

  /**
   * @return the git SHA
   */
  public String getGitSha() {
    return this.gitSha;
  }

  /**
   * @param gitSha the git SHA to set
   */
  @JsonProperty("gitSha")
  public void setGitSha(final String gitSha) {
    if (gitSha == null) {
      this.gitSha = "";
    } else {
      this.gitSha = gitSha;
    }
  }

  /**
   * @return the release-train version
   */
  public String getReleaseVersion() {
    return this.releaseVersion;
  }

  /**
   * @param releaseVersion the release-train version to set
   */
  @JsonProperty("releaseVersion")
  public void setReleaseVersion(final String releaseVersion) {
    if (releaseVersion == null) {
      this.releaseVersion = "";
    } else {
      this.releaseVersion = releaseVersion;
    }
  }

  /**
   * @return the source hash
   */
  public String getSourceHash() {
    return this.sourceHash;
  }

  /**
   * @param sourceHash the source hash to set
   */
  @JsonProperty("sourceHash")
  public void setSourceHash(final String sourceHash) {
    if (sourceHash == null) {
      this.sourceHash = "";
    } else {
      this.sourceHash = sourceHash;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.version.hashCode();
    result = (prime * result) + this.gitDescriptor.hashCode();
    result = (prime * result) + this.gitSha.hashCode();
    result = (prime * result) + this.releaseVersion.hashCode();
    result = (prime * result) + this.sourceHash.hashCode();

    return result;
  }

  @Override
  public boolean equals(final Object other) {
    if (this == other) {
      return true;
    }
    if (other == null) {
      return false;
    }
    if (this.getClass() != other.getClass()) {
      return false;
    }

    return (this.compareTo((JsonInfoVersionEntry) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoVersionEntry other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = this.version.compareTo(other.version);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.gitDescriptor.compareTo(other.gitDescriptor);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.gitSha.compareTo(other.gitSha);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.releaseVersion.compareTo(other.releaseVersion);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.sourceHash.compareTo(other.sourceHash);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoVersionEntry [version=");
    builder.append(this.version);
    builder.append(", gitDescriptor=");
    builder.append(this.gitDescriptor);
    builder.append(", gitSha=");
    builder.append(this.gitSha);
    builder.append(", releaseVersion=");
    builder.append(this.releaseVersion);
    builder.append(", sourceHash=");
    builder.append(this.sourceHash);
    builder.append("]");
    return builder.toString();
  }
}