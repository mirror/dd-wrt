package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.util.CompareUtils;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The base class of every jsoninfo OLSRd plugin response: every response contains these fields.
 */
@ProviderType
public class JsonInfoBase {
  private long   pid                   = 0;
  private long   systemTime            = 0;
  private long   timeSinceStartup      = 0;
  private String configurationChecksum = "";
  private String uuid                  = ""; /* optional */
  private String error                 = ""; /* optional */

  /**
   * @return the PID
   */
  public long getPid() {
    return this.pid;
  }

  /**
   * @param pid the PID to set
   */
  @JsonProperty("pid")
  public void setPid(final long pid) {
    this.pid = pid;
  }

  /**
   * @return the number of seconds since Epoch
   */
  public long getSystemTime() {
    return this.systemTime;
  }

  /**
   * @param systemTime the number of seconds since Epoch to set
   */
  @JsonProperty("systemTime")
  public void setSystemTime(final long systemTime) {
    this.systemTime = systemTime;
  }

  /**
   * @return the number of milliseconds since startup of OLSRd
   */
  public long getTimeSinceStartup() {
    return this.timeSinceStartup;
  }

  /**
   * @param timeSinceStartup the number of milliseconds since startup of OLSRd to set
   */
  @JsonProperty("timeSinceStartup")
  public void setTimeSinceStartup(final long timeSinceStartup) {
    this.timeSinceStartup = timeSinceStartup;
  }

  /**
   * @return the configuration checksum
   */
  public String getConfigurationChecksum() {
    return this.configurationChecksum;
  }

  /**
   * @param configurationChecksum the configuration checksum to set
   */
  @JsonProperty("configurationChecksum")
  public void setConfigurationChecksum(final String configurationChecksum) {
    if (configurationChecksum == null) {
      this.configurationChecksum = "";
    } else {
      this.configurationChecksum = configurationChecksum;
    }
  }

  /**
   * @return the UUID
   */
  public String getUuid() {
    return this.uuid;
  }

  /**
   * @param uuid the UUID to set
   */
  @JsonProperty("uuid")
  public void setUuid(final String uuid) {
    if (uuid == null) {
      this.uuid = "";
    } else {
      this.uuid = uuid;
    }
  }

  /**
   * @return the error (optional field)
   */
  public String getError() {
    return this.error;
  }

  /**
   * @param error the error to set
   */
  @JsonProperty("error")
  public void setError(final String error) {
    if (error == null) {
      this.error = "";
    } else {
      this.error = error;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (int) (this.pid ^ (this.pid >>> 32));
    result = (prime * result) + (int) (this.systemTime ^ (this.systemTime >>> 32));
    result = (prime * result) + (int) (this.timeSinceStartup ^ (this.timeSinceStartup >>> 32));
    result = (prime * result) + this.configurationChecksum.hashCode();
    result = (prime * result) + this.uuid.hashCode();
    result = (prime * result) + this.error.hashCode();
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

    return (this.compareTo((JsonInfoBase) other) == 0);
  }

  public int compareTo(final JsonInfoBase other) {
    if (other == null) {
      return -1;
    }

    int result = Long.compare(this.pid, other.pid);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.systemTime, other.systemTime);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.timeSinceStartup, other.timeSinceStartup);
    if (result != 0) {
      return result;
    }

    result = this.configurationChecksum.compareTo(other.configurationChecksum);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.uuid.compareTo(other.uuid);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    result = this.error.compareTo(other.error);
    if (result != 0) {
      return CompareUtils.clip(result);
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoBase [pid=");
    builder.append(this.pid);
    builder.append(", systemTime=");
    builder.append(this.systemTime);
    builder.append(", timeSinceStartup=");
    builder.append(this.timeSinceStartup);
    builder.append(", configurationChecksum=");
    builder.append(this.configurationChecksum);
    builder.append(", uuid=");
    builder.append(this.uuid);
    builder.append(", error=");
    builder.append(this.error);
    builder.append("]");
    return builder.toString();
  }
}