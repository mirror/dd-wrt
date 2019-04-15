package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoSgwRouteInfo {
  private JsonInfoSgwRouteInfo impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgwRouteInfo();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getActive()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getFamily()), equalTo(Integer.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getRtTable()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getFlags()), equalTo(Long.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getScope()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getIfIndex()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMetric()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getProtocol()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getSrcSet()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getGwSet()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getDstSet()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getDelSimilar()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getBlackhole()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getSrcStore(), equalTo(""));
    assertThat(this.impl.getGwStore(), equalTo(""));
    assertThat(this.impl.getDstStore(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getDstStoreLength()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    this.impl.setActive(true);
    this.impl.setFamily(1);
    this.impl.setRtTable(2);
    this.impl.setFlags(3);
    this.impl.setScope(4);
    this.impl.setIfIndex(5);
    this.impl.setMetric(6);
    this.impl.setProtocol(7);
    this.impl.setSrcSet(true);
    this.impl.setGwSet(true);
    this.impl.setDstSet(true);
    this.impl.setDelSimilar(true);
    this.impl.setBlackhole(true);
    this.impl.setSrcStore(addr1);
    this.impl.setGwStore(addr2);
    this.impl.setDstStore(addr3);
    this.impl.setDstStoreLength(8);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getActive()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getFamily()), equalTo(Integer.valueOf(1)));
    assertThat(Long.valueOf(this.impl.getRtTable()), equalTo(Long.valueOf(2)));
    assertThat(Long.valueOf(this.impl.getFlags()), equalTo(Long.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getScope()), equalTo(Integer.valueOf(4)));
    assertThat(Integer.valueOf(this.impl.getIfIndex()), equalTo(Integer.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getMetric()), equalTo(Integer.valueOf(6)));
    assertThat(Integer.valueOf(this.impl.getProtocol()), equalTo(Integer.valueOf(7)));
    assertThat(Boolean.valueOf(this.impl.getSrcSet()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getGwSet()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getDstSet()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getDelSimilar()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getBlackhole()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getSrcStore(), equalTo(addr1.getHostAddress()));
    assertThat(this.impl.getGwStore(), equalTo(addr2.getHostAddress()));
    assertThat(this.impl.getDstStore(), equalTo(addr3.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getDstStoreLength()), equalTo(Integer.valueOf(8)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoSgwRouteInfo other = new JsonInfoSgwRouteInfo();
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    /* active */

    boolean booleanOrg = this.impl.getActive();

    this.impl.setActive(false);
    other.setActive(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setActive(false);
    other.setActive(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setActive(true);
    other.setActive(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setActive(true);
    other.setActive(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setActive(booleanOrg);
    other.setActive(booleanOrg);

    /* family */

    int intOrg = this.impl.getFamily();

    this.impl.setFamily(1);
    other.setFamily(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setFamily(2);
    other.setFamily(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setFamily(1);
    other.setFamily(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setFamily(intOrg);
    other.setFamily(intOrg);

    /* rtTable */

    long longOrg = this.impl.getRtTable();

    this.impl.setRtTable(1);
    other.setRtTable(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setRtTable(2);
    other.setRtTable(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setRtTable(1);
    other.setRtTable(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setRtTable(longOrg);
    other.setRtTable(longOrg);

    /* flags */

    longOrg = this.impl.getFlags();

    this.impl.setFlags(1);
    other.setFlags(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setFlags(2);
    other.setFlags(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setFlags(1);
    other.setFlags(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setFlags(longOrg);
    other.setFlags(longOrg);

    /* scope */

    intOrg = this.impl.getScope();

    this.impl.setScope(1);
    other.setScope(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setScope(2);
    other.setScope(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setScope(1);
    other.setScope(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setScope(intOrg);
    other.setScope(intOrg);

    /* ifIndex */

    intOrg = this.impl.getIfIndex();

    this.impl.setIfIndex(1);
    other.setIfIndex(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIfIndex(2);
    other.setIfIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIfIndex(1);
    other.setIfIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIfIndex(intOrg);
    other.setIfIndex(intOrg);

    /* metric */

    intOrg = this.impl.getMetric();

    this.impl.setMetric(1);
    other.setMetric(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMetric(2);
    other.setMetric(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMetric(1);
    other.setMetric(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMetric(intOrg);
    other.setMetric(intOrg);

    /* protocol */

    intOrg = this.impl.getProtocol();

    this.impl.setProtocol(1);
    other.setProtocol(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setProtocol(2);
    other.setProtocol(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setProtocol(1);
    other.setProtocol(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setProtocol(intOrg);
    other.setProtocol(intOrg);

    /* srcSet */

    booleanOrg = this.impl.getSrcSet();

    this.impl.setSrcSet(false);
    other.setSrcSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSrcSet(false);
    other.setSrcSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSrcSet(true);
    other.setSrcSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSrcSet(true);
    other.setSrcSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSrcSet(booleanOrg);
    other.setSrcSet(booleanOrg);

    /* gwSet */

    booleanOrg = this.impl.getGwSet();

    this.impl.setGwSet(false);
    other.setGwSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGwSet(false);
    other.setGwSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGwSet(true);
    other.setGwSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGwSet(true);
    other.setGwSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGwSet(booleanOrg);
    other.setGwSet(booleanOrg);

    /* dstSet */

    booleanOrg = this.impl.getDstSet();

    this.impl.setDstSet(false);
    other.setDstSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDstSet(false);
    other.setDstSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDstSet(true);
    other.setDstSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDstSet(true);
    other.setDstSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDstSet(booleanOrg);
    other.setDstSet(booleanOrg);

    /* delSimilar */

    booleanOrg = this.impl.getDelSimilar();

    this.impl.setDelSimilar(false);
    other.setDelSimilar(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDelSimilar(false);
    other.setDelSimilar(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDelSimilar(true);
    other.setDelSimilar(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDelSimilar(true);
    other.setDelSimilar(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDelSimilar(booleanOrg);
    other.setDelSimilar(booleanOrg);

    /* blackhole */

    booleanOrg = this.impl.getBlackhole();

    this.impl.setBlackhole(false);
    other.setBlackhole(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBlackhole(false);
    other.setBlackhole(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBlackhole(true);
    other.setBlackhole(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBlackhole(true);
    other.setBlackhole(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBlackhole(booleanOrg);
    other.setBlackhole(booleanOrg);

    /* srcStore */

    String addrOrg = this.impl.getSrcStore();

    this.impl.setSrcStore(null);
    other.setSrcStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSrcStore(null);
    other.setSrcStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSrcStore(addr1);
    other.setSrcStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSrcStore(addr1);
    other.setSrcStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSrcStore(addr1);
    other.setSrcStore(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSrcStore(addr2);
    other.setSrcStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSrcStore(InetAddress.getByName(addrOrg));
    other.setSrcStore(InetAddress.getByName(addrOrg));

    /* gwStore */

    addrOrg = this.impl.getGwStore();

    this.impl.setGwStore(null);
    other.setGwStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGwStore(null);
    other.setGwStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGwStore(addr1);
    other.setGwStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGwStore(addr1);
    other.setGwStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGwStore(addr1);
    other.setGwStore(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGwStore(addr2);
    other.setGwStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGwStore(InetAddress.getByName(addrOrg));
    other.setGwStore(InetAddress.getByName(addrOrg));

    /* dstStore */

    addrOrg = this.impl.getDstStore();

    this.impl.setDstStore(null);
    other.setDstStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDstStore(null);
    other.setDstStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDstStore(addr1);
    other.setDstStore(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDstStore(addr1);
    other.setDstStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDstStore(addr1);
    other.setDstStore(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDstStore(addr2);
    other.setDstStore(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDstStore(InetAddress.getByName(addrOrg));
    other.setDstStore(InetAddress.getByName(addrOrg));

    /* dstStoreLength */

    intOrg = this.impl.getDstStoreLength();

    this.impl.setDstStoreLength(1);
    other.setDstStoreLength(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDstStoreLength(2);
    other.setDstStoreLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDstStoreLength(1);
    other.setDstStoreLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDstStoreLength(intOrg);
    other.setDstStoreLength(intOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoSgwRouteInfo other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoSgwRouteInfo();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setFamily(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(176712201)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    this.impl.setActive(true);
    this.impl.setFamily(1);
    this.impl.setRtTable(2);
    this.impl.setFlags(3);
    this.impl.setScope(4);
    this.impl.setIfIndex(5);
    this.impl.setMetric(6);
    this.impl.setProtocol(7);
    this.impl.setSrcSet(true);
    this.impl.setGwSet(true);
    this.impl.setDstSet(true);
    this.impl.setDelSimilar(true);
    this.impl.setBlackhole(true);
    this.impl.setSrcStore(addr1);
    this.impl.setGwStore(addr2);
    this.impl.setDstStore(addr3);
    this.impl.setDstStoreLength(8);

    r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(280329411)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}