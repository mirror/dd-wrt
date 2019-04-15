package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoOlsrInterface {
  private JsonInfoOlsrInterface impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoOlsrInterface();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getUp()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getIpv4Address(), equalTo(""));
    assertThat(this.impl.getIpv4Netmask(), equalTo(""));
    assertThat(this.impl.getIpv4Broadcast(), equalTo(""));
    assertThat(this.impl.getMode(), equalTo(""));
    assertThat(this.impl.getIpv6Address(), equalTo(""));
    assertThat(this.impl.getIpv6Multicast(), equalTo(""));
    assertThat(this.impl.getIpAddress(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getEmulatedInterface()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getOlsrSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(this.impl.getSendSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(this.impl.getMetric()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMtu()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getFlags()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getIndex()), equalTo(Integer.valueOf(-1)));
    assertThat(Boolean.valueOf(this.impl.getWireless()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getName(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getSeqNum()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getMessageTimes(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getIcmpRedirectBackup()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getSpoofFilterBackup()), equalTo(Boolean.FALSE));
    assertThat(Long.valueOf(this.impl.getHelloEmissionInterval()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getValidityTimes(), notNullValue());
    assertThat(Long.valueOf(this.impl.getForwardingTimeout()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getSgwZeroBwTimeout()), equalTo(Long.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTtlIndex()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getImmediateSendTc()), equalTo(Boolean.FALSE));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    final InetAddress addr4 = InetAddress.getByName("127.0.0.4");
    final InetAddress addr5 = InetAddress.getByName("127.0.0.5");
    final InetAddress addr6 = InetAddress.getByName("127.0.0.6");
    final JsonInfoMessageTimes messageTimes = new JsonInfoMessageTimes();
    messageTimes.setHello(100);
    final JsonInfoMessageTimes validityTimes = new JsonInfoMessageTimes();
    validityTimes.setHello(200);

    this.impl.setUp(true);
    this.impl.setIpv4Address(addr1);
    this.impl.setIpv4Netmask(addr2);
    this.impl.setIpv4Broadcast(addr3);
    this.impl.setMode("mode");
    this.impl.setIpv6Address(addr4);
    this.impl.setIpv6Multicast(addr5);
    this.impl.setIpAddress(addr6);
    this.impl.setEmulatedInterface(true);
    this.impl.setOlsrSocket(1);
    this.impl.setSendSocket(2);
    this.impl.setMetric(3);
    this.impl.setMtu(4);
    this.impl.setFlags(5);
    this.impl.setIndex(6);
    this.impl.setWireless(true);
    this.impl.setName("name");
    this.impl.setSeqNum(7);
    this.impl.setMessageTimes(messageTimes);
    this.impl.setIcmpRedirectBackup(true);
    this.impl.setSpoofFilterBackup(true);
    this.impl.setHelloEmissionInterval(8);
    this.impl.setValidityTimes(validityTimes);
    this.impl.setForwardingTimeout(9);
    this.impl.setSgwZeroBwTimeout(10);
    this.impl.setTtlIndex(11);
    this.impl.setImmediateSendTc(true);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getUp()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getIpv4Address(), equalTo(addr1.getHostAddress()));
    assertThat(this.impl.getIpv4Netmask(), equalTo(addr2.getHostAddress()));
    assertThat(this.impl.getIpv4Broadcast(), equalTo(addr3.getHostAddress()));
    assertThat(this.impl.getMode(), equalTo("mode"));
    assertThat(this.impl.getIpv6Address(), equalTo(addr4.getHostAddress()));
    assertThat(this.impl.getIpv6Multicast(), equalTo(addr5.getHostAddress()));
    assertThat(this.impl.getIpAddress(), equalTo(addr6.getHostAddress()));
    assertThat(Boolean.valueOf(this.impl.getEmulatedInterface()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getOlsrSocket()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getSendSocket()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getMetric()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getMtu()), equalTo(Integer.valueOf(4)));
    assertThat(Integer.valueOf(this.impl.getFlags()), equalTo(Integer.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getIndex()), equalTo(Integer.valueOf(6)));
    assertThat(Boolean.valueOf(this.impl.getWireless()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getName(), equalTo("name"));
    assertThat(Integer.valueOf(this.impl.getSeqNum()), equalTo(Integer.valueOf(7)));
    assertThat(this.impl.getMessageTimes(), equalTo(messageTimes));
    assertThat(Boolean.valueOf(this.impl.getIcmpRedirectBackup()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getSpoofFilterBackup()), equalTo(Boolean.TRUE));
    assertThat(Long.valueOf(this.impl.getHelloEmissionInterval()), equalTo(Long.valueOf(8)));
    assertThat(this.impl.getValidityTimes(), equalTo(validityTimes));
    assertThat(Long.valueOf(this.impl.getForwardingTimeout()), equalTo(Long.valueOf(9)));
    assertThat(Long.valueOf(this.impl.getSgwZeroBwTimeout()), equalTo(Long.valueOf(10)));
    assertThat(Integer.valueOf(this.impl.getTtlIndex()), equalTo(Integer.valueOf(11)));
    assertThat(Boolean.valueOf(this.impl.getImmediateSendTc()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoOlsrInterface other = new JsonInfoOlsrInterface();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");
    final JsonInfoMessageTimes messageTimes = new JsonInfoMessageTimes();
    messageTimes.setHello(100);
    final JsonInfoMessageTimes validityTimes = new JsonInfoMessageTimes();
    validityTimes.setHello(200);

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    /* up */

    boolean booleanOrg = this.impl.getUp();

    this.impl.setUp(false);
    other.setUp(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUp(false);
    other.setUp(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUp(true);
    other.setUp(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUp(true);
    other.setUp(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUp(booleanOrg);
    other.setUp(booleanOrg);

    /* ipv4Address */

    String ipOrg = this.impl.getIpv4Address();

    this.impl.setIpv4Address(null);
    other.setIpv4Address(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Address(originator2);
    other.setIpv4Address(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv4Address(originator1);
    other.setIpv4Address(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Address(originator1);
    other.setIpv4Address(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv4Address(InetAddress.getByName(ipOrg));
    other.setIpv4Address(InetAddress.getByName(ipOrg));

    /* ipv4Netmask */

    ipOrg = this.impl.getIpv4Netmask();

    this.impl.setIpv4Netmask(null);
    other.setIpv4Netmask(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Netmask(originator2);
    other.setIpv4Netmask(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv4Netmask(originator1);
    other.setIpv4Netmask(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Netmask(originator1);
    other.setIpv4Netmask(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv4Netmask(InetAddress.getByName(ipOrg));
    other.setIpv4Netmask(InetAddress.getByName(ipOrg));

    /* ipv4Broadcast */

    ipOrg = this.impl.getIpv4Broadcast();

    this.impl.setIpv4Broadcast(null);
    other.setIpv4Broadcast(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Broadcast(originator2);
    other.setIpv4Broadcast(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv4Broadcast(originator1);
    other.setIpv4Broadcast(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv4Broadcast(originator1);
    other.setIpv4Broadcast(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv4Broadcast(InetAddress.getByName(ipOrg));
    other.setIpv4Broadcast(InetAddress.getByName(ipOrg));

    /* mode */

    String stringOrg = this.impl.getMode();

    this.impl.setMode(null);
    other.setMode("mode");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMode("mode");
    other.setMode(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMode("mode1");
    other.setMode("mode2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMode("mode");
    other.setMode("mode");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMode(stringOrg);
    other.setMode(stringOrg);

    /* ipv6Address */

    ipOrg = this.impl.getIpv6Address();

    this.impl.setIpv6Address(null);
    other.setIpv6Address(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv6Address(originator2);
    other.setIpv6Address(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv6Address(originator1);
    other.setIpv6Address(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv6Address(originator1);
    other.setIpv6Address(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv6Address(InetAddress.getByName(ipOrg));
    other.setIpv6Address(InetAddress.getByName(ipOrg));

    /* ipv6Multicast */

    ipOrg = this.impl.getIpv6Multicast();

    this.impl.setIpv6Multicast(null);
    other.setIpv6Multicast(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv6Multicast(originator2);
    other.setIpv6Multicast(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIpv6Multicast(originator1);
    other.setIpv6Multicast(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIpv6Multicast(originator1);
    other.setIpv6Multicast(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIpv6Multicast(InetAddress.getByName(ipOrg));
    other.setIpv6Multicast(InetAddress.getByName(ipOrg));

    /* ipAddress */

    ipOrg = this.impl.getIpAddress();

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

    this.impl.setIpAddress(InetAddress.getByName(ipOrg));
    other.setIpAddress(InetAddress.getByName(ipOrg));

    /* emulatedInterface */

    booleanOrg = this.impl.getEmulatedInterface();

    this.impl.setEmulatedInterface(false);
    other.setEmulatedInterface(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEmulatedInterface(false);
    other.setEmulatedInterface(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setEmulatedInterface(true);
    other.setEmulatedInterface(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setEmulatedInterface(true);
    other.setEmulatedInterface(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEmulatedInterface(booleanOrg);
    other.setEmulatedInterface(booleanOrg);

    /* olsrSocket */

    int intOrg = this.impl.getOlsrSocket();

    this.impl.setOlsrSocket(1);
    other.setOlsrSocket(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setOlsrSocket(2);
    other.setOlsrSocket(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setOlsrSocket(1);
    other.setOlsrSocket(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setOlsrSocket(intOrg);
    other.setOlsrSocket(intOrg);

    /* sendSocket */

    intOrg = this.impl.getSendSocket();

    this.impl.setSendSocket(1);
    other.setSendSocket(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSendSocket(2);
    other.setSendSocket(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSendSocket(1);
    other.setSendSocket(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSendSocket(intOrg);
    other.setSendSocket(intOrg);

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

    /* mtu */

    intOrg = this.impl.getMtu();

    this.impl.setMtu(1);
    other.setMtu(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMtu(2);
    other.setMtu(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMtu(1);
    other.setMtu(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMtu(intOrg);
    other.setMtu(intOrg);

    /* flags */

    intOrg = this.impl.getFlags();

    this.impl.setFlags(1);
    other.setFlags(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setFlags(2);
    other.setFlags(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setFlags(1);
    other.setFlags(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setFlags(intOrg);
    other.setFlags(intOrg);

    /* index */

    intOrg = this.impl.getIndex();

    this.impl.setIndex(1);
    other.setIndex(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIndex(2);
    other.setIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIndex(1);
    other.setIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIndex(intOrg);
    other.setIndex(intOrg);

    /* wireless */

    booleanOrg = this.impl.getWireless();

    this.impl.setWireless(false);
    other.setWireless(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setWireless(false);
    other.setWireless(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setWireless(true);
    other.setWireless(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setWireless(true);
    other.setWireless(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setWireless(booleanOrg);
    other.setWireless(booleanOrg);

    /* name */

    stringOrg = this.impl.getName();

    this.impl.setName(null);
    other.setName("name");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setName("name");
    other.setName(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setName("name1");
    other.setName("name2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setName("name");
    other.setName("name");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setName(stringOrg);
    other.setName(stringOrg);

    /* seqNum */

    intOrg = this.impl.getSeqNum();

    this.impl.setSeqNum(1);
    other.setSeqNum(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSeqNum(2);
    other.setSeqNum(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSeqNum(1);
    other.setSeqNum(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSeqNum(intOrg);
    other.setSeqNum(intOrg);

    /* messageTimes */

    JsonInfoMessageTimes timesOrg = this.impl.getMessageTimes();

    this.impl.setMessageTimes(null);
    other.setMessageTimes(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMessageTimes(messageTimes);
    other.setMessageTimes(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMessageTimes(null);
    other.setMessageTimes(messageTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMessageTimes(messageTimes);
    other.setMessageTimes(messageTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMessageTimes(messageTimes);
    other.setMessageTimes(validityTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMessageTimes(validityTimes);
    other.setMessageTimes(messageTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMessageTimes(timesOrg);
    other.setMessageTimes(timesOrg);

    /* icmpRedirectBackup */

    booleanOrg = this.impl.getIcmpRedirectBackup();

    this.impl.setIcmpRedirectBackup(false);
    other.setIcmpRedirectBackup(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIcmpRedirectBackup(false);
    other.setIcmpRedirectBackup(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIcmpRedirectBackup(true);
    other.setIcmpRedirectBackup(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIcmpRedirectBackup(true);
    other.setIcmpRedirectBackup(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIcmpRedirectBackup(booleanOrg);
    other.setIcmpRedirectBackup(booleanOrg);

    /* spoofFilterBackup */

    booleanOrg = this.impl.getSpoofFilterBackup();

    this.impl.setSpoofFilterBackup(false);
    other.setSpoofFilterBackup(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSpoofFilterBackup(false);
    other.setSpoofFilterBackup(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSpoofFilterBackup(true);
    other.setSpoofFilterBackup(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSpoofFilterBackup(true);
    other.setSpoofFilterBackup(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSpoofFilterBackup(booleanOrg);
    other.setSpoofFilterBackup(booleanOrg);

    /* helloEmissionInterval */

    long longOrg = this.impl.getHelloEmissionInterval();

    this.impl.setHelloEmissionInterval(1);
    other.setHelloEmissionInterval(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setHelloEmissionInterval(2);
    other.setHelloEmissionInterval(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setHelloEmissionInterval(1);
    other.setHelloEmissionInterval(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setHelloEmissionInterval(longOrg);
    other.setHelloEmissionInterval(longOrg);

    /* validityTimes */

    timesOrg = this.impl.getValidityTimes();

    this.impl.setValidityTimes(null);
    other.setValidityTimes(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setValidityTimes(validityTimes);
    other.setValidityTimes(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setValidityTimes(null);
    other.setValidityTimes(validityTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setValidityTimes(validityTimes);
    other.setValidityTimes(validityTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setValidityTimes(messageTimes);
    other.setValidityTimes(validityTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setValidityTimes(validityTimes);
    other.setValidityTimes(messageTimes);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setValidityTimes(timesOrg);
    other.setValidityTimes(timesOrg);

    /* forwardingTimeout */

    longOrg = this.impl.getForwardingTimeout();

    this.impl.setForwardingTimeout(1);
    other.setForwardingTimeout(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setForwardingTimeout(2);
    other.setForwardingTimeout(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setForwardingTimeout(1);
    other.setForwardingTimeout(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setForwardingTimeout(longOrg);
    other.setForwardingTimeout(longOrg);

    /* sgwZeroBwTimeout */

    longOrg = this.impl.getSgwZeroBwTimeout();

    this.impl.setSgwZeroBwTimeout(1);
    other.setSgwZeroBwTimeout(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setSgwZeroBwTimeout(2);
    other.setSgwZeroBwTimeout(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setSgwZeroBwTimeout(1);
    other.setSgwZeroBwTimeout(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setSgwZeroBwTimeout(longOrg);
    other.setSgwZeroBwTimeout(longOrg);

    /* ttlIndex */

    intOrg = this.impl.getTtlIndex();

    this.impl.setTtlIndex(1);
    other.setTtlIndex(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTtlIndex(2);
    other.setTtlIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTtlIndex(1);
    other.setTtlIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTtlIndex(intOrg);
    other.setTtlIndex(intOrg);

    /* immediateSendTc */

    booleanOrg = this.impl.getImmediateSendTc();

    this.impl.setImmediateSendTc(false);
    other.setImmediateSendTc(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setImmediateSendTc(false);
    other.setImmediateSendTc(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setImmediateSendTc(true);
    other.setImmediateSendTc(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setImmediateSendTc(true);
    other.setImmediateSendTc(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setImmediateSendTc(booleanOrg);
    other.setImmediateSendTc(booleanOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoOlsrInterface other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoOlsrInterface();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setFlags(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-372752406)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    final InetAddress addr4 = InetAddress.getByName("127.0.0.4");
    final InetAddress addr5 = InetAddress.getByName("127.0.0.5");
    final InetAddress addr6 = InetAddress.getByName("127.0.0.6");
    final JsonInfoMessageTimes messageTimes = new JsonInfoMessageTimes();
    messageTimes.setHello(100);
    final JsonInfoMessageTimes validityTimes = new JsonInfoMessageTimes();
    validityTimes.setHello(200);

    this.impl.setUp(true);
    this.impl.setIpv4Address(addr1);
    this.impl.setIpv4Netmask(addr2);
    this.impl.setIpv4Broadcast(addr3);
    this.impl.setMode("mode");
    this.impl.setIpv6Address(addr4);
    this.impl.setIpv6Multicast(addr5);
    this.impl.setIpAddress(addr6);
    this.impl.setEmulatedInterface(true);
    this.impl.setOlsrSocket(1);
    this.impl.setSendSocket(2);
    this.impl.setMetric(3);
    this.impl.setMtu(4);
    this.impl.setFlags(5);
    this.impl.setIndex(6);
    this.impl.setWireless(true);
    this.impl.setName("name");
    this.impl.setSeqNum(7);
    this.impl.setMessageTimes(messageTimes);
    this.impl.setIcmpRedirectBackup(true);
    this.impl.setSpoofFilterBackup(true);
    this.impl.setHelloEmissionInterval(8);
    this.impl.setValidityTimes(validityTimes);
    this.impl.setForwardingTimeout(9);
    this.impl.setSgwZeroBwTimeout(10);
    this.impl.setTtlIndex(11);
    this.impl.setImmediateSendTc(true);

    r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-825938964)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}