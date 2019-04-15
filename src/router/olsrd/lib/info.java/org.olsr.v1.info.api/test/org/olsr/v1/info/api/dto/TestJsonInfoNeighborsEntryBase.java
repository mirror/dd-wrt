package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.olsr.v1.info.api.contants.Willingness;

public class TestJsonInfoNeighborsEntryBase {
  private JsonInfoNeighborsEntryBase impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoNeighborsEntryBase();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getIpAddress(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getSymmetric()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getWillingness(), equalTo(Willingness.UNKNOWN));
    assertThat(Boolean.valueOf(this.impl.getMultiPointRelay()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.wasMultiPointRelay()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getMultiPointRelaySelector()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getSkip()), equalTo(Boolean.FALSE));
    assertThat(Long.valueOf(this.impl.getNeighbor2nocov()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getLinkcount()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getTwoHopNeighborCount()), equalTo(Long.valueOf(0)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setIpAddress(ipAddress);
    this.impl.setSymmetric(true);
    this.impl.setWillingness(Willingness.ALWAYS.getValue());
    this.impl.setIsMultiPointRelay(true);
    this.impl.setWasMultiPointRelay(true);
    this.impl.setMultiPointRelaySelector(true);
    this.impl.setSkip(true);
    this.impl.setNeighbor2nocov(11);
    this.impl.setLinkcount(12);
    this.impl.setTwoHopNeighborCount(32);

    /* get */
    assertThat(this.impl.getIpAddress(), equalTo(ipAddress.getHostAddress()));
    assertThat(Boolean.valueOf(this.impl.getSymmetric()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getWillingness(), equalTo(Willingness.ALWAYS));
    assertThat(Boolean.valueOf(this.impl.getMultiPointRelay()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.wasMultiPointRelay()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getMultiPointRelaySelector()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getSkip()), equalTo(Boolean.TRUE));
    assertThat(Long.valueOf(this.impl.getNeighbor2nocov()), equalTo(Long.valueOf(11)));
    assertThat(Long.valueOf(this.impl.getLinkcount()), equalTo(Long.valueOf(12)));
    assertThat(Long.valueOf(this.impl.getTwoHopNeighborCount()), equalTo(Long.valueOf(32)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoNeighborsEntryBase other = new JsonInfoNeighborsEntryBase();
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

    /* symmetric */

    final boolean symmetricOrg = this.impl.getSymmetric();

    this.impl.setSymmetric(false);
    other.setSymmetric(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSymmetric(true);
    other.setSymmetric(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSymmetric(true);
    other.setSymmetric(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSymmetric(symmetricOrg);
    other.setSymmetric(symmetricOrg);

    /* willingness */

    final int willingnessOrg = this.impl.getWillingness().getValue();

    this.impl.setWillingness(Willingness.LOW.getValue());
    other.setWillingness(Willingness.DEFAULT.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setWillingness(Willingness.DEFAULT.getValue());
    other.setWillingness(Willingness.LOW.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setWillingness(Willingness.LOW.getValue());
    other.setWillingness(Willingness.LOW.getValue());
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setWillingness(willingnessOrg);
    other.setWillingness(willingnessOrg);

    /* isMultiPointRelay */

    final boolean isMultiPointRelayOrg = this.impl.getMultiPointRelay();

    this.impl.setIsMultiPointRelay(false);
    other.setIsMultiPointRelay(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIsMultiPointRelay(true);
    other.setIsMultiPointRelay(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIsMultiPointRelay(true);
    other.setIsMultiPointRelay(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIsMultiPointRelay(isMultiPointRelayOrg);
    other.setIsMultiPointRelay(isMultiPointRelayOrg);

    /* wasMultiPointRelay */

    final boolean wasMultiPointRelayOrg = this.impl.wasMultiPointRelay();

    this.impl.setWasMultiPointRelay(false);
    other.setWasMultiPointRelay(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setWasMultiPointRelay(true);
    other.setWasMultiPointRelay(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setWasMultiPointRelay(true);
    other.setWasMultiPointRelay(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setWasMultiPointRelay(wasMultiPointRelayOrg);
    other.setWasMultiPointRelay(wasMultiPointRelayOrg);

    /* multiPointRelaySelector */

    final boolean multiPointRelaySelectorOrg = this.impl.getMultiPointRelaySelector();

    this.impl.setMultiPointRelaySelector(false);
    other.setMultiPointRelaySelector(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMultiPointRelaySelector(true);
    other.setMultiPointRelaySelector(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMultiPointRelaySelector(true);
    other.setMultiPointRelaySelector(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMultiPointRelaySelector(multiPointRelaySelectorOrg);
    other.setMultiPointRelaySelector(multiPointRelaySelectorOrg);

    /* skip */

    final boolean skipOrg = this.impl.getSkip();

    this.impl.setSkip(false);
    other.setSkip(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSkip(true);
    other.setSkip(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSkip(true);
    other.setSkip(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSkip(skipOrg);
    other.setSkip(skipOrg);

    /* neighbor2nocov */

    final int neighbor2nocovOrg = this.impl.getNeighbor2nocov();

    this.impl.setNeighbor2nocov(1);
    other.setNeighbor2nocov(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNeighbor2nocov(2);
    other.setNeighbor2nocov(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNeighbor2nocov(1);
    other.setNeighbor2nocov(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNeighbor2nocov(neighbor2nocovOrg);
    other.setNeighbor2nocov(neighbor2nocovOrg);

    /* linkcount */

    final int linkcountOrg = this.impl.getLinkcount();

    this.impl.setLinkcount(1);
    other.setLinkcount(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLinkcount(2);
    other.setLinkcount(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLinkcount(1);
    other.setLinkcount(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLinkcount(linkcountOrg);
    other.setLinkcount(linkcountOrg);

    /* twoHopNeighborCount */

    final int twoHopNeighborCountOrg = this.impl.getTwoHopNeighborCount();

    this.impl.setTwoHopNeighborCount(1);
    other.setTwoHopNeighborCount(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTwoHopNeighborCount(2);
    other.setTwoHopNeighborCount(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTwoHopNeighborCount(1);
    other.setTwoHopNeighborCount(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTwoHopNeighborCount(twoHopNeighborCountOrg);
    other.setTwoHopNeighborCount(twoHopNeighborCountOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoNeighborsEntryBase other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoNeighborsEntryBase();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setIsMultiPointRelay(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1118777225)));

    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setIpAddress(ipAddress);
    this.impl.setSymmetric(true);
    this.impl.setWillingness(Willingness.ALWAYS.getValue());
    this.impl.setIsMultiPointRelay(true);
    this.impl.setWasMultiPointRelay(true);
    this.impl.setMultiPointRelaySelector(true);
    this.impl.setSkip(true);
    this.impl.setNeighbor2nocov(11);
    this.impl.setLinkcount(12);
    this.impl.setTwoHopNeighborCount(32);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1631203563)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}