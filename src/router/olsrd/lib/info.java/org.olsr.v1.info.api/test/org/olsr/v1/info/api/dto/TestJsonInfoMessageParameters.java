package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoMessageParameters {
  private JsonInfoMessageParameters impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoMessageParameters();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Double.valueOf(this.impl.getEmissionInterval()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getValidityTime()), equalTo(Double.valueOf(0.0)));

    /* set */
    this.impl.setEmissionInterval(1.0);
    this.impl.setValidityTime(2.0);

    /* get */
    assertThat(Double.valueOf(this.impl.getEmissionInterval()), equalTo(Double.valueOf(1.0)));
    assertThat(Double.valueOf(this.impl.getValidityTime()), equalTo(Double.valueOf(2.0)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoMessageParameters other = new JsonInfoMessageParameters();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* emissionInterval */

    final double emissionIntervalOrg = this.impl.getEmissionInterval();

    this.impl.setEmissionInterval(1.0);
    other.setEmissionInterval(2.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setEmissionInterval(2.0);
    other.setEmissionInterval(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setEmissionInterval(1.0);
    other.setEmissionInterval(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setEmissionInterval(emissionIntervalOrg);
    other.setEmissionInterval(emissionIntervalOrg);
    /* validityTime */

    final double validityTimeOrg = this.impl.getValidityTime();

    this.impl.setValidityTime(1.0);
    other.setValidityTime(2.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setValidityTime(2.0);
    other.setValidityTime(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setValidityTime(1.0);
    other.setValidityTime(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setValidityTime(validityTimeOrg);
    other.setValidityTime(validityTimeOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoMessageParameters other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoMessageParameters();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setValidityTime(11.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    this.impl.setEmissionInterval(1.0);
    this.impl.setValidityTime(2.0);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-32504895)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}