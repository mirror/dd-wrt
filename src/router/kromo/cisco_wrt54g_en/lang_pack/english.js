//////////////////////////////////////////////////////////////////////////////////////////////
//		English reference translation file - DD-WRT V23 SP1 by Botho 17/05/2006				//
//////////////////////////////////////////////////////////////////////////////////////////////


// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var share = new Object();
share.firmware="Firmware";
share.time="Time";
share.interipaddr="Internet IP Address";
share.more="More...";
share.help="Help";
share.enable="Enable";
share.enabled="Enabled";
share.disable="Disable";
share.disabled="Disabled";
share.usrname="User Name";
share.passwd="Password";
share.hostname="Host Name";
share.domainname="Domain Name";
share.wandomainname="WAN Domain Name";
share.landomainname="LAN Domain Name";
share.statu="Status";
share.start="Start";
share.end="End";
share.proto="Protocol";
share.ip="IP Address";
share.mac="MAC Address";
share.none="None";
share.none2="no";
share.both="Both";
share.del="Delete";
share.remove="Remove";
share.descr="Description";
share.from="From";
share.to="To";
share.about="About";
share.everyday="Everyday";
share.sun="Sunday";
share.sun_s="Sun";
share.sun_s1="S";
share.mon="Monday";
share.mon_s="Mon";
share.mon_s1="M";
share.tue="Tuesday";
share.tue_s="Tue";
share.tue_s1="T";
share.wed="Wednesday";
share.wed_s="Wed";
share.wed_s1="W";
share.thu="Thursday";
share.thu_s="Thu";
share.thu_s1="T";
share.fri="Friday";
share.fri_s="Fri";
share.fri_s1="F";
share.sat="Saturday";
share.sat_s="Sat";
share.sat_s1="S";
share.expires="Expires";
share.yes="Yes";
share.no="No";
share.allow="Allow";
share.deny="Deny";
share.range="Range";
share.use="Use";
share.mins="Min.";
share.secs="Sec.";
share.routername="Router Name";
share.manual="Manual";
share.port="Port";
share.ssid="SSID";
share.channel="Channel";
share.rssi="Rssi";
share.signal="Signal";
share.noise="Noise";
share.beacon="beacon";
share.openn="Open";
share.dtim="dtim";
share.rates="Rate";
share.low="Low";
share.medium="Medium";
share.high="High";
share.option="Options";
share.rule="Rule";
share.lan="LAN";
share.point2point="Point to Point";
share.nat="NAT";
share.subnet="Subnet Mask";
share.unmask="Unmask";
share.deflt="Default";  //don't use share.default !!!
share.all="All";
share.auto="Auto";
share.right="Right";
share.left="Left";
share.share_key="Shared Key";
share.inter="Interval (in seconds)";
share.srv="Service Name";
share.port_range="Port Range";
share.priority="Priority";
share.gateway="Gateway";
share.intrface="Interface";  //don't use share.interface, Mozilla problem!!!
share.pintrface="Physical Interface";
share.vintrface="Virtual Interfaces";
share.router="Router";
share.static_lease="Static Leases";
share.srvip="Server IP";
share.localdns="Local DNS";
share.minutes="minutes";
share.oui="OUI Search";
share.sttic="Static";
share.connecting="Connecting";
share.connect="Connect";
share.connected="Connected";
share.disconnect="Disconnect";
share.disconnected="Disconnected";
share.info="Information";
share.state="State";
share.mode="Mode";
share.encrypt="Encryption";
share.key="Key";
share.wireless="Wireless";
share.dhcp="DHCP";
share.styl="Style";
share.err="error";
share.errs="errors";
share.meters="meters";
share.full="Full";
share.half="Half";
share.quarter="Quarter";
share.seealso="See also";
share.never="never";
share.unknown="Unknown";
share.expired="expired";
share.logout="logout";
share.nmounted="Not mounted";
share.fssize="Total / Free Size";
share.src="Source Address";
share.dst="Remote Address";
share.name_resolution="Name Resolution";
share.timeout="Timeout (s)";
share.detail="Click to see detail";
share.tmpmem="Temporary Memory";
share._1h="Every hour";
share._2h="Every 2 hours";
share._3h="Every 3 hours";
share._4h="Every 4 hours";
share._5h="Every 5 hours";
share._6h="Every 6 hours";
share._12h="Every 12 hours";
share._24h="Every day";
share._48h="Every 2 days";
share._168h="Every week";
share.days="Days";
share.standard="Standard";


var sbutton = new Object();
sbutton.save="Save Settings";
sbutton.saving="Saved";
sbutton.cmd="Executing";
sbutton.cancel="Cancel Changes";
sbutton.refres="Refresh";
sbutton.clos="Close";
sbutton.del="Delete";
sbutton.continu="Continue";
sbutton.add="Add";
sbutton.remove="Remove";
sbutton.modify="Modify";
sbutton.deleted="Deleted";
sbutton.delall="Delete All";
sbutton.autorefresh="Auto-Refresh is On";
sbutton.backup="Backup";
sbutton.restore="Restore";
sbutton.cptotext="Edit";
sbutton.runcmd="Run Commands";
sbutton.startup="Save Startup";
sbutton.firewall="Save Firewall";
sbutton.wol="Wake Up";
sbutton.add_wol="Add Host";
sbutton.manual_wol="Manual Wake Up";
sbutton.summary="Summary";
sbutton.filterIP="Edit List of PCs";
sbutton.filterMac="Edit MAC Filter List";
sbutton.filterSer="Add/Edit Service";
sbutton.reboot="Reboot Router";
sbutton.help="   Help  ";
sbutton.wl_client_mac="Wireless Client MAC List";
sbutton.update_filter="Update Filter List";
sbutton.join="Join";
sbutton.log_in="Incoming Log";
sbutton.log_out="Outgoing Log";
sbutton.apply="Apply";
sbutton.edit_srv="Add/Edit Service";
sbutton.routingtab="Show Routing Table";
sbutton.wanmac="Get Current PC MAC Address";
sbutton.dhcprel="DHCP Release";
sbutton.dhcpren="DHCP Renew";
sbutton.survey="Site Survey";
sbutton.upgrading="Upgrading";
sbutton.upgrade="Upgrade";
sbutton.preview="Preview";
sbutton.allways_on="Always On";
sbutton.allways_off="Always Off";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//
var errmsg = new Object();
errmsg.err0="You must input a username.";
errmsg.err1="You must input a Router Name.";
errmsg.err2="Out of range, please adjust start IP address or user&#39;s numbers.";
errmsg.err3="You must at least select a day."
errmsg.err4="The end time must be bigger than start time.";
errmsg.err5="The MAC Address length is not correct.";
errmsg.err6="You must input a password.";
errmsg.err7="You must input a hostname.";
errmsg.err8="You must input an IP Address or Domain Name.";
errmsg.err9="Illegal DMZ IP Address.";
errmsg.err10="Confirmed password did not match Entered Password. Please re-enter password.";
errmsg.err11="No spaces are allowed in Password";
errmsg.err12="You must input a command to run.";
errmsg.err13="Upgrade failed.";
errmsg.err45="Not available in HTTPS! Please connect in HTTP mode.";
errmsg.err46="Not available in HTTPS";


//common.js error messages
errmsg.err14=" value is out of range [";
errmsg.err15="The WAN MAC Address is out of range [00 - ff].";
errmsg.err16="The second character of MAC must be even number : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="The MAC Address is not correct.";
errmsg.err18="The MAC Address length is not correct.";
errmsg.err19="The MAC Address cannot be the broadcast address."
errmsg.err20="Enter MAC Address in (xx:xx:xx:xx:xx:xx) format.";
errmsg.err21="Invalid MAC address format.";
errmsg.err22="The WAN MAC Address is not correct.";
errmsg.err23="Invalid hex value ";
errmsg.err24=" found in MAC address ";
errmsg.err25="The key value is not correct.";
errmsg.err26="The key length is not correct.";
errmsg.err27="Illegal subnet mask.";
errmsg.err28=" have illegal characters, must be [ 0 - 9 ].";
errmsg.err29=" have illegal ascii code.";
errmsg.err30=" have illegal hexadecimal digits.";
errmsg.err31=" value is illegal.";
errmsg.err32="IP address and gateway is not at same subnet mask.";
errmsg.err33="IP address and gateway can't be same.";
errmsg.err34=" is not allowed to contain a space.";

//Wol.asp error messages
errmsg.err35="You must input a MAC address to run.";
errmsg.err36="You must input a network broadcast address to run.";
errmsg.err37="You must input a UDP port to run.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Please enter a Shared Key!";
errmsg.err39="Invalid Key, must be between 8 and 63 ASCII characters or 64 hexadecimal digits"
errmsg.err40="You have to enter a key for Key ";
errmsg.err41="Invalid Length in key ";
errmsg.err43="Rekey interval";

//config.asp error messages
errmsg.err42="Please select a configuration file to restore.";

//WL_ActiveTable.asp error messages
errmsg.err44="The total checks exceed 128 counts.";

//Site_Survey.asp error messages
errmsg.err47="invalid SSID.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS is not compatible with the current configuration of the router. Please check the following points :\n * Wireless Mode must be set to AP \n * WPA2 is not supported under WDS \n * Wireless Network B-Only mode is not supported under WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius is only available in AP mode.";

//Wireless_Basic.asp error messages
errmsg.err50="You must input a SSID.";

// Management.asp error messages
errmsg.err51="The Router is currently set to its default password. As a security measure, you must change the password before the Remote Management feature can be enabled. Click the OK button to change your password. Click the Cancel button to leave the Remote Management feature disabled.";
errmsg.err52="Password confirmation doesn't match.";

// Port_Services.asp error messages
errmsg.err53="After finished all actions, click the Apply button to save the settings.";
errmsg.err54="You must input a Service Name.";
errmsg.err55="The Service Name exists.";

// QoS.asp error messages
errmsg.err56="Port value is out of range [0 - 65535]";

// Routing.asp error messages
errmsg.err57="Delete the Entry ?";

// Status_Lan.asp error messages
errmsg.err58="Click to delete lease";

//Status_Wireless.asp error messages
errmsg.err59="Not available! Please enable Wireless Network.";

//Upgrade.asp error messages
errmsg.err60="Please select a file to upgrade.";
errmsg.err61="Incorrect image file.";

//Services.asp error messages
errmsg.err62=" is already defined as a static lease.";

//Saving message
errmsg.err100="Processing...<br/>Please, wait.";
errmsg.err101="Restoring configuration file...<br/>Please, wait.";
errmsg.err102="Upgrading firmware...<br/>Please, wait.";

// *******************************************  COMMON MENU ENTRIES  *******************************************//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basic Setup";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Clone";
bmenu.setuprouting="Advanced Routing";
bmenu.setupvlan="VLANs";
bmenu.setupeop="EoIP Tunnel";

bmenu.wireless="Wireless";
bmenu.wirelessBasic="Basic Settings";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Wireless Security";
bmenu.wirelessMac="MAC Filter";
bmenu.wirelessAdvanced="Advanced Settings";
bmenu.wirelessWds="WDS";

bmenu.security="Security";
bmenu.firwall="Firewall";
bmenu.vpn="VPN";

bmenu.accrestriction="Access Restrictions";
bmenu.webaccess="Internet Access";


bmenu.applications="Applications &amp; Gaming";
bmenu.applicationsprforwarding="Port Range Forwarding";
bmenu.applicationspforwarding="Port Forwarding";
bmenu.applicationsptriggering="Port Triggering";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";
bmenu.applicationsP2P="P2P";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Overview";
bmenu.sipathphone="Phonebook";
bmenu.sipathstatus="Status";

bmenu.admin="Administration";
bmenu.adminManagement="Management";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Services";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Log";
bmenu.adminDiag="Commands";
bmenu.adminWol="WOL";
bmenu.adminFactory="Factory Defaults";
bmenu.adminUpgrade="Firmware Upgrade";
bmenu.adminBackup="Backup";


bmenu.statu="Status";
bmenu.statuRouter="Router";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Sputnik Agent";
bmenu.statuWLAN="Wireless";
bmenu.statuVPN="OpenVPN";
bmenu.statuBand="Bandwidth";
bmenu.statuSysInfo="Sys-Info";


// ******************************************* Alive.asp *******************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Schedule Reboot";
alive.sevr1="Schedule Reboot";
alive.hour="At a set Time";
alive.legend2="WDS/Connection Watchdog";
alive.sevr2="Enable Watchdog";
alive.IP="IP Addresses";
alive.legend3="Proxy/Connection Watchdog";
alive.sevr3="Enable Proxy Watchdog";
alive.IP2="Proxy IP Address";
alive.port="Proxy Port";

//help container
var halive = new Object();
halive.right2="Choose when reboot the router. Cron must be enabled in the managment tab.";
halive.right4="A maximum of three IPs separated by a <em>SPACE</em> is allowed.<BR/>IPs Format is xxx.xxx.xxx.xxx.";



// ******************************************* config.asp *******************************************//

var config = new Object();
config.titl=" - Backup & Restore";
config.h2="Backup Configuration";
config.legend="Backup Settings";
config.mess1="Click the \"" + sbutton.backup + "\" button to download the configuration backup file to your computer.";
config.h22="Restore Configuration";
config.legend2="Restore Settings";
config.mess2="Please select a file to restore";
config.mess3="W A R N I N G";
config.mess4="Only upload files backed up using this firmware and from the same model of router.<br />Do not upload any files that were not created by this interface!";

//help container
var hconfig = new Object();
hconfig.right2="You may backup your current configuration in case you need to reset the router back to its factory default settings.<br /><br />Click the <em>Backup</em> button to backup your current configuration.";
hconfig.right4="Click the <em>Browse...</em> button to browse for a configuration file that is currently saved on your PC.<br /><br />Click the <em>" + sbutton.restore + "</em> button to overwrite all current configurations with the ones in the configuration file.";



// ******************************************* DDNS.asp *******************************************//

var ddns = new Object();
ddns.titl=" - Dynamic DNS"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS Service";
ddns.emailaddr="E-mail Address";
ddns.typ="Type";
ddns.dynamic="Dynamic";
ddns.custom="Custom";
ddns.wildcard="Wildcard";
ddns.statu="DDNS Status";
ddns.system="DYNDNS Server";
ddns.options="Additional DDNS Options";
ddns.forceupd="Force Update Interval";

var ddnsm = new Object();
ddnsm.all_closed="DDNS server is currently closed";
ddnsm.all_resolving="Resolving domain name";
ddnsm.all_errresolv="Domain name resolv fail";
ddnsm.all_connecting="Connecting to server";
ddnsm.all_connectfail="Connect to server fail";
ddnsm.all_disabled="DDNS function is disabled";
ddnsm.all_noip="No Internet connection";

//help container
var hddns = new Object();
hddns.right2="DDNS allows you to access your network using domain names instead of IP addresses. The service manages changing IP addresses and updates your domain information dynamically. You must sign up for service through DynDNS.org, freedns.afraid.org, ZoneEdit.com, No-IP.com or Custom.";
hddns.right4="Type an integer number in the box to set the force update interval (in days). Updates should only be performed automaticaly when your IP address has changed. Beware of your DDNS provider update abuse policy to avoid your hostname or domain to be blocked.";



// ******************************************* Diagnostics.asp *******************************************//

var diag = new Object();
diag.titl=" - Diagnostics";
diag.h2="Diagnostics";
diag.legend="Command Shell";
diag.cmd="Commands";
diag.startup="Startup";
diag.firewall="Firewall";

//help container
var hdiag = new Object();
hdiag.right2="You can run command lines via the web interface. Fill the text area with your command and click <em>" + sbutton.runcmd + "</em> to submit.";



// ******************************************* DMZ.asp *******************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Demilitarized Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="Use DMZ";
dmz.host="DMZ Host IP Address";


//help container
var hdmz = new Object();
hdmz.right2="Enabling this option will expose the specified host to the Internet. All ports will be accessible from the Internet.";



// ******************************************* Factory_Defaults.asp *******************************************//

var factdef = new Object();
factdef.titl=" - Factory Defaults";
factdef.h2="Factory Defaults";
factdef.legend="Reset router settings";
factdef.restore="Restore Factory Defaults";

factdef.mess1="Warning! If you click OK, the device will reset to factory default and all previous settings will be erased.";

//help container
var hfactdef = new Object();
hfactdef.right1="This will reset all settings back to factory defaults. All of your settings will be erased.";



// ******************************************* FilterIPMAC.asp *******************************************//

var filterIP = new Object();
filterIP.titl=" - List of PCs";
filterIP.h2="List of PCs";
filterIP.h3="Enter MAC Address of the PCs in this format: xx:xx:xx:xx:xx:xx";
filterIP.h32="Enter the IP Address of the PCs";
filterIP.h33="Enter the IP Range of the PCs";
filterIP.ip_range="IP Range";



// ******************************************* Filter.asp *******************************************//

var filter = new Object();
filter.titl=" - Access Restrictions";
filter.h2="Internet Access";
filter.legend="Access Policy";
filter.restore="Restore Factory Defaults";
filter.pol="Policy";
filter.polname="Policy Name";
filter.pcs="PCs";
filter.polallow="Internet access during selected days and hours.";
filter.legend2="Days";
filter.time="Times";
filter.h24="24 Hours";
filter.legend3="Blocked Services";
filter.catchall="Catch all P2P Protocols";
filter.legend4="Website Blocking by URL Address";
filter.legend5="Website Blocking by Keyword";

filter.mess1="Delete the Policy?";
filter.mess2="You must at least select a day.";
filter.mess3="The end time must be bigger than start time.";

//help container
var hfilter = new Object();
hfilter.right2="You may define up to 10 access policies. Click <em>" + sbutton.del + "</em> to delete a policy or <em>" + sbutton.summary + "</em> to see a summary of the policy.";
hfilter.right4="Enable or disable a policy.";
hfilter.right6="You may assign a name to your policy.";
hfilter.right8="Choose the day of the week you would like your policy to be applied.";
hfilter.right10="Enter the time of the day you would like your policy to apply.";
hfilter.right12="You may choose to block access to certain services. Click <em>" + sbutton.filterSer + "</em> to modify these settings.";
hfilter.right14="You can block access to certain websites by entering their URL.";
hfilter.right16="You can block access to certain website by the keywords contained in their webpage.";



// ******************************************* FilterSummary.asp *******************************************//

var filterSum = new Object();
filterSum.titl=" - Access Restrictions Summary";
filterSum.h2="Internet Policy Summary";
filterSum.polnum="No.";
filterSum.polday="Time of Day";



// ******************************************* Firewall.asp *******************************************//

var firewall = new Object();
firewall.titl=" - Firewall";
firewall.h2="Security";
firewall.legend="Firewall Protection";
firewall.firewall="SPI Firewall";
firewall.legend2="Additional Filters";
firewall.proxy="Filter Proxy";
firewall.cookies="Filter Cookies";
firewall.applet="Filter Java Applets";
firewall.activex="Filter ActiveX";
firewall.legend3="Block WAN Requests";
firewall.ping="Block Anonymous Internet Requests (ping)";
firewall.muticast="Filter Multicast";
filter.nat="Filter Internet NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container
var hfirewall = new Object();
hfirewall.right2="Enable or disable the SPI firewall.";



// ******************************************* Forward.asp *******************************************//

var prforward = new Object();
prforward.titl=" - Port Range Forwarding";
prforward.h2="Port Range Forward";
prforward.legend="Forwards";
prforward.app="Application";

//help container
var hprforward = new Object();
hprforward.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";


// ******************************************* P2P.asp *******************************************//

var p2p = new Object();
p2p.titl=" - Peer-to-Peer Apps";
p2p.h2="BitTorrent Client";
p2p.legend="CTorrent";
p2p.ctorrent_srv="Ctorrent Service";


//help container
//var hp2p = new Object();
//hpp2p.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";



// ******************************************* ForwardSpec.asp *******************************************//

var pforward = new Object();
pforward.titl=" - Port Forwarding";
pforward.h2="Port Forward";
pforward.legend="Forwards";
pforward.app="Application";
pforward.from="Port from";
pforward.to="Port to";

//help container
var hpforward = new Object();
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>Enable</em> checkbox after you are finished.";



// ******************************************* Hotspot.asp *******************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot Portal";
hotspot.legend="Chillispot";
hotspot.nowifibridge="Separate Wifi from the LAN Bridge";
hotspot.hotspot="Chillispot";
hotspot.pserver="Primary Radius Server IP/DNS";
hotspot.bserver="Backup Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="Redirect URL";
hotspot.dhcp="DHCP Interface";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM Secret";
hotspot.uamdns="UAM Any DNS";
hotspot.allowuam="UAM Allowed";
hotspot.macauth="MACauth";
hotspot.option="Additional Chillispot Options";

hotspot.fon_chilli="Chillispot Local User Management";
hotspot.fon_user="User List";

hotspot.http_legend="HTTP Redirect";
hotspot.http_srv="HTTP Redirect";
hotspot.http_ip="HTTP Destination IP";
hotspot.http_port="HTTP Destination Port";
hotspot.http_net="HTTP Source Network";

hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Gateway Name";
hotspot.nocat_home="Home Page";
hotspot.nocat_allowweb="Allowed Web Hosts";
hotspot.nocat_docroot="Document Root";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="Exclude Ports";
hotspot.nocat_timeout="Login Timeout";
hotspot.nocat_verbose="Verbosity";
hotspot.nocat_route="Route Only";
hotspot.nocat_MAClist="MAC White List";

hotspot.smtp_legend="SMTP Redirect";
hotspot.smtp_srv="SMTP Redirect";
hotspot.smtp_ip="SMTP Destination IP";
hotspot.smtp_net="SMTP Source Network";

hotspot.shat_legend="Zero IP Config";
hotspot.shat_srv="Zero IP Config";
hotspot.shat_srv2="Zero IP Config enabled";

hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik Agent";
hotspot.sputnik_mode="Sputnik Mode";
hotspot.sputnik_id="Sputnik Server ID";
hotspot.sputnik_instant="Use Sputnik Instant Setup";
hotspot.sputnik_express="Use SputnikNet Express";
hotspot.sputnik_about="about Sputnik";
hotspot.sputnik_learn="Learn more";

hotspot.wifidog_legend="Wifidog";
hotspot.wifidog_srv="Wifidog daemon";
hotspot.wifidog_id="Gateway ID";
hotspot.wifidog_url="Portal's URL";
hotspot.wifidog_port="Port";
hotspot.wifidog_httpdname="Web Server Name";
hotspot.wifidog_httpdconn="Max Users";
hotspot.wifidog_checkinter="CheckInterval (in sec.)";
hotspot.wifidog_checktimeout="ClientTimeout";
hotspot.wifidog_authsrv="AuthServer Hostname";
hotspot.wifidog_authsrvssl="AuthServer SSL Available";
hotspot.wifidog_authsrvsslport="AuthServer SSL Port";
hotspot.wifidog_authsrvhttpport="AuthServer HTTP Port";
hotspot.wifidog_authsrvpath="AuthServer Path";


// ******************************************* Info.htm *******************************************//

var info = new Object();
info.titl=" - Info";
info.h2="System Information";
info.wlanmac="Wireless MAC";
info.srv="Services";
info.ap="Access Point";



// ******************************************* index_heartbeat.asp *******************************************//

var idx_h = new Object();
idx_h.srv="Heart Beat Server";
idx_h.con_strgy="Connection Strategy";
idx_h.max_idle="Connect on Demand: Max Idle Time";
idx_h.alive="Keep Alive: Redial Period";



// ******************************************* index_l2tp.asp *******************************************//

var idx_l = new Object();
idx_l.srv="L2TP Server";



// ******************************************* index_pppoe.asp *******************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="Use RP PPPoE";



// ******************************************* index_pptp.asp *******************************************//

var idx_pptp = new Object();
idx_pptp.srv="Use DHCP";
idx_pptp.wan_ip="Internet IP Address";
idx_pptp.gateway="Gateway (PPTP Server)";
idx_pptp.encrypt="PPTP Encyption";



// ******************************************* index_static.asp *******************************************//

var idx_static = new Object();
idx_static.dns="Static DNS";



// ******************************************* index.asp *******************************************//

var idx = new Object();
idx.titl=" - Setup";
idx.h2="Internet Setup";
idx.h22="Wireless Setup";
idx.legend="Internet Connection Type";
idx.conn_type="Connection Type";
idx.stp="STP";
idx.stp_mess="(disable for COMCAST ISP)";
idx.optional="Optional Settings (required by some ISPs)";
idx.mtu="MTU";
idx.h23="Network Setup";
idx.routerip="Router IP";
idx.lanip="Local IP Address";
idx.legend2="WAN Port";
idx.wantoswitch="Assign WAN Port to Switch";
idx.legend3="Time Settings";
idx.timeset="Time Zone / Summer Time (DST)";
idx.localtime="Use local time";
idx.static_ip="Static IP";
idx.dhcp="Automatic Configuration - DHCP";
idx.dhcp_legend="Network Address Server Settings (DHCP)";
idx.dhcp_type="DHCP Type";
idx.dhcp_srv="DHCP Server";
idx.dhcp_fwd="DHCP Forwarder";
idx.dhcp_start="Start IP Address";
idx.dhcp_end="End IP Address";		//used in Status_Lan.asp
idx.dhcp_maxusers="Maximum DHCP Users";
idx.dhcp_lease="Client Lease Time";
idx.dhcp_dnsmasq="Use DNSMasq for DHCP";
idx.dns_dnsmasq="Use DNSMasq for DNS";
idx.auth_dnsmasq="DHCP-Authoritative";
idx.summt_opt1="none";
idx.summt_opt2="first Sun Apr - last Sun Oct";
idx.summt_opt3="last Sun Mar - last Sun Oct";
idx.summt_opt4="last Sun Oct - last Sun Mar";
idx.summt_opt5="2nd Sun Mar - first Sun Nov";



//help container
var hidx = new Object();
hidx.right2="This setting is most commonly used by Cable operators.";
hidx.right4="Enter the host name provided by your ISP.";
hidx.right6="Enter the domain name provided by your ISP.";
hidx.right8="This is the address of the router.";
hidx.right10="This is the subnet mask of the router.";
hidx.right12="Allows the router to manage your IP addresses.";
hidx.right14="The address you would like to start with.";
hidx.right16="You may limit the number of addresses your router hands out.";
hidx.right18="Choose the time zone you are in and Summer Time (DST) period. The router can use local time or UTC time.";



// ******************************************* Join.asp *******************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Join";
join.mess1="Successfully joined the following network as a client: ";



// ******************************************* Log_incoming.asp *******************************************//

var log_in = new Object();
log_in.titl=" - Incoming Log Table";
log_in.h2="Incoming Log Table";
log_in.th_ip="Source IP";
log_in.th_port="Destination Port Number";



// ******************************************* Log_outgoing.asp *******************************************//

var log_out = new Object();
log_out.titl=" - Outgoing Log Table";
log_out.h2="Outgoing Log Table";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destination URL/IP";
log_out.th_port="Service/Port Number";



// ******************************************* Log.asp *******************************************//

var log = new Object();
log.titl=" - Log";
log.h2="Log Management";
log.legend="Log";
log.lvl="Log Level";
log.drop="Dropped";
log.reject="Rejected";
log.accept="Accepted";



// ******************************************* Management.asp *******************************************//

var management = new Object();
management.titl=" - Administration";
management.h2="Router Management";

management.psswd_legend="Router Password";
management.psswd_user="Router Username";
management.psswd_pass="Router Password";
management.pass_conf="Re-enter to confirm";

management.remote_legend="Remote Access";
management.remote_gui="Web GUI Management";
management.remote_https="Use HTTPS";
management.remote_guiport="Web GUI Port";
management.remote_ssh="SSH Management";
management.remote_sshport="SSH Remote Port";

management.web_legend="Web Access";
management.web_refresh="Auto-Refresh (in seconds)";
management.web_sysinfo="Enable Info Site";
management.web_sysinfopass="Info Site Password Protection";
management.web_sysinfomasq="Info Site MAC Masquerading";

management.boot_legend="Boot Wait";
management.boot_srv="Boot Wait";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="NTP Client";
management.ntp_srv="NTP";

management.rst_legend="Reset Button";
management.rst_srv="Reset Button";

management.routing_legend="Routing";
management.routing_srv="Routing";

management.ipv6_legend="IPv6 Support";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd enabled";
management.ipv6_radconf="Radvd config";

management.jffs_legend="JFFS2 Support";
management.jffs_srv="JFFS2";
management.jffs_clean="Clean JFFS2";

management.lang_legend="Language Selection";
management.lang_srv="Language";
management.lang_bulgarian="bulgarian";
management.lang_chinese_traditional="chinese traditional";
management.lang_chinese_simplified="chinese simplified";
management.lang_croatian="croatian";
management.lang_czech="czech";
management.lang_dutch="dutch";
management.lang_portuguese_braz="portuguese (brazilian)";
management.lang_english="english";
management.lang_polish="polish";
management.lang_french="french";
management.lang_german="german";
management.lang_italian="italian";
management.lang_brazilian="brazilian";
management.lang_slovenian="slovenian";
management.lang_spanish="spanish";
management.lang_swedish="swedish";
management.lang_japanese="japanese";

management.net_legend="IP Filter Settings (adjust these for P2P)";
management.net_port="Maximum Ports";
management.net_tcptimeout="TCP Timeout (in seconds)";
management.net_udptimeout="UDP Timeout (in seconds)";

management.clock_legend="Overclocking";
management.clock_frq="Frequency";
management.clock_support="Not Supported";

management.mmc_legend="MMC/SD Card Support";
management.mmc_srv="MMC Device";

management.samba_legend="Samba FS Automount";
management.samba_srv="SMB Filesystem";
management.samba_share="Share";
management.samba_stscript="Startscript";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP Port";
management.SIPatH_domain="SIP Domain";

management.gui_style="Router GUI Style";



//help container
var hmanagement = new Object();
hmanagement.right1="Auto-Refresh:";
hmanagement.right2="Adjusts the Web GUI automatic refresh interval. 0 disables this feature completely.";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

var portserv = new Object();
portserv.titl=" - Port Services";
portserv.h2="Port Services";



// ******************************************* QoS.asp *******************************************//

var qos = new Object();
qos.titl=" - Quality of Service";
qos.h2="Quality Of Service (QoS)";
qos.legend="QoS Settings";
qos.srv="Start QoS";
qos.type="Packet Scheduler";
qos.uplink="Uplink (kbps)";
qos.dnlink="Downlink (kbps)";
qos.gaming="Optimize for Gaming";
qos.legend2="Services Priority";
qos.prio_x="Exempt";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_b="Bulk";
qos.legend3="Netmask Priority";
qos.ipmask="IP/Mask";
qos.maxrate_b="Max Kbits";
qos.maxrate_o="Max Rate";
qos.legend4="MAC Priority";
qos.legend5="Ethernet Port Priority";
qos.legend6="Default Bandwith Level";
qos.bandwith="Bandwith in Kbits";

//help container
var hqos = new Object();
hqos.right1="Uplink:";
hqos.right2="Set this to 80%-95% (max) of your total upload limit.";
hqos.right3="Downlink:";
hqos.right4="Set this to 80%-100% (max) of your total download limit.";
hqos.right6="You may control your data rate with respect to the application that is consuming bandwidth.";
hqos.right8="You may specify priority for all traffic from a given IP address or IP Range.";
hqos.right10="You may specify priority for all traffic from a device on your network by giving the device a Device Name, specifying priority and entering its MAC address.";
hqos.right12="You may control your data rate according to which physical LAN port your device is plugged into. You may assign Priorities accordingly for devices connected on LAN ports 1 through 4.";



// ******************************************* RouteTable.asp *******************************************//

var routetbl = new Object();
routetbl.titl=" - Routing Table";
routetbl.h2="Routing Table Entry List";
routetbl.th1="Destination LAN IP";



// ******************************************* Routing.asp *******************************************//

var route = new Object();
route.titl=" - Routing";
route.h2="Advanced Routing";
route.mod="Operating Mode";
route.bgp_legend="BGP Settings";
route.bgp_ip="Neighbor IP";
route.bgp_as="Neighbor AS#";
route.rip2_mod="RIP2 Router";
route.ospf_mod="OSPF Router";
route.gateway_legend="Dynamic Routing";
route.static_legend="Static Routing";
route.static_setno="Select set number";
route.static_name="Route Name";
route.static_ip="Destination LAN IP";

//help container
var hroute = new Object();
hroute.right2="If the router is hosting your Internet connection, select <em>Gateway</em> mode. If another router exists on your network, select <em>Router</em> mode.";
hroute.right4="This is the unique route number, you may set up to 20 routes.";
hroute.right6="Enter the name you would like to assign to this route.";
hroute.right8="This is the remote host to which you would like to assign the static route.";
hroute.right10="Determines the host and the network portion.";


// ******************************************* Site_Survey.asp *******************************************//

var survey = new Object();
survey.titl=" - Site Survey";
survey.h2="Neighbor&#39;s Wireless Networks";
survey.thjoin="Join Site";



// ******************************************* Services.asp *******************************************//

var service = new Object();

service.titl=" - Services";
service.h2="Services Management";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Start Kaid";
service.kaid_mac="Console Macs: (must end with;)";

//DHCPd
service.dhcp_legend="DHCP Client";
service.dhcp_vendor="Set Vendorclass";
service.dhcp_legend2="DHCP Server";
service.dhcp_srv="DHCP Daemon";
service.dhcp_jffs2="Use JFFS2 for client lease DB";
service.dhcp_nvramlease="Use NVRAM for client lease DB";
service.dhcp_domain="Used Domain";
service.dhcp_landomain="LAN Domain";
service.dhcp_option="Additional DHCPd Options";
service.dnsmasq_legend="DNSMasq";
service.dnsmasq_srv="DNSMasq";
service.dnsmasq_loc="Local DNS";
service.dnsmasq_opt="Additional DNS Options";

//pptp.webservices
service.pptp_legend="PPTP";
service.pptp_srv="PPTP Server";
service.pptp_client="Client IP(s)";
service.pptp_chap="CHAP-Secrets";

//syslog.webservices
service.syslog_legend="System Log";
service.syslog_srv="Syslogd";
service.syslog_ip="Remote Server";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//pptpd_client.webservices
service.pptpd_legend="PPTP Client";
service.pptpd_option="PPTP Client Options";
service.pptpd_ipdns="Server IP or DNS Name";
service.pptpd_subnet="Remote Subnet";
service.pptpd_subnetmask="Remote Subnet Mask";
service.pptpd_encry="MPPE Encryption";
service.pptpd_mtu="MTU";
service.pptpd_mru="MRU";
service.pptpd_nat="NAT";

//rflow.webservices
service.rflow_legend="RFlow / MACupd";
service.rflow_srv1="RFlow";
service.rflow_srv2="MACupd";

//pppoe-relay.webservices
service.pppoe_legend="PPPOE Relay";
service.pppoe_srv="Relay";

//snmp.webservices
service.snmp_legend="SNMP";
service.snmp_srv="SNMP";
service.snmp_loc="Location";
service.snmp_contact="Contact";
service.snmp_name="Name";
service.snmp_read="RO Community";
service.snmp_write="RW Community";

//openvpn.webservices
service.vpn_legend="OpenVPN Client";
service.vpn_srv="Start OpenVPN";
service.vpn_ipname="Server IP/Name";
service.vpn_mtu="TUN MTU Setting";
service.vpn_mru="TUN MTU Extra";
service.vpn_mss="TCP MSS";
service.vpn_compress="Use LZO Compression";
service.vpn_tunnel="Tunnel Protocol";
service.vpn_srvcert="Public Server Cert";
service.vpn_clicert="Public Client Cert";
service.vpn_certtype="nsCertType";
service.vpn_clikey="Private Client Key";

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Password Login";
service.ssh_key="Authorized Keys";

//radiooff.webservices
service.radiooff_legend="SES Button";
service.radiooff_srv="Use SES for turning off radio";

//hwmon.webservices
service.hwmon_legend="Hardware Monitoring";
service.hwmon_critemp="Critical Temperature (FAN Switch On)";
service.hwmon_hystemp="Hysteresis Temperature (FAN Switch Off)";

//rstat.webservices
service.rstats_legend="Bandwidth Monitoring";
service.rstats_srv="rstats Daemon";
service.rstats_path="Save Bandwidth Data to";
service.rstats_time="Saving Interval";
service.rstats_usrdir="User Directory";


// ******************************************* eop-tunnel.asp *******************************************//

var eoip = new Object();
eoip.titl=" - EoIP Tunnel";
eoip.tunnel="Tunnel";
eoip.legend="Ethernet Over IP Tunneling";
eoip.srv="EoIP Tunnel";
eoip.remoteIP="Remote IP Address";
eoip.tunnelID="Tunnel ID";
eoip.comp="Compression";
eoip.passtos="TOS passthrough";
eoip.frag="fragment";
eoip.mssfix="mssfix";
eoip.shaper="shaper";
eoip.bridging="Bridging";



// ******************************************* Sipath.asp + cgi *******************************************//

var sipath = new Object();
sipath.titl=" - SiPath Overview";
sipath.phone_titl=" - Phonebook";
sipath.status_titl=" - Status";



// ******************************************* Status_Lan.asp *******************************************//

var status_lan = new Object();
status_lan.titl=" - LAN Status";
status_lan.h2="Local Network";
status_lan.legend="LAN Status";
status_lan.h22="Dynamic Host Configuration Protocol";
status_lan.legend2="DHCP Status";
status_lan.legend3="DHCP Clients";
status_lan.legend4="Active Clients";

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="This is the Router's MAC Address, as seen on your local, Ethernet network.";
hstatus_lan.right4="This shows the Router's IP Address, as it appears on your local, Ethernet network.";
hstatus_lan.right6="When the Router is using a Subnet Mask, it is shown here.";
hstatus_lan.right8="If you are using the Router as a DHCP server, that will be displayed here.";
hstatus_lan.right10="By clicking on any MAC address, you will obtain the Organizationally Unique Identifier of the network interface (IEEE Standards OUI database search).";


// ******************************************* Status_Bandwidth.asp *******************************************//

var status_band = new Object();
status_band.titl=" - Bandwidth Monitoring";
status_band.h2="Bandwidth Monitoring";
status_band.legend="System";
status_band.sys_model="Router Model";
status_band.sys_firmver="Firmware Version";
status_band.sys_time="Current Time";
status_band.sys_up="Uptime";




// ******************************************* Status_Router.asp *******************************************//

var status_router = new Object();
status_router.titl=" - Router Status";
status_router.h2="Router Information";
status_router.legend="System";
status_router.sys_model="Router Model";
status_router.sys_firmver="Firmware Version";
status_router.sys_time="Current Time";
status_router.sys_up="Uptime";
status_router.sys_load="Load Average";
status_router.legend2="CPU";
status_router.cpu="CPU Model";
status_router.clock="CPU Clock";
status_router.legend3="Memory";
status_router.mem_tot="Total Available";
status_router.mem_free="Free";
status_router.mem_used="Used";
status_router.mem_buf="Buffers";
status_router.mem_cached="Cached";
status_router.mem_active="Active";
status_router.mem_inactive="Inactive";
status_router.legend4="Network";
status_router.net_maxports="IP Filter Maximum Ports";
status_router.net_conntrack="Active IP Connections";
status_router.h22="Internet";
status_router.legend5="Configuration Type";
status_router.www_loginstatus="Login Status";
status_router.leasetime="Remaining Lease Time";
status_router.notavail="Not available";
status_router.legend6="Space Usage";

//help container
var hstatus_router = new Object();
hstatus_router.right2="This is the specific name for the router, which you set on the <i>Setup</i> tab.";
hstatus_router.right4="This is the router's MAC Address, as seen by your ISP.";
hstatus_router.right6="This is the router's current firmware.";
hstatus_router.right8="This is time received from the ntp server set on the <em>" + bmenu.admin + " | " + bmenu.adminManagement + "</em> tab.";
hstatus_router.right10="This is a measure of the time the router has been \"up\" and running.";
hstatus_router.right12="This is given as three numbers that represent the system load during the last one, five, and fifteen minute periods.";
hstatus_router.right14="This shows the information required by your ISP for connection to the Internet. This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.";


// ******************************************* Status_Conntrack.asp *******************************************//

var status_conn = new Object();
status_conn.titl=" - Active IP Connections Table";
status_conn.h2="Active IP Connections";



// ******************************************* Status_SputnikAPD.asp *******************************************//

var status_sputnik = new Object();
status_sputnik.titl=" - Sputnik Agent Status";
status_sputnik.h2="Sputnik&reg; Agent&trade;";
status_sputnik.manage="Managed By";
status_sputnik.license="SCC License No.";

//help container
var hstatus_sputnik = new Object();
hstatus_sputnik.right1="Sputnik Agent Status";
hstatus_sputnik.right2="This screen displays the status of the Sputnik Agent process.";
hstatus_sputnik.right4="The Sputnik Control Center that this access point is connected to.";
hstatus_sputnik.right6="The current Agent status.";
hstatus_sputnik.right8="The license number of your Sputnik Control Center.";



// ******************************************* Status_Wireless.asp *******************************************//

var status_wireless = new Object();
status_wireless.titl=" - Wireless Status";
status_wireless.h2="Wireless";
status_wireless.legend="Wireless Status";
status_wireless.net="Network";
status_wireless.pptp="PPTP Status";
status_wireless.legend2="Wireless Packet Info";
status_wireless.rx="Received (RX)";
status_wireless.tx="Transmitted (TX)";
status_wireless.h22="Wireless Nodes";
status_wireless.legend3="Clients";
status_wireless.signal_qual="Signal Quality";
status_wireless.wds="WDS Nodes";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="This is the Router's MAC Address, as seen on your local, wireless network.";
hstatus_wireless.right4="As selected from the Wireless tab, this will display the wireless mode (Mixed, G-Only, B-Only or Disabled) used by the network.";



// ******************************************* Status_OpenVPN.asp *******************************************//

var status_openvpn = new Object();
status_openvpn.titl=" - OpenVPN Status";



// ******************************************* Triggering.asp *******************************************//

var trforward = new Object();
trforward.titl=" - Port Triggering";
trforward.h2="Port Triggering";
trforward.legend="Forwards";
trforward.trrange="Triggered Port Range";
trforward.fwdrange="Forwarded Port Range";
trforward.app="Application";

//help container
var htrforward = new Object();
htrforward.right2="Enter the application name of the trigger.";
htrforward.right4="For each application, list the triggered port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right6="For each application, list the forwarded port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right8="Enter the starting port number of the Triggered and Forwarded Range.";
htrforward.right10="Enter the ending port number of the Triggered and Forwarded Range.";



// ******************************************* Upgrade.asp *******************************************//

var upgrad = new Object();
upgrad.titl=" - Firmware Upgrade";
upgrad.h2="Firmware Management";
upgrad.legend="Firmware Upgrade";
upgrad.info1="After flashing, reset to";
upgrad.resetOff="Don't reset";
upgrad.resetOn="Reset to Default settings";
upgrad.file="Please select a file to upgrade";
upgrad.warning="W A R N I N G";
upgrad.mess1="Upgrading firmware may take a few minutes.<br />Do not turn off the power or press the reset button!";

//help container
var hupgrad = new Object();
hupgrad.right2="Click on the <em>Browse...</em> button to select the firmware file to be uploaded to the router.<br /><br /> Click the <em>Upgrade</em> button to begin the upgrade process. Upgrade must not be interrupted.";



// ******************************************* UPnP.asp *******************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Forwards";
upnp.legend2="UPnP Configuration";
upnp.serv="UPnP Service";
upnp.clear="Clear port forwards at startup";
upnp.url="Send presentation URL";
upnp.msg1="Click to delete entry";
upnp.msg2="Delete all entries?";


//help container
var hupnp = new Object();
hupnp.right2="Click the trash can to delete an individual entry.";
hupnp.right4="Allows applications to automatically setup port forwardings.";



// ******************************************* VPN.asp *******************************************//

var vpn = new Object();
vpn.titl=" - VPN";
vpn.h2="Virtual Private Network (VPN)";
vpn.legend="VPN Passthrough";
vpn.ipsec="IPSec Passthrough";
vpn.pptp="PPTP Passthrough";
vpn.l2tp="L2TP Passthrough";

//help container
var hvpn = new Object();
hvpn.right1="You may choose to enable IPSec, PPTP and/or L2TP passthrough to allow your network devices to communicate via VPN.";


// ******************************************* Vlan.asp *******************************************//

var vlan = new Object();
vlan.titl=" - Virtual LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Assigned To<br />Bridge";
vlan.tagged="Tagged";
vlan.negociate="Auto-Negotiate";
vlan.aggregation="Link Aggregation<br>on Ports 3 & 4";
vlan.trunk="Trunk";


// ******************************************* WEP.asp *******************************************//

var wep = new Object();
wep.defkey="Default Transmit Key";
wep.passphrase="Passphrase";



// ******************************************* WOL.asp *******************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Available Hosts";
wol.legend2="WOL Addresses";
wol.legend3="Output";
wol.legend4="Manual WOL";
wol.enable="Enable WOL?";
wol.mac="MAC Address(es)";
wol.broadcast="Net Broadcast";
wol.udp="UDP Port";
wol.msg1="Click to remove WOL host";

wol.h22="Automatic Wake-On-LAN";
wold.legend="Wake-On-LAN daemon";
wold.srv="WOL daemon";
wold.pass="SecureOn Password";

//help container
var hwol = new Object();
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router). You can manually wake up hosts by clicking the <em>"+sbutton.wol+"</em> or you can program an automatic schedule wake up thanks to the "+wold.wol_srv+".";
hwol.right4="MAC Address(es) are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB) and must be separated by a <em>SPACE</em>";
hwol.right6="IP Address is typically the broadcast address for the local network, but could be a remote address if the target host is not connected to the router's local network."



// ******************************************* WanMAC.asp *******************************************//

var wanmac = new Object();
wanmac.titl=" - MAC Address Clone";
wanmac.h2="MAC Address Clone";
wanmac.legend="MAC Clone";
wanmac.wan="Clone WAN MAC";
wanmac.wlan="Clone Wireless MAC";

//help container
var hwanmac = new Object();
hwanmac.right2="Some ISPs will require you to register your MAC address. If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

var wpa = new Object();
wpa.titl=" - Wireless Security";
wpa.h2="Wireless Security";
wpa.secmode="Security Mode";
wpa.legend="Wireless Encryption";
wpa.auth_mode="Network Authentication";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval (in seconds)";
wpa.radius_ipaddr="RADIUS Server Address";
wpa.radius_port="RADIUS Server Port";
wpa.radius_key="RADIUS Key";
wpa.algorithms="WPA Algorithms";
wpa.shared_key="WPA Shared Key";

//help container
var hwpa = new Object();
hwpa.right2="You may choose from Disable, WEP, WPA Pre-Shared Key, WPA RADIUS, or RADIUS. All devices on your network must use the same security mode.";



// ******************************************* WL_FilterTable.asp *******************************************//

var wl_filter = new Object();
wl_filter.titl=" - MAC Address Filter List";
wl_filter.h2="MAC Address Filter List";
wl_filter.h3="Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";



// ******************************************* WL_ActiveTable.asp *******************************************//

var wl_active = new Object();
wl_active.titl=" - Wireless Active Client MAC List";
wl_active.h2="Wireless Client MAC List";
wl_active.h3="Enable MAC Filter";
wl_active.active="Active PC";
wl_active.inactive="Inactive PC";



// ******************************************* Wireless_WDS.asp *******************************************//

var wds = new Object();
wds.titl=" - WDS";
wds.h2="Wireless Distribution System";
wds.legend="WDS Settings";
wds.label="Lazy WDS";
wds.label2="WDS Subnet";
wds.wl_mac="Wireless MAC";
wds.lazy_default="Default: Disable";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="Subnet";
wds.legend2="Extra Options";



// ******************************************* Wireless_radauth.asp *******************************************//

var radius = new Object();
radius.titl=" - Radius";
radius.h2="Remote Authentication Dial-In User Service";
radius.legend="Radius";
radius.label="MAC Radius Client";
radius.label2="MAC Format";
radius.label3="RADIUS Server Address";
radius.label4="RADIUS Server Port";
radius.label5="Maximum Unauthenticated Users";
radius.label6="Password Format";
radius.label7="RADIUS Shared Secret";
radius.label8="Override Radius if server is unavailable";



// ******************************************* Wireless_MAC.asp *******************************************//

var wl_mac = new Object();
wl_mac.titl=" - MAC Filter";
wl_mac.h2="Wireless MAC Filter";
wl_mac.legend="MAC Filter";
wl_mac.label="Use Filter";
wl_mac.label2="Filter Mode";
wl_mac.deny="Prevent PCs listed from accessing the wireless network";
wl_mac.allow="Permit only PCs listed to access the wireless network";



// ******************************************* Wireless_Basic.asp *******************************************//

var wl_basic = new Object();
wl_basic.titl=" - Wireless";
wl_basic.h2="Wireless";
wl_basic.legend="Basic Settings";
wl_basic.label="Wireless Mode";
wl_basic.label2="Wireless Network Mode";
wl_basic.label3="Wireless Network Name (SSID)";
wl_basic.label4="Wireless Channel";
wl_basic.label5="Wireless SSID Broadcast";
wl_basic.label6="Sensitivity Range (ACK Timing)";
wl_basic.ap="AP";
wl_basic.client="Client";
wl_basic.repeater="Repeater";
wl_basic.clientBridge="Client Bridge";
wl_basic.adhoc="Adhoc";
wl_basic.wdssta="WDS Station";
wl_basic.wdsap="WDS AP";
wl_basic.mixed="Mixed";
wl_basic.b="B-Only";
wl_basic.a="A-Only";
wl_basic.g="G-Only";
wl_basic.bg="BG-Mixed";
wl_basic.n="N-Only";
wl_basic.legend2="Radio Time Restrictions";
wl_basic.radio="Radio";
wl_basic.radiotimer="Radio Scheduling";
wl_basic.radio_on="Radio is On";
wl_basic.radio_off="Radio is Off";

wl_basic.h2_v24="Wireless Physical Interface";
wl_basic.h2_vi="Virtual Interfaces";
wl_basic.regdom="Regulatory Domain";
wl_basic.TXpower="TX Power";
wl_basic.diversity="Diversity";
wl_basic.primary="Primary";
wl_basic.secondary="Secondary";
wl_basic.network="Network Configuration";
wl_basic.unbridged="Unbridged";
wl_basic.bridged="bridged";
wl_basic.turbo="Turbo Mode";
wl_basic.extrange="Extended Range";
wl_basic.extchannel="Extended Channel Mode";
wl_basic.outband="Outdoor Band";
wl_basic.channel_width="Channel Width";
wl_basic.channel_wide="Wide Channel";



//help container
var hwl_basic = new Object();
hwl_basic.right2="If you wish to exclude Wireless-G clients, choose <em>B-Only</em> mode. If you would like to disable wireless access, choose <em>Disable</em>.";
hwl_basic.right3="Sensitivity Range: ";
hwl_basic.right4="Adjusts the ack timing. 0 disables ack timing completely.";



// ******************************************* Wireless_Advanced.asp *******************************************//

var wl_adv = new Object();
wl_adv.titl=" - Advanced Wireless Settings";
wl_adv.h2="Advanced Wireless Settings";
wl_adv.legend="Advanced Settings";
wl_adv.legend2="Wireless Multimedia Support Settings";
wl_adv.label="Authentication Type";
wl_adv.label2="Basic Rate";
wl_adv.label3="Transmission Rate";
wl_adv.label4="CTS Protection Mode";
wl_adv.label5="Frame Burst";
wl_adv.label6="Beacon Interval";
wl_adv.label7="DTIM Interval";
wl_adv.label8="Fragmentation Threshold";
wl_adv.label9="RTS Threshold";
wl_adv.label10="Max Associated Clients";
wl_adv.label11="AP Isolation";
wl_adv.label12="TX Antenna";
wl_adv.label13="RX Antenna";
wl_adv.label14="Preamble";
wl_adv.reference="Noise Reference";
wl_adv.label15="Xmit Power";
wl_adv.label16="Afterburner";
wl_adv.label17="Wireless GUI Access";
wl_adv.label18="WMM Support";
wl_adv.label19="No-Acknowledgement";
wl_adv.table1="EDCA AP Parameters (AP to Client)";
wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="Admin Forced";
wl_adv.row1="Background";
wl_adv.row2="Best Effort";
wl_adv.row3="Video";
wl_adv.row4="Voice";
wl_adv.table2="EDCA STA Parameters (Client to AP)";
wl_adv.lng="Long"; 					//************* don't use .long ! *************
wl_adv.shrt="Short"; 				//************* don't use .short ! **************

//help container
var hwl_adv = new Object();
hwl_adv.right2="You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.";
hwl_basic.right6="Click any hour to enable or disable the radio signal (<em>green</em> indicates allowed Wireless access, and <em>red</em> indicates blocked Wireless access)";


// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

var fail = new Object();
fail.mess1="The values you entered are invalid. Please try again.";
fail.mess2="Upgrade failed.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="Settings saved.";
success.restore="Settings restored.<br/>Unit is rebooting now. Please wait a moment...";
success.upgrade="Upgrade successful.<br/>Unit is rebooting now. Please wait a moment...";
success.success_noreboot="Settings are successful.";
success.success_reboot=success.success_noreboot + "<br />Unit is rebooting now. Please wait a moment...";

success.alert_reset="All configuration settings have been restored to their default values.<br /><br />";
success.alert1="Please check the followings before connecting again:";
success.alert2="If you have changed your router&#39;s IP address, please note that you must release/renew your client(s) address(s) on the network.";
success.alert3="If you are connected via WLAN, please join the network and then click <em>Continue</em>.";

// ******************************************* Logout.asp  *******************************************//
var logout = new Object();
logout.message="You have successfully logged out.<br />Thank you for using DD-WRT !";


// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

var dhcp = new Object();
dhcp.titl=" - DHCP Active IP Table";
dhcp.h2="DHCP Active IP Table";
dhcp.server="DHCP Server IP Address :";
dhcp.tclient="Client Host Name";

var donate = new Object();
donate.mb="You may also donate through the Moneybookers account mb@dd-wrt.com";
