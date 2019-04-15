package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoGatewaysEntry {
  private JsonInfoGatewaysEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoGatewaysEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getSelected()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getSelectable()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getOriginator(), equalTo(""));
    assertThat(this.impl.getPrefix(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getPrefixLen()), equalTo(Integer.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getUplink()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getDownlink()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Boolean.valueOf(this.impl.getIpv4()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getIpv4Nat()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getIpv6()), equalTo(Boolean.FALSE));
    assertThat(Long.valueOf(this.impl.getExpireTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getCleanupTime()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Integer.valueOf(this.impl.getHops()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress originator = InetAddress.getByName("127.0.0.1");
    final InetAddress prefix = InetAddress.getByName("127.0.0.2");

    this.impl.setSelected(true);
    this.impl.setSelectable(true);
    this.impl.setOriginator(originator);
    this.impl.setPrefix(prefix);
    this.impl.setPrefixLen(56789);
    this.impl.setUplink(1234);
    this.impl.setDownlink(4321);
    this.impl.setCost(93421.21);
    this.impl.setIpv4(true);
    this.impl.setIpv4Nat(true);
    this.impl.setIpv6(true);
    this.impl.setExpireTime(1234);
    this.impl.setCleanupTime(4321);
    this.impl.setPathCost(8765.56);
    this.impl.setHops(5);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getSelected()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getSelectable()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getOriginator(), equalTo(originator.getHostAddress()));
    assertThat(this.impl.getPrefix(), equalTo(prefix.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getPrefixLen()), equalTo(Integer.valueOf(56789)));
    assertThat(Long.valueOf(this.impl.getUplink()), equalTo(Long.valueOf(1234)));
    assertThat(Long.valueOf(this.impl.getDownlink()), equalTo(Long.valueOf(4321)));
    assertThat(Double.valueOf(this.impl.getCost()), equalTo(Double.valueOf(93421.21)));
    assertThat(Boolean.valueOf(this.impl.getIpv4()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv4Nat()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv6()), equalTo(Boolean.TRUE));
    assertThat(Long.valueOf(this.impl.getExpireTime()), equalTo(Long.valueOf(1234)));
    assertThat(Long.valueOf(this.impl.getCleanupTime()), equalTo(Long.valueOf(4321)));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(8765.56)));
    assertThat(Integer.valueOf(this.impl.getHops()), equalTo(Integer.valueOf(5)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoGatewaysEntry other = new JsonInfoGatewaysEntry();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");
    final InetAddress prefix1 = InetAddress.getByName("127.0.1.1");
    final InetAddress prefix2 = InetAddress.getByName("127.0.1.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* selected */

    final boolean selectedOrg = this.impl.getSelected();

    this.impl.setSelected(false);
    other.setSelected(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSelected(true);
    other.setSelected(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSelected(true);
    other.setSelected(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSelected(selectedOrg);
    other.setSelected(selectedOrg);

    /* selectable */

    final boolean selectableOrg = this.impl.getSelectable();

    this.impl.setSelectable(false);
    other.setSelectable(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSelectable(true);
    other.setSelectable(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSelectable(true);
    other.setSelectable(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSelectable(selectableOrg);
    other.setSelectable(selectableOrg);

    /* originator */

    String stringOrg = this.impl.getOriginator();

    this.impl.setOriginator(null);
    other.setOriginator(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setOriginator(originator2);
    other.setOriginator(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setOriginator(originator1);
    other.setOriginator(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setOriginator(originator1);
    other.setOriginator(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setOriginator(InetAddress.getByName(stringOrg));
    other.setOriginator(InetAddress.getByName(stringOrg));

    /* prefix */

    stringOrg = this.impl.getPrefix();

    this.impl.setPrefix(null);
    other.setPrefix(prefix2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPrefix(prefix2);
    other.setPrefix(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPrefix(prefix1);
    other.setPrefix(prefix2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPrefix(prefix1);
    other.setPrefix(prefix1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPrefix(InetAddress.getByName(stringOrg));
    other.setPrefix(InetAddress.getByName(stringOrg));

    /* prefixLen */

    final int prefixLenOrg = this.impl.getPrefixLen();

    this.impl.setPrefixLen(1);
    other.setPrefixLen(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPrefixLen(2);
    other.setPrefixLen(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPrefixLen(1);
    other.setPrefixLen(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPrefixLen(prefixLenOrg);
    other.setPrefixLen(prefixLenOrg);

    /* uplink */

    final long uplinkOrg = this.impl.getUplink();

    this.impl.setUplink(1);
    other.setUplink(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setUplink(2);
    other.setUplink(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setUplink(1);
    other.setUplink(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setUplink(uplinkOrg);
    other.setUplink(uplinkOrg);

    /* downlink */

    final long downlinkOrg = this.impl.getDownlink();

    this.impl.setDownlink(1);
    other.setDownlink(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setDownlink(2);
    other.setDownlink(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setDownlink(1);
    other.setDownlink(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setDownlink(downlinkOrg);
    other.setDownlink(downlinkOrg);

    /* cost */

    double doubleOrg = this.impl.getCost();

    this.impl.setCost(1);
    other.setCost(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setCost(2);
    other.setCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setCost(1);
    other.setCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setCost(doubleOrg);
    other.setCost(doubleOrg);

    /* ipv4 */

    final boolean ipv4Org = this.impl.getIpv4();

    this.impl.setIpv4(false);
    other.setIpv4(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4(true);
    other.setIpv4(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv4(true);
    other.setIpv4(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv4(ipv4Org);
    other.setIpv4(ipv4Org);

    /* ipv4Nat */

    final boolean ipv4NatOrg = this.impl.getIpv4Nat();

    this.impl.setIpv4Nat(false);
    other.setIpv4Nat(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Nat(true);
    other.setIpv4Nat(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv4Nat(true);
    other.setIpv4Nat(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv4Nat(ipv4NatOrg);
    other.setIpv4Nat(ipv4NatOrg);

    /* ipv6 */

    final boolean ipv6Org = this.impl.getIpv6();

    this.impl.setIpv6(false);
    other.setIpv6(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv6(true);
    other.setIpv6(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv6(true);
    other.setIpv6(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv6(ipv6Org);
    other.setIpv6(ipv6Org);

    /* expireTime */

    final long expireTimeOrg = this.impl.getExpireTime();

    this.impl.setExpireTime(1);
    other.setExpireTime(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setExpireTime(2);
    other.setExpireTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setExpireTime(1);
    other.setExpireTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setExpireTime(expireTimeOrg);
    other.setExpireTime(expireTimeOrg);

    /* cleanupTime */

    final long cleanupTimeOrg = this.impl.getCleanupTime();

    this.impl.setCleanupTime(1);
    other.setCleanupTime(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setCleanupTime(2);
    other.setCleanupTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setCleanupTime(1);
    other.setCleanupTime(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setCleanupTime(cleanupTimeOrg);
    other.setCleanupTime(cleanupTimeOrg);

    /* pathcost */

    doubleOrg = this.impl.getPathCost();

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

    /* hops */

    final int hopsOrg = this.impl.getHops();

    this.impl.setHops(1);
    other.setHops(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHops(2);
    other.setHops(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHops(1);
    other.setHops(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHops(hopsOrg);
    other.setHops(hopsOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoGatewaysEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoGatewaysEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setCost(1234);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-955768076)));

    final InetAddress originator = InetAddress.getByName("127.0.0.1");
    final InetAddress prefix = InetAddress.getByName("127.0.0.2");

    this.impl.setSelected(true);
    this.impl.setSelectable(true);
    this.impl.setOriginator(originator);
    this.impl.setPrefix(prefix);
    this.impl.setPrefixLen(56789);
    this.impl.setUplink(1234);
    this.impl.setDownlink(4321);
    this.impl.setCost(93421);
    this.impl.setIpv4(true);
    this.impl.setIpv4Nat(true);
    this.impl.setIpv6(true);
    this.impl.setExpireTime(1234);
    this.impl.setCleanupTime(4321);
    this.impl.setPathCost(8765);
    this.impl.setHops(5);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-200802475)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}