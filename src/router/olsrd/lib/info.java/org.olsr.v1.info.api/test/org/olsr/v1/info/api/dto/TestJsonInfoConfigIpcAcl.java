package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigIpcAcl {
  private JsonInfoConfigIpcAcl impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigIpcAcl();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getHost()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getIpAddress(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getGenmask()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setHost(true);
    this.impl.setIpAddress(addr);
    this.impl.setGenmask(11);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getHost()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getIpAddress(), equalTo(addr.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getGenmask()), equalTo(Integer.valueOf(11)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoConfigIpcAcl other = new JsonInfoConfigIpcAcl();
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* host */

    final boolean booleanOrg = this.impl.getHost();

    this.impl.setHost(false);
    other.setHost(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHost(true);
    other.setHost(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHost(true);
    other.setHost(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHost(booleanOrg);
    other.setHost(booleanOrg);

    /* ipAddress */

    final String stringOrg = this.impl.getIpAddress();

    this.impl.setIpAddress(null);
    other.setIpAddress(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpAddress(null);
    other.setIpAddress(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpAddress(addr);
    other.setIpAddress(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpAddress(addr);
    other.setIpAddress(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpAddress(addr2);
    other.setIpAddress(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpAddress(addr);
    other.setIpAddress(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpAddress(InetAddress.getByName(stringOrg));
    other.setIpAddress(InetAddress.getByName(stringOrg));

    /* genmask */

    final int intOrg = this.impl.getGenmask();

    this.impl.setGenmask(1);
    other.setGenmask(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGenmask(2);
    other.setGenmask(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGenmask(1);
    other.setGenmask(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGenmask(intOrg);
    other.setGenmask(intOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigIpcAcl other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigIpcAcl();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setGenmask(11);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1218548)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setHost(true);
    this.impl.setIpAddress(addr);
    this.impl.setGenmask(11);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-557483108)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}