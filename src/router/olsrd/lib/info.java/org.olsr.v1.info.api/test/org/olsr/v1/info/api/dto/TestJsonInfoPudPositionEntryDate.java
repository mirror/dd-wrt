package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPositionEntryDate {
  private JsonInfoPudPositionEntryDate impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPositionEntryDate();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getYear()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMonth()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getDay()), equalTo(Integer.valueOf(0)));

    /* set */
    this.impl.setYear(1);
    this.impl.setMonth(2);
    this.impl.setDay(3);

    /* get */
    assertThat(Integer.valueOf(this.impl.getYear()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getMonth()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getDay()), equalTo(Integer.valueOf(3)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPudPositionEntryDate other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoPudPositionEntryDate();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoPudPositionEntryDate();
    other.setYear(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoPudPositionEntryDate other = new JsonInfoPudPositionEntryDate();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* year */

    int intOrg = this.impl.getYear();

    this.impl.setYear(1);
    other.setYear(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setYear(2);
    other.setYear(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setYear(1);
    other.setYear(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setYear(intOrg);
    other.setYear(intOrg);

    /* month */

    intOrg = this.impl.getMonth();

    this.impl.setMonth(1);
    other.setMonth(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMonth(2);
    other.setMonth(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMonth(1);
    other.setMonth(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMonth(intOrg);
    other.setMonth(intOrg);

    /* day */

    intOrg = this.impl.getDay();

    this.impl.setDay(1);
    other.setDay(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDay(2);
    other.setDay(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDay(1);
    other.setDay(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDay(intOrg);
    other.setDay(intOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(29791)));

    /* set */
    this.impl.setYear(1);
    this.impl.setMonth(2);
    this.impl.setDay(3);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(30817)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}