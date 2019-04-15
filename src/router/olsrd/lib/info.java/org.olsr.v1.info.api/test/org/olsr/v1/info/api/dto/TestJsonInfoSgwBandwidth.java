package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoSgwBandwidth {
  private JsonInfoSgwBandwidth impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgwBandwidth();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getRequireNetwork()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getRequireGateway()), equalTo(Boolean.TRUE));
    assertThat(Long.valueOf(this.impl.getEgressUk()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getEgressDk()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(this.impl.getNetwork(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getNetworkLength()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getGateway(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getNetworkSet()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getGatewaySet()), equalTo(Boolean.FALSE));
    assertThat(Double.valueOf(this.impl.getCosts()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    this.impl.setRequireNetwork(false);
    this.impl.setRequireGateway(false);
    this.impl.setEgressUk(1);
    this.impl.setEgressDk(2);
    this.impl.setPathCost(3.32);
    this.impl.setNetwork(addr1);
    this.impl.setNetworkLength(4);
    this.impl.setGateway(addr2);
    this.impl.setNetworkSet(true);
    this.impl.setGatewaySet(true);
    this.impl.setCosts(5.123);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getRequireNetwork()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getRequireGateway()), equalTo(Boolean.FALSE));
    assertThat(Long.valueOf(this.impl.getEgressUk()), equalTo(Long.valueOf(1)));
    assertThat(Long.valueOf(this.impl.getEgressDk()), equalTo(Long.valueOf(2)));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(3.32)));
    assertThat(this.impl.getNetwork(), equalTo(addr1.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getNetworkLength()), equalTo(Integer.valueOf(4)));
    assertThat(this.impl.getGateway(), equalTo(addr2.getHostAddress()));
    assertThat(Boolean.valueOf(this.impl.getNetworkSet()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getGatewaySet()), equalTo(Boolean.TRUE));
    assertThat(Double.valueOf(this.impl.getCosts()), equalTo(Double.valueOf(5.123)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoSgwBandwidth other = new JsonInfoSgwBandwidth();
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    /* egressUk */

    long longOrg = this.impl.getEgressUk();

    this.impl.setEgressUk(1);
    other.setEgressUk(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setEgressUk(2);
    other.setEgressUk(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setEgressUk(1);
    other.setEgressUk(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setEgressUk(longOrg);
    other.setEgressUk(longOrg);

    /* egressDk */

    longOrg = this.impl.getEgressDk();

    this.impl.setEgressDk(1);
    other.setEgressDk(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setEgressDk(2);
    other.setEgressDk(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setEgressDk(1);
    other.setEgressDk(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setEgressDk(longOrg);
    other.setEgressDk(longOrg);

    /* pathCost */

    double doubleOrg = this.impl.getPathCost();

    this.impl.setPathCost(1);
    other.setPathCost(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setPathCost(2);
    other.setPathCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setPathCost(1);
    other.setPathCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setPathCost(doubleOrg);
    other.setPathCost(doubleOrg);

    /* network */

    String addrOrg = this.impl.getNetwork();

    this.impl.setNetwork(null);
    other.setNetwork(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetwork(null);
    other.setNetwork(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetwork(addr1);
    other.setNetwork(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetwork(addr1);
    other.setNetwork(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetwork(addr1);
    other.setNetwork(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetwork(addr2);
    other.setNetwork(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetwork(InetAddress.getByName(addrOrg));
    other.setNetwork(InetAddress.getByName(addrOrg));

    /* networkLength */

    final int intOrg = this.impl.getNetworkLength();

    this.impl.setNetworkLength(1);
    other.setNetworkLength(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkLength(2);
    other.setNetworkLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetworkLength(1);
    other.setNetworkLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkLength(intOrg);
    other.setNetworkLength(intOrg);

    /* gateway */

    addrOrg = this.impl.getGateway();

    this.impl.setGateway(null);
    other.setGateway(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGateway(null);
    other.setGateway(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(addr1);
    other.setGateway(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGateway(addr1);
    other.setGateway(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGateway(addr1);
    other.setGateway(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(addr2);
    other.setGateway(addr1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGateway(InetAddress.getByName(addrOrg));
    other.setGateway(InetAddress.getByName(addrOrg));

    /* requireNetwork */

    boolean booleanOrg = this.impl.getRequireNetwork();

    this.impl.setRequireNetwork(false);
    other.setRequireNetwork(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRequireNetwork(false);
    other.setRequireNetwork(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRequireNetwork(true);
    other.setRequireNetwork(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRequireNetwork(true);
    other.setRequireNetwork(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRequireNetwork(booleanOrg);
    other.setRequireNetwork(booleanOrg);

    /* requireGateway */

    booleanOrg = this.impl.getRequireGateway();

    this.impl.setRequireGateway(false);
    other.setRequireGateway(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRequireGateway(false);
    other.setRequireGateway(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRequireGateway(true);
    other.setRequireGateway(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRequireGateway(true);
    other.setRequireGateway(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRequireGateway(booleanOrg);
    other.setRequireGateway(booleanOrg);

    /* networkSet */

    booleanOrg = this.impl.getNetworkSet();

    this.impl.setNetworkSet(false);
    other.setNetworkSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkSet(false);
    other.setNetworkSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkSet(true);
    other.setNetworkSet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetworkSet(true);
    other.setNetworkSet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkSet(booleanOrg);
    other.setNetworkSet(booleanOrg);

    /* gatewaySet */

    booleanOrg = this.impl.getGatewaySet();

    this.impl.setGatewaySet(false);
    other.setGatewaySet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGatewaySet(false);
    other.setGatewaySet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGatewaySet(true);
    other.setGatewaySet(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGatewaySet(true);
    other.setGatewaySet(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGatewaySet(booleanOrg);
    other.setGatewaySet(booleanOrg);

    /* costs */

    doubleOrg = this.impl.getCosts();

    this.impl.setCosts(1);
    other.setCosts(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setCosts(2);
    other.setCosts(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setCosts(1);
    other.setCosts(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setCosts(doubleOrg);
    other.setCosts(doubleOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoSgwBandwidth other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoSgwBandwidth();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setCosts(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-125640481)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    this.impl.setRequireNetwork(false);
    this.impl.setRequireGateway(false);
    this.impl.setEgressUk(1);
    this.impl.setEgressDk(2);
    this.impl.setPathCost(3);
    this.impl.setNetwork(addr1);
    this.impl.setNetworkLength(4);
    this.impl.setGateway(addr2);
    this.impl.setNetworkSet(true);
    this.impl.setGatewaySet(true);
    this.impl.setCosts(5);

    r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1702663673)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}