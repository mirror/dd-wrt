package org.olsr.v1.info.api.dto;

import java.net.InetAddress;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A config entry in the {@link InfoCommand#CONFIG} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoConfigEntry {
  private String                          configurationChecksum    = "";
  private final List<String>              cli                      = new LinkedList<>();
  private String                          configurationFile        = "";
  private int                             olsrPort                 = 0;
  private int                             debugLevel               = 0;
  private boolean                         noFork                   = false;
  private String                          pidFile                  = "";
  private boolean                         hostEmulation            = false;
  private int                             ipVersion                = 0;
  private boolean                         allowNoInt               = false;
  private int                             tosValue                 = 0;
  private int                             rtProto                  = 0;
  private JsonInfoConfigRtTable           rtTable                  = new JsonInfoConfigRtTable();
  private JsonInfoConfigWillingness       willingness              = new JsonInfoConfigWillingness();
  private JsonInfoConfigFib               fib                      = new JsonInfoConfigFib();
  private JsonInfoConfigHysteresis        hysteresis               = new JsonInfoConfigHysteresis();
  private final Set<JsonInfoHnaEntry>     hna                      = new TreeSet<>();
  private double                          pollrate                 = 0.0;
  private double                          nicChgsPollInt           = 0.0;
  private boolean                         clearScreen              = false;
  private int                             tcRedundancy             = 0;
  private int                             mprCoverage              = 0;
  private JsonInfoConfigLinkQuality       linkQuality              = new JsonInfoConfigLinkQuality();
  private double                          minTCVTime               = 0.0;
  private boolean                         setIpForward             = false;
  private String                          lockFile                 = "";
  private boolean                         useNiit                  = false;
  private JsonInfoConfigSgw               smartGateway             = new JsonInfoConfigSgw();
  private String                          mainIp                   = "";
  private String                          unicastSourceIpAddress   = "";
  private boolean                         srcIpRoutes              = false;
  private int                             maxPrefixLength          = 0;
  private int                             ipSize                   = 0;
  private boolean                         delgw                    = false;
  private double                          maxSendMessageJitter     = 0.0;
  private int                             exitValue                = 0;
  private double                          maxTcValidTime           = 0.0;
  private int                             niit4to6InterfaceIndex   = 0;
  private int                             niit6to4InterfaceIndex   = 0;
  private boolean                         hasIpv4Gateway           = false;
  private boolean                         hasIpv6Gateway           = false;
  private int                             ioctlSocket              = -1;
  private int                             routeNetlinkSocket       = -1;
  private int                             routeMonitorSocket       = -1;
  private int                             routeChangeSocket        = -1;
  private double                          linkQualityNatThreshold  = 0.0;
  private long                            brokenLinkCost           = 0;
  private long                            brokenRouteCost          = 0;
  private int                             ipcConnectMaxConnections = 0;
  private final Set<JsonInfoConfigIpcAcl> ipcConnectAllowed        = new TreeSet<>();
  private JsonInfoInterfaceConfiguration  interfaceDefaults        = new JsonInfoInterfaceConfiguration();
  private String                          os                       = "";
  private long                            startTime                = 0;

  /**
   * @return the configuration checksum
   */
  public String getConfigurationChecksum() {
    return this.configurationChecksum;
  }

  /**
   * @param configurationChecksum the configuration checksum to set
   */
  @JsonProperty("configurationChecksum")
  public void setConfigurationChecksum(final String configurationChecksum) {
    if (configurationChecksum == null) {
      this.configurationChecksum = "";
    } else {
      this.configurationChecksum = configurationChecksum;
    }
  }

  /**
   * @return the CLI
   */
  public List<String> getCli() {
    return this.cli;
  }

  /**
   * @param cli the CLI to set
   */
  @JsonProperty("cli")
  public void setCli(final List<String> cli) {
    this.cli.clear();
    if (cli != null) {
      this.cli.addAll(cli);
    }
  }

  /**
   * @return the configuration file
   */
  public String getConfigurationFile() {
    return this.configurationFile;
  }

  /**
   * @param configurationFile the configuration file to set
   */
  @JsonProperty("configurationFile")
  public void setConfigurationFile(final String configurationFile) {
    if (configurationFile == null) {
      this.configurationFile = "";
    } else {
      this.configurationFile = configurationFile;
    }
  }

  /**
   * @return the olsr port
   */
  public int getOlsrPort() {
    return this.olsrPort;
  }

  /**
   * @param olsrPort the olsr port to set
   */
  @JsonProperty("olsrPort")
  public void setOlsrPort(final int olsrPort) {
    this.olsrPort = olsrPort;
  }

  /**
   * @return the debug level
   */
  public int getDebugLevel() {
    return this.debugLevel;
  }

  /**
   * @param debugLevel the debug level to set
   */
  @JsonProperty("debugLevel")
  public void setDebugLevel(final int debugLevel) {
    this.debugLevel = debugLevel;
  }

  /**
   * @return the no-fork setting
   */
  public boolean getNoFork() {
    return this.noFork;
  }

  /**
   * @param noFork the no-fork setting to set
   */
  @JsonProperty("noFork")
  public void setNoFork(final boolean noFork) {
    this.noFork = noFork;
  }

  /**
   * @return the pid file
   */
  public String getPidFile() {
    return this.pidFile;
  }

  /**
   * @param pidFile the pid file to set
   */
  @JsonProperty("pidFile")
  public void setPidFile(final String pidFile) {
    if (pidFile == null) {
      this.pidFile = "";
    } else {
      this.pidFile = pidFile;
    }
  }

  /**
   * @return the host emulation status
   */
  public boolean getHostEmulation() {
    return this.hostEmulation;
  }

  /**
   * @param hostEmulation the host emulation status to set
   */
  @JsonProperty("hostEmulation")
  public void setHostEmulation(final boolean hostEmulation) {
    this.hostEmulation = hostEmulation;
  }

  /**
   * @return the IP version
   */
  public int getIpVersion() {
    return this.ipVersion;
  }

  /**
   * @param ipVersion the IP version to set
   */
  @JsonProperty("ipVersion")
  public void setIpVersion(final int ipVersion) {
    this.ipVersion = ipVersion;
  }

  /**
   * @return the allow-no-interfaces setting
   */
  public boolean getAllowNoInt() {
    return this.allowNoInt;
  }

  /**
   * @param allowNoInt the allow-no-interfaces setting to set
   */
  @JsonProperty("allowNoInt")
  public void setAllowNoInt(final boolean allowNoInt) {
    this.allowNoInt = allowNoInt;
  }

  /**
   * @return the TOS value
   */
  public int getTosValue() {
    return this.tosValue;
  }

  /**
   * @param tosValue the TOS value to set
   */
  @JsonProperty("tosValue")
  public void setTosValue(final int tosValue) {
    this.tosValue = tosValue;
  }

  /**
   * @return the routing protocol
   */
  public int getRtProto() {
    return this.rtProto;
  }

  /**
   * @param rtProto the routing protocol to set
   */
  @JsonProperty("rtProto")
  public void setRtProto(final int rtProto) {
    this.rtProto = rtProto;
  }

  /**
   * @return the routing table settings
   */
  public JsonInfoConfigRtTable getRtTable() {
    return this.rtTable;
  }

  /**
   * @param rtTable the routing table settings to set
   */
  @JsonProperty("rtTable")
  public void setRtTable(final JsonInfoConfigRtTable rtTable) {
    if (rtTable == null) {
      this.rtTable = new JsonInfoConfigRtTable();
    } else {
      this.rtTable = rtTable;
    }
  }

  /**
   * @return the willingness settings
   */
  public JsonInfoConfigWillingness getWillingness() {
    return this.willingness;
  }

  /**
   * @param willingness the willingness settings to set
   */
  @JsonProperty("willingness")
  public void setWillingness(final JsonInfoConfigWillingness willingness) {
    if (willingness == null) {
      this.willingness = new JsonInfoConfigWillingness();
    } else {
      this.willingness = willingness;
    }
  }

  /**
   * @return the FIB settings
   */
  public JsonInfoConfigFib getFib() {
    return this.fib;
  }

  /**
   * @param fib the FIB settings to set
   */
  @JsonProperty("fib")
  public void setFib(final JsonInfoConfigFib fib) {
    if (fib == null) {
      this.fib = new JsonInfoConfigFib();
    } else {
      this.fib = fib;
    }
  }

  /**
   * @return the hysteresis settings
   */
  public JsonInfoConfigHysteresis getHysteresis() {
    return this.hysteresis;
  }

  /**
   * @param hysteresis the hysteresis settings to set
   */
  @JsonProperty("hysteresis")
  public void setHysteresis(final JsonInfoConfigHysteresis hysteresis) {
    if (hysteresis == null) {
      this.hysteresis = new JsonInfoConfigHysteresis();
    } else {
      this.hysteresis = hysteresis;
    }
  }

  /**
   * @return the advertised HNAs
   */
  public Set<JsonInfoHnaEntry> getHna() {
    return this.hna;
  }

  /**
   * @param hna the advertised HNAs to set
   */
  @JsonProperty("hna")
  public void setHna(final Set<JsonInfoHnaEntry> hna) {
    this.hna.clear();
    if (hna != null) {
      this.hna.addAll(hna);
    }
  }

  /**
   * @return the poll rate
   */
  public double getPollrate() {
    return this.pollrate;
  }

  /**
   * @param pollrate the poll rate to set
   */
  @JsonProperty("pollrate")
  public void setPollrate(final double pollrate) {
    this.pollrate = pollrate;
  }

  /**
   * @return the NIC change poll interval
   */
  public double getNicChgsPollInt() {
    return this.nicChgsPollInt;
  }

  /**
   * @param nicChgsPollInt the NIC change poll interval to set
   */
  @JsonProperty("nicChgsPollInt")
  public void setNicChgsPollInt(final double nicChgsPollInt) {
    this.nicChgsPollInt = nicChgsPollInt;
  }

  /**
   * @return the clear screen setting
   */
  public boolean getClearScreen() {
    return this.clearScreen;
  }

  /**
   * @param clearScreen the clear screen setting to set
   */
  @JsonProperty("clearScreen")
  public void setClearScreen(final boolean clearScreen) {
    this.clearScreen = clearScreen;
  }

  /**
   * @return the TC redundancy
   */
  public int getTcRedundancy() {
    return this.tcRedundancy;
  }

  /**
   * @param tcRedundancy the TC redundancy to set
   */
  @JsonProperty("tcRedundancy")
  public void setTcRedundancy(final int tcRedundancy) {
    this.tcRedundancy = tcRedundancy;
  }

  /**
   * @return the MPR coverage
   */
  public int getMprCoverage() {
    return this.mprCoverage;
  }

  /**
   * @param mprCoverage the MPR coverage to set
   */
  @JsonProperty("mprCoverage")
  public void setMprCoverage(final int mprCoverage) {
    this.mprCoverage = mprCoverage;
  }

  /**
   * @return the link quality settings
   */
  public JsonInfoConfigLinkQuality getLinkQuality() {
    return this.linkQuality;
  }

  /**
   * @param linkQuality the link quality settings to set
   */
  @JsonProperty("linkQuality")
  public void setLinkQuality(final JsonInfoConfigLinkQuality linkQuality) {
    if (linkQuality == null) {
      this.linkQuality = new JsonInfoConfigLinkQuality();
    } else {
      this.linkQuality = linkQuality;
    }
  }

  /**
   * @return the minimum TC validity time
   */
  public double getMinTCVTime() {
    return this.minTCVTime;
  }

  /**
   * @param minTCVTime the minimum TC validity time to set
   */
  @JsonProperty("minTCVTime")
  public void setMinTCVTime(final double minTCVTime) {
    this.minTCVTime = minTCVTime;
  }

  /**
   * @return the set IP-Forward setting
   */
  public boolean getSetIpForward() {
    return this.setIpForward;
  }

  /**
   * @param setIpForward the set IP-Forward setting to set
   */
  @JsonProperty("setIpForward")
  public void setSetIpForward(final boolean setIpForward) {
    this.setIpForward = setIpForward;
  }

  /**
   * @return the lock file
   */
  public String getLockFile() {
    return this.lockFile;
  }

  /**
   * @param lockFile the lock file to set
   */
  @JsonProperty("lockFile")
  public void setLockFile(final String lockFile) {
    if (lockFile == null) {
      this.lockFile = "";
    } else {
      this.lockFile = lockFile;
    }
  }

  /**
   * @return the use NIIT setting
   */
  public boolean getUseNiit() {
    return this.useNiit;
  }

  /**
   * @param useNiit the use NIIT settingto set
   */
  @JsonProperty("useNiit")
  public void setUseNiit(final boolean useNiit) {
    this.useNiit = useNiit;
  }

  /**
   * @return the smart gateway settings
   */
  public JsonInfoConfigSgw getSmartGateway() {
    return this.smartGateway;
  }

  /**
   * @param smartGateway the smart gateway settings to set
   */
  @JsonProperty("smartGateway")
  public void setSmartGateway(final JsonInfoConfigSgw smartGateway) {
    if (smartGateway == null) {
      this.smartGateway = new JsonInfoConfigSgw();
    } else {
      this.smartGateway = smartGateway;
    }
  }

  /**
   * @return the main IP address
   */
  public String getMainIp() {
    return this.mainIp;
  }

  /**
   * @param mainIp the main IP address to set
   */
  @JsonProperty("mainIp")
  public void setMainIp(final InetAddress mainIp) {
    if (mainIp == null) {
      this.mainIp = "";
    } else {
      this.mainIp = mainIp.getHostAddress();
    }
  }

  /**
   * @return the unicast source IP address
   */
  public String getUnicastSourceIpAddress() {
    return this.unicastSourceIpAddress;
  }

  /**
   * @param unicastSourceIpAddress the unicast source IP address to set
   */
  @JsonProperty("unicastSourceIpAddress")
  public void setUnicastSourceIpAddress(final InetAddress unicastSourceIpAddress) {
    if (unicastSourceIpAddress == null) {
      this.unicastSourceIpAddress = "";
    } else {
      this.unicastSourceIpAddress = unicastSourceIpAddress.getHostAddress();
    }
  }

  /**
   * @return the source IP routes setting
   */
  public boolean getSrcIpRoutes() {
    return this.srcIpRoutes;
  }

  /**
   * @param srcIpRoutes the source IP routes setting to set
   */
  @JsonProperty("srcIpRoutes")
  public void setSrcIpRoutes(final boolean srcIpRoutes) {
    this.srcIpRoutes = srcIpRoutes;
  }

  /**
   * @return the maximum prefix length
   */
  public int getMaxPrefixLength() {
    return this.maxPrefixLength;
  }

  /**
   * @param maxPrefixLength the maximum prefix length to set
   */
  @JsonProperty("maxPrefixLength")
  public void setMaxPrefixLength(final int maxPrefixLength) {
    this.maxPrefixLength = maxPrefixLength;
  }

  /**
   * @return the IP size
   */
  public int getIpSize() {
    return this.ipSize;
  }

  /**
   * @param ipSize the IP size to set
   */
  @JsonProperty("ipSize")
  public void setIpSize(final int ipSize) {
    this.ipSize = ipSize;
  }

  /**
   * @return the delete gateway setting
   */
  public boolean getDelgw() {
    return this.delgw;
  }

  /**
   * @param delgw the delete gateway setting to set
   */
  @JsonProperty("delgw")
  public void setDelgw(final boolean delgw) {
    this.delgw = delgw;
  }

  /**
   * @return the maximum send message jitter
   */
  public double getMaxSendMessageJitter() {
    return this.maxSendMessageJitter;
  }

  /**
   * @param maxSendMessageJitter the maximum send message jitter to set
   */
  @JsonProperty("maxSendMessageJitter")
  public void setMaxSendMessageJitter(final double maxSendMessageJitter) {
    this.maxSendMessageJitter = maxSendMessageJitter;
  }

  /**
   * @return the exit value
   */
  public int getExitValue() {
    return this.exitValue;
  }

  /**
   * @param exitValue the exit value to set
   */
  @JsonProperty("exitValue")
  public void setExitValue(final int exitValue) {
    this.exitValue = exitValue;
  }

  /**
   * @return the maximum TC validity time
   */
  public double getMaxTcValidTime() {
    return this.maxTcValidTime;
  }

  /**
   * @param maxTcValidTime the maximum TC validity time to set
   */
  @JsonProperty("maxTcValidTime")
  public void setMaxTcValidTime(final double maxTcValidTime) {
    this.maxTcValidTime = maxTcValidTime;
  }

  /**
   * @return the NIIT 4-to-6 interface index
   */
  public int getNiit4to6InterfaceIndex() {
    return this.niit4to6InterfaceIndex;
  }

  /**
   * @param niit4to6InterfaceIndex the NIIT 4-to-6 interface index to set
   */
  @JsonProperty("niit4to6InterfaceIndex")
  public void setNiit4to6InterfaceIndex(final int niit4to6InterfaceIndex) {
    this.niit4to6InterfaceIndex = niit4to6InterfaceIndex;
  }

  /**
   * @return the NIIT 6-to-4 interface index
   */
  public int getNiit6to4InterfaceIndex() {
    return this.niit6to4InterfaceIndex;
  }

  /**
   * @param niit6to4InterfaceIndex the NIIT 6-to-4 interface index to set
   */
  @JsonProperty("niit6to4InterfaceIndex")
  public void setNiit6to4InterfaceIndex(final int niit6to4InterfaceIndex) {
    this.niit6to4InterfaceIndex = niit6to4InterfaceIndex;
  }

  /**
   * @return the has IPv4 gateway status
   */
  public boolean getHasIpv4Gateway() {
    return this.hasIpv4Gateway;
  }

  /**
   * @param hasIpv4Gateway the has IPv4 gateway status to set
   */
  @JsonProperty("hasIpv4Gateway")
  public void setHasIpv4Gateway(final boolean hasIpv4Gateway) {
    this.hasIpv4Gateway = hasIpv4Gateway;
  }

  /**
   * @return the has IPv6 gateway status
   */
  public boolean getHasIpv6Gateway() {
    return this.hasIpv6Gateway;
  }

  /**
   * @param hasIpv6Gateway the has IPv6 gateway status to set
   */
  @JsonProperty("hasIpv6Gateway")
  public void setHasIpv6Gateway(final boolean hasIpv6Gateway) {
    this.hasIpv6Gateway = hasIpv6Gateway;
  }

  /**
   * @return the ioctl socket
   */
  public int getIoctlSocket() {
    return this.ioctlSocket;
  }

  /**
   * @param ioctlSocket the ioctl socket to set
   */
  @JsonProperty("ioctlSocket")
  public void setIoctlSocket(final int ioctlSocket) {
    this.ioctlSocket = ioctlSocket;
  }

  /**
   * @return the route netlink socket
   */
  public int getRouteNetlinkSocket() {
    return this.routeNetlinkSocket;
  }

  /**
   * @param routeNetlinkSocket the route netlink socket to set
   */
  @JsonProperty("routeNetlinkSocket")
  public void setRouteNetlinkSocket(final int routeNetlinkSocket) {
    this.routeNetlinkSocket = routeNetlinkSocket;
  }

  /**
   * @return the route monitor socket
   */
  public int getRouteMonitorSocket() {
    return this.routeMonitorSocket;
  }

  /**
   * @param routeMonitorSocket the route monitor socket to set
   */
  @JsonProperty("routeMonitorSocket")
  public void setRouteMonitorSocket(final int routeMonitorSocket) {
    this.routeMonitorSocket = routeMonitorSocket;
  }

  /**
   * @return the route change socket
   */
  public int getRouteChangeSocket() {
    return this.routeChangeSocket;
  }

  /**
   * @param routeChangeSocket the route change socket to set
   */
  @JsonProperty("routeChangeSocket")
  public void setRouteChangeSocket(final int routeChangeSocket) {
    this.routeChangeSocket = routeChangeSocket;
  }

  /**
   * @return the link quality NAT threshold
   */
  public double getLinkQualityNatThreshold() {
    return this.linkQualityNatThreshold;
  }

  /**
   * @param linkQualityNatThreshold the link quality NAT threshold to set
   */
  @JsonProperty("linkQualityNatThreshold")
  public void setLinkQualityNatThreshold(final double linkQualityNatThreshold) {
    this.linkQualityNatThreshold = linkQualityNatThreshold;
  }

  /**
   * @return the broken link cost
   */
  public long getBrokenLinkCost() {
    return this.brokenLinkCost;
  }

  /**
   * @param brokenLinkCost the broken link cost to set
   */
  @JsonProperty("brokenLinkCost")
  public void setBrokenLinkCost(final long brokenLinkCost) {
    this.brokenLinkCost = brokenLinkCost;
  }

  /**
   * @return the broken route cost
   */
  public long getBrokenRouteCost() {
    return this.brokenRouteCost;
  }

  /**
   * @param brokenRouteCost the broken route cost to set
   */
  @JsonProperty("brokenRouteCost")
  public void setBrokenRouteCost(final long brokenRouteCost) {
    this.brokenRouteCost = brokenRouteCost;
  }

  /**
   * @return the maximum number of IPC connections
   */
  public int getIpcConnectMaxConnections() {
    return this.ipcConnectMaxConnections;
  }

  /**
   * @param ipcConnectMaxConnections the maximum number of IPC connections to set
   */
  @JsonProperty("ipcConnectMaxConnections")
  public void setIpcConnectMaxConnections(final int ipcConnectMaxConnections) {
    this.ipcConnectMaxConnections = ipcConnectMaxConnections;
  }

  /**
   * @return the IPC connection ACLs
   */
  public Set<JsonInfoConfigIpcAcl> getIpcConnectAllowed() {
    return this.ipcConnectAllowed;
  }

  /**
   * @param ipcConnectAllowed the IPC connection ACLs to set
   */
  @JsonProperty("ipcConnectAllowed")
  public void setIpcConnectAllowed(final Set<JsonInfoConfigIpcAcl> ipcConnectAllowed) {
    this.ipcConnectAllowed.clear();
    if (ipcConnectAllowed != null) {
      this.ipcConnectAllowed.addAll(ipcConnectAllowed);
    }
  }

  /**
   * @return the interface defaults
   */
  public JsonInfoInterfaceConfiguration getInterfaceDefaults() {
    return this.interfaceDefaults;
  }

  /**
   * @param interfaceDefaults the interface defaults to set
   */
  @JsonProperty("interfaceDefaults")
  public void setInterfaceDefaults(final JsonInfoInterfaceConfiguration interfaceDefaults) {
    if (interfaceDefaults == null) {
      this.interfaceDefaults = new JsonInfoInterfaceConfiguration();
    } else {
      this.interfaceDefaults = interfaceDefaults;
    }
  }

  /**
   * @return the OS
   */
  public String getOs() {
    return this.os;
  }

  /**
   * @param os the OS to set
   */
  @JsonProperty("os")
  public void setOs(final String os) {
    if (os == null) {
      this.os = "";
    } else {
      this.os = os;
    }
  }

  /**
   * @return the start time
   */
  public long getStartTime() {
    return this.startTime;
  }

  /**
   * @param startTime the start time to set
   */
  @JsonProperty("startTime")
  public void setStartTime(final long startTime) {
    this.startTime = startTime;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.configurationChecksum.hashCode();
    result = (prime * result) + this.cli.hashCode();
    result = (prime * result) + this.configurationFile.hashCode();
    result = (prime * result) + this.olsrPort;
    result = (prime * result) + this.debugLevel;
    result = (prime * result) + (this.noFork ? 1231 : 1237);
    result = (prime * result) + this.pidFile.hashCode();
    result = (prime * result) + (this.hostEmulation ? 1231 : 1237);
    result = (prime * result) + this.ipVersion;
    result = (prime * result) + (this.allowNoInt ? 1231 : 1237);
    result = (prime * result) + this.tosValue;
    result = (prime * result) + this.rtProto;
    result = (prime * result) + this.rtTable.hashCode();
    result = (prime * result) + this.willingness.hashCode();
    result = (prime * result) + this.fib.hashCode();
    result = (prime * result) + this.hysteresis.hashCode();
    result = (prime * result) + this.hna.hashCode();
    long temp = Double.doubleToLongBits(this.pollrate);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.nicChgsPollInt);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (this.clearScreen ? 1231 : 1237);
    result = (prime * result) + this.tcRedundancy;
    result = (prime * result) + this.mprCoverage;
    result = (prime * result) + this.linkQuality.hashCode();
    temp = Double.doubleToLongBits(this.minTCVTime);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (this.setIpForward ? 1231 : 1237);
    result = (prime * result) + this.lockFile.hashCode();
    result = (prime * result) + (this.useNiit ? 1231 : 1237);
    result = (prime * result) + this.smartGateway.hashCode();
    result = (prime * result) + this.mainIp.hashCode();
    result = (prime * result) + this.unicastSourceIpAddress.hashCode();
    result = (prime * result) + (this.srcIpRoutes ? 1231 : 1237);
    result = (prime * result) + this.maxPrefixLength;
    result = (prime * result) + this.ipSize;
    result = (prime * result) + (this.delgw ? 1231 : 1237);
    temp = Double.doubleToLongBits(this.maxSendMessageJitter);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.exitValue;
    temp = Double.doubleToLongBits(this.maxTcValidTime);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + this.niit4to6InterfaceIndex;
    result = (prime * result) + this.niit6to4InterfaceIndex;
    result = (prime * result) + (this.hasIpv4Gateway ? 1231 : 1237);
    result = (prime * result) + (this.hasIpv6Gateway ? 1231 : 1237);
    result = (prime * result) + this.ioctlSocket;
    result = (prime * result) + this.routeNetlinkSocket;
    result = (prime * result) + this.routeMonitorSocket;
    result = (prime * result) + this.routeChangeSocket;
    temp = Double.doubleToLongBits(this.linkQualityNatThreshold);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (int) (this.brokenLinkCost ^ (this.brokenLinkCost >>> 32));
    result = (prime * result) + (int) (this.brokenRouteCost ^ (this.brokenRouteCost >>> 32));
    result = (prime * result) + this.ipcConnectMaxConnections;
    result = (prime * result) + this.ipcConnectAllowed.hashCode();
    result = (prime * result) + this.interfaceDefaults.hashCode();
    result = (prime * result) + this.os.hashCode();
    result = (prime * result) + (int) (this.startTime ^ (this.startTime >>> 32));
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (this.getClass() != obj.getClass()) {
      return false;
    }

    final JsonInfoConfigEntry other = (JsonInfoConfigEntry) obj;

    if (!this.configurationChecksum.equals(other.configurationChecksum)) {
      return false;
    }
    if (!this.cli.equals(other.cli)) {
      return false;
    }
    if (!this.configurationFile.equals(other.configurationFile)) {
      return false;
    }
    if (this.olsrPort != other.olsrPort) {
      return false;
    }
    if (this.debugLevel != other.debugLevel) {
      return false;
    }
    if (this.noFork != other.noFork) {
      return false;
    }
    if (!this.pidFile.equals(other.pidFile)) {
      return false;
    }
    if (this.hostEmulation != other.hostEmulation) {
      return false;
    }
    if (this.ipVersion != other.ipVersion) {
      return false;
    }
    if (this.allowNoInt != other.allowNoInt) {
      return false;
    }
    if (this.tosValue != other.tosValue) {
      return false;
    }
    if (this.rtProto != other.rtProto) {
      return false;
    }
    if (!this.rtTable.equals(other.rtTable)) {
      return false;
    }
    if (!this.willingness.equals(other.willingness)) {
      return false;
    }
    if (!this.fib.equals(other.fib)) {
      return false;
    }
    if (!this.hysteresis.equals(other.hysteresis)) {
      return false;
    }
    if (!this.hna.equals(other.hna)) {
      return false;
    }
    if (Double.compare(this.pollrate, other.pollrate) != 0) {
      return false;
    }
    if (Double.compare(this.nicChgsPollInt, other.nicChgsPollInt) != 0) {
      return false;
    }
    if (this.clearScreen != other.clearScreen) {
      return false;
    }
    if (this.tcRedundancy != other.tcRedundancy) {
      return false;
    }
    if (this.mprCoverage != other.mprCoverage) {
      return false;
    }
    if (!this.linkQuality.equals(other.linkQuality)) {
      return false;
    }
    if (Double.compare(this.minTCVTime, other.minTCVTime) != 0) {
      return false;
    }
    if (this.setIpForward != other.setIpForward) {
      return false;
    }
    if (!this.lockFile.equals(other.lockFile)) {
      return false;
    }
    if (this.useNiit != other.useNiit) {
      return false;
    }
    if (!this.smartGateway.equals(other.smartGateway)) {
      return false;
    }
    if (!this.mainIp.equals(other.mainIp)) {
      return false;
    }
    if (!this.unicastSourceIpAddress.equals(other.unicastSourceIpAddress)) {
      return false;
    }
    if (this.srcIpRoutes != other.srcIpRoutes) {
      return false;
    }
    if (this.maxPrefixLength != other.maxPrefixLength) {
      return false;
    }
    if (this.ipSize != other.ipSize) {
      return false;
    }
    if (this.delgw != other.delgw) {
      return false;
    }
    if (Double.compare(this.maxSendMessageJitter, other.maxSendMessageJitter) != 0) {
      return false;
    }
    if (this.exitValue != other.exitValue) {
      return false;
    }
    if (Double.compare(this.maxTcValidTime, other.maxTcValidTime) != 0) {
      return false;
    }
    if (this.niit4to6InterfaceIndex != other.niit4to6InterfaceIndex) {
      return false;
    }
    if (this.niit6to4InterfaceIndex != other.niit6to4InterfaceIndex) {
      return false;
    }
    if (this.hasIpv4Gateway != other.hasIpv4Gateway) {
      return false;
    }
    if (this.hasIpv6Gateway != other.hasIpv6Gateway) {
      return false;
    }
    if (this.ioctlSocket != other.ioctlSocket) {
      return false;
    }
    if (this.routeNetlinkSocket != other.routeNetlinkSocket) {
      return false;
    }
    if (this.routeMonitorSocket != other.routeMonitorSocket) {
      return false;
    }
    if (this.routeChangeSocket != other.routeChangeSocket) {
      return false;
    }
    if (Double.compare(this.linkQualityNatThreshold, other.linkQualityNatThreshold) != 0) {
      return false;
    }
    if (this.brokenLinkCost != other.brokenLinkCost) {
      return false;
    }
    if (this.brokenRouteCost != other.brokenRouteCost) {
      return false;
    }
    if (this.ipcConnectMaxConnections != other.ipcConnectMaxConnections) {
      return false;
    }
    if (!this.ipcConnectAllowed.equals(other.ipcConnectAllowed)) {
      return false;
    }
    if (!this.interfaceDefaults.equals(other.interfaceDefaults)) {
      return false;
    }
    if (!this.os.equals(other.os)) {
      return false;
    }
    if (this.startTime != other.startTime) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoConfigEntry [configurationChecksum=");
    builder.append(this.configurationChecksum);
    builder.append(", cli=");
    builder.append(this.cli);
    builder.append(", configurationFile=");
    builder.append(this.configurationFile);
    builder.append(", olsrPort=");
    builder.append(this.olsrPort);
    builder.append(", debugLevel=");
    builder.append(this.debugLevel);
    builder.append(", noFork=");
    builder.append(this.noFork);
    builder.append(", pidFile=");
    builder.append(this.pidFile);
    builder.append(", hostEmulation=");
    builder.append(this.hostEmulation);
    builder.append(", ipVersion=");
    builder.append(this.ipVersion);
    builder.append(", allowNoInt=");
    builder.append(this.allowNoInt);
    builder.append(", tosValue=");
    builder.append(this.tosValue);
    builder.append(", rtProto=");
    builder.append(this.rtProto);
    builder.append(", rtTable=");
    builder.append(this.rtTable);
    builder.append(", willingness=");
    builder.append(this.willingness);
    builder.append(", fib=");
    builder.append(this.fib);
    builder.append(", hysteresis=");
    builder.append(this.hysteresis);
    builder.append(", hna=");
    builder.append(this.hna);
    builder.append(", pollrate=");
    builder.append(this.pollrate);
    builder.append(", nicChgsPollInt=");
    builder.append(this.nicChgsPollInt);
    builder.append(", clearScreen=");
    builder.append(this.clearScreen);
    builder.append(", tcRedundancy=");
    builder.append(this.tcRedundancy);
    builder.append(", mprCoverage=");
    builder.append(this.mprCoverage);
    builder.append(", linkQuality=");
    builder.append(this.linkQuality);
    builder.append(", minTCVTime=");
    builder.append(this.minTCVTime);
    builder.append(", setIpForward=");
    builder.append(this.setIpForward);
    builder.append(", lockFile=");
    builder.append(this.lockFile);
    builder.append(", useNiit=");
    builder.append(this.useNiit);
    builder.append(", smartGateway=");
    builder.append(this.smartGateway);
    builder.append(", mainIp=");
    builder.append(this.mainIp);
    builder.append(", unicastSourceIpAddress=");
    builder.append(this.unicastSourceIpAddress);
    builder.append(", srcIpRoutes=");
    builder.append(this.srcIpRoutes);
    builder.append(", maxPrefixLength=");
    builder.append(this.maxPrefixLength);
    builder.append(", ipSize=");
    builder.append(this.ipSize);
    builder.append(", delgw=");
    builder.append(this.delgw);
    builder.append(", maxSendMessageJitter=");
    builder.append(this.maxSendMessageJitter);
    builder.append(", exitValue=");
    builder.append(this.exitValue);
    builder.append(", maxTcValidTime=");
    builder.append(this.maxTcValidTime);
    builder.append(", niit4to6InterfaceIndex=");
    builder.append(this.niit4to6InterfaceIndex);
    builder.append(", niit6to4InterfaceIndex=");
    builder.append(this.niit6to4InterfaceIndex);
    builder.append(", hasIpv4Gateway=");
    builder.append(this.hasIpv4Gateway);
    builder.append(", hasIpv6Gateway=");
    builder.append(this.hasIpv6Gateway);
    builder.append(", ioctlSocket=");
    builder.append(this.ioctlSocket);
    builder.append(", routeNetlinkSocket=");
    builder.append(this.routeNetlinkSocket);
    builder.append(", routeMonitorSocket=");
    builder.append(this.routeMonitorSocket);
    builder.append(", routeChangeSocket=");
    builder.append(this.routeChangeSocket);
    builder.append(", linkQualityNatThreshold=");
    builder.append(this.linkQualityNatThreshold);
    builder.append(", brokenLinkCost=");
    builder.append(this.brokenLinkCost);
    builder.append(", brokenRouteCost=");
    builder.append(this.brokenRouteCost);
    builder.append(", ipcConnectMaxConnections=");
    builder.append(this.ipcConnectMaxConnections);
    builder.append(", ipcConnectAllowed=");
    builder.append(this.ipcConnectAllowed);
    builder.append(", interfaceDefaults=");
    builder.append(this.interfaceDefaults);
    builder.append(", os=");
    builder.append(this.os);
    builder.append(", startTime=");
    builder.append(this.startTime);
    builder.append("]");
    return builder.toString();
  }
}