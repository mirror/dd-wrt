package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigHysteresis {
  private JsonInfoConfigHysteresis impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigHysteresis();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getEnabled()), equalTo(Boolean.FALSE));
    assertThat(Double.valueOf(this.impl.getScaling()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getThresholdLow()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getThresholdHigh()), equalTo(Double.valueOf(0.0)));

    /* set */
    this.impl.setEnabled(true);
    this.impl.setScaling(1.1);
    this.impl.setThresholdLow(2.2);
    this.impl.setThresholdHigh(3.3);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getEnabled()), equalTo(Boolean.TRUE));
    assertThat(Double.valueOf(this.impl.getScaling()), equalTo(Double.valueOf(1.1)));
    assertThat(Double.valueOf(this.impl.getThresholdLow()), equalTo(Double.valueOf(2.2)));
    assertThat(Double.valueOf(this.impl.getThresholdHigh()), equalTo(Double.valueOf(3.3)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigHysteresis other = new JsonInfoConfigHysteresis();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* enabled */

    final boolean booleanOrg = this.impl.getEnabled();

    this.impl.setEnabled(false);
    other.setEnabled(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setEnabled(true);
    other.setEnabled(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setEnabled(true);
    other.setEnabled(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEnabled(booleanOrg);
    other.setEnabled(booleanOrg);

    /* scaling */

    double doubleOrg = this.impl.getScaling();

    this.impl.setScaling(1.0);
    other.setScaling(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setScaling(2.0);
    other.setScaling(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setScaling(1.0);
    other.setScaling(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setScaling(doubleOrg);
    other.setScaling(doubleOrg);

    /* thresholdLow */

    doubleOrg = this.impl.getThresholdLow();

    this.impl.setThresholdLow(1.0);
    other.setThresholdLow(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setThresholdLow(2.0);
    other.setThresholdLow(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setThresholdLow(1.0);
    other.setThresholdLow(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setThresholdLow(doubleOrg);
    other.setThresholdLow(doubleOrg);

    /* thresholdHigh */

    doubleOrg = this.impl.getThresholdHigh();

    this.impl.setThresholdHigh(1.0);
    other.setThresholdHigh(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setThresholdHigh(2.0);
    other.setThresholdHigh(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setThresholdHigh(1.0);
    other.setThresholdHigh(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setThresholdHigh(doubleOrg);
    other.setThresholdHigh(doubleOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigHysteresis other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigHysteresis();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setEnabled(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(37774988)));

    /* set */
    this.impl.setEnabled(true);
    this.impl.setScaling(1.1);
    this.impl.setThresholdLow(2.2);
    this.impl.setThresholdHigh(3.3);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(779728882)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}