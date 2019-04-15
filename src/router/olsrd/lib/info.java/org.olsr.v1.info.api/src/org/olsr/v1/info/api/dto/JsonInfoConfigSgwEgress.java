package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Smart-gateway egress configuration in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigSgwEgress {
  private final Set<String> interfaces      = new TreeSet<>();
  private int               interfacesCount = 0;
  private String            file            = "";
  private long              filePeriod      = 0;

  /**
   * @return the egress interfaces
   */
  public Set<String> getInterfaces() {
    return this.interfaces;
  }

  /**
   * @param interfaces the egress interfaces to set
   */
  @JsonProperty("interfaces")
  public void setInterfaces(final Set<String> interfaces) {
    this.interfaces.clear();
    if (interfaces != null) {
      this.interfaces.addAll(interfaces);
    }
  }

  /**
   * @return the egress interfaces count
   */
  public int getInterfacesCount() {
    return this.interfacesCount;
  }

  /**
   * @param interfacesCount the egress interfaces count to set
   */
  @JsonProperty("interfacesCount")
  public void setInterfacesCount(final int interfacesCount) {
    this.interfacesCount = interfacesCount;
  }

  /**
   * @return the egress configuration file
   */
  public String getFile() {
    return this.file;
  }

  /**
   * @param file the egress configuration file to set
   */
  @JsonProperty("file")
  public void setFile(final String file) {
    if (file == null) {
      this.file = "";
    } else {
      this.file = file;
    }
  }

  /**
   * @return the egress configuration file period
   */
  public long getFilePeriod() {
    return this.filePeriod;
  }

  /**
   * @param filePeriod the egress configuration file period to set
   */
  @JsonProperty("filePeriod")
  public void setFilePeriod(final long filePeriod) {
    this.filePeriod = filePeriod;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.interfaces.hashCode();
    result = (prime * result) + this.interfacesCount;
    result = (prime * result) + this.file.hashCode();
    result = (prime * result) + (int) (this.filePeriod ^ (this.filePeriod >>> 32));
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
    final JsonInfoConfigSgwEgress other = (JsonInfoConfigSgwEgress) obj;
    if (!this.interfaces.equals(other.interfaces)) {
      return false;
    }
    if (this.interfacesCount != other.interfacesCount) {
      return false;
    }
    if (!this.file.equals(other.file)) {
      return false;
    }
    if (this.filePeriod != other.filePeriod) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigSgwEgress [interfaces=");
    builder.append(this.interfaces);
    builder.append(", interfacesCount=");
    builder.append(this.interfacesCount);
    builder.append(", file=");
    builder.append(this.file);
    builder.append(", filePeriod=");
    builder.append(this.filePeriod);
    builder.append("]");
    return builder.toString();
  }
}