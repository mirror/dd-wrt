
package org.olsr.v1.info.api.dto;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A validity times entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoMessageTimes implements Comparable<JsonInfoMessageTimes> {
  private long hello = 0;
  private long tc    = 0;
  private long mid   = 0;
  private long hna   = 0;

  /**
   * @return the hello validity time
   */
  public long getHello() {
    return this.hello;
  }

  /**
   * @param hello the hello validity time to set
   */
  @JsonProperty("hello")
  public void setHello(final long hello) {
    this.hello = hello;
  }

  /**
   * @return the tc validity time
   */
  public long getTc() {
    return this.tc;
  }

  /**
   * @param tc the tc validity time to set
   */
  @JsonProperty("tc")
  public void setTc(final long tc) {
    this.tc = tc;
  }

  /**
   * @return the mid validity time
   */
  public long getMid() {
    return this.mid;
  }

  /**
   * @param mid the mid validity time to set
   */
  @JsonProperty("mid")
  public void setMid(final long mid) {
    this.mid = mid;
  }

  /**
   * @return the hna validity time
   */
  public long getHna() {
    return this.hna;
  }

  /**
   * @param hna the hna validity time to set
   */
  @JsonProperty("hna")
  public void setHna(final long hna) {
    this.hna = hna;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + (int) (this.hello ^ (this.hello >>> 32));
    result = (prime * result) + (int) (this.tc ^ (this.tc >>> 32));
    result = (prime * result) + (int) (this.mid ^ (this.mid >>> 32));
    result = (prime * result) + (int) (this.hna ^ (this.hna >>> 32));
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

    return (this.compareTo((JsonInfoMessageTimes) other) == 0);
  }

  @Override
  public int compareTo(final JsonInfoMessageTimes other) {
    if (other == null) {
      return -1;
    }

    int result;

    result = Long.compare(this.hello, other.hello);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.tc, other.tc);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.mid, other.mid);
    if (result != 0) {
      return result;
    }

    result = Long.compare(this.hna, other.hna);
    if (result != 0) {
      return result;
    }

    return 0;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoMessageTimes [hello=");
    builder.append(this.hello);
    builder.append(", tc=");
    builder.append(this.tc);
    builder.append(", mid=");
    builder.append(this.mid);
    builder.append(", hna=");
    builder.append(this.hna);
    builder.append("]");
    return builder.toString();
  }
}