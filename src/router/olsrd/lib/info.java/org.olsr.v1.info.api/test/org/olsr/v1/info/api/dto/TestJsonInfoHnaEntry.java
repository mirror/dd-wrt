package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoHnaEntry {
  private JsonInfoHnaEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoHnaEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getGateway(), equalTo(""));
    assertThat(this.impl.getDestination(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getDestinationPrefixLength()), equalTo(Integer.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(0)));

    /* set */
    final InetAddress gw = InetAddress.getByName("127.0.0.31");
    final InetAddress destination = InetAddress.getByName("127.0.0.3");

    this.impl.setGateway(gw);
    this.impl.setDestination(destination);
    this.impl.setDestinationPrefixLength(3);
    this.impl.setValidityTime(4);

    /* get */
    assertThat(this.impl.getGateway(), equalTo(gw.getHostAddress()));
    assertThat(this.impl.getDestination(), equalTo(destination.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getDestinationPrefixLength()), equalTo(Integer.valueOf(3)));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(4)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoHnaEntry other = new JsonInfoHnaEntry();
    final InetAddress gateway1 = InetAddress.getByName("127.0.0.31");
    final InetAddress gateway2 = InetAddress.getByName("127.0.0.32");
    final InetAddress destination1 = InetAddress.getByName("127.0.0.1");
    final InetAddress destination2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* gateway */

    String stringOrg = this.impl.getGateway();

    this.impl.setGateway(null);
    other.setGateway(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(gateway2);
    other.setGateway(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGateway(gateway1);
    other.setGateway(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(gateway1);
    other.setGateway(gateway1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGateway(InetAddress.getByName(stringOrg));
    other.setGateway(InetAddress.getByName(stringOrg));

    /* destination */

    stringOrg = this.impl.getDestination();

    this.impl.setDestination(null);
    other.setDestination(destination2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(destination2);
    other.setDestination(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestination(destination1);
    other.setDestination(destination2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(destination1);
    other.setDestination(destination1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestination(InetAddress.getByName(stringOrg));
    other.setDestination(InetAddress.getByName(stringOrg));

    /* genmask */

    final int genmaskOrg = this.impl.getDestinationPrefixLength();

    this.impl.setDestinationPrefixLength(1);
    other.setDestinationPrefixLength(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestinationPrefixLength(2);
    other.setDestinationPrefixLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestinationPrefixLength(1);
    other.setDestinationPrefixLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestinationPrefixLength(genmaskOrg);
    other.setDestinationPrefixLength(genmaskOrg);

    /* ruleNr */

    final long validityTimeOrg = this.impl.getValidityTime();

    this.impl.setValidityTime(1);
    other.setValidityTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setValidityTime(2);
    other.setValidityTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setValidityTime(1);
    other.setValidityTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setValidityTime(validityTimeOrg);
    other.setValidityTime(validityTimeOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoHnaEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoHnaEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setValidityTime(12);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    final InetAddress gw = InetAddress.getByName("127.0.0.31");
    final InetAddress destination = InetAddress.getByName("127.0.0.3");

    this.impl.setGateway(gw);
    this.impl.setDestination(destination);
    this.impl.setDestinationPrefixLength(3);
    this.impl.setValidityTime(4);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1246786097)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}