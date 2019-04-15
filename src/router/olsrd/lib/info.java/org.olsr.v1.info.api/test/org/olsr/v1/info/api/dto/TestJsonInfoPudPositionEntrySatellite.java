package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPositionEntrySatellite {
  private JsonInfoPudPositionEntrySatellite impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPositionEntrySatellite();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getId()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getElevation()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getAzimuth()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getSignal()), equalTo(Integer.valueOf(0)));

    /* set */
    this.impl.setId(1);
    this.impl.setElevation(2);
    this.impl.setAzimuth(3);
    this.impl.setSignal(4);

    /* get */
    assertThat(Integer.valueOf(this.impl.getId()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getElevation()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getAzimuth()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getSignal()), equalTo(Integer.valueOf(4)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPudPositionEntrySatellite other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoPudPositionEntrySatellite();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoPudPositionEntrySatellite();
    other.setId(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoPudPositionEntrySatellite other = new JsonInfoPudPositionEntrySatellite();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* id */

    int intOrg = this.impl.getId();

    this.impl.setId(1);
    other.setId(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setId(2);
    other.setId(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setId(1);
    other.setId(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setId(intOrg);
    other.setId(intOrg);

    /* elevation */

    intOrg = this.impl.getElevation();

    this.impl.setElevation(1);
    other.setElevation(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setElevation(2);
    other.setElevation(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setElevation(1);
    other.setElevation(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setElevation(intOrg);
    other.setElevation(intOrg);

    /* azimuth */

    intOrg = this.impl.getAzimuth();

    this.impl.setAzimuth(1);
    other.setAzimuth(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAzimuth(2);
    other.setAzimuth(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAzimuth(1);
    other.setAzimuth(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAzimuth(intOrg);
    other.setAzimuth(intOrg);

    /* signal */

    intOrg = this.impl.getSignal();

    this.impl.setSignal(1);
    other.setSignal(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSignal(2);
    other.setSignal(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSignal(1);
    other.setSignal(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSignal(intOrg);
    other.setSignal(intOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    /* set */
    this.impl.setId(1);
    this.impl.setElevation(2);
    this.impl.setAzimuth(3);
    this.impl.setSignal(4);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(955331)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}