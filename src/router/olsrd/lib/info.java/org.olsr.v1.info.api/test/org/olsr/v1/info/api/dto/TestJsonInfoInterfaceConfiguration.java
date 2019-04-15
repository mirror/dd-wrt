package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoInterfaceConfiguration {
  private JsonInfoInterfaceConfiguration impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoInterfaceConfiguration();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getIpv4Broadcast(), equalTo(""));
    assertThat(this.impl.getIpv6Multicast(), equalTo(""));
    assertThat(this.impl.getIpv4Source(), equalTo(""));
    assertThat(this.impl.getIpv6Source(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getIpv6SourcePrefixLength()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getMode(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getWeightValue()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getWeightFixed()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getHello(), notNullValue());
    assertThat(this.impl.getTc(), notNullValue());
    assertThat(this.impl.getMid(), notNullValue());
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(this.impl.getLinkQualityMultipliers(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getLinkQualityMultipliers().size()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getLinkQualityMultipliersCount()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getAutoDetectChanges()), equalTo(Boolean.FALSE));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    final InetAddress addr4 = InetAddress.getByName("127.0.0.4");
    final JsonInfoMessageParameters hello = new JsonInfoMessageParameters();
    hello.setEmissionInterval(1.0);
    final JsonInfoMessageParameters tc = new JsonInfoMessageParameters();
    tc.setEmissionInterval(2.0);
    final JsonInfoMessageParameters mid = new JsonInfoMessageParameters();
    mid.setEmissionInterval(3.0);
    final JsonInfoMessageParameters hna = new JsonInfoMessageParameters();
    hna.setEmissionInterval(4.0);
    final Set<JsonInfoLinkQualityMultiplier> linkQualityMultipliers = new TreeSet<>();
    final JsonInfoLinkQualityMultiplier entry = new JsonInfoLinkQualityMultiplier();
    entry.setMultiplier(1.0);
    linkQualityMultipliers.add(entry);

    this.impl.setIpv4Broadcast(addr1);
    this.impl.setIpv6Multicast(addr2);
    this.impl.setIpv4Source(addr3);
    this.impl.setIpv6Source(addr4);
    this.impl.setIpv6SourcePrefixLength(1);
    this.impl.setMode("mode");
    this.impl.setWeightValue(2);
    this.impl.setWeightFixed(true);
    this.impl.setHello(hello);
    this.impl.setTc(tc);
    this.impl.setMid(mid);
    this.impl.setHna(hna);
    this.impl.setLinkQualityMultipliers(linkQualityMultipliers);
    this.impl.setLinkQualityMultipliersCount(3);
    this.impl.setAutoDetectChanges(true);

    /* get */
    assertThat(this.impl.getIpv4Broadcast(), equalTo(addr1.getHostAddress()));
    assertThat(this.impl.getIpv6Multicast(), equalTo(addr2.getHostAddress()));
    assertThat(this.impl.getIpv4Source(), equalTo(addr3.getHostAddress()));
    assertThat(this.impl.getIpv6Source(), equalTo(addr4.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getIpv6SourcePrefixLength()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getMode(), equalTo("mode"));
    assertThat(Integer.valueOf(this.impl.getWeightValue()), equalTo(Integer.valueOf(2)));
    assertThat(Boolean.valueOf(this.impl.getWeightFixed()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getHello(), equalTo(hello));
    assertThat(this.impl.getTc(), equalTo(tc));
    assertThat(this.impl.getMid(), equalTo(mid));
    assertThat(this.impl.getHna(), equalTo(hna));
    assertThat(this.impl.getLinkQualityMultipliers(), equalTo(linkQualityMultipliers));
    assertThat(Integer.valueOf(this.impl.getLinkQualityMultipliers().size()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getLinkQualityMultipliersCount()), equalTo(Integer.valueOf(3)));
    assertThat(Boolean.valueOf(this.impl.getAutoDetectChanges()), equalTo(Boolean.TRUE));

    this.impl.setLinkQualityMultipliers(null);
    assertThat(this.impl.getLinkQualityMultipliers(), notNullValue());
  }

  @Test(timeout = 8000)
  public void testEquals() throws UnknownHostException {
    boolean r;
    JsonInfoInterfaceConfiguration other;
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final JsonInfoMessageParameters mp = new JsonInfoMessageParameters();
    mp.setEmissionInterval(123.321);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoInterfaceConfiguration();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* ipv4Broadcast */

    String savedIp = this.impl.getIpv4Broadcast();

    this.impl.setIpv4Broadcast(null);
    other.setIpv4Broadcast(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv4Broadcast(null);
    other.setIpv4Broadcast(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv4Broadcast(addr);
    other.setIpv4Broadcast(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv4Broadcast(addr);
    other.setIpv4Broadcast(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv4Broadcast(InetAddress.getByName(savedIp));
    other.setIpv4Broadcast(InetAddress.getByName(savedIp));

    /* ipv6Multicast */

    savedIp = this.impl.getIpv6Multicast();

    this.impl.setIpv6Multicast(null);
    other.setIpv6Multicast(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv6Multicast(null);
    other.setIpv6Multicast(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6Multicast(addr);
    other.setIpv6Multicast(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6Multicast(addr);
    other.setIpv6Multicast(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv6Multicast(InetAddress.getByName(savedIp));
    other.setIpv6Multicast(InetAddress.getByName(savedIp));

    /* ipv4Source */

    savedIp = this.impl.getIpv4Source();

    this.impl.setIpv4Source(null);
    other.setIpv4Source(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv4Source(null);
    other.setIpv4Source(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv4Source(addr);
    other.setIpv4Source(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv4Source(addr);
    other.setIpv4Source(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv4Source(InetAddress.getByName(savedIp));
    other.setIpv4Source(InetAddress.getByName(savedIp));

    /* ipv6Source */

    savedIp = this.impl.getIpv6Source();

    this.impl.setIpv6Source(null);
    other.setIpv6Source(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv6Source(null);
    other.setIpv6Source(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6Source(addr);
    other.setIpv6Source(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6Source(addr);
    other.setIpv6Source(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv6Source(InetAddress.getByName(savedIp));
    other.setIpv6Source(InetAddress.getByName(savedIp));

    /* ipv6SourcePrefixLength */

    int savedInt = this.impl.getIpv6SourcePrefixLength();

    this.impl.setIpv6SourcePrefixLength(1);
    other.setIpv6SourcePrefixLength(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6SourcePrefixLength(3);
    other.setIpv6SourcePrefixLength(3);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv6SourcePrefixLength(savedInt);
    other.setIpv6SourcePrefixLength(savedInt);

    /* mode */

    final String savedString = this.impl.getMode();

    this.impl.setMode(null);
    other.setMode(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMode(null);
    other.setMode("mode");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMode("mode");
    other.setMode(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMode("mode");
    other.setMode("mode");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMode(savedString);
    other.setMode(savedString);

    /* weightValue */

    savedInt = this.impl.getWeightValue();

    this.impl.setWeightValue(1);
    other.setWeightValue(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setWeightValue(3);
    other.setWeightValue(3);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setWeightValue(savedInt);
    other.setWeightValue(savedInt);

    /* weightFixed */

    boolean savedBoolean = this.impl.getWeightFixed();

    this.impl.setWeightFixed(false);
    other.setWeightFixed(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setWeightFixed(true);
    other.setWeightFixed(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setWeightFixed(savedBoolean);
    other.setWeightFixed(savedBoolean);

    /* hello */

    JsonInfoMessageParameters savedMP = this.impl.getHello();

    this.impl.setHello(null);
    other.setHello(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHello(null);
    other.setHello(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHello(mp);
    other.setHello(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHello(mp);
    other.setHello(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHello(savedMP);
    other.setHello(savedMP);

    /* tc */

    savedMP = this.impl.getTc();

    this.impl.setTc(null);
    other.setTc(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTc(null);
    other.setTc(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTc(mp);
    other.setTc(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTc(mp);
    other.setTc(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTc(savedMP);
    other.setTc(savedMP);

    /* mid */

    savedMP = this.impl.getMid();

    this.impl.setMid(null);
    other.setMid(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMid(null);
    other.setMid(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMid(mp);
    other.setMid(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMid(mp);
    other.setMid(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMid(savedMP);
    other.setMid(savedMP);

    /* hna */

    savedMP = this.impl.getHna();

    this.impl.setHna(null);
    other.setHna(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHna(null);
    other.setHna(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHna(mp);
    other.setHna(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHna(mp);
    other.setHna(mp);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHna(savedMP);
    other.setHna(savedMP);

    /* linkQualityMultipliers */

    final Set<JsonInfoLinkQualityMultiplier> lqm = new TreeSet<>();
    final JsonInfoLinkQualityMultiplier e = new JsonInfoLinkQualityMultiplier();
    e.setMultiplier(10.1);
    lqm.add(e);

    final Set<JsonInfoLinkQualityMultiplier> savedLQM = this.impl.getLinkQualityMultipliers();

    this.impl.setLinkQualityMultipliers(null);
    other.setLinkQualityMultipliers(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQualityMultipliers(null);
    other.setLinkQualityMultipliers(lqm);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQualityMultipliers(lqm);
    other.setLinkQualityMultipliers(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQualityMultipliers(lqm);
    other.setLinkQualityMultipliers(lqm);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQualityMultipliers(savedLQM);
    other.setLinkQualityMultipliers(savedLQM);

    /* linkQualityMultipliersCount */

    savedInt = this.impl.getLinkQualityMultipliersCount();

    this.impl.setLinkQualityMultipliersCount(1);
    other.setLinkQualityMultipliersCount(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQualityMultipliersCount(3);
    other.setLinkQualityMultipliersCount(3);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQualityMultipliersCount(savedInt);
    other.setLinkQualityMultipliersCount(savedInt);

    /* autoDetectChanges */

    savedBoolean = this.impl.getAutoDetectChanges();

    this.impl.setAutoDetectChanges(false);
    other.setAutoDetectChanges(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAutoDetectChanges(true);
    other.setAutoDetectChanges(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAutoDetectChanges(savedBoolean);
    other.setAutoDetectChanges(savedBoolean);
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(123757567)));

    /* set */
    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");
    final InetAddress addr3 = InetAddress.getByName("127.0.0.3");
    final InetAddress addr4 = InetAddress.getByName("127.0.0.4");
    final JsonInfoMessageParameters hello = new JsonInfoMessageParameters();
    hello.setEmissionInterval(1.0);
    final JsonInfoMessageParameters tc = new JsonInfoMessageParameters();
    tc.setEmissionInterval(2.0);
    final JsonInfoMessageParameters mid = new JsonInfoMessageParameters();
    mid.setEmissionInterval(3.0);
    final JsonInfoMessageParameters hna = new JsonInfoMessageParameters();
    hna.setEmissionInterval(4.0);
    final Set<JsonInfoLinkQualityMultiplier> linkQualityMultipliers = new TreeSet<>();
    final JsonInfoLinkQualityMultiplier entry = new JsonInfoLinkQualityMultiplier();
    entry.setMultiplier(1.0);
    linkQualityMultipliers.add(entry);

    this.impl.setIpv4Broadcast(addr1);
    this.impl.setIpv6Multicast(addr2);
    this.impl.setIpv4Source(addr3);
    this.impl.setIpv6Source(addr4);
    this.impl.setIpv6SourcePrefixLength(1);
    this.impl.setMode("mode");
    this.impl.setWeightValue(2);
    this.impl.setWeightFixed(true);
    this.impl.setHello(hello);
    this.impl.setTc(tc);
    this.impl.setMid(mid);
    this.impl.setHna(hna);
    this.impl.setLinkQualityMultipliers(linkQualityMultipliers);
    this.impl.setLinkQualityMultipliersCount(3);
    this.impl.setAutoDetectChanges(true);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-867257189)));

    this.impl.setLinkQualityMultipliers(null);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-934240998)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}