package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoTimedIpAddress {
  private JsonInfoTimedIpAddress impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoTimedIpAddress();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getIpAddress(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(0)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setIpAddress(ipAddress);
    this.impl.setValidityTime(11);

    /* get */
    assertThat(this.impl.getIpAddress(), equalTo(ipAddress.getHostAddress()));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(11)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoTimedIpAddress other = new JsonInfoTimedIpAddress();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* ipAddress */

    final String ipAddressOrg = this.impl.getIpAddress();

    this.impl.setIpAddress(null);
    other.setIpAddress(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpAddress(originator2);
    other.setIpAddress(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpAddress(originator1);
    other.setIpAddress(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpAddress(originator1);
    other.setIpAddress(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpAddress(InetAddress.getByName(ipAddressOrg));
    other.setIpAddress(InetAddress.getByName(ipAddressOrg));

    /* validityTime */

    final long validityTimeOrg = this.impl.getValidityTime();

    this.impl.setValidityTime(1);
    other.setValidityTime(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setValidityTime(2);
    other.setValidityTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setValidityTime(1);
    other.setValidityTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setValidityTime(validityTimeOrg);
    other.setValidityTime(validityTimeOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoTimedIpAddress other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoTimedIpAddress();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setValidityTime(11);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setIpAddress(ipAddress);
    this.impl.setValidityTime(11);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-558694929)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}