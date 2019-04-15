package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoLinkQualityMultiplier {
  private JsonInfoLinkQualityMultiplier impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoLinkQualityMultiplier();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getRoute(), equalTo(""));
    assertThat(Double.valueOf(this.impl.getMultiplier()), equalTo(Double.valueOf(0.0)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setRoute(addr);
    this.impl.setMultiplier(2.0);

    /* get */
    assertThat(this.impl.getRoute(), equalTo(addr.getHostAddress()));
    assertThat(Double.valueOf(this.impl.getMultiplier()), equalTo(Double.valueOf(2.0)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoLinkQualityMultiplier other = new JsonInfoLinkQualityMultiplier();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* route */

    final String routeOrg = this.impl.getRoute();

    this.impl.setRoute(null);
    other.setRoute(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRoute(originator2);
    other.setRoute(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRoute(originator1);
    other.setRoute(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRoute(originator1);
    other.setRoute(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRoute(InetAddress.getByName(routeOrg));
    other.setRoute(InetAddress.getByName(routeOrg));

    /* multiplier */

    final double multiplierOrg = this.impl.getMultiplier();

    this.impl.setMultiplier(1.0);
    other.setMultiplier(2.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setMultiplier(2.0);
    other.setMultiplier(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setMultiplier(1.0);
    other.setMultiplier(1.0);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setMultiplier(multiplierOrg);
    other.setMultiplier(multiplierOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoLinkQualityMultiplier other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoLinkQualityMultiplier();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setMultiplier(11.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));
    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setRoute(addr);
    this.impl.setMultiplier(2.0);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(515046884)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}