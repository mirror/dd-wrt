package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigSgwBandwidth {
  private JsonInfoConfigSgwBandwidth impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigSgwBandwidth();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Long.valueOf(this.impl.getUplinkKbps()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getDownlinkKbps()), equalTo(Long.valueOf(0)));

    /* set */
    this.impl.setUplinkKbps(1);
    this.impl.setDownlinkKbps(2);

    /* get */
    assertThat(Long.valueOf(this.impl.getUplinkKbps()), equalTo(Long.valueOf(1)));
    assertThat(Long.valueOf(this.impl.getDownlinkKbps()), equalTo(Long.valueOf(2)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigSgwBandwidth other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigSgwBandwidth();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoConfigSgwBandwidth();
    other.setUplinkKbps(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigSgwBandwidth other = new JsonInfoConfigSgwBandwidth();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* uplinkKbps */

    long longOrg = this.impl.getUplinkKbps();

    this.impl.setUplinkKbps(1);
    other.setUplinkKbps(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUplinkKbps(2);
    other.setUplinkKbps(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUplinkKbps(1);
    other.setUplinkKbps(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUplinkKbps(longOrg);
    other.setUplinkKbps(longOrg);

    /* downlinkKbps */

    longOrg = this.impl.getDownlinkKbps();

    this.impl.setDownlinkKbps(1);
    other.setDownlinkKbps(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDownlinkKbps(2);
    other.setDownlinkKbps(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDownlinkKbps(1);
    other.setDownlinkKbps(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDownlinkKbps(longOrg);
    other.setDownlinkKbps(longOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    this.impl.setUplinkKbps(1);
    this.impl.setDownlinkKbps(2);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(994)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}