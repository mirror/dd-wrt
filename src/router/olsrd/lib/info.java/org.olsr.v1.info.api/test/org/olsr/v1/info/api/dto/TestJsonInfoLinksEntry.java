package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoLinksEntry {
  private JsonInfoLinksEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoLinksEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getLocalIP(), equalTo(""));
    assertThat(this.impl.getRemoteIP(), equalTo(""));
    assertThat(this.impl.getOlsrInterface(), equalTo(""));
    assertThat(this.impl.getIfName(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getSymmetryTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getAsymmetryTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getVtime()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getCurrentLinkStatus(), equalTo(""));
    assertThat(this.impl.getPreviousLinkStatus(), equalTo(""));
    assertThat(Double.valueOf(this.impl.getHysteresis()), equalTo(Double.valueOf(0.0)));
    assertThat(Boolean.valueOf(this.impl.getPending()), equalTo(Boolean.FALSE));
    assertThat(Long.valueOf(this.impl.getLostLinkTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getHelloTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getLastHelloTime()), equalTo(Long.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getSeqnoValid()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getSeqno()), equalTo(Integer.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getLossHelloInterval()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getLossTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getLossMultiplier()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getLinkCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Double.valueOf(this.impl.getLinkQuality()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getNeighborLinkQuality()), equalTo(Double.valueOf(0.0)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    this.impl.setLocalIP(addr1);
    this.impl.setRemoteIP(addr2);
    this.impl.setOlsrInterface("olsrInterface");
    this.impl.setIfName("ifName");
    this.impl.setValidityTime(1);
    this.impl.setSymmetryTime(2);
    this.impl.setAsymmetryTime(3);
    this.impl.setVtime(4);
    this.impl.setCurrentLinkStatus("currentLinkStatus");
    this.impl.setPreviousLinkStatus("previousLinkStatus");
    this.impl.setHysteresis(1.1);
    this.impl.setPending(true);
    this.impl.setLostLinkTime(5);
    this.impl.setHelloTime(6);
    this.impl.setLastHelloTime(7);
    this.impl.setSeqnoValid(true);
    this.impl.setSeqno(8);
    this.impl.setLossHelloInterval(9);
    this.impl.setLossTime(10);
    this.impl.setLossMultiplier(11);
    this.impl.setLinkCost(12.21);
    this.impl.setLinkQuality(2.2);
    this.impl.setNeighborLinkQuality(3.3);

    /* get */
    assertThat(this.impl.getLocalIP(), equalTo(addr1.getHostAddress()));
    assertThat(this.impl.getRemoteIP(), equalTo(addr2.getHostAddress()));
    assertThat(this.impl.getOlsrInterface(), equalTo("olsrInterface"));
    assertThat(this.impl.getIfName(), equalTo("ifName"));
    assertThat(Long.valueOf(this.impl.getValidityTime()), equalTo(Long.valueOf(1)));
    assertThat(Long.valueOf(this.impl.getSymmetryTime()), equalTo(Long.valueOf(2)));
    assertThat(Long.valueOf(this.impl.getAsymmetryTime()), equalTo(Long.valueOf(3)));
    assertThat(Long.valueOf(this.impl.getVtime()), equalTo(Long.valueOf(4)));
    assertThat(this.impl.getCurrentLinkStatus(), equalTo("currentLinkStatus"));
    assertThat(this.impl.getPreviousLinkStatus(), equalTo("previousLinkStatus"));
    assertThat(Double.valueOf(this.impl.getHysteresis()), equalTo(Double.valueOf(1.1)));
    assertThat(Boolean.valueOf(this.impl.getPending()), equalTo(Boolean.TRUE));
    assertThat(Long.valueOf(this.impl.getLostLinkTime()), equalTo(Long.valueOf(5)));
    assertThat(Long.valueOf(this.impl.getHelloTime()), equalTo(Long.valueOf(6)));
    assertThat(Long.valueOf(this.impl.getLastHelloTime()), equalTo(Long.valueOf(7)));
    assertThat(Boolean.valueOf(this.impl.getSeqnoValid()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getSeqno()), equalTo(Integer.valueOf(8)));
    assertThat(Long.valueOf(this.impl.getLossHelloInterval()), equalTo(Long.valueOf(9)));
    assertThat(Long.valueOf(this.impl.getLossTime()), equalTo(Long.valueOf(10)));
    assertThat(Long.valueOf(this.impl.getLossMultiplier()), equalTo(Long.valueOf(11)));
    assertThat(Double.valueOf(this.impl.getLinkCost()), equalTo(Double.valueOf(12.21)));
    assertThat(Double.valueOf(this.impl.getLinkQuality()), equalTo(Double.valueOf(2.2)));
    assertThat(Double.valueOf(this.impl.getNeighborLinkQuality()), equalTo(Double.valueOf(3.3)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoLinksEntry other = new JsonInfoLinksEntry();
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* localIP */

    String ipOrg = this.impl.getLocalIP();

    this.impl.setLocalIP(null);
    other.setLocalIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLocalIP(null);
    other.setLocalIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLocalIP(addr);
    other.setLocalIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLocalIP(addr);
    other.setLocalIP(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLocalIP(addr2);
    other.setLocalIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLocalIP(addr);
    other.setLocalIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLocalIP(InetAddress.getByName(ipOrg));
    other.setLocalIP(InetAddress.getByName(ipOrg));

    /* remoteIP */

    ipOrg = this.impl.getRemoteIP();

    this.impl.setRemoteIP(null);
    other.setRemoteIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRemoteIP(null);
    other.setRemoteIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRemoteIP(addr);
    other.setRemoteIP(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRemoteIP(addr);
    other.setRemoteIP(addr2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRemoteIP(addr2);
    other.setRemoteIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRemoteIP(addr);
    other.setRemoteIP(addr);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRemoteIP(InetAddress.getByName(ipOrg));
    other.setRemoteIP(InetAddress.getByName(ipOrg));

    /* olsrInterface */

    String stringOrg = this.impl.getOlsrInterface();

    this.impl.setOlsrInterface(null);
    other.setOlsrInterface(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setOlsrInterface(null);
    other.setOlsrInterface("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setOlsrInterface("a");
    other.setOlsrInterface(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setOlsrInterface("a");
    other.setOlsrInterface("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setOlsrInterface("a1");
    other.setOlsrInterface("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setOlsrInterface("a2");
    other.setOlsrInterface("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setOlsrInterface(stringOrg);
    other.setOlsrInterface(stringOrg);

    /* ifName */

    stringOrg = this.impl.getIfName();

    this.impl.setIfName(null);
    other.setIfName(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIfName(null);
    other.setIfName("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIfName("a");
    other.setIfName(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIfName("a");
    other.setIfName("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIfName("a1");
    other.setIfName("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIfName("a2");
    other.setIfName("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIfName(stringOrg);
    other.setIfName(stringOrg);

    /* validityTime */

    long longOrg = this.impl.getValidityTime();

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

    this.impl.setValidityTime(longOrg);
    other.setValidityTime(longOrg);

    /* symmetryTime */

    longOrg = this.impl.getSymmetryTime();

    this.impl.setSymmetryTime(1);
    other.setSymmetryTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSymmetryTime(2);
    other.setSymmetryTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSymmetryTime(1);
    other.setSymmetryTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSymmetryTime(longOrg);
    other.setSymmetryTime(longOrg);

    /* asymmetryTime; */

    longOrg = this.impl.getAsymmetryTime();

    this.impl.setAsymmetryTime(1);
    other.setAsymmetryTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAsymmetryTime(2);
    other.setAsymmetryTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAsymmetryTime(1);
    other.setAsymmetryTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAsymmetryTime(longOrg);
    other.setAsymmetryTime(longOrg);

    /* vtime; */

    longOrg = this.impl.getVtime();

    this.impl.setVtime(1);
    other.setVtime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setVtime(2);
    other.setVtime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setVtime(1);
    other.setVtime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setVtime(longOrg);
    other.setVtime(longOrg);

    /* currentLinkStatus */

    stringOrg = this.impl.getCurrentLinkStatus();

    this.impl.setCurrentLinkStatus(null);
    other.setCurrentLinkStatus(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setCurrentLinkStatus(null);
    other.setCurrentLinkStatus("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setCurrentLinkStatus("a");
    other.setCurrentLinkStatus(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setCurrentLinkStatus("a");
    other.setCurrentLinkStatus("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setCurrentLinkStatus("a1");
    other.setCurrentLinkStatus("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setCurrentLinkStatus("a2");
    other.setCurrentLinkStatus("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setCurrentLinkStatus(stringOrg);
    other.setCurrentLinkStatus(stringOrg);

    /* previousLinkStatus */

    stringOrg = this.impl.getPreviousLinkStatus();

    this.impl.setPreviousLinkStatus(null);
    other.setPreviousLinkStatus(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPreviousLinkStatus(null);
    other.setPreviousLinkStatus("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPreviousLinkStatus("a");
    other.setPreviousLinkStatus(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPreviousLinkStatus("a");
    other.setPreviousLinkStatus("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPreviousLinkStatus("a1");
    other.setPreviousLinkStatus("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPreviousLinkStatus("a2");
    other.setPreviousLinkStatus("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPreviousLinkStatus(stringOrg);
    other.setPreviousLinkStatus(stringOrg);

    /* hysteresis */

    double doubleOrg = this.impl.getHysteresis();

    this.impl.setHysteresis(1.0);
    other.setHysteresis(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHysteresis(2.0);
    other.setHysteresis(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHysteresis(1.0);
    other.setHysteresis(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHysteresis(doubleOrg);
    other.setHysteresis(doubleOrg);

    /* pending */

    boolean booleanOrg = this.impl.getPending();

    this.impl.setPending(false);
    other.setPending(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPending(true);
    other.setPending(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPending(true);
    other.setPending(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPending(booleanOrg);
    other.setPending(booleanOrg);

    /* lostLinkTime */

    longOrg = this.impl.getLostLinkTime();

    this.impl.setLostLinkTime(1);
    other.setLostLinkTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLostLinkTime(2);
    other.setLostLinkTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLostLinkTime(1);
    other.setLostLinkTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLostLinkTime(longOrg);
    other.setLostLinkTime(longOrg);

    /* helloTime */

    longOrg = this.impl.getHelloTime();

    this.impl.setHelloTime(1);
    other.setHelloTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setHelloTime(2);
    other.setHelloTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setHelloTime(1);
    other.setHelloTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setHelloTime(longOrg);
    other.setHelloTime(longOrg);

    /* lastHelloTime */

    longOrg = this.impl.getLastHelloTime();

    this.impl.setLastHelloTime(1);
    other.setLastHelloTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLastHelloTime(2);
    other.setLastHelloTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLastHelloTime(1);
    other.setLastHelloTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLastHelloTime(longOrg);
    other.setLastHelloTime(longOrg);

    /* seqnoValid */

    booleanOrg = this.impl.getSeqnoValid();

    this.impl.setSeqnoValid(false);
    other.setSeqnoValid(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSeqnoValid(true);
    other.setSeqnoValid(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSeqnoValid(true);
    other.setSeqnoValid(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSeqnoValid(booleanOrg);
    other.setSeqnoValid(booleanOrg);

    /* seqno */

    final int intOrg = this.impl.getSeqno();

    this.impl.setSeqno(1);
    other.setSeqno(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSeqno(2);
    other.setSeqno(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSeqno(1);
    other.setSeqno(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSeqno(intOrg);
    other.setSeqno(intOrg);

    /* lossHelloInterval */

    longOrg = this.impl.getLossHelloInterval();

    this.impl.setLossHelloInterval(1);
    other.setLossHelloInterval(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLossHelloInterval(2);
    other.setLossHelloInterval(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLossHelloInterval(1);
    other.setLossHelloInterval(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLossHelloInterval(longOrg);
    other.setLossHelloInterval(longOrg);

    /* lossTime */

    longOrg = this.impl.getLossTime();

    this.impl.setLossTime(1);
    other.setLossTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLossTime(2);
    other.setLossTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLossTime(1);
    other.setLossTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLossTime(longOrg);
    other.setLossTime(longOrg);

    /* lossMultiplier */

    longOrg = this.impl.getLossMultiplier();

    this.impl.setLossMultiplier(1);
    other.setLossMultiplier(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLossMultiplier(2);
    other.setLossMultiplier(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLossMultiplier(1);
    other.setLossMultiplier(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLossMultiplier(longOrg);
    other.setLossMultiplier(longOrg);

    /* linkCost */

    doubleOrg = this.impl.getLinkCost();

    this.impl.setLinkCost(1);
    other.setLinkCost(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLinkCost(2);
    other.setLinkCost(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLinkCost(1);
    other.setLinkCost(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLinkCost(doubleOrg);
    other.setLinkCost(doubleOrg);

    /* linkQuality */

    doubleOrg = this.impl.getLinkQuality();

    this.impl.setLinkQuality(1.0);
    other.setLinkQuality(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLinkQuality(2.0);
    other.setLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLinkQuality(1.0);
    other.setLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLinkQuality(doubleOrg);
    other.setLinkQuality(doubleOrg);

    /* neighborLinkQuality */

    doubleOrg = this.impl.getNeighborLinkQuality();

    this.impl.setNeighborLinkQuality(1.0);
    other.setNeighborLinkQuality(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNeighborLinkQuality(2.0);
    other.setNeighborLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNeighborLinkQuality(1.0);
    other.setNeighborLinkQuality(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNeighborLinkQuality(doubleOrg);
    other.setNeighborLinkQuality(doubleOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoLinksEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoLinksEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setValidityTime(12);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1617130613)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    this.impl.setLocalIP(addr1);
    this.impl.setRemoteIP(addr2);
    this.impl.setOlsrInterface("olsrInterface");
    this.impl.setIfName("ifName");
    this.impl.setValidityTime(1);
    this.impl.setSymmetryTime(2);
    this.impl.setAsymmetryTime(3);
    this.impl.setVtime(4);
    this.impl.setCurrentLinkStatus("currentLinkStatus");
    this.impl.setPreviousLinkStatus("previousLinkStatus");
    this.impl.setHysteresis(1.1);
    this.impl.setPending(true);
    this.impl.setLostLinkTime(5);
    this.impl.setHelloTime(6);
    this.impl.setLastHelloTime(7);
    this.impl.setSeqnoValid(true);
    this.impl.setSeqno(8);
    this.impl.setLossHelloInterval(9);
    this.impl.setLossTime(10);
    this.impl.setLossMultiplier(11);
    this.impl.setLinkCost(12);
    this.impl.setLinkQuality(2.2);
    this.impl.setNeighborLinkQuality(3.3);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1938050407)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}