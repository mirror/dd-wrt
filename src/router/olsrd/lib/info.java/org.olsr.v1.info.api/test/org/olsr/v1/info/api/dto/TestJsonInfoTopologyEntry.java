package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoTopologyEntry {
  private JsonInfoTopologyEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoTopologyEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getLastHopIP(), equalTo(""));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getRefCount()), equalTo(Long.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMsgSeq()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMsgHops()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getHops()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getAnsn()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTcIgnored()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getErrSeq()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getErrSeqValid()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getDestinationIP(), equalTo(""));
    assertThat(Double.valueOf(this.impl.getTcEdgeCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Integer.valueOf(this.impl.getAnsnEdge()), equalTo(Integer.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getLinkQuality()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getNeighborLinkQuality()), equalTo(Double.valueOf(0.0)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setLastHopIP(ipAddress);
    this.impl.setPathCost(1.234);
    this.impl.setValidityTime(2);
    this.impl.setRefCount(3);
    this.impl.setMsgSeq(4);
    this.impl.setMsgHops(5);
    this.impl.setHops(6);
    this.impl.setAnsn(7);
    this.impl.setTcIgnored(8);
    this.impl.setErrSeq(9);
    this.impl.setErrSeqValid(true);
    this.impl.setDestinationIP(ipAddress);
    this.impl.setTcEdgeCost(10.987);
    this.impl.setAnsnEdge(11);
    this.impl.setLinkQuality(1.1);
    this.impl.setNeighborLinkQuality(2.2);

    /* get */
    assertThat(this.impl.getLastHopIP(), equalTo(ipAddress.getHostAddress()));
    assertThat(Double.valueOf(this.impl.getPathCost()), equalTo(Double.valueOf(1.234)));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(2)));
    assertThat(Long.valueOf(this.impl.getRefCount()), equalTo(Long.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getMsgSeq()), equalTo(Integer.valueOf(4)));
    assertThat(Integer.valueOf(this.impl.getMsgHops()), equalTo(Integer.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getHops()), equalTo(Integer.valueOf(6)));
    assertThat(Integer.valueOf(this.impl.getAnsn()), equalTo(Integer.valueOf(7)));
    assertThat(Integer.valueOf(this.impl.getTcIgnored()), equalTo(Integer.valueOf(8)));
    assertThat(Integer.valueOf(this.impl.getErrSeq()), equalTo(Integer.valueOf(9)));
    assertThat(Boolean.valueOf(this.impl.getErrSeqValid()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getDestinationIP(), equalTo(ipAddress.getHostAddress()));
    assertThat(Double.valueOf(this.impl.getTcEdgeCost()), equalTo(Double.valueOf(10.987)));
    assertThat(Integer.valueOf(this.impl.getAnsnEdge()), equalTo(Integer.valueOf(11)));
    assertThat(Double.valueOf(this.impl.getLinkQuality()), equalTo(Double.valueOf(1.1)));
    assertThat(Double.valueOf(this.impl.getNeighborLinkQuality()), equalTo(Double.valueOf(2.2)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoTopologyEntry other = new JsonInfoTopologyEntry();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* lastHopIP */

    String ipOrg = this.impl.getLastHopIP();

    this.impl.setLastHopIP(null);
    other.setLastHopIP(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLastHopIP(originator2);
    other.setLastHopIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLastHopIP(originator1);
    other.setLastHopIP(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLastHopIP(originator1);
    other.setLastHopIP(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLastHopIP(InetAddress.getByName(ipOrg));
    other.setLastHopIP(InetAddress.getByName(ipOrg));

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

    /* refCount */

    final long refCountOrg = this.impl.getRefCount();

    this.impl.setRefCount(1);
    other.setRefCount(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setRefCount(2);
    other.setRefCount(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setRefCount(1);
    other.setRefCount(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setRefCount(refCountOrg);
    other.setRefCount(refCountOrg);

    /* msgSeq */

    final int msgSeqOrg = this.impl.getMsgSeq();

    this.impl.setMsgSeq(1);
    other.setMsgSeq(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMsgSeq(2);
    other.setMsgSeq(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMsgSeq(1);
    other.setMsgSeq(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMsgSeq(msgSeqOrg);
    other.setMsgSeq(msgSeqOrg);

    /* msgHops */

    final int msgHopsOrg = this.impl.getMsgHops();

    this.impl.setMsgHops(1);
    other.setMsgHops(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMsgHops(2);
    other.setMsgHops(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMsgHops(1);
    other.setMsgHops(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMsgHops(msgHopsOrg);
    other.setMsgHops(msgHopsOrg);

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

    /* ansn */

    final int ansnOrg = this.impl.getAnsn();

    this.impl.setAnsn(1);
    other.setAnsn(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAnsn(2);
    other.setAnsn(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAnsn(1);
    other.setAnsn(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAnsn(ansnOrg);
    other.setAnsn(ansnOrg);

    /* tcIgnored */

    final int tcIgnoredOrg = this.impl.getTcIgnored();

    this.impl.setTcIgnored(1);
    other.setTcIgnored(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTcIgnored(2);
    other.setTcIgnored(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTcIgnored(1);
    other.setTcIgnored(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTcIgnored(tcIgnoredOrg);
    other.setTcIgnored(tcIgnoredOrg);

    /* errSeq */

    final int errSeqOrg = this.impl.getErrSeq();

    this.impl.setErrSeq(1);
    other.setErrSeq(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setErrSeq(2);
    other.setErrSeq(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setErrSeq(1);
    other.setErrSeq(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setErrSeq(errSeqOrg);
    other.setErrSeq(errSeqOrg);

    /* errSeqValid */

    final boolean errSeqValidOrg = this.impl.getErrSeqValid();

    this.impl.setErrSeqValid(false);
    other.setErrSeqValid(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setErrSeqValid(true);
    other.setErrSeqValid(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setErrSeqValid(false);
    other.setErrSeqValid(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setErrSeqValid(errSeqValidOrg);
    other.setErrSeqValid(errSeqValidOrg);

    /* destinationIP */

    ipOrg = this.impl.getDestinationIP();

    this.impl.setDestinationIP(null);
    other.setDestinationIP(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestinationIP(originator2);
    other.setDestinationIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestinationIP(originator1);
    other.setDestinationIP(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestinationIP(originator1);
    other.setDestinationIP(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestinationIP(InetAddress.getByName(ipOrg));
    other.setDestinationIP(InetAddress.getByName(ipOrg));

    /* tcEdgeCost */

    doubleOrg = this.impl.getTcEdgeCost();

    this.impl.setTcEdgeCost(1);
    other.setTcEdgeCost(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setTcEdgeCost(2);
    other.setTcEdgeCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setTcEdgeCost(1);
    other.setTcEdgeCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setTcEdgeCost(doubleOrg);
    other.setTcEdgeCost(doubleOrg);

    /* ansnEdge */

    final int ansnEdgeOrg = this.impl.getAnsnEdge();

    this.impl.setAnsnEdge(1);
    other.setAnsnEdge(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAnsnEdge(2);
    other.setAnsnEdge(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAnsnEdge(1);
    other.setAnsnEdge(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAnsnEdge(ansnEdgeOrg);
    other.setAnsnEdge(ansnEdgeOrg);

    /* linkQuality */

    final double linkQualityOrg = this.impl.getLinkQuality();

    this.impl.setLinkQuality(1.0);
    other.setLinkQuality(2.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(-1)));

    this.impl.setLinkQuality(2.0);
    other.setLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(1)));

    this.impl.setLinkQuality(1.0);
    other.setLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(0)));

    this.impl.setLinkQuality(linkQualityOrg);
    other.setLinkQuality(linkQualityOrg);

    /* neighborLinkQuality */

    final double neighborLinkQualityOrg = this.impl.getNeighborLinkQuality();

    this.impl.setNeighborLinkQuality(1.0);
    other.setNeighborLinkQuality(2.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(-1)));

    this.impl.setNeighborLinkQuality(2.0);
    other.setNeighborLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(1)));

    this.impl.setNeighborLinkQuality(1.0);
    other.setNeighborLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(0)));

    this.impl.setNeighborLinkQuality(neighborLinkQualityOrg);
    other.setNeighborLinkQuality(neighborLinkQualityOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoTopologyEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoTopologyEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setAnsn(11);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1518037428)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setLastHopIP(ipAddress);
    this.impl.setPathCost(1);
    this.impl.setValidityTime(2);
    this.impl.setRefCount(3);
    this.impl.setMsgSeq(4);
    this.impl.setMsgHops(5);
    this.impl.setHops(6);
    this.impl.setAnsn(7);
    this.impl.setTcIgnored(8);
    this.impl.setErrSeq(9);
    this.impl.setErrSeqValid(true);
    this.impl.setDestinationIP(ipAddress);
    this.impl.setTcEdgeCost(10);
    this.impl.setAnsnEdge(11);
    this.impl.setLinkQuality(1.1);
    this.impl.setNeighborLinkQuality(2.2);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1136083489)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}