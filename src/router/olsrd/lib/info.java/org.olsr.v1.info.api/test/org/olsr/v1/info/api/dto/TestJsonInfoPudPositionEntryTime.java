package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPositionEntryTime {
  private JsonInfoPudPositionEntryTime impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPositionEntryTime();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getHour()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMinute()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getSecond()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getHsec()), equalTo(Integer.valueOf(0)));

    /* set */
    this.impl.setHour(1);
    this.impl.setMinute(2);
    this.impl.setSecond(3);
    this.impl.setHsec(4);

    /* get */
    assertThat(Integer.valueOf(this.impl.getHour()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getMinute()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getSecond()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getHsec()), equalTo(Integer.valueOf(4)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPudPositionEntryTime other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoPudPositionEntryTime();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoPudPositionEntryTime();
    other.setHour(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoPudPositionEntryTime other = new JsonInfoPudPositionEntryTime();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* hour */

    int intOrg = this.impl.getHour();

    this.impl.setHour(1);
    other.setHour(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHour(2);
    other.setHour(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHour(1);
    other.setHour(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHour(intOrg);
    other.setHour(intOrg);

    /* minute */

    intOrg = this.impl.getMinute();

    this.impl.setMinute(1);
    other.setMinute(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMinute(2);
    other.setMinute(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMinute(1);
    other.setMinute(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMinute(intOrg);
    other.setMinute(intOrg);

    /* second */

    intOrg = this.impl.getSecond();

    this.impl.setSecond(1);
    other.setSecond(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSecond(2);
    other.setSecond(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSecond(1);
    other.setSecond(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSecond(intOrg);
    other.setSecond(intOrg);

    /* hsec */

    intOrg = this.impl.getHsec();

    this.impl.setHsec(1);
    other.setHsec(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHsec(2);
    other.setHsec(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHsec(1);
    other.setHsec(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHsec(intOrg);
    other.setHsec(intOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    /* set */
    this.impl.setHour(1);
    this.impl.setMinute(2);
    this.impl.setSecond(3);
    this.impl.setHsec(4);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(955331)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}