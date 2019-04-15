package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.olsr.v1.info.api.contants.Willingness;

public class TestJsonInfoConfigWillingness {
  private JsonInfoConfigWillingness impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigWillingness();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getWillingness(), equalTo(Willingness.UNKNOWN));
    assertThat(Boolean.valueOf(this.impl.getAuto()), equalTo(Boolean.FALSE));
    assertThat(Double.valueOf(this.impl.getUpdateInterval()), equalTo(Double.valueOf(0.0)));

    /* set */
    this.impl.setWillingness(Willingness.ALWAYS.getValue());
    this.impl.setAuto(true);
    this.impl.setUpdateInterval(1.2);

    /* get */
    assertThat(this.impl.getWillingness(), equalTo(Willingness.ALWAYS));
    assertThat(Boolean.valueOf(this.impl.getAuto()), equalTo(Boolean.TRUE));
    assertThat(Double.valueOf(this.impl.getUpdateInterval()), equalTo(Double.valueOf(1.2)));

    this.impl.setWillingness(-10);
    assertThat(this.impl.getWillingness(), equalTo(Willingness.UNKNOWN));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigWillingness other = new JsonInfoConfigWillingness();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* willingness */

    final Willingness willingnessOrg = this.impl.getWillingness();

    this.impl.setWillingness(Willingness.DEFAULT.getValue());
    other.setWillingness(Willingness.HIGH.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setWillingness(Willingness.HIGH.getValue());
    other.setWillingness(Willingness.DEFAULT.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setWillingness(Willingness.DEFAULT.getValue());
    other.setWillingness(Willingness.DEFAULT.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setWillingness(willingnessOrg.getValue());
    other.setWillingness(willingnessOrg.getValue());

    /* auto */

    final boolean booleanOrg = this.impl.getAuto();

    this.impl.setAuto(false);
    other.setAuto(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAuto(true);
    other.setAuto(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAuto(true);
    other.setAuto(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAuto(booleanOrg);
    other.setAuto(booleanOrg);

    /* updateInterval */

    final double doubleOrg = this.impl.getUpdateInterval();

    this.impl.setUpdateInterval(1.0);
    other.setUpdateInterval(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUpdateInterval(2.0);
    other.setUpdateInterval(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUpdateInterval(1.0);
    other.setUpdateInterval(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpdateInterval(doubleOrg);
    other.setUpdateInterval(doubleOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigWillingness other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigWillingness();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setAuto(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(67177)));

    /* set */
    this.impl.setWillingness(Willingness.ALWAYS.getValue());
    this.impl.setAuto(true);
    this.impl.setUpdateInterval(1.2);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(213984183)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}