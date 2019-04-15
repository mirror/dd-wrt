package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigEntry {
  private JsonInfoConfigEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getConfigurationChecksum(), equalTo(""));
    assertThat(this.impl.getCli(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getCli().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getConfigurationFile(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getOlsrPort()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getDebugLevel()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getNoFork()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getPidFile(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getHostEmulation()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getIpVersion()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getAllowNoInt()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getTosValue()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getRtProto()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getRtTable(), notNullValue());
    assertThat(this.impl.getWillingness(), notNullValue());
    assertThat(this.impl.getFib(), notNullValue());
    assertThat(this.impl.getHysteresis(), notNullValue());
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getPollrate()), equalTo(Double.valueOf(0.0)));
    assertThat(Double.valueOf(this.impl.getNicChgsPollInt()), equalTo(Double.valueOf(0.0)));
    assertThat(Boolean.valueOf(this.impl.getClearScreen()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getTcRedundancy()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getMprCoverage()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getLinkQuality(), notNullValue());
    assertThat(Double.valueOf(this.impl.getMinTCVTime()), equalTo(Double.valueOf(0.0)));
    assertThat(Boolean.valueOf(this.impl.getSetIpForward()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getLockFile(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getUseNiit()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getSmartGateway(), notNullValue());
    assertThat(this.impl.getMainIp(), equalTo(""));
    assertThat(this.impl.getUnicastSourceIpAddress(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getSrcIpRoutes()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getMaxPrefixLength()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getIpSize()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getDelgw()), equalTo(Boolean.FALSE));
    assertThat(Double.valueOf(this.impl.getMaxSendMessageJitter()), equalTo(Double.valueOf(0.0)));
    assertThat(Integer.valueOf(this.impl.getExitValue()), equalTo(Integer.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getMaxTcValidTime()), equalTo(Double.valueOf(0.0)));
    assertThat(Integer.valueOf(this.impl.getNiit4to6InterfaceIndex()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getNiit6to4InterfaceIndex()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getHasIpv4Gateway()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getHasIpv6Gateway()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getIoctlSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(this.impl.getRouteNetlinkSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(this.impl.getRouteMonitorSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(this.impl.getRouteChangeSocket()), equalTo(Integer.valueOf(-1)));
    assertThat(Double.valueOf(this.impl.getLinkQualityNatThreshold()), equalTo(Double.valueOf(0.0)));
    assertThat(Long.valueOf(this.impl.getBrokenLinkCost()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getBrokenRouteCost()), equalTo(Long.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getIpcConnectMaxConnections()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getIpcConnectAllowed(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getIpcConnectAllowed().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getInterfaceDefaults(), notNullValue());
    assertThat(this.impl.getOs(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getStartTime()), equalTo(Long.valueOf(0)));

    /* set */
    final List<String> cli = new LinkedList<>();
    cli.add("-f");
    cli.add("/etc/olsrd/olsrd.conf");

    final JsonInfoConfigRtTable rtTable = new JsonInfoConfigRtTable();
    rtTable.setDefault(11);

    final JsonInfoConfigWillingness willingness = new JsonInfoConfigWillingness();
    willingness.setUpdateInterval(123.321);

    final JsonInfoConfigFib fib = new JsonInfoConfigFib();
    fib.setMetric("metric");

    final JsonInfoConfigHysteresis hysteresis = new JsonInfoConfigHysteresis();
    hysteresis.setScaling(231.65);

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hnaEntry.setDestinationPrefixLength(11);
    hna.add(hnaEntry);

    final JsonInfoConfigLinkQuality linkQuality = new JsonInfoConfigLinkQuality();
    linkQuality.setAlgorithm("algorithm");

    final JsonInfoConfigSgw smartGateway = new JsonInfoConfigSgw();
    smartGateway.setInstanceId("instanceId");

    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    final Set<JsonInfoConfigIpcAcl> ipcConnectAllowed = new TreeSet<>();
    final JsonInfoConfigIpcAcl acl1 = new JsonInfoConfigIpcAcl();
    ipcConnectAllowed.add(acl1);

    final JsonInfoInterfaceConfiguration interfaceDefaults = new JsonInfoInterfaceConfiguration();
    interfaceDefaults.setMode("mode");

    this.impl.setConfigurationChecksum("configurationChecksum");
    this.impl.setCli(cli);
    this.impl.setConfigurationFile("configurationFile");
    this.impl.setOlsrPort(1);
    this.impl.setDebugLevel(2);
    this.impl.setNoFork(true);
    this.impl.setPidFile("pidFile");
    this.impl.setHostEmulation(true);
    this.impl.setIpVersion(4);
    this.impl.setAllowNoInt(true);
    this.impl.setTosValue(5);
    this.impl.setRtProto(6);
    this.impl.setRtTable(rtTable);
    this.impl.setWillingness(willingness);
    this.impl.setFib(fib);
    this.impl.setHysteresis(hysteresis);
    this.impl.setHna(hna);
    this.impl.setPollrate(1.1);
    this.impl.setNicChgsPollInt(2.2);
    this.impl.setClearScreen(true);
    this.impl.setTcRedundancy(7);
    this.impl.setMprCoverage(8);
    this.impl.setLinkQuality(linkQuality);
    this.impl.setMinTCVTime(3.3);
    this.impl.setSetIpForward(true);
    this.impl.setLockFile("lockFile");
    this.impl.setUseNiit(true);
    this.impl.setSmartGateway(smartGateway);
    this.impl.setMainIp(addr1);
    this.impl.setUnicastSourceIpAddress(addr2);
    this.impl.setSrcIpRoutes(true);
    this.impl.setMaxPrefixLength(9);
    this.impl.setIpSize(10);
    this.impl.setDelgw(true);
    this.impl.setMaxSendMessageJitter(4.4);
    this.impl.setExitValue(11);
    this.impl.setMaxTcValidTime(5.5);
    this.impl.setNiit4to6InterfaceIndex(12);
    this.impl.setNiit6to4InterfaceIndex(13);
    this.impl.setHasIpv4Gateway(true);
    this.impl.setHasIpv6Gateway(true);
    this.impl.setIoctlSocket(14);
    this.impl.setRouteNetlinkSocket(15);
    this.impl.setRouteMonitorSocket(16);
    this.impl.setRouteChangeSocket(17);
    this.impl.setLinkQualityNatThreshold(6.6);
    this.impl.setBrokenLinkCost(18);
    this.impl.setBrokenRouteCost(19);
    this.impl.setIpcConnectMaxConnections(20);
    this.impl.setIpcConnectAllowed(ipcConnectAllowed);
    this.impl.setInterfaceDefaults(interfaceDefaults);
    this.impl.setOs("Linux");
    this.impl.setStartTime(21);

    /* get */
    assertThat(this.impl.getConfigurationChecksum(), equalTo("configurationChecksum"));
    assertThat(this.impl.getCli(), equalTo(cli));
    assertThat(Integer.valueOf(this.impl.getCli().size()), equalTo(Integer.valueOf(2)));
    assertThat(this.impl.getConfigurationFile(), equalTo("configurationFile"));
    assertThat(Integer.valueOf(this.impl.getOlsrPort()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getDebugLevel()), equalTo(Integer.valueOf(2)));
    assertThat(Boolean.valueOf(this.impl.getNoFork()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getPidFile(), equalTo("pidFile"));
    assertThat(Boolean.valueOf(this.impl.getHostEmulation()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getIpVersion()), equalTo(Integer.valueOf(4)));
    assertThat(Boolean.valueOf(this.impl.getAllowNoInt()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getTosValue()), equalTo(Integer.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getRtProto()), equalTo(Integer.valueOf(6)));
    assertThat(this.impl.getRtTable(), equalTo(rtTable));
    assertThat(this.impl.getWillingness(), equalTo(willingness));
    assertThat(this.impl.getFib(), equalTo(fib));
    assertThat(this.impl.getHysteresis(), equalTo(hysteresis));
    assertThat(this.impl.getHna(), equalTo(hna));
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(1)));
    assertThat(Double.valueOf(this.impl.getPollrate()), equalTo(Double.valueOf(1.1)));
    assertThat(Double.valueOf(this.impl.getNicChgsPollInt()), equalTo(Double.valueOf(2.2)));
    assertThat(Boolean.valueOf(this.impl.getClearScreen()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getTcRedundancy()), equalTo(Integer.valueOf(7)));
    assertThat(Integer.valueOf(this.impl.getMprCoverage()), equalTo(Integer.valueOf(8)));
    assertThat(this.impl.getLinkQuality(), equalTo(linkQuality));
    assertThat(Double.valueOf(this.impl.getMinTCVTime()), equalTo(Double.valueOf(3.3)));
    assertThat(Boolean.valueOf(this.impl.getSetIpForward()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getLockFile(), equalTo("lockFile"));
    assertThat(Boolean.valueOf(this.impl.getUseNiit()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getSmartGateway(), equalTo(smartGateway));
    assertThat(this.impl.getMainIp(), equalTo(addr1.getHostAddress()));
    assertThat(this.impl.getUnicastSourceIpAddress(), equalTo(addr2.getHostAddress()));
    assertThat(Boolean.valueOf(this.impl.getSrcIpRoutes()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getMaxPrefixLength()), equalTo(Integer.valueOf(9)));
    assertThat(Integer.valueOf(this.impl.getIpSize()), equalTo(Integer.valueOf(10)));
    assertThat(Boolean.valueOf(this.impl.getDelgw()), equalTo(Boolean.TRUE));
    assertThat(Double.valueOf(this.impl.getMaxSendMessageJitter()), equalTo(Double.valueOf(4.4)));
    assertThat(Integer.valueOf(this.impl.getExitValue()), equalTo(Integer.valueOf(11)));
    assertThat(Double.valueOf(this.impl.getMaxTcValidTime()), equalTo(Double.valueOf(5.5)));
    assertThat(Integer.valueOf(this.impl.getNiit4to6InterfaceIndex()), equalTo(Integer.valueOf(12)));
    assertThat(Integer.valueOf(this.impl.getNiit6to4InterfaceIndex()), equalTo(Integer.valueOf(13)));
    assertThat(Boolean.valueOf(this.impl.getHasIpv4Gateway()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getHasIpv6Gateway()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getIoctlSocket()), equalTo(Integer.valueOf(14)));
    assertThat(Integer.valueOf(this.impl.getRouteNetlinkSocket()), equalTo(Integer.valueOf(15)));
    assertThat(Integer.valueOf(this.impl.getRouteMonitorSocket()), equalTo(Integer.valueOf(16)));
    assertThat(Integer.valueOf(this.impl.getRouteChangeSocket()), equalTo(Integer.valueOf(17)));
    assertThat(Double.valueOf(this.impl.getLinkQualityNatThreshold()), equalTo(Double.valueOf(6.6)));
    assertThat(Long.valueOf(this.impl.getBrokenLinkCost()), equalTo(Long.valueOf(18)));
    assertThat(Long.valueOf(this.impl.getBrokenRouteCost()), equalTo(Long.valueOf(19)));
    assertThat(Integer.valueOf(this.impl.getIpcConnectMaxConnections()), equalTo(Integer.valueOf(20)));
    assertThat(this.impl.getIpcConnectAllowed(), equalTo(ipcConnectAllowed));
    assertThat(Integer.valueOf(this.impl.getIpcConnectAllowed().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getInterfaceDefaults(), equalTo(interfaceDefaults));
    assertThat(this.impl.getOs(), equalTo("Linux"));
    assertThat(Long.valueOf(this.impl.getStartTime()), equalTo(Long.valueOf(21)));

    this.impl.setCli(null);
    this.impl.setHna(null);
    this.impl.setIpcConnectAllowed(null);

    assertThat(this.impl.getCli(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getCli().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getIpcConnectAllowed(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getIpcConnectAllowed().size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testEquals() throws UnknownHostException {
    boolean r;
    JsonInfoConfigEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* configurationChecksum */

    String stringOrg = this.impl.getConfigurationChecksum();

    this.impl.setConfigurationChecksum(null);
    other.setConfigurationChecksum(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfigurationChecksum(null);
    other.setConfigurationChecksum("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigurationChecksum("string");
    other.setConfigurationChecksum(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigurationChecksum("string");
    other.setConfigurationChecksum("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfigurationChecksum(stringOrg);
    other.setConfigurationChecksum(stringOrg);

    /* cli */

    final List<String> cli = new LinkedList<>();
    cli.add("-f");
    cli.add("/etc/olsrd/olsrd.conf");

    final List<String> cliOrg = this.impl.getCli();

    this.impl.setCli(null);
    other.setCli(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setCli(null);
    other.setCli(cli);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setCli(cli);
    other.setCli(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setCli(cli);
    other.setCli(cli);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setCli(cliOrg);
    other.setCli(cliOrg);

    /* configurationFile */

    stringOrg = this.impl.getConfigurationFile();

    this.impl.setConfigurationFile(null);
    other.setConfigurationFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfigurationFile(null);
    other.setConfigurationFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigurationFile("string");
    other.setConfigurationFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigurationFile("string");
    other.setConfigurationFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfigurationFile(stringOrg);
    other.setConfigurationFile(stringOrg);

    /* olsrPort */

    int intOrg = this.impl.getOlsrPort();

    this.impl.setOlsrPort(1);
    other.setOlsrPort(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setOlsrPort(1);
    other.setOlsrPort(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOlsrPort(2);
    other.setOlsrPort(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOlsrPort(intOrg);
    other.setOlsrPort(intOrg);

    /* debugLevel */

    intOrg = this.impl.getDebugLevel();

    this.impl.setDebugLevel(1);
    other.setDebugLevel(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setDebugLevel(1);
    other.setDebugLevel(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setDebugLevel(2);
    other.setDebugLevel(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setDebugLevel(intOrg);
    other.setDebugLevel(intOrg);

    /* noFork */

    boolean booleanOrg = this.impl.getNoFork();

    this.impl.setNoFork(false);
    other.setNoFork(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNoFork(false);
    other.setNoFork(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNoFork(true);
    other.setNoFork(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNoFork(true);
    other.setNoFork(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNoFork(booleanOrg);
    other.setNoFork(booleanOrg);

    /* pidFile */

    stringOrg = this.impl.getPidFile();

    this.impl.setPidFile(null);
    other.setPidFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPidFile(null);
    other.setPidFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPidFile("string");
    other.setPidFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPidFile("string");
    other.setPidFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPidFile(stringOrg);
    other.setPidFile(stringOrg);

    /* hostEmulation */

    booleanOrg = this.impl.getHostEmulation();

    this.impl.setHostEmulation(false);
    other.setHostEmulation(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHostEmulation(false);
    other.setHostEmulation(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulation(true);
    other.setHostEmulation(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulation(true);
    other.setHostEmulation(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHostEmulation(booleanOrg);
    other.setHostEmulation(booleanOrg);

    /* ipVersion */

    intOrg = this.impl.getIpVersion();

    this.impl.setIpVersion(1);
    other.setIpVersion(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpVersion(1);
    other.setIpVersion(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpVersion(2);
    other.setIpVersion(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpVersion(intOrg);
    other.setIpVersion(intOrg);

    /* allowNoInt */

    booleanOrg = this.impl.getAllowNoInt();

    this.impl.setAllowNoInt(false);
    other.setAllowNoInt(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAllowNoInt(false);
    other.setAllowNoInt(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAllowNoInt(true);
    other.setAllowNoInt(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAllowNoInt(true);
    other.setAllowNoInt(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAllowNoInt(booleanOrg);
    other.setAllowNoInt(booleanOrg);

    /* tosValue */

    intOrg = this.impl.getTosValue();

    this.impl.setTosValue(1);
    other.setTosValue(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTosValue(1);
    other.setTosValue(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTosValue(2);
    other.setTosValue(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTosValue(intOrg);
    other.setTosValue(intOrg);

    /* rtProto */

    intOrg = this.impl.getRtProto();

    this.impl.setRtProto(1);
    other.setRtProto(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRtProto(1);
    other.setRtProto(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRtProto(2);
    other.setRtProto(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRtProto(intOrg);
    other.setRtProto(intOrg);

    /* rtTable */

    final JsonInfoConfigRtTable rtTable = new JsonInfoConfigRtTable();
    rtTable.setMain(123);

    final JsonInfoConfigRtTable rtTableOrg = this.impl.getRtTable();

    this.impl.setRtTable(null);
    other.setRtTable(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRtTable(null);
    other.setRtTable(rtTable);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRtTable(rtTable);
    other.setRtTable(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRtTable(rtTable);
    other.setRtTable(rtTable);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRtTable(rtTableOrg);
    other.setRtTable(rtTableOrg);

    /* willingness */

    final JsonInfoConfigWillingness willingness = new JsonInfoConfigWillingness();
    willingness.setUpdateInterval(213.123);

    final JsonInfoConfigWillingness willingnessOrg = this.impl.getWillingness();

    this.impl.setWillingness(null);
    other.setWillingness(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setWillingness(null);
    other.setWillingness(willingness);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setWillingness(willingness);
    other.setWillingness(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setWillingness(willingness);
    other.setWillingness(willingness);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setWillingness(willingnessOrg);
    other.setWillingness(willingnessOrg);

    /* fib */

    final JsonInfoConfigFib fib = new JsonInfoConfigFib();
    fib.setMetric("metric");

    final JsonInfoConfigFib fibOrg = this.impl.getFib();

    this.impl.setFib(null);
    other.setFib(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setFib(null);
    other.setFib(fib);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setFib(fib);
    other.setFib(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setFib(fib);
    other.setFib(fib);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setFib(fibOrg);
    other.setFib(fibOrg);

    /* hysteresis */

    final JsonInfoConfigHysteresis hysteresis = new JsonInfoConfigHysteresis();
    hysteresis.setScaling(543.123);

    final JsonInfoConfigHysteresis hysteresisOrg = this.impl.getHysteresis();

    this.impl.setHysteresis(null);
    other.setHysteresis(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHysteresis(null);
    other.setHysteresis(hysteresis);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHysteresis(hysteresis);
    other.setHysteresis(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHysteresis(hysteresis);
    other.setHysteresis(hysteresis);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHysteresis(hysteresisOrg);
    other.setHysteresis(hysteresisOrg);

    /* hna */

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hnaEntry.setDestinationPrefixLength(11);
    hna.add(hnaEntry);

    final Set<JsonInfoHnaEntry> hnaOrg = this.impl.getHna();

    this.impl.setHna(null);
    other.setHna(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHna(null);
    other.setHna(hna);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHna(hna);
    other.setHna(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHna(hna);
    other.setHna(hna);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHna(hnaOrg);
    other.setHna(hnaOrg);

    /* pollrate */

    double doubleOrg = this.impl.getPollrate();

    this.impl.setPollrate(1.0);
    other.setPollrate(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPollrate(1.0);
    other.setPollrate(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPollrate(2.0);
    other.setPollrate(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPollrate(doubleOrg);
    other.setPollrate(doubleOrg);

    /* nicChgsPollInt */

    doubleOrg = this.impl.getNicChgsPollInt();

    this.impl.setNicChgsPollInt(1.0);
    other.setNicChgsPollInt(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNicChgsPollInt(1.0);
    other.setNicChgsPollInt(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNicChgsPollInt(2.0);
    other.setNicChgsPollInt(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNicChgsPollInt(doubleOrg);
    other.setNicChgsPollInt(doubleOrg);

    /* clearScreen */

    booleanOrg = this.impl.getClearScreen();

    this.impl.setClearScreen(false);
    other.setClearScreen(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setClearScreen(false);
    other.setClearScreen(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setClearScreen(true);
    other.setClearScreen(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setClearScreen(true);
    other.setClearScreen(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setClearScreen(booleanOrg);
    other.setClearScreen(booleanOrg);

    /* tcRedundancy */

    intOrg = this.impl.getTcRedundancy();

    this.impl.setTcRedundancy(1);
    other.setTcRedundancy(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTcRedundancy(1);
    other.setTcRedundancy(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTcRedundancy(2);
    other.setTcRedundancy(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTcRedundancy(intOrg);
    other.setTcRedundancy(intOrg);

    /* mprCoverage */

    intOrg = this.impl.getMprCoverage();

    this.impl.setMprCoverage(1);
    other.setMprCoverage(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMprCoverage(1);
    other.setMprCoverage(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMprCoverage(2);
    other.setMprCoverage(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMprCoverage(intOrg);
    other.setMprCoverage(intOrg);

    /* linkQuality */

    final JsonInfoConfigLinkQuality linkQuality = new JsonInfoConfigLinkQuality();
    linkQuality.setAlgorithm("algorithm");

    final JsonInfoConfigLinkQuality linkQualityOrg = this.impl.getLinkQuality();

    this.impl.setLinkQuality(null);
    other.setLinkQuality(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQuality(null);
    other.setLinkQuality(linkQuality);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQuality(linkQuality);
    other.setLinkQuality(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQuality(linkQuality);
    other.setLinkQuality(linkQuality);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQuality(linkQualityOrg);
    other.setLinkQuality(linkQualityOrg);

    /* minTCVTime */

    doubleOrg = this.impl.getMinTCVTime();

    this.impl.setMinTCVTime(1.0);
    other.setMinTCVTime(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMinTCVTime(1.0);
    other.setMinTCVTime(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMinTCVTime(2.0);
    other.setMinTCVTime(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMinTCVTime(doubleOrg);
    other.setMinTCVTime(doubleOrg);

    /* setIpForward */

    booleanOrg = this.impl.getSetIpForward();

    this.impl.setSetIpForward(false);
    other.setSetIpForward(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSetIpForward(false);
    other.setSetIpForward(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSetIpForward(true);
    other.setSetIpForward(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSetIpForward(true);
    other.setSetIpForward(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSetIpForward(booleanOrg);
    other.setSetIpForward(booleanOrg);

    /* lockFile */

    stringOrg = this.impl.getLockFile();

    this.impl.setLockFile(null);
    other.setLockFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLockFile(null);
    other.setLockFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLockFile("string");
    other.setLockFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLockFile("string");
    other.setLockFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLockFile(stringOrg);
    other.setLockFile(stringOrg);

    /* useNiit */

    booleanOrg = this.impl.getUseNiit();

    this.impl.setUseNiit(false);
    other.setUseNiit(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUseNiit(false);
    other.setUseNiit(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUseNiit(true);
    other.setUseNiit(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUseNiit(true);
    other.setUseNiit(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUseNiit(booleanOrg);
    other.setUseNiit(booleanOrg);

    /* smartGateway */

    final JsonInfoConfigSgw sgw = new JsonInfoConfigSgw();
    sgw.setInstanceId("instanceId");

    final JsonInfoConfigSgw sgwOrg = this.impl.getSmartGateway();

    this.impl.setSmartGateway(null);
    other.setSmartGateway(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSmartGateway(null);
    other.setSmartGateway(sgw);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSmartGateway(sgw);
    other.setSmartGateway(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSmartGateway(sgw);
    other.setSmartGateway(sgw);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSmartGateway(sgwOrg);
    other.setSmartGateway(sgwOrg);

    /* mainIp */

    final InetAddress addr = InetAddress.getByName("127.0.0.1");

    stringOrg = this.impl.getMainIp();

    this.impl.setMainIp(null);
    other.setMainIp(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMainIp(null);
    other.setMainIp(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMainIp(addr);
    other.setMainIp(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMainIp(addr);
    other.setMainIp(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMainIp(InetAddress.getByName(stringOrg));
    other.setMainIp(InetAddress.getByName(stringOrg));

    /* unicastSourceIpAddress */

    stringOrg = this.impl.getUnicastSourceIpAddress();

    this.impl.setUnicastSourceIpAddress(null);
    other.setUnicastSourceIpAddress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUnicastSourceIpAddress(null);
    other.setUnicastSourceIpAddress(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUnicastSourceIpAddress(addr);
    other.setUnicastSourceIpAddress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUnicastSourceIpAddress(addr);
    other.setUnicastSourceIpAddress(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUnicastSourceIpAddress(InetAddress.getByName(stringOrg));
    other.setUnicastSourceIpAddress(InetAddress.getByName(stringOrg));

    /* srcIpRoutes */

    booleanOrg = this.impl.getSrcIpRoutes();

    this.impl.setSrcIpRoutes(false);
    other.setSrcIpRoutes(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSrcIpRoutes(false);
    other.setSrcIpRoutes(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSrcIpRoutes(true);
    other.setSrcIpRoutes(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSrcIpRoutes(true);
    other.setSrcIpRoutes(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSrcIpRoutes(booleanOrg);
    other.setSrcIpRoutes(booleanOrg);

    /* maxPrefixLength */

    intOrg = this.impl.getMaxPrefixLength();

    this.impl.setMaxPrefixLength(1);
    other.setMaxPrefixLength(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMaxPrefixLength(1);
    other.setMaxPrefixLength(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxPrefixLength(2);
    other.setMaxPrefixLength(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxPrefixLength(intOrg);
    other.setMaxPrefixLength(intOrg);

    /* ipSize */

    intOrg = this.impl.getIpSize();

    this.impl.setIpSize(1);
    other.setIpSize(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpSize(1);
    other.setIpSize(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpSize(2);
    other.setIpSize(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpSize(intOrg);
    other.setIpSize(intOrg);

    /* delgw */

    booleanOrg = this.impl.getDelgw();

    this.impl.setDelgw(false);
    other.setDelgw(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setDelgw(false);
    other.setDelgw(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setDelgw(true);
    other.setDelgw(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setDelgw(true);
    other.setDelgw(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setDelgw(booleanOrg);
    other.setDelgw(booleanOrg);

    /* maxSendMessageJitter */

    doubleOrg = this.impl.getMaxSendMessageJitter();

    this.impl.setMaxSendMessageJitter(1.0);
    other.setMaxSendMessageJitter(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMaxSendMessageJitter(1.0);
    other.setMaxSendMessageJitter(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxSendMessageJitter(2.0);
    other.setMaxSendMessageJitter(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxSendMessageJitter(doubleOrg);
    other.setMaxSendMessageJitter(doubleOrg);

    /* exitValue */

    intOrg = this.impl.getExitValue();

    this.impl.setExitValue(1);
    other.setExitValue(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setExitValue(1);
    other.setExitValue(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setExitValue(2);
    other.setExitValue(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setExitValue(intOrg);
    other.setExitValue(intOrg);

    /* maxTcValidTime */

    doubleOrg = this.impl.getMaxTcValidTime();

    this.impl.setMaxTcValidTime(1.0);
    other.setMaxTcValidTime(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMaxTcValidTime(1.0);
    other.setMaxTcValidTime(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxTcValidTime(2.0);
    other.setMaxTcValidTime(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxTcValidTime(doubleOrg);
    other.setMaxTcValidTime(doubleOrg);

    /* niit4to6InterfaceIndex */

    intOrg = this.impl.getNiit4to6InterfaceIndex();

    this.impl.setNiit4to6InterfaceIndex(1);
    other.setNiit4to6InterfaceIndex(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNiit4to6InterfaceIndex(1);
    other.setNiit4to6InterfaceIndex(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNiit4to6InterfaceIndex(2);
    other.setNiit4to6InterfaceIndex(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNiit4to6InterfaceIndex(intOrg);
    other.setNiit4to6InterfaceIndex(intOrg);

    /* niit6to4InterfaceIndex */

    intOrg = this.impl.getNiit6to4InterfaceIndex();

    this.impl.setNiit6to4InterfaceIndex(1);
    other.setNiit6to4InterfaceIndex(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNiit6to4InterfaceIndex(1);
    other.setNiit6to4InterfaceIndex(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNiit6to4InterfaceIndex(2);
    other.setNiit6to4InterfaceIndex(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNiit6to4InterfaceIndex(intOrg);
    other.setNiit6to4InterfaceIndex(intOrg);

    /* hasIpv4Gateway */

    booleanOrg = this.impl.getHasIpv4Gateway();

    this.impl.setHasIpv4Gateway(false);
    other.setHasIpv4Gateway(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHasIpv4Gateway(false);
    other.setHasIpv4Gateway(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHasIpv4Gateway(true);
    other.setHasIpv4Gateway(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHasIpv4Gateway(true);
    other.setHasIpv4Gateway(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHasIpv4Gateway(booleanOrg);
    other.setHasIpv4Gateway(booleanOrg);

    /* hasIpv6Gateway */

    booleanOrg = this.impl.getHasIpv6Gateway();

    this.impl.setHasIpv6Gateway(false);
    other.setHasIpv6Gateway(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHasIpv6Gateway(false);
    other.setHasIpv6Gateway(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHasIpv6Gateway(true);
    other.setHasIpv6Gateway(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHasIpv6Gateway(true);
    other.setHasIpv6Gateway(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHasIpv6Gateway(booleanOrg);
    other.setHasIpv6Gateway(booleanOrg);

    /* ioctlSocket */

    intOrg = this.impl.getIoctlSocket();

    this.impl.setIoctlSocket(1);
    other.setIoctlSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIoctlSocket(1);
    other.setIoctlSocket(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIoctlSocket(2);
    other.setIoctlSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIoctlSocket(intOrg);
    other.setIoctlSocket(intOrg);

    /* routeNetlinkSocket */

    intOrg = this.impl.getRouteNetlinkSocket();

    this.impl.setRouteNetlinkSocket(1);
    other.setRouteNetlinkSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRouteNetlinkSocket(1);
    other.setRouteNetlinkSocket(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteNetlinkSocket(2);
    other.setRouteNetlinkSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteNetlinkSocket(intOrg);
    other.setRouteNetlinkSocket(intOrg);

    /* routeMonitorSocket */

    intOrg = this.impl.getRouteMonitorSocket();

    this.impl.setRouteMonitorSocket(1);
    other.setRouteMonitorSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRouteMonitorSocket(1);
    other.setRouteMonitorSocket(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteMonitorSocket(2);
    other.setRouteMonitorSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteMonitorSocket(intOrg);
    other.setRouteMonitorSocket(intOrg);

    /* routeChangeSocket */

    intOrg = this.impl.getRouteChangeSocket();

    this.impl.setRouteChangeSocket(1);
    other.setRouteChangeSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRouteChangeSocket(1);
    other.setRouteChangeSocket(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteChangeSocket(2);
    other.setRouteChangeSocket(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRouteChangeSocket(intOrg);
    other.setRouteChangeSocket(intOrg);

    /* linkQualityNatThreshold */

    doubleOrg = this.impl.getLinkQualityNatThreshold();

    this.impl.setLinkQualityNatThreshold(1.0);
    other.setLinkQualityNatThreshold(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setLinkQualityNatThreshold(1.0);
    other.setLinkQualityNatThreshold(2.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQualityNatThreshold(2.0);
    other.setLinkQualityNatThreshold(1.0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinkQualityNatThreshold(doubleOrg);
    other.setLinkQualityNatThreshold(doubleOrg);

    /* brokenLinkCost */

    long longOrg = this.impl.getBrokenLinkCost();

    this.impl.setBrokenLinkCost(1);
    other.setBrokenLinkCost(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setBrokenLinkCost(1);
    other.setBrokenLinkCost(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBrokenLinkCost(2);
    other.setBrokenLinkCost(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBrokenLinkCost(longOrg);
    other.setBrokenLinkCost(longOrg);

    /* brokenRouteCost */

    longOrg = this.impl.getBrokenRouteCost();

    this.impl.setBrokenRouteCost(1);
    other.setBrokenRouteCost(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setBrokenRouteCost(1);
    other.setBrokenRouteCost(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBrokenRouteCost(2);
    other.setBrokenRouteCost(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBrokenRouteCost(longOrg);
    other.setBrokenRouteCost(longOrg);

    /* ipcConnectMaxConnections */

    intOrg = this.impl.getIpcConnectMaxConnections();

    this.impl.setIpcConnectMaxConnections(1);
    other.setIpcConnectMaxConnections(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpcConnectMaxConnections(1);
    other.setIpcConnectMaxConnections(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpcConnectMaxConnections(2);
    other.setIpcConnectMaxConnections(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpcConnectMaxConnections(intOrg);
    other.setIpcConnectMaxConnections(intOrg);

    /* ipcConnectAllowed */

    final Set<JsonInfoConfigIpcAcl> ipcConnectAllowed = new TreeSet<>();
    final JsonInfoConfigIpcAcl acl1 = new JsonInfoConfigIpcAcl();
    ipcConnectAllowed.add(acl1);

    final Set<JsonInfoConfigIpcAcl> ipcConnectAllowedOrg = this.impl.getIpcConnectAllowed();

    this.impl.setIpcConnectAllowed(null);
    other.setIpcConnectAllowed(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpcConnectAllowed(null);
    other.setIpcConnectAllowed(ipcConnectAllowed);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpcConnectAllowed(ipcConnectAllowed);
    other.setIpcConnectAllowed(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpcConnectAllowed(ipcConnectAllowed);
    other.setIpcConnectAllowed(ipcConnectAllowed);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpcConnectAllowed(ipcConnectAllowedOrg);
    other.setIpcConnectAllowed(ipcConnectAllowedOrg);

    /* interfaceDefaults */

    final JsonInfoInterfaceConfiguration interfaceDefaults = new JsonInfoInterfaceConfiguration();
    interfaceDefaults.setMode("mode");

    final JsonInfoInterfaceConfiguration interfaceDefaultsOrg = this.impl.getInterfaceDefaults();

    this.impl.setInterfaceDefaults(null);
    other.setInterfaceDefaults(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaceDefaults(null);
    other.setInterfaceDefaults(interfaceDefaults);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceDefaults(interfaceDefaults);
    other.setInterfaceDefaults(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceDefaults(interfaceDefaults);
    other.setInterfaceDefaults(interfaceDefaults);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaceDefaults(interfaceDefaultsOrg);
    other.setInterfaceDefaults(interfaceDefaultsOrg);

    /* os */

    stringOrg = this.impl.getOs();

    this.impl.setOs(null);
    other.setOs(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setOs(null);
    other.setOs("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOs("string");
    other.setOs(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOs("string");
    other.setOs("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setOs(stringOrg);
    other.setOs(stringOrg);

    /* startTime */

    longOrg = this.impl.getStartTime();

    this.impl.setStartTime(1);
    other.setStartTime(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setStartTime(1);
    other.setStartTime(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStartTime(2);
    other.setStartTime(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStartTime(longOrg);
    other.setStartTime(longOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(185530515)));

    /* set */
    final List<String> cli = new LinkedList<>();
    cli.add("-f");
    cli.add("/etc/olsrd/olsrd.conf");

    final JsonInfoConfigRtTable rtTable = new JsonInfoConfigRtTable();
    rtTable.setDefault(11);

    final JsonInfoConfigWillingness willingness = new JsonInfoConfigWillingness();
    willingness.setUpdateInterval(123.321);

    final JsonInfoConfigFib fib = new JsonInfoConfigFib();
    fib.setMetric("metric");

    final JsonInfoConfigHysteresis hysteresis = new JsonInfoConfigHysteresis();
    hysteresis.setScaling(231.65);

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hnaEntry.setDestinationPrefixLength(11);
    hna.add(hnaEntry);

    final JsonInfoConfigLinkQuality linkQuality = new JsonInfoConfigLinkQuality();
    linkQuality.setAlgorithm("algorithm");

    final JsonInfoConfigSgw smartGateway = new JsonInfoConfigSgw();
    smartGateway.setInstanceId("instanceId");

    final InetAddress addr1 = InetAddress.getByName("127.0.0.1");
    final InetAddress addr2 = InetAddress.getByName("127.0.0.2");

    final Set<JsonInfoConfigIpcAcl> ipcConnectAllowed = new TreeSet<>();
    final JsonInfoConfigIpcAcl acl1 = new JsonInfoConfigIpcAcl();
    ipcConnectAllowed.add(acl1);

    final JsonInfoInterfaceConfiguration interfaceDefaults = new JsonInfoInterfaceConfiguration();
    interfaceDefaults.setMode("mode");

    this.impl.setConfigurationChecksum("configurationChecksum");
    this.impl.setCli(cli);
    this.impl.setConfigurationFile("configurationFile");
    this.impl.setOlsrPort(1);
    this.impl.setDebugLevel(2);
    this.impl.setNoFork(true);
    this.impl.setPidFile("pidFile");
    this.impl.setHostEmulation(true);
    this.impl.setIpVersion(4);
    this.impl.setAllowNoInt(true);
    this.impl.setTosValue(5);
    this.impl.setRtProto(6);
    this.impl.setRtTable(rtTable);
    this.impl.setWillingness(willingness);
    this.impl.setFib(fib);
    this.impl.setHysteresis(hysteresis);
    this.impl.setHna(hna);
    this.impl.setPollrate(1.1);
    this.impl.setNicChgsPollInt(2.2);
    this.impl.setClearScreen(true);
    this.impl.setTcRedundancy(7);
    this.impl.setMprCoverage(8);
    this.impl.setLinkQuality(linkQuality);
    this.impl.setMinTCVTime(3.3);
    this.impl.setSetIpForward(true);
    this.impl.setLockFile("lockFile");
    this.impl.setUseNiit(true);
    this.impl.setSmartGateway(smartGateway);
    this.impl.setMainIp(addr1);
    this.impl.setUnicastSourceIpAddress(addr2);
    this.impl.setSrcIpRoutes(true);
    this.impl.setMaxPrefixLength(9);
    this.impl.setIpSize(10);
    this.impl.setDelgw(true);
    this.impl.setMaxSendMessageJitter(4.4);
    this.impl.setExitValue(11);
    this.impl.setMaxTcValidTime(5.5);
    this.impl.setNiit4to6InterfaceIndex(12);
    this.impl.setNiit6to4InterfaceIndex(13);
    this.impl.setHasIpv4Gateway(true);
    this.impl.setHasIpv6Gateway(true);
    this.impl.setIoctlSocket(14);
    this.impl.setRouteNetlinkSocket(15);
    this.impl.setRouteMonitorSocket(16);
    this.impl.setRouteChangeSocket(17);
    this.impl.setLinkQualityNatThreshold(6.6);
    this.impl.setBrokenLinkCost(18);
    this.impl.setBrokenRouteCost(19);
    this.impl.setIpcConnectMaxConnections(20);
    this.impl.setIpcConnectAllowed(ipcConnectAllowed);
    this.impl.setInterfaceDefaults(interfaceDefaults);
    this.impl.setOs("Linux");
    this.impl.setStartTime(21);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1824937455)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}