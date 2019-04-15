package org.olsr.v1.info.api.contants;

import org.osgi.annotation.versioning.ProviderType;

/**
 * OLSRd willingness
 */
@ProviderType
public enum Willingness {
  UNKNOWN(-1), //
  NEVER(0), //
  LOW(1), //
  DEFAULT(3), //
  HIGH(6), //
  ALWAYS(7);

  private int value = 0;

  private Willingness(final int value) {
    this.value = value;
  }

  public int getValue() {
    return this.value;
  }

  /**
   * Convert a value to a willingness.
   *
   * @param value the value to convert to a willingness
   * @return the willingness, or UNKNOWN when not found
   */
  static public Willingness fromValue(final int value) {
    for (final Willingness w : Willingness.values()) {
      if (w.getValue() == value) {
        return w;
      }
    }

    return Willingness.UNKNOWN;
  }
}