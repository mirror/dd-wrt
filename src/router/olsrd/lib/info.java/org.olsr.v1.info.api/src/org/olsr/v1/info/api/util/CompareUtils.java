package org.olsr.v1.info.api.util;

/**
 * Compare utilities
 */
public class CompareUtils {
  /**
   * Clip a value for compare
   *
   * @param value the value to clip
   * @return the value in the range [-1, 1]
   */
  public static final int clip(final int value) {
    if (value <= -1) {
      return -1;
    }
    if (value >= 1) {
      return 1;
    }
    return 0;
  }
}