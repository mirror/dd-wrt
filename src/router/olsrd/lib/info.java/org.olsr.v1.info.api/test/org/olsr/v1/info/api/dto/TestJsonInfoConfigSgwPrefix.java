package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigSgwPrefix {
  private JsonInfoConfigSgwPrefix impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigSgwPrefix();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getPrefix(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getLength()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setPrefix(addr);
    this.impl.setLength(11);

    /* get */
    assertThat(this.impl.getPrefix(), equalTo(addr.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getLength()), equalTo(Integer.valueOf(11)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoConfigSgwPrefix other = new JsonInfoConfigSgwPrefix();
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* prefix */

    final String stringOrg = this.impl.getPrefix();

    this.impl.setPrefix(null);
    other.setPrefix(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPrefix(null);
    other.setPrefix(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPrefix(addr);
    other.setPrefix(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPrefix(addr);
    other.setPrefix(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPrefix(addr2);
    other.setPrefix(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPrefix(addr);
    other.setPrefix(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPrefix(InetAddress.getByName(stringOrg));
    other.setPrefix(InetAddress.getByName(stringOrg));

    /* length */

    final int intOrg = this.impl.getLength();

    this.impl.setLength(1);
    other.setLength(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLength(2);
    other.setLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLength(1);
    other.setLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLength(intOrg);
    other.setLength(intOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigSgwPrefix other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigSgwPrefix();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setLength(11);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    this.impl.setPrefix(addr);
    this.impl.setLength(11);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-558694929)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}