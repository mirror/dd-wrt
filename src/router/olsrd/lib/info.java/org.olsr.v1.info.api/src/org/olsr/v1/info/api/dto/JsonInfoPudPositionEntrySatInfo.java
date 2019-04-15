package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import com.fasterxml.jackson.annotation.JsonProperty;

public class JsonInfoPudPositionEntrySatInfo {
  private int                                          inUseCount  = 0;
  private final Set<Integer>                           inUse       = new TreeSet<>();
  private int                                          inViewCount = 0;
  private final Set<JsonInfoPudPositionEntrySatellite> inView      = new TreeSet<>();

  /**
   * @return the in-use count
   */
  public int getInUseCount() {
    return this.inUseCount;
  }

  /**
   * @param inUseCount the in-use count set
   */
  @JsonProperty("inUseCount")
  public void setInUseCount(final int inUseCount) {
    this.inUseCount = inUseCount;
  }

  /**
   * @return the set of satellite ids that are in use
   */
  public Set<Integer> getInUse() {
    return this.inUse;
  }

  /**
   * @param inUse the set of satellite ids to set that are in use
   */
  @JsonProperty("inUse")
  public void setInUse(final Set<Integer> inUse) {
    this.inUse.clear();
    if (inUse != null) {
      this.inUse.addAll(inUse);
    }
  }

  /**
   * @return the in-view count
   */
  public int getInViewCount() {
    return this.inViewCount;
  }

  /**
   * @param inViewCount the in-view count set
   */
  @JsonProperty("inViewCount")
  public void setInViewCount(final int inViewCount) {
    this.inViewCount = inViewCount;
  }

  /**
   * @return the inView
   */
  public Set<JsonInfoPudPositionEntrySatellite> getInView() {
    return this.inView;
  }

  /**
   * @param inView the inView to set
   */
  @JsonProperty("inView")
  public void setInView(final Set<JsonInfoPudPositionEntrySatellite> inView) {
    this.inView.clear();
    if (inView != null) {
      this.inView.addAll(inView);
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.inUseCount;
    result = (prime * result) + this.inUse.hashCode();
    result = (prime * result) + this.inViewCount;
    result = (prime * result) + this.inView.hashCode();
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
    final JsonInfoPudPositionEntrySatInfo other = (JsonInfoPudPositionEntrySatInfo) obj;
    if (this.inUseCount != other.inUseCount) {
      return false;
    }
    if (!this.inUse.equals(other.inUse)) {
      return false;
    }
    if (this.inViewCount != other.inViewCount) {
      return false;
    }
    if (!this.inView.equals(other.inView)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPositionEntrySatInfo [inUseCount=");
    builder.append(this.inUseCount);
    builder.append(", inUse=");
    builder.append(this.inUse);
    builder.append(", inViewCount=");
    builder.append(this.inViewCount);
    builder.append(", inView=");
    builder.append(this.inView);
    builder.append("]");
    return builder.toString();
  }
}