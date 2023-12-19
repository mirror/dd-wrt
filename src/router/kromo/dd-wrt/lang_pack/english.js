////////////////////////////////////////////////////////////////////////////////////
//     English reference translation file - DD-WRT V3 -  Updated on 29/10/2022    //
////////////////////////////////////////////////////////////////////////////////////

// ** COMMON SHARE LABEL **//
var lang_charset=new Object();
lang_charset.set="utf-8";

var share=new Object();
share.packets="Packets";
share.annex="Annex Type";
share.apn="APN";
share.pin="PIN";
share.dial="Dial String";
share.mode_3g="Connection Type";
share.mode_3g_auto="Auto LTE-4G / 3G / 2G";
share.mode_3g_4g="Force LTE-4G";
share.mode_3g_3g="Force 3G";
share.mode_3g_2g="Force 2G";
share.mode_3g_prefer_3g="Prefer 3G";
share.mode_3g_prefer_2g="Prefer 2G";
share.mode_3g_3g2g="3G First, on Error 2G";
share.firmware="Firmware";
share.time="Time";
share.interipaddr="WAN IP Address";
// choice dd-wrt.c line 1442
share.choice="Please Choose";
share.more="more...";
share.help="Help";
share.enable="Enable";
share.enabled="Enabled";
share.disable="Disable";
share.disabled="Disabled";
//not set used in src/router/openvpn/config2/openvpn.webvpn
share.notset="Not Set";
share.usrname="Username";
share.token="Token";
share.passwd="Password";
share.hostname="Hostname";
share.advanced="Advanced Settings";
share.vdsl="Advanced VLAN Tagging";
share.vdslvlan7="T-Online VLAN 7 Support";
share.vdslvlan8="T-Online VLAN 8 Support";
share.vdslbng="T-Online BNG support";
share.wan_vlantag="VLAN Tag ID";
share.wan_dualaccess="Dual-Access Mode";
share.compression="PPP Compression (MPPC)";
share.mlppp="Single Line Multi Link";
share.vpi_vci="VPI / VCI";
share.encaps="Encapsulation";
share.payload="Payload Type";
share.domainname="Domain Name";
share.wandomainname="WAN Domain Name";
share.landomainname="LAN Domain Name";
share.statu="Status";
share.start="Start";
share.end="End";
share.proto="Protocol";
share.ip="IP Address";
share.ipaddrmask="IP Addresses / Netmask (CIDR)";
share.ipv4="IPv4 Address";
share.ipv6="IPv6 Address";
share.localip="Local IP";
share.remoteip="Remote IP";
share.mac="MAC Address";
share.none="None";
share.none2="no";
share.both="Both";
share.add="Add";
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
share.jan="January";
share.feb="February";
share.mar="March";
share.apr="April";
share.may="May";
share.jun="June";
share.jul="July";
share.aug="August";
share.sep="September";
share.oct="October";
share.nov="November";
share.dec="December";
share.expires="Expires";
share.yes="Yes";
share.no="No";
share.filter="Filter";
share.deny="Deny";
share.range="Range";
share.use="Use";
share.mins="min";
share.secs="s";
share.seconds="seconds";
share.ms="ms";
share.routername="Router Name";
share.manual="Manual";
share.port="Port";
// dd-wrt.c line 2873
share.guest_port="Guest Port";
share.ssid="SSID";
share.channel="Channel";
share.stations="Stations";
share.frequency="Frequency";
share.rssi="RSSI";
share.signal="Signal";
share.noise="Noise";
share.quality="Quality";
share.beacon="Beacon";
share.openn="Open";
share.dtim="DTIM";
share.rates="Rate";
share.rate="Rate";
share.txrate="TX Rate";
share.rxrate="RX Rate";
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
share.deflt="Default";  //don't use share.default!!!
share.reboot="Reboot after change";
share.all="All";
share.auto="Auto";
share.desired="Desired";
share.required="Required";
share.requiremts="Requirements";
share.right="Right";
share.left="Left";
share.share_key="Shared Key";
share.inter="Interval";
share.srv="Service Name";
share.host_uniq="Host Uniq";
share.port_range="Port Range";
share.priority="Priority";
share.gateway="Gateway";
share.intrface="Interface";  //don't use share.interface, Mozilla problem!!!
share.iftbl="IF";
share.input="In";
share.output="Out";
share.total="Total";
share.radioname="Name";
share.ccq="CCQ";
share.pintrface="Physical Interface";
share.vintrface="Virtual Interfaces";
share.router="Router";
share.static_lease="Static Leases";
share.srvip="Server IP";
share.srvipname="Server IP / Name";
share.localdns="Local DNS";
share.minutes="min";
share.oui="OUI Search";
share.sttic="Static";
share.off="Off";
share.on="On";
share.forwarding="Forwarding";
share.stp="STP";
share.mstp="MSTP";
share.rstp="RSTP";
share.dynamic="Dynamic";
share.connecting="Connecting";
share.connect="Connect";
share.connected="Connected";
share.disconnect="Disconnect";
share.disconnected="Disconnected";
share.info="Information";
share.infotbl="Info";
share.state="State";
share.mode="Radio Mode";
share.encrypt="Encryption";
share.key="Key";
share.wireless="Wireless";
share.dhcp="DHCP";
share.styl="Select a Style";
share.theme="Select a Theme";
share.styl_dark="Enable Dark Styles";
share.styl_opt="Enable Sticky Footer";
share.err="error";
share.errs="errors";
share.meters="meters";
share.vht80plus80="VHT80+80 (160 MHz)";
share.vht160="VHT160 (160 MHz)";
share.vht80="VHT80 (80 MHz)";
share.ht40="Wide HT40 (40 MHz)";
share.ht20="Full HT20 (20 MHz)";
share.dynamicturbo="Dynamic (20/40 MHz)";
share.turbo="Turbo (40 MHz)";
share.full="Full (20 MHz)";
share.half="Half (10 MHz)";
share.quarter="Quarter (5 MHz)";
share.subquarter="Eighth (2.5 MHz)";
share.seealso="See also";
share.never="never";
share.unknown="Unknown";
share.empty="Empty";
share.expired="expired";
share.logout="Logout";
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
share.from2=share.from;
share.to2=share.to;
share.days_genetive=share.days;
share.standard="Standard";
share.execscript="Execute script";
share.user="User";
share.privatekey="Private Key";
share.bytes="B";
share.kbytes="KB";
share.mbytes="MB";
share.gbytes="GB";
share.preempt="Preemption";
share.acktiming="ACK Timing";
share.broadcast="Broadcast Support";
share.secondcharacter="s";
share.change="User Password Change";
share.copy="Copy";
share.paste="Paste";
share.bad_request="BAD REQUEST";
share.unauthorized="UNAUTHORIZED";
share.tcp_error="TCP ERROR";
share.request_timeout="REQUEST TIMEOUT";
share.not_implemented="NOT IMPLEMENTED";
share.not_found="NOT FOUND";
share.file_not_found=" The file %s was not found.";
share.auth_required=" Authorization required. Wrong username and/or password!";
share.unexpected_connection_close=" Unexpected connection close in initial request.";
share.unexpected_connection_close_2=" Unexpected connection close.";
share.request_timeout_desc=" No request appeared within a reasonable time period.";
share.cant_parse_no_path=" Cannot parse request. (no path given)";
share.cant_parse_no_proto=" Cannot parse request. (no protocol given)";
share.method_unimpl=" Method %s is not implemented.";
share.no_slash=" Bad filename. (no leading slash)";
share.threaten_fs=" Illegal filename. (filename will threaten local filesystem)";
share.cross_site=" Cross Site Action detected!";
share.cross_site_ref=" Cross Site Action detected! (referer %s)";
share.no_wifi_access=" Cannot use the wireless interface to access Web UI.";
share.sysloglegend="System Log";
share.syslogdisabled="No messages available, syslogd is not enabled!";
share.actiontbl="Action";
share.show="Show";
share.hide="Hide";
share.down="down";
share.excluded="Excluded";
share.not_prefered="Not prefered";
share.prefered="Prefered";

var sbutton=new Object();
sbutton.save="Save";
sbutton.download_config="Export Peer Config";
sbutton.savetitle="Only save settings without commit";
sbutton.apply="Apply Settings";
sbutton.applied="Applying Settings";
sbutton.applytitle="Apply settings immediately";
sbutton.saving="Saved";
sbutton.cmd="Executing";
sbutton.cancel="Cancel Changes";
sbutton.canceltitle="Cancel changes in this form only";
sbutton.refres="Refresh";
sbutton.clos="Close";
sbutton.scrub="Scrub";
sbutton.del="Delete";
sbutton.continu="Continue";
sbutton.add="Add";
sbutton.remove="Remove";
sbutton.modify="Modify";
sbutton.deleted="Deleted";
sbutton.delall="Delete All";
sbutton.autorefresh="Auto Refresh is On";
sbutton.backup="Backup";
sbutton.restore="Restore";
sbutton.restored="Restoring";
sbutton.cptotext="Edit";
sbutton.runcmd="Run Commands";
sbutton.startup="Save Startup";
sbutton.shutdown="Save Shutdown";
sbutton.firewall="Save Firewall";
sbutton.custom="Save Custom";
sbutton.usb="Save USB";
sbutton.wol="Wake Up";
sbutton.add_wol="Add Host";
sbutton.manual_wol="Manual Wake Up";
sbutton.summary="Summary";
sbutton.filterIP="Edit Filter List";
sbutton.filterMac="Edit MAC Filter List";
sbutton.filterSer="Add/Edit Service";
sbutton.reboot="Reboot Router";
//sbutton.help="   Help  ";
sbutton.wl_client_mac="Wireless Client MAC List";
sbutton.update_filter="Update Filter List";
sbutton.join="Join";
sbutton.log_in="Incoming Log";
sbutton.log_out="Outgoing Log";
sbutton.edit_srv="Add/Edit Service";
sbutton.routingtab="Show Routing Table";
sbutton.policytab="Show Policy Table";
sbutton.wanmac="Get Current PC MAC Address";
sbutton.dhcprel="DHCP Release";
sbutton.dhcpren="DHCP Renew";
sbutton.spectral_survey="Spectrum";
sbutton.survey="Site Survey";
sbutton.csurvey="Channel Survey";
sbutton.wsurvey="Wiviz Survey";
sbutton.upgrading="Upgrading";
sbutton.upgrade="Upgrade";
sbutton.preview="Preview";
sbutton.allways_on="Always On";
sbutton.allways_off="Always Off";
sbutton.download="Download";
sbutton.next="Next ›";
sbutton.prev="‹ Previous";

// ** COMMON ERROR MESSAGES  **//
var errmsg=new Object();
errmsg.err0="You must input a username.";
errmsg.err1="You must input a Router Name.";
errmsg.err2="Out of range, please adjust the start IP address or the number of maximum DHCP users.";
errmsg.err3="You must at least select a day.";
errmsg.err4="The end time must be bigger than start time.";
errmsg.err5="The MAC Address length is not correct.";
errmsg.err6="You must input a password.";
errmsg.err7="You must input a hostname.";
errmsg.err8="You must input an IP Address or Domain Name.";
errmsg.err9="Illegal DMZ IP Address.";
errmsg.err10="Confirmed password did not match the entered password. Please re-enter password.";
errmsg.err11="No spaces are allowed in the password value";
errmsg.err12="You must input a command to run.";
errmsg.err13="Upgrade failed.";
errmsg.err45="Not available in HTTPS! Please connect in HTTP mode.";
errmsg.err46="Not available in HTTPS";

//common.js error messages
errmsg.err14=" value is out of range [";
errmsg.err15="The WAN MAC Address is out of range [00 - ff].";
errmsg.err16="The second character of MAC must be even number: [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="The MAC Address is not correct.";
errmsg.err18="The MAC Address length is not correct.";
errmsg.err19="The MAC Address cannot be the broadcast address.";
errmsg.err20="Enter MAC Address in (xx:xx:xx:xx:xx:xx) format.";
errmsg.err21="Invalid MAC address format.";
errmsg.err22="The WAN MAC Address is not correct.";
errmsg.err23="Invalid hex value ";
errmsg.err24=" found in MAC address ";
errmsg.err25="The key value is not correct.";
errmsg.err26="The key length is not correct.";
errmsg.err27="Illegal subnet mask.";
errmsg.err28=" contained illegal characters, must be [ 0 - 9 ].";
errmsg.err29=" contained illegal ASCII code.";
errmsg.err30=" contained illegal hexadecimal digits.";
errmsg.err31=" value is illegal.";
errmsg.err32="IP address and gateway is not at same subnet mask.";
errmsg.err33="IP address and gateway cannot be same.";
errmsg.err34=" is not allowed to contain a space.";
errmsg.err110="End number must be greater than the start number";
errmsg.err111="Invalid IP address";
errmsg.err112="Invalid input characters \"<invchars>\" in field \"<fieldname>\"";
errmsg.err113="The mobility domain must be a 4 digit hex value";

//Wol.asp error messages
errmsg.err35="You must input a MAC address to run.";
errmsg.err36="You must input a network broadcast address to run.";
errmsg.err37="You must input a UDP port to run.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Please enter a Shared Key!";
errmsg.err39="Invalid Key, must be between 8 and 63 ASCII characters or 64 hexadecimal digits";
errmsg.err40="You have to enter a key for Key ";
errmsg.err41="Invalid Length in key ";
errmsg.err43="Rekey interval";

//config.asp error messages
errmsg.err42="Please select a configuration file to restore.";

//WL_ActiveTable.asp error messages
errmsg.err44="The total checks exceed 256 counts.";

//Site_Survey.asp error messages
errmsg.err47="invalid SSID.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS is not compatible with the current configuration of the router. Please check the following points :\n * Wireless Mode must be set to AP \n * WPA2 is not supported under WDS \n * Wireless Network B Only mode is not supported under WDS";

//Wireless_radauth.asp error messages
errmsg.err49="RADIUS is only available when radio is in AP mode.";

//Wireless_Basic.asp error messages
errmsg.err50="You must input a SSID.";

// Management.asp error messages
errmsg.err51="The router is currently set to the default username and password. As a security precaution, you must enter a new username and password before the Remote Management feature can be enabled. Click the OK button to change your username/password. Click the <em>Cancel</em> button to leave the Remote Management feature disabled.";
errmsg.err52="Password confirmation does not match.";

// Port_Services.asp error messages
// err53 browser dialog box no HTML support
errmsg.err53="After completing all actions, click the " + sbutton.apply + " button to save the settings.";
errmsg.err54="You must input a Service Name.";
errmsg.err55="The Service Name exists.";

// QoS.asp error messages
errmsg.err56="Port value is out of range [0 - 65535]";

// Routing.asp error messages
errmsg.err57="Delete the Entry?";
errmsg.err103=" must be lower than ";

// Status_Lan.asp error messages
errmsg.err58="Click to remove static lease entry";
errmsg.err581="Click to disconnect PPTP client";
errmsg.err582="Click to add entry to static leases";

//Status_Wireless.asp error messages
errmsg.err59="Not available! Please enable the Wireless Network.";

//Upgrade.asp error messages
errmsg.err60="Please select a firmware file to upgrade.";
errmsg.err61="Incorrect firmware file!";

//Services.asp error messages
errmsg.err62=" is already defined as a static lease.";

//Saving message
errmsg.err100="Processing...<br />Please wait.";
errmsg.err101="Restoring configuration file...<br />Please wait.";
errmsg.err102="Upgrading firmware...<br />Please wait.";
errmsg.err103="Invalid symbol detected";

// **  COMMON MENU ENTRIES  **//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basic Setup";
bmenu.setupipv6="IPv6";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Clone";
bmenu.setuprouting="Advanced Routing";
bmenu.setupvlan="Switch Config";
bmenu.setupeop="Tunnels";
bmenu.networking="Networking";

bmenu.wireless="Wireless";
bmenu.wirelessBasic="Basic Settings";
bmenu.wirelessRadius="RADIUS";
bmenu.wirelessSuperchannel="SuperChannel";
bmenu.wimax="WiMAX";
bmenu.wirelessSecurity="Wireless Security";
bmenu.wirelessAoss="AOSS";
bmenu.wirelessAossWPS="AOSS / WPS";
bmenu.wirelessWPS="WPS";
bmenu.wirelessMac="MAC Filter";
bmenu.wirelessAdvanced="Advanced Settings";
bmenu.wirelessAdvancedwl0="wl0-Advanced";
bmenu.wirelessAdvancedwl1="wl1-Advanced";
bmenu.wirelessAdvancedwl2="wl2-Advanced";
bmenu.wirelessWds="WDS";
bmenu.wirelessWds0="wlan0-WDS";
bmenu.wirelessWds1="wlan1-WDS";
bmenu.wirelessWds2="wlan2-WDS";
bmenu.wirelessWds3="wlan3-WDS";
bmenu.wirelessWdswl0="wl0-WDS";
bmenu.wirelessWdswl1="wl1-WDS";
bmenu.wirelessWdswl2="wl2-WDS";
bmenu.wirelessRoaming="Roaming";
bmenu.security="Security";
bmenu.firwall="Firewall";
bmenu.vpn="VPN Passthrough";

bmenu.accrestriction="Access Restrictions";
bmenu.webaccess="WAN Access";

bmenu.applications="NAT / QoS";
bmenu.applicationsprforwarding="Port Range Forwarding";
bmenu.applicationspforwarding="Port Forwarding";
bmenu.applicationsipforwarding="IP Forwarding (1:1 NAT)";
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
bmenu.adminAlive="Keep Alive";
bmenu.adminSysctl="Sysctl";
bmenu.adminLog="Log";
bmenu.adminDiag="Commands";
bmenu.adminWol="WOL";
bmenu.adminFactory="Factory Defaults";
bmenu.adminUpgrade="Firmware Upgrade";
bmenu.adminBackup="Backup";

bmenu.services="Services";
bmenu.servicesServices="Services";
bmenu.servicesRadius="FreeRADIUS";
bmenu.servicesPppoesrv="PPPoE Server";
bmenu.servicesPptp="VPN";
bmenu.servicesUSB="USB";
bmenu.servicesNAS="NAS";
bmenu.servicesHotspot="Hotspot";
bmenu.servicesNintendo="Nintendo";
bmenu.servicesPrivoxy="Ad Blocking";
bmenu.servicesSpeedchecker="SpeedChecker";

bmenu.statu="Status";
bmenu.statuRouter="Router";
bmenu.statuInet="WAN";
bmenu.statuLAN="LAN";
bmenu.statuWLAN="Wireless";
bmenu.statuVPN="OpenVPN";
bmenu.statuBand="Bandwidth";
bmenu.statuSysInfo="Sys Info";
bmenu.statuActivate="Activate";
bmenu.statuMyPage="My Page";
bmenu.statuGpio="GPIO I/O";
bmenu.statuSyslog="Syslog";
bmenu.setupnetw="Network";
bmenu.adminman="Management";

// sysctl.asp
var sysctl=new Object();
sysctl.titl="Sysctl Configuration";

// ** Alive.asp **//
var alive=new Object();
alive.titl="Keep Alive";
alive.h2="Keep Alive Management";
alive.legend="Schedule Reboot";
alive.sevr1="Enable Schedule";
alive.hour="At a Set Time";
alive.legend2="WDS / Connection Watchdog";
alive.sevr2="Enable Watchdog";
alive.IP="IP Addresses";
alive.legend3="Proxy / Connection Watchdog";
alive.sevr3="Enable Watchdog";
alive.IP2="Proxy IP Address";
alive.port="Proxy Port";
alive.mode0="Any Dropped IPs for Reboot";
alive.mode1="All Dropped IPs for Reboot";
alive.timeout="Ping Timeout";

//help container
var halive=new Object();
halive.right2="Choose a schedule when to reboot the router. Cron <b>must be enabled</b> in the management tab.";
halive.right4="<b>" + alive.IP + "</b>: Only a maximum of <b>three</b> IP addresses separated by a <em>SPACE</em> are allowed.";

//help page
halive.page1="<dd>You can schedule regular reboots for the router:<ul><li>Regularly after the set amount of seconds.</li><li>At a specific date time each week or everyday.</li></ul><br /><div class=\"note\"><h4>Note:</h4><div>For date based reboots Cron must be activated. See <a href=\"HManagement.asp\">Management</a> for Cron activation.</div></div></dd>";
halive.page2="<dd></dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.reboot +"</em> button to reboot the router immediately.</dd>";

// ** config.asp **//
var config=new Object();
config.titl="Backup & Restore";
config.h2="Backup Configuration";
config.legend="Backup Settings";
config.mess1="Click the <em>" + sbutton.backup + "</em> button to download your current configuration settings file to disk.";
config.h22="Restore Configuration";
config.legend2="Restore Settings";
config.mess2="Select a file to restore";
config.mess3="W A R N I N G";
config.mess4="Only upload a backup file generated with DD-WRT's firmware and from the same model of router.<br />Do not upload any backup configuration files that were not created by this interface!";

//help container
var hconfig=new Object();
hconfig.right2="You may backup your current configuration in case you need to reset the router back to factory default settings.<br /><br />Tip: Over terminal type <b>nvram show > /tmp/mybackup.txt</b> and grab that file to your desktop for a human readable backup of your current configuration, which can be used for reference purposes only.";
hconfig.right4="Click the <em>Browse...</em> button to search for a backup configuration file that you have previously saved to disk.<br /><br />Click the <em>" + sbutton.restore + "</em> button to overwrite <b>all current configuration settings</b> with the values in the backup file.";

// help page
hconfig.page1="<dd>You may backup your current configuration in case you need to reset the router back to factory default settings.</dd><dt>";
hconfig.page2="</dt><dd>Click the <em>" + sbutton.backup + "</em> button to backup your current configuration to a file saved to disk.<br /> Tip: Over terminal type <b>nvram show > /tmp/mybackup.txt</b> and grab that file to desktop for a human readable backup of your current configuration.</dd>";
hconfig.page3="<dd>Click the <i>Browse...</i> button to search for a backup configuration file that you have previously saved to disk.<br>Click the <em>" + sbutton.restore + "</em> button to overwrite <b>all current configuration settings</b> with the values in the backup file.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Only upload a backup file generated with DD-WRT's firmware and from the same model of router.</dd></div></dd>";

// ** DDNS.asp **//
var ddns=new Object();
ddns.titl="Dynamic DNS";
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS Configuration";
ddns.srv="Enable Service";
ddns.emailaddr="Email Address";
ddns.typ="Type";
ddns.custom="Custom";
ddns.wildcard="Wildcard";
ddns.statu="Service Status";
ddns.system="Dynamic DNS Server";
ddns.options="Additional Options";
ddns.forceupd="Force Update Interval";
ddns.wanip="Use External IP Check";
ddns.hlp="DDNS Service";
ddns.ssl="Use SSL";
ddns.ipv6_only="Update only IPv6 Address";

var ddnsm=new Object();
ddnsm.all_closed="The DDNS server is currently closed";
ddnsm.all_resolving="Resolving domain name";
ddnsm.all_errresolv="Domain name resolve failed";
ddnsm.all_connecting="Connecting to server";
ddnsm.all_success="Success";
ddnsm.all_connectfail="Connect to server failed";
ddnsm.all_disabled="DDNS function is disabled";
ddnsm.all_noip="No WAN connection";

//help container
var hddns=new Object();
hddns.right2="DDNS allows you to access your network using domain names instead of IP addresses. The service manager changes IP addresses and updates your domain information dynamically. You must sign up for services through e.g. DynDNS.org, freedns.afraid.org or other similar dynamic DNS service.";
hddns.right4="Type an integer number in the box to set the force update interval (in days). Updates should only be performed automatically when your IP address has changed. Be aware of your DNS provider's update abuse policy to avoid having your hostname or domain blocked.";

//help page
hddns.page1="<dd>The router offers a Dynamic Domain Name System (DDNS) feature. DDNS lets you assign a fixed host and domain name to a dynamic external IP address. It is useful when you are hosting your own website, or any other server behind the router which is accessible via the Internet. To use this feature, you need to sign up for DDNS service at e.g. <a href=\"http:\/\/www.dyndns.org\" target=\"_new\">www.dyndns.org</a>, one of the several DDNS service provider supported by DD-WRT.</dd>";
hddns.page2="<dd>To disable DDNS service, keep the default setting, <em>" + share.disable + "</em>. To enable DDNS service, follow these instructions:<ol class=\"wide\"><li>Sign up for DDNS service at e.g. <a href=\"http:\/\/www.dyndns.org\" target=\"_new\">www.dyndns.org</a>, and write down your User Name, Password, and Host Name information.</li><li>On the DDNS screen, select the <i>DynDNS.org</i> option from the drop down menu.</li><li>Complete the <em>" + share.usrname + "</em>, <em>" + share.passwd + "</em>, and <em>" + share.hostname + "</em> fields.</li><li>Click the <em>" + sbutton.save + "</em> button to save your changes. Click the <em>" + sbutton.cancel + "</em> button to cancel unsaved changes.</li></ol><br />You can now access your router from the Internet with the domain you have chosen.</dd>";
hddns.page3="<dd>The <em>Static</em> DNS service is similar to the <em>Dynamic</em> DNS service, in that it allows a hostname such as yourname.dyndns.org to point to your IP address. Unlike a <em>Dynamic</em> DNS host, a <em>Static</em> DNS host does not expire after 35 days without updates, but updates take longer to propagate though the DNS system.<br />DynDNS' <em>Custom</em> DNS service provides a managed primary DNS solution, giving you complete control over an entire domain name and providing a unified primary/secondary DNS service. A web-based interface provides two levels of control over your domain, catering to average or power users.</dd>";
hddns.page4="<dd>Enabling the wildcard feature for your host causes *.yourhost.dyndns.org to be aliased to the same IP address as yourhost.dyndns.org. This feature is useful if you want to be able to use, for example, www.yourhost.dyndns.org and still reach your hostname.</dd>";
hddns.page5="<dd>Type an integer number in the box to set the force update interval (in days). Force update is an update which will be done although your IP address is still the same. Force update is required, especially for non donator of dyndns.org users, in order to avoid the host name of being deleted.</dd>";
hddns.page6="<dd>The status of the DDNS service connection is displayed here.</dd>";

// ** Diagnostics.asp **//
var diag=new Object();
diag.titl="Diagnostics and Commands";
diag.h2="Diagnostics and Commands";
diag.legend="Command Shell";
diag.cmd="Commands";
diag.output="Output";
diag.startup="Startup";
diag.shutdown="Shutdown";
diag.firewall="Firewall";
diag.usb="USB Script";
diag.custom="Custom Script";
diag.running="Running";
diag.stopped="Stopped";

//help container
var hdiag=new Object();
hdiag.right2="You can run commands via the Web interface. Fill the input area with the desired command and click the <em>" + sbutton.runcmd + "</em> button to execute.<br/><b>Recommended</b>: a terminal connection via SSH/Telnet is a more suitable, flexible, faster and reliable for some commands.";

//help page
hdiag.page1="<dd><b>Recommended</b>: A terminal connection via SSH/Telnet is a more suitable, flexible, faster and reliable for some commands.</dd>";
hdiag.page2="<dd>Fill the input area with the desired command and click the <em>" + sbutton.runcmd + "</em> button to execute command.</dd>";
hdiag.page3="<dd>You can specify certain commands to be executed during router startup. Fill the input area with the desired commands (only one command per row) and click the <em>" + sbutton.startup + "</em> button.</dd>";
hdiag.page4="<dd>Each time the firewall is started custom firewall rules can added to the chain. Fill the input area with additional iptables/ip6tables commands (only one command per row) and click the <em>" + sbutton.firewall + "</em> button.</dd>";
hdiag.page5="<dd>A custom script is stored in <b>/tmp/.rc_custom</b>. You can run it manually or use Cron to execute the contents. Fill the input area with the desired commands (only one command per row) and click the <em>" + sbutton.custom + "</em> button.<br /><br /><div class=\"note\"><h4>Note:</h4><div><ul><li>Startup commands are stored in NVRAM rc_startup variable</li><li>Firewall commands are stored in NVRAM rc_firewall variable</li><li>Custom script is stored in NVRAM rc_custom variable</li></ul></div></div></dd>";

// ** DMZ.asp **//
var dmz=new Object();
dmz.titl="DMZ";
dmz.h2="Demilitarized Zone (DMZ)";
dmz.legend="Perimeter Network";
dmz.serv="Enable DMZ";
dmz.host="DMZ Host IP Address";

//help container
var hdmz=new Object();
hdmz.right2="Enabling this option will expose the specified host to the Internet. All ports will be accessible from the Internet.";

//help page
hdmz.page1="<dd>The DMZ (Demilitarized Zone) hosting feature allows one local user to be exposed to the Internet for use of a special-purpose service such as Internet gaming or videoconferencing. DMZ hosting forwards all the ports at the same time to one device. The Port Forwarding feature is more secure because it only opens the ports you want to have opened, while DMZ hosting opens all the ports of one computer, exposing the device so the Internet can see it.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Any device whose port is being forwarded must have a new static IP address assigned to it because the IP address may change when using the DHCP function.</div></div></dd>";
hdmz.page2="<dd>To expose one device to the Internet, select <em>" + share.enable + "</em> and enter the computer's IP address in the <em>" + dmz.host + "</em> field.<br /><br />To disable the DMZ, keep the default setting, <em>" + share.disable + "</em>.</dd><dd>Click the <em>" + sbutton.save + "</em> button to save your settings or click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** Factory_Defaults.asp **//
var factdef=new Object();
factdef.titl="Factory Defaults";
factdef.h2="Configuration Management";
factdef.legend="NVRAM Settings";
factdef.restore="Restore to Factory Defaults";
factdef.mess1="Warning! By clicking the OK button, the device will be reset to the factory default settings, erasing the current configuration.";

//help container
var hfactdef=new Object();
hfactdef.right1="This will reset all current NVRAM settings back to the DD-WRT's default values. <b>All of your current settings will be erased</b>.";

// help page
hfactdef.page1="<dd>If you are having problems with your router (which might be the result of changing certain settings) you can restore the factory defaults.</dd>";
hfactdef.page2="<dd>Click the <em>" + share.yes + "</em> button to reset all configuration settings to their default values. Then click the <em>" + sbutton.apply + "</em> button.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Any settings you have saved will be lost when the default settings are restored. After resetting to defaults the router will reboot and will become accessible via the default IP address 192.168.1.1 and entering a new username and password is required before proceeding to the Web interface.</div></div></dd>";

// ** FilterIPMAC.asp **//
var filterIP=new Object();
filterIP.titl="List of Clients";
filterIP.h2="List of Clients";
filterIP.legend1="Enter MAC Address of the clients in this format: xx:xx:xx:xx:xx:xx";
filterIP.legend2="Enter the IP Address of the clients";
filterIP.legend3="Enter the IP Range of the clients";
filterIP.ip_range="IP Range";

// ** Filter.asp **//
var filter=new Object();
filter.titl="Access Restrictions";
filter.h2="WAN Access";
filter.legend="Access Policy";
filter.pol="Policy";
filter.polname="Policy Name";
filter.pcs="List of Clients";
filter.polallow="Internet access on the selected day(s) or time period.";
filter.legend2="Day(s)";
filter.time="Time Period";
filter.h24="24-Hour";
filter.legend3="Blocked Services";
filter.catchall="Catch all P2P Protocols";
filter.legend4="Website Blocking by URL";
filter.legend5="Website Blocking by Keyword";
filter.mess1="Delete the Policy?";
filter.mess2="You must select a day or days.";
filter.mess3="The selected end time must be greater than the start time.";
filter.none="";
filter.packetcount="Filtered Packets";

// ** FilterSummary.asp **//
var filterSum=new Object();
filterSum.titl="Access Restrictions Summary";
filterSum.h2="Internet Policy Summary";
filterSum.polnum="No.";
filterSum.polday=filter.time;

//help container
var hfilter=new Object();
hfilter.right2="You may define up to 20 access policies.<br />Click the <em>" + sbutton.del + "</em> button to delete a policy or the <em>" + sbutton.summary + "</em> button to see a summary of the policy.";
hfilter.right4="Enable or disable a policy.";
hfilter.right6="You may assign a name to your policy.";
hfilter.right8="Choose the day of the week you would like your policy to be applied.";
hfilter.right10="Enter the time of the day you would like your policy to apply.";
hfilter.right12="You may choose to block access to certain services. Click the <em>" + sbutton.filterSer + "</em> button to modify these settings.";
hfilter.right14="You can block access to certain websites by entering their URL.";
hfilter.right16="You can block access to certain websites by the keywords contained in their Web page.";
hfilter.pageintro="<dd>This screen allows you to block or allow specific kinds of Internet usage. You can set up Internet access policies for specific computers and set up filters by using network port numbers.</dd>";
hfilter.page1="<dd>This feature allows you to customize up to ten different Internet Access Policies for particular devices, which are identified by their IP or MAC addresses. For each policy designated devices, during the days and time periods specified.<br /><br />To create or edit a policy, follow these instructions:<ol class=\"wide\"><li>Select the policy number (1-20) from the dropdown menu.</li><li>Enter a name into the <em>" + filter.polname + "</em> field.</li><li>Click the <em>" + sbutton.filterIP + "</em> button.</li><li>On the <em>" + filterIP.titl + "</em> screen, specify computers by IP address or MAC address. Enter the appropriate IP addresses into the <i>IP</i> fields. If you have a range of IP addresses to filter, complete the appropriate <i>IP Range</i> fields. Enter the appropriate MAC addresses into the <i>MAC</i> fields.</li><li>Click the <em>" + sbutton.apply + "</em> button to apply your changes. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.clos + "</em> button to return to the <b>"+ bmenu.accrestriction + "</b> screen.</li><li>If you want to block the listed computers from Internet access during the designated days and time, then keep the default setting, <em>" + share.deny + "</em>. If you want the listed computers to have Internet filtered during the designated days and time, then click the radio button next to <em>" + share.filter + "</em>.</li><li>Set the days when access will be filtered. Select <em>" + share.everyday + "</em> or the appropriate days of the week.</li><li>Set the time when access will be filtered. Select <em>" + filter.h24 + "</em>, or check the box next to <i>From</i> and use the dropdown menus to designate a specific time period.</li><li>Click the <em>" + sbutton.save + "</em> button to save your changes and activate it.</li><li>To create or edit additional policies, repeat steps 1-9.</li></ol><br />To delete an Internet Access Policy, select the policy number, and click the <em>" + share.del + "</em> button.</dd>";
hfilter.page2="<dd>To see a summary of all the policies, click the <em>" + sbutton.summary + "</em> button. The <em>" + filterSum.titl + "</em> screen will show each policy's in a ordered list, Policy Number, Name, Days, and Time of Day. To delete a policy, select the relevant checkbox, and then click the <em>" + share.del + "</em> button. Click the <em>" + sbutton.clos + "</em> button to return to the <b>"+ bmenu.accrestriction + "</b> screen.</dd>";

// ** Firewall.asp **//
var firewall=new Object();
firewall.titl="Firewall";
firewall.h2="Security";
firewall.legend="Firewall Protection";
firewall.firewall="SPI Firewall";
firewall.legend2="Additional Filters";
firewall.proxy="Filter Proxy";
firewall.cookies="Filter Cookies";
firewall.applet="Filter Java Applets";
firewall.activex="Filter ActiveX";
firewall.legend3="Block WAN Requests";
firewall.legend4="Impede WAN DoS / Bruteforce";
firewall.ping="Anonymous WAN Requests (ping)";
firewall.muticast="Multicast Communication";
firewall.ssh="Limit SSH Access";
firewall.telnet="Limit Telnet Access";
firewall.pptp="Limit PPTP Server Access";
firewall.ftp="Limit FTP Server Access";
firewall.arp_spoofing="ARP Spoofing Protection";
firewall.filter_tos="Filter ToS / DSCP";
firewall.filter_invalid="Filter invalid packets";

filter.nat="WAN NAT Redirection";
filter.port113="IDENT (Port 113)";
filter.snmp="WAN SNMP Access";

// ** Firewall.asp **//
var log=new Object();
log.label="Firewall Log";
log.h2="Log Management";
log.legend="Log";
log.lvl="Log Level";
log.drop="Dropped";
log.reject="Rejected";
log.accept="Accepted";

// ** Log_incoming.asp **//
var log_in=new Object();
log_in.titl="Incoming Log Table";
log_in.h2="Incoming Log Table";
log_in.th_ip="Source IP";
log_in.th_port="Destination Port Number";

// ** Log_outgoing.asp **//
var log_out=new Object();
log_out.titl="Outgoing Log Table";
log_out.h2="Outgoing Log Table";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destination URL / IP";
log_out.th_port="Service / Port Number";

//help container
var hfirewall=new Object();
hfirewall.right2="Enable or disable the SPI firewall.";

// help page
hfirewall.page1="<dd>Blocks HTTP requests containing the &quot;<i>Host:</i>&quot; string.</dd>";
hfirewall.page2="<dd>Identifies HTTP requests that contain the &quot;<i>Cookie:</i>&quot; string and mangle the cookie. Attempts to stop cookies from being used.</dd>";
hfirewall.page3="<dd>Blocks HTTP requests containing an URL ending in &quot;<i>.js</i>&quot; or &quot;<i>.class</i>&quot;.</dd>";
hfirewall.page4="<dd>Blocks HTTP requests containing an URL ending in &quot;<i>.ocx</i>&quot; or &quot;<i>.cab</i>&quot;.</dd>";
hfirewall.page5="<dd>Stops the router from responding to &quot;pings&quot; from the WAN.</dd>";
hfirewall.page6="<dd>Prevents multicast packets from reaching the LAN.</dd>";
hfirewall.page7="<dd>Prevents hosts on LAN from using WAN address of router to contact servers on the LAN (which have been configured using port redirection).</dd>";
hfirewall.page8="<dd>Prevents WAN access to port 113.</dd>";
hfirewall.page9="<dd>The router can keep logs of all incoming or outgoing traffic for your Internet connection.</dd>";
hfirewall.page10="<dd>To keep activity logs, select <em>" + share.enable + "</em>. To stop logging, select <em>" + share.disable + "</em>.</dd>";
hfirewall.page11="<dd>Set this to the required amount of information. Set <em>" + log.lvl + "</em> higher to log more actions.</dd>";
hfirewall.page12="<dd>To see a temporary log of the router's most recent incoming traffic, click the <em>" + sbutton.log_in + "</em> button.</dd>";
hfirewall.page13="<dd>To see a temporary log of the router's most recent outgoing traffic, click the <em>" + sbutton.log_out + "</em> button.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** Forward.asp **//
var prforward=new Object();
prforward.titl="Port Range Forwarding";
prforward.h2="Port Range Forwarding";
prforward.legend="Forwards";
prforward.app="Application";

//help container
var hprforward=new Object();
hprforward.right2="Some applications require specific open ports in order to function correctly. Examples of these applications include but are not limited to servers or online games. When the router receives a request from the Internet for a specific port, the data is then routed to the device you specify. Due to security concerns, you may want to limit port forwarding to only those ports in use, and disable them when you are finished.";

//help page
hprforward.page1="<dd>Port Range Forwarding allows you to set up public services on your network, such as Web servers, FTP servers, email servers, or other specialized Internet applications. Specialized Internet applications are any applications that use Internet access to perform functions such as videoconferencing or online gaming. When this type of request is sent to the network via the Internet, the router will forward those requests to the appropriate device.<br /><br />If you only want to forward a single port, see <a href=\"HForward.asp\">Port Forwarding</a>.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Any device whose port is being forwarded must have a static IP address assigned to it because the IP address may change when using the default dynamic DHCP setup.</div></div></dd><dd>To add a new Port Range Forwarding rule, click the <em>" + share.add + "</em> button and fill in the fields below. To remove the last rule, click the  <i>remove</i> icon.</dd>";
hprforward.page2="<dd>Enter the name of the application in the field provided.</dd>";
hprforward.page3="<dd>Enter the number of the first port of the range you want to seen by users on the Internet and forwarded to your device.</dd>";
hprforward.page4="<dd>Enter the number of the last port of the range you want to seen by users on the Internet and forwarded to your device.</dd>";
hprforward.page5="<dd>Chose the right protocol <i>TCP</i>, <i>UDP</i> or <i>Both</i>. Set this to what the application requires.</dd>";
hprforward.page6="<dd>Enter the IP Address of the device running the application.</dd>";
hprforward.page7="<dd>Click the <em>" + share.enable + "</em> checkbox to enable port forwarding for the application.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Remember to save your changes before adding another forwarding rule.</div></div></dd>";

// ** P2P.asp **//
var p2p=new Object();
p2p.titl="Peer-to-Peer Apps";
p2p.h2="BitTorrent Client";
p2p.legend="CTorrent";
p2p.ctorrent_srv="Ctorrent Service";

//help container
//var hp2p=new Object();
//hpp2p.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the device you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";

// ** ForwardSpec.asp **//
var pforward=new Object();
pforward.titl="Port Forwarding";
pforward.h2="Port Forwarding";
pforward.legend="Forwards";
pforward.app="Application";
pforward.src="Source Net";
pforward.from="Port From";
pforward.to="Port To";

//help container
var hpforward=new Object();
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the device you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>Enable</em> checkbox after you are finished.";

// help page
hpforward.page1="<dd>Port Forwarding allows you to set up public services on your network, such as Web servers, FTP servers, email servers, or other specialized Internet applications. Specialized Internet applications are any applications that use Internet access to perform functions such as videoconferencing or online gaming. When users send this type of request to your network via the Internet, the router will forward those requests to the appropriate computer / device.<br /><br />If you want to forward a whole range of ports, see <a href=\"HForward.asp\">";
hpforward.page2="</a>.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Any device whose port is being forwarded must have a static IP address assigned to it because the IP address may change when using the DHCP function.</div></div></dd><dd>To add a new Port Forwarding rule, click the <em>" + sbutton.add + "</em> and fill in the fields below. To remove the last rule, click the <i>remove</i> icon.</dd>";
hpforward.page3="<dd>Enter the name of the application in the field provided.</dd>";
hpforward.page4="<dd>Chose the right protocol <i>TCP</i>, <i>UDP</i> or <i>Both</i>. Set this to what the application requires.</dd>";
hpforward.page5="<dd>Forward only if sender matches this ip/net (example 192.168.1.0/24).</dd>";
hpforward.page6="<dd>Enter the number of the external port (the port number seen by users on the Internet).</dd>";
hpforward.page7="<dd>Enter the IP Address of the device running the application.</dd>";
hpforward.page8="<dd>Enter the number of the internal port (the port number used by the application).</dd>";
hpforward.page9="<dd>Click the <em>" + share.enable + "</em> checkbox to enable port forwarding for the application.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Remember to save your changes before adding another forwarding rule.</div></div></dd>";

// ** IP Forward - 1:1 NAT **//
var pforwardip=new Object();
pforwardip.titl="IP Forward";
pforwardip.h2="IP Forward - 1:1 NAT";
pforwardip.legend="Forwards";
pforwardip.name="Name";
pforwardip.src="Source IP";
pforwardip.dest="Destination IP";

// ** USB.asp **//
var usb=new Object();
usb.titl="USB";
usb.usb_legend="USB Support";
usb.usb_core="Core USB Support";
usb.usb_uhci="USB 1.1 Support (UHCI)";
usb.usb_ohci="USB 1.1 Support (OHCI)";
usb.usb_ehci="USB 2.0 Support";
usb.usb_storage="USB Storage Support";
usb.usb_ip="USB over IP";
usb.usb_printer="USB Printer Support";
usb.usb_automnt="Automatic Drive Mount";
usb.usb_mntpoint="Disk Mount Point";
usb.usb_runonmount="Run on Mount Script Name";
usb.usb_diskinfo="Disk Info";
usb.usb_diskspace="Disk Space";
usb.usb_mntjffs="Mount Partition to /jffs";
usb.usb_mntopt="Mount Partition to /opt";
usb.usb_ses_umount="Use SES Button to Remove Drives";
usb.drive_ra="Drive Read-Ahead Buffer";
usb.drive_ra_sectors="sectors";

//help page
var husb=new Object();
husb.page1="<dd><ul><li>Enable USB Support</li></ul></dd>";
husb.page2="<dd><ul><li>Enable Printer Support</li></ul></dd>";
husb.page3="<dd><ul><li>Enable support for external drives</li></ul></dd>";
husb.page4="<dd><ul><li>Auto mount connected drives</li></ul></dd>";
husb.page5="<dt>Options</dt><dd><ul><li>Run script from the specified path whenever a drive is mounted by automount</li><li>Mount partition with given UUID to specified mount point e.g. /opt</li><li>Use SES Button to un mount drives before disconnecting them</li></ul></dd>";
husb.page6="<dd><ul><li>Displays disk info e.g. partition size, volume name if set, as well as UUID e.g. <i>B965FA66-CC65-4DK8-1254-DD0A78D19A90</i> for all connected drives</li></ul></dd><div class=\"note\"><h4>Note:</h4><div>Automount by default mounts all drives to /mnt/<devname> e.g. <i>/mnt/sda1</i> <br />If your volume name is <b>opt</b> the partition will be mounted to <b>/opt</b>, if set to jffs -> <b>/jffs</b>.<br />You can override this by entering a partitions UUID in the option fields.</div></div><br />";

// ** NAS.asp **//
var nas=new Object();
nas.titl="NAS";
nas.h2="Network Attached Storage (NAS)";
nas.proftpd_legend="FTP Access";
nas.proftpd_srv="ProFTPD Server";
nas.proftpd_srv_label="Enable Server";
nas.proftpd_port="Server Port";
nas.proftpd_dir="Files Directory";
nas.proftpd_passw="User Password List";
nas.proftpd_writeen="Allow Write";
nas.proftpd_anon="Anonymous Login (Read-only)";
nas.proftpd_anon_dir="Anonymous Home Directory";

nas.samba3="Samba Server";
nas.samba3_legend="File Sharing";
nas.samba3_legend1="Share Configuration";
nas.samba3_legend2="Samba Configuration";
nas.samba3_srv_label=nas.proftpd_srv_label;
nas.nfs="Network File System Server (NFS)";
nas.rsync="Remote Synchronisation";

// ** DLNA **
nas.dlna_legend="DLNA Server";
nas.dlna_srv="MiniDLNA";
nas.dlna_label=nas.proftpd_srv_label;
nas.dlna_merge="Merge Media Dirs";
nas.dlna_no_art="Ignore Album Art";
nas.dlna_subtitles="Enable Subtitles";
nas.dlna_thumb="Include Cover Artwork";
nas.dlna_dir="Files Directory";
nas.dlna_cleandb="Clean Database";
nas.dlna_metadata="Keep Metadata on Storage";
nas.dlna_rescan="Cyclic Rescan of Folders";

// ** Common **
nas.shareadd="Add Share";
nas.sharedel="Remove";
nas.useradd="Add User";
nas.userdel="Remove";
nas.perm_rw="Read / Write";
nas.perm_ro="Read Only";
nas.uname="User Name";
nas.pwd="Password";
nas.format="Format";
nas.raidmanager="RAID Storage Manager";
nas.raidmember="RAID Member";
nas.raidnametbl="Name";
nas.raidleveltbl="Level";
nas.raiddeduptbl="Dedup";
nas.mirror="Mirror";
nas.fs="File System";
nas.drivemanager="Drive Manager";
nas.drive="Drive";
nas.compression="Compression";

// Help container
var hnas=new Object();
hnas.right2="In order to share resources add new shares by supplying a path and share name. Define users that can access the share through FTP or Samba.<br /><br /><b>MiniDLNA Warning:</b> Unless you mount a partition to <b>/jffs</b> the index DB will be stored in RAM. This can fill up your RAM and will initiate a DB reindex at every boot.";

//help page
hnas.page1="<dd>A FTP server enables you to share files: <br /><ul><li>Over the Internet - WAN</li><li>Over - LAN / WLAN</li></ul></dd>";
hnas.page2="<dd>A DLNA server enables you to share media: <br /><ul><li>You need a DLNA capable client e.g. a TV to view files served by the router.</li></ul></dd>";
hnas.page3="<dd>A Samba server enables you to access files: <br /><ul><li>Via file explorer on your client device the shares you have setup and have connected to the router's USB ports.</li></ul></dd>";
hnas.page4="<dd><ul><li>Path: Path to mounted partition. See currently mounted disks under <span class=\"link\" onclick=\"window.open('../USB.asp')\">Disk Info</span></li><li>Subdir: Directory name on given partition e.g. public or folder/sub_folder/sub_sub_folder </li><li>Name: Share name displayed when browsing the network shares e.g. <b>\\router\name</b> </li><li>Public: Everyone can access this share. No user account required.</li></ul></dd><div class=\"note\"><h4>Note:</h4><div>For Windows users: a connected USB drive shows up in Windows under e.g. <b>D:</b> and contains two directories <i>public and secret</i>.<br />You want to share <i>D:\public</i>. To do this connect the USB drive and lookup or specify a mountpoint under <span class=\"link\" onclick=\"window.open('../USB.asp')\">USB</span> settings page.<br /><b>/dev/sdX</b> equals a mount point under Linux. In order to share <i>D:\public</i>, select current mount point and specify subdir <i>public</i> give it a name e.g. <i>Guest and setup access permissions</i>.</div></div><br />";

//** Privoxy.asp **//
var privoxy=new Object();
privoxy.titl="Adblocking";
privoxy.legend="Filtering Proxy Server";
privoxy.server="Privoxy";
privoxy.server_label="Enable Web Proxy";
privoxy.pac="Provide Proxy Autoconfig";
privoxy.transp="Transparent Mode";
privoxy.exclude="Exclude IP";
privoxy.whitel="Whitelist";
privoxy.custom="Custom Configuration";
privoxy.maxclient="Max Client Connections";

var hprivoxy=new Object();
hprivoxy.right2="Enable Privoxy and configure your clients proxy settings.<br /><br />Proxy IP = <b>router IP</b><br />Proxy Port = <b>8118</b>";

//help page
hprivoxy.page1="<dd>Is a <i>privacy enhancing proxy</i>, filtering web pages and removing common advertisements.</dd>";
hprivoxy.page2="<dd><br /><ul><li>Publishes a WPAD / PAC file that clients use to automatically setup proxy details.</li><li>On some clients you need to set the option to use automatic proxy config</li><li>If your client is part of a domain you need to setup dnsmasq to send the local domain via DHCP to the client</li></ul></dd>";
hprivoxy.page3="<dd><br /><ul><li>Traffic to Port 80 is intercepted by Privoxy even if the client did not configure any proxy settings.</li><li>Thus you can enforce ad filtering.</li></ul></br /><div class=\"note\"><h4>Note:</h4> Transparent mode cannot intercept HTTPS connections. All HTTPS traffic will still bypass Privoxy unless the client uses the <b>proxy autoconfig</b> which pipes these HTTPS connections through the proxy</div></dd>";
hprivoxy.page4="<dd><br /><ul><li>Allows you to specify custom settings and paths to filters stored on external media e.g. a USB disk</li></dd>";

//** Lighttpd.asp **//
var lighttpd=new Object();
lighttpd.titl="Web Server";
lighttpd.legend="Lighttpd Web Server";
lighttpd.server=nas.proftpd_srv_label;
lighttpd.port="HTTP Port";
lighttpd.sslport="HTTPS Port";
lighttpd.wan="WAN Access";
lighttpd.url="URL";

var hlighttpd=new Object();
hlighttpd.right2="Enable lighttpd and configure the HTTP or HTTPS ports where lighttpd will listen for a request.<br /><br />The default Web server's root is located on <b>/jffs/www</b> where you can place your website's files.";

var lltd=new Object();
lltd.legend="Link Layer Topology Discovery (LLTD)";
lltd.srv_label="Enable Discovery";

// ** Hotspot.asp **//
var hotspot=new Object();
hotspot.titl="Hotspot";
hotspot.h2="Hotspot Portal";
hotspot.legend="CoovaChilli";
hotspot.label="Enable Portal";
hotspot.hotspot="CoovaChilli";
hotspot.nowifibridge="Separate WiFi from the LAN Bridge";

hotspot.pserver="Primary RADIUS Server IP / DNS";
hotspot.bserver="Backup RADIUS Server IP / DNS";
hotspot.dns="DNS Server IP";
hotspot.url="Redirect URL";
hotspot.dhcp="DHCP Interface";
hotspot.radnas="RADIUS NAS ID";
hotspot.net="Hotspot Network";
hotspot.uam="UAM Secret";
hotspot.uamserver="UAM Server";
hotspot.uamport="UAM Port";
hotspot.uamdns="UAM any DNS";
hotspot.allowuam="UAM Allowed (comma separated)";
hotspot.allowuad="UAM Domains (space separated)";
hotspot.macauth="MAC Authentication";
hotspot.macpasswd="MAC Password";
hotspot.sec8021Xauth="802.1X Authentication (EAP)";
hotspot.option="Additional Options";
hotspot.fon_chilli="Local User Management";
hotspot.fon_user="User List";
hotspot.http_legend="HTTP Redirect";
hotspot.http_srv="Enable Redirect";
hotspot.http_ip="Destination IP";
hotspot.http_port="Destination Port";
hotspot.http_net="Source Network";

hotspot.nodog_legend="NoDogSplash";
hotspot.nodog_srv="Enable Portal";
hotspot.nodog_gateway="Gateway Name";
hotspot.nodog_gatewayport="Gateway Port";
hotspot.nodog_gatewayaddr="Gateway IP";
hotspot.nodog_gatewayiprange="Gateway IP Range";
hotspot.nodog_home="Homepage";
hotspot.nodog_extifname="External Interface";
hotspot.nodog_ifname="Internal Interface";
hotspot.nodog_redirect="Homepage Redirection";
hotspot.nodog_allowweb="Allowed Web Hosts";
hotspot.nodog_docroot="Document Root";
hotspot.nodog_splash="Splash URL";
hotspot.nodog_port="Exclude Ports";
hotspot.nodog_timeout="Login Timeout";
hotspot.nodog_verbose="Verbosity";
hotspot.nodog_route="Route Only";
hotspot.nodog_MAClist="MAC Whitelist";
hotspot.nodog_maxclients="Max Number of Clients";
hotspot.nodog_downloadlimit="Download Limit";
hotspot.nodog_uploadlimit="Upload Limit";

hotspot.smtp_legend="SMTP Redirect";
hotspot.smtp_srv="Enable Redirect";
hotspot.smtp_ip="Destination IP";
hotspot.smtp_net="Source Network";

hotspot.shat_legend="Zero IP Configuration";
hotspot.shat_srv="Zero IP";
hotspot.shat_srv2="Enable Service";

hotspot.wifidog_legend="WiFiDog";
hotspot.wifidog_srv="Enable Gateway";
hotspot.wifidog_id="Gateway ID";
hotspot.wifidog_url="Portal URL";
hotspot.wifidog_port="Port";
hotspot.wifidog_httpdname="Webserver Name";
hotspot.wifidog_httpdconn="Max Users";
hotspot.wifidog_checkinter="Check Interval(s)";
hotspot.wifidog_checktimeout="Client Timeout";
hotspot.wifidog_tmaclist="Trusted MAC List";
hotspot.wifidog_authsrv="AuthServer Hostname";
hotspot.wifidog_authsrvssl="Enable SSL AuthServer";
hotspot.wifidog_authsrvsslport="AuthServer SSL Port";
hotspot.wifidog_authsrvhttpport="AuthServer HTTP Port";
hotspot.wifidog_authsrvpath="AuthServer Path";
hotspot.wifidog_config="Firewall Ruleset";
hotspot.wifidog_messagefile="HTML Message File";
hotspot.wifidog_realm="HTTP Server Realm";
hotspot.wifidog_username="HTTP Server Username";
hotspot.wifidog_password="HTTP Server Password";
hotspot.wifidog_auth="Server Authentication Support";

//help container
var hstatus_hots=new Object();
hstatus_hots.right1="conup/condown:<br />When USB or JFFS2 is mounted to <b>/jffs</b>, connection scripts are accessible at <b>/jffs/etc/chilli/</b><br />Local Users:<br />When only local users are used, set the primary RADIUS to <b>127.0.0.1</b>";

// help page
hstatus_hots.page1="<dd>You can use the router as an Hotspot gateway (CoovaChilli solution) with authentication and accounting (RADIUS). CoovaChilli is an opensource captive portal or wireless LAN access point controller. It is used for authenticating users of a wireless LAN. It supports Web based login while also supporting Wireless Protected Access (WPA). Authentication, authorization and accounting (AAA) is handled by your RADIUS server.</dd>";
hstatus_hots.page2="<dd>You can have your hotspot portal managed by HotspotSystem.com. They provide free and pay-per-use hotspot solutions with billing. For more information please visit <a href=\"http:\/\/www.hotspotsystem.com\">www.hotspotsystem.com</a></dd>";
hstatus_hots.page3="<dd>The Operator Username which you have registered at <a href=\"http:\/\/www.hotspotsystem.com\">www.hotspotsystem.com</a></dd>";
hstatus_hots.page4="<dd>The number of the location you would like to set up.</dd>";
hstatus_hots.page5="<dd>If this option is enabled (default), then WiFi and LAN are handled separately. For example you can forward WiFi users to a splash page, while LAN ports remain completely free. If you set this to disable, then all ports are handled together.</dd>";
hstatus_hots.page6="<dd>This has to be the interface you would like to use for the hotspot portal.</dd>";
hstatus_hots.page7="<dd>Network mask where you wish to run the hotspot service.</dd>";
hstatus_hots.page8="<dd>Here you can make changes on your splash page. You can add more sites for free browsing. If you are a White Label operator you can use your own domain name on your splash pages. Options are:<ul><li>White Label Protocol: choose HTTPS if you have an SSL certificate installed on the White Label domain.</li><li>White Label Domain: Put your White Label domain here or other servers provided by HotspotSystem.com</li><li>Login on Splash Page: You can have the login box displayed on the main splash page. You also need to set this behavior in HotspotSystem.com's Control Center, on Manage > Locations > click on location > Modify Hotspot Settings > Splash Page Settings!</li><li>Custom Splash Page (Walled Garden): You can forward users to your own splash page first. For this option you have to add your domain to the UAM allowed list below, and set it in the Control Center. You also have to enter a backlink to our splash page so your customers will be able to log in or buy access / use a voucher.</li><li>UAM Allowed: IP addresses or network segments the client can access without first authenticating (comma separated list of domain names). e.g. <i>www.chillispot.info,10.11.12.0/24</i></li></ul></dd>";
hstatus_hots.page9="<dd>The IP addresses of RADIUS server 1 and 2.</dd>";
hstatus_hots.page10="<dd>DNS Server IP. It is used to inform the client about the DNS address to use for host name resolution. If this option is not given the system primary DNS is used.</dd>";
hstatus_hots.page11="<dd>URL of Web server to use for authenticating clients.</dd>";
hstatus_hots.page12="<dd>RADIUS shared secret for both servers. This secret should be changed in order not to compromise security.</dd>";
hstatus_hots.page13="<dd>Ethernet interface to listen to for the downlink interface. This option must be specified.</dd>";
hstatus_hots.page14="<dd>Network access server identifier.</dd>";
hstatus_hots.page15="<dd>Shared secret between uamserver and chilli. This secret should be set in order not to compromise security.</dd>";
hstatus_hots.page16="<dd>Allow any DNS server. Normally unauthenticated clients are only allowed to communicate with the DNS servers specified by the dns1 and dns2 options. This option will allow the client to use all DNS servers. This is convenient for clients which are configured to use a fixed set of DNS servers.<br /><br /><div class=\"note\"><h4>Note:</h4><div>For security reasons this option should be combined with a destination NAT firewall rule which forwards all DNS requests to a given DNS server.</div></div></dd>";
hstatus_hots.page17="<dd>IP addresses or network segments the client can access without first authenticating (comma separated list of domain names) e.g. <i>www.chillispot.info,10.11.12.0/24</i></dd>";
hstatus_hots.page18="<dd>If this option is given CoovaChilli will try to authenticate all users based solely on their respective devices MAC address(es).</dd>";
hstatus_hots.page19="<dd>You can specify here additional options.<br /><br /><div class=\"note\"><h4>Note:</h4><div>For more informations about the different options : <a href=\"https:\/\/coova.github.io\">coova.github.io</a>.</div></div></dd>";
hstatus_hots.page20="<dd>Enabling the use of NoDogSplash allows you to redirect a client to a specific Web page when connecting via wireless or wired.</dd>";
hstatus_hots.page21="<dd>The name of the gateway. Whatever you want to call it. \"Joe's Pizza Shop and free DSL Cafe\" for example. Use the variable $GatewayName in your splash.html page to display this.</dd>";
hstatus_hots.page22="<dd>Configures the Redirection URL after splash login</dd>";
hstatus_hots.page23="<dd>Enables the redirection to a specific Homepage after splash login, see above</dd>";
hstatus_hots.page24="<dd>Space separated list of hostnames. List any hosts (for example, the Web server containing the splash page or other websites) that you wish to allow clients to have Web access to (TCP port 80 (HTTP) and 443 (HTTPS)) before users <i>log in</i> (and click on <i>I Accept</i> in your splash page). Note: however actual authentication is not supported. List any Web servers that you wish connecting clients to be able to access, before clicking on <i>I Agree</i> on the initial nocatsplash screen, such as the Web server hosting your EULA or welcome page, if it is not hosted on the router itself.</dd>";
hstatus_hots.page25="<dd>Where all of the application templates (including SplashForm) are hiding (splash.html is the form displayed to users on capture).</dd>";
hstatus_hots.page26="<dd>Optional URL to fetch dynamic remote splash page from. This should end with <i>/splash.html</i> or the name of your splash page.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Leave empty if using a page stored on the router.</div></div></dd>";
hstatus_hots.page27="<dd>Space separated list of ports. Specify TCP ports to denied access to when public class users login. All others will be allowed. If nothing is specified, access is granted to all ports to public class users.<br /><br /><div class=\"note\"><h4>Note:</h4><div>You should <b>always</b> exclude port 25 (SMTP), unless you want to run a portal for wanton spam sending. Users should have their own way of sending mail.</div></div></dd>";
hstatus_hots.page28="<dd>How much time, in seconds, elapses before the client has to see the splash screen again, and click on 'I Agree'. How often a client is shown the EULA or other designated splash page.</dd>";
hstatus_hots.page29="<dd>Log verbosity (to syslogd and /tmp/nocat.log). Syslogd service must be enabled.<ul><li>0 is (almost) no logging.</li><li>10 is log everything.</li><li>5 is probably a safe middle road.</li></ul></dd>";
hstatus_hots.page30="<dd>Required only if you DO NOT want your gateway to act as a NAT. Enable this only if you are running a strictly routed network, and do not need the gateway to enable NAT for you. You would not normally use this option. So if you do not understand it, leave it Disabled</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.reboot + "</em> button to reboot your router immediately.</dd>";

// ** Hotspotsystem **//
var hotspotsys=new Object();
hotspotsys.legend="Hotspot System";
hotspotsys.nobridge="Separate WiFi from LAN Bridge";
hotspotsys.uamenable="Special Settings";
hotspotsys.loginonsplash="Login on Splash Page";
hotspotsys.allowuam="UAM Allowed";
hotspotsys.allowuad="UAM Domains (space separated)";
hotspotsys.whitelabelproto="White Label Protocol";
hotspotsys.whitelabel="White Label Domain";
hotspotsys.operatorid="Operator Username";
hotspotsys.locationid="Location ID";
hotspotsys.dhcp="DHCP Interface";
hotspotsys.net="Remote Network";
hotspotsys.customsplash="Custom Splash Page (Walled Garden)";

// ** Info.htm **//
var info=new Object();
info.titl="Info";
info.h2="System Information";
info.wlanmac="Wireless MAC";
info.srv="Services";
info.ap="Access Point";

// ** index_heartbeat.asp **//
var idx_h=new Object();
idx_h.srv="Heartbeat Server";
idx_h.con_strgy="Connection Strategy";
idx_h.max_idle="Connect on Demand: Max Idle Time";
idx_h.alive="Keep Alive: Redial Period";
idx_h.reconnect="Force Reconnect";

// ** index_l2tp.asp **//
var idx_l=new Object();
idx_l.srv="Gateway (L2TP Server)";
idx_l.req_chap="Require CHAP";
idx_l.ref_pap="Refuse PAP";
idx_l.req_auth="Require Authentication";
idx_l.iptv="Receive IPTV";

// ** index_pppoe.asp **//
var idx_pppoe=new Object();
idx_pppoe.use_rp="Use RP-PPPoE";

// ** index_pptp.asp **//
var idx_pptp=new Object();
idx_pptp.srv="Use DHCP";
idx_pptp.wan_ip="WAN IP Address";
idx_pptp.gateway="Gateway (PPTP Server)";
//idx_pptp.encrypt="PPTP Encryption"; //should be needed anymore. del soon
idx_pptp.reorder="Packet Reordering";
idx_pptp.addopt="Additional Options";
idx_pptp.iptv="Receive IPTV";

// ** index_static.asp **//
var idx_static=new Object();
idx_static.dns="Static DNS";

// ** index.asp **//
var idx=new Object();
idx.titl="Setup";
idx.h2="WAN Setup";
idx.h22="Wireless Setup";
idx.legend="WAN Connection Type";
idx.conn_type="Connection Type";
idx.ignore_wan_dns="Ignore WAN DNS";
idx.stp="STP";
idx.sfe="Shortcut Forwarding Engine";
idx.fa="Flow Acceleration";
idx.stp_mess="(disable for COMCAST ISP)";
idx.optional="Optional Settings";
idx.mtu="MTU";
idx.txqlen="TX Queue Length";
idx.label="Label";
idx.h23="Network Setup";
idx.routerip="Router IP";
idx.lanip="Local IP Address";
idx.legend2="WAN Port";
idx.wantoswitch="Assign WAN Port to Switch";
idx.static_ip="Static IP";
idx.dhcp="Automatic Configuration - DHCP";
idx.dsl_mdm_bdg="DSL Modem Bridge";
idx.pppoe_dual="PPPoE Dual (MLPPP)";
idx.heartbeat_sig="Heartbeat Signal";
idx.iphone_tether="iPhone Tethering";
idx.mobile_bb="Mobile Broadband";
idx.dhcp_auth="DHCP Authentication";
idx.dhcp6c_auth="DHCP IPv6 Authentication";
idx.dhcp_userclass="DHCP User Class";
idx.dhcp6c_userclass="DHCP IPv6 User Class";
idx.dhcp_clientid="DHCP Client ID";
idx.dhcp6c_clientid="DHCP IPv6 Client ID";
idx.dhcp_legend="Dynamic Host Configuration Protocol (DHCP)";
idx.dhcp_type="DHCP Type";
idx.dhcp_srv="DHCP Server";
idx.dhcp_fwd="DHCP Forwarder";
idx.dhcp_start="Start IP Address";
idx.dhcp_end="End IP Address";		//used in Status_Lan.asp
idx.dhcp_maxusers="Maximum DHCP Users";
idx.dhcp_lease="Lease Expiration";
idx.dhcp_dnsmasq="Use dnsmasq for DHCP";
idx.dns_dnsmasq="Use dnsmasq for DNS";
idx.wan_priority="Use VLAN Priority";
idx.auth_dnsmasq="DHCP-Authoritative";
idx.force_dnsmasq="Forced DNS Redirection";
idx.force_dnsmasqdot="Forced DNS Redirection DoT";
idx.recursive_dns="Recursive DNS Resolving (Unbound)";
idx.dns_redirect="Optional DNS Target";
idx.summt_opt1="None";
idx.summt_opt2="first Sun Apr - last Sun Oct";
idx.summt_opt3="last Sun Mar - last Sun Oct";
idx.summt_opt4="last Sun Oct - last Sun Mar";
idx.summt_opt5="2nd Sun Mar - first Sun Nov";
idx.summt_opt6="first Sun Oct - 3rd Sun Mar";
idx.summt_opt7="last Sun Sep - first Sun Apr";
idx.summt_opt8="3rd Sun Oct - 3rd Sun Mar";
idx.summt_opt9="first Sun Oct - first Sun Apr";
idx.summt_opt10="3rd Sun Oct - 3rd Sun Feb";
idx.interface_h2="Interface Setup";
idx.portsetup="Port Setup";
idx.wanport="WAN Port Assignment";
idx.legend3="NTP Client Settings";
idx.timeset="Time Zone";
idx.ntp_input_placeholder="2.pool.ntp.org 212.18.3.19 88.99.174.22";
idx.ntp_client="Enable Client";
idx.ntp_timer="Update Interval";

//help container
var hidx=new Object();
hidx.right2="This setting is most commonly used by cable operators.";
hidx.right4="Enter the hostname provided by your ISP.";
hidx.right6="Enter the domain name provided by your ISP.";
hidx.right8="This is the LAN-side IP address of the router.";
hidx.right10="This is the subnet mask of the router.";
hidx.right12="Allows the router to manage your IP addresses.";
hidx.right14="The address you would like to start with.";
hidx.right16="You may limit the number of addresses your router hands out. 0 means only predefined static leases will be handed out.";
hidx.right18="Select your current time zone. When the " + share.srvipname + " is left blank, an automated NTP server resolution address will be used.";
hidx.sas="The setup assistant guides you through the basic setup steps to configure your router.";

//help page
hidx.intro="<dd>The Setup screen is the first screen you will see when accessing the router. For a basic working router configuration while using only the settings on this screen is possible. Some Internet Service Providers (ISPs) will require that you enter specific information, such as a username, password, IP address, default gateway address, or a DNS IP address. This information can be obtained from your ISP, if required.<br /><br /><div class=\"note\"><h4>Note:</h4><div>After you have configured these settings, you should set a new password for the router using the <a href=\"HManagement.asp\">Management</a> screen. This will enhance security by protecting the router from unauthorized changes. All users that try to access the router's Web interface or Setup Wizard will be prompted to enter the router's login credentials.</div></div></dd>";
hidx.wanctype="<dd>The router supports several connection types:<ul><li>" + share.disabled + "</li><li>" + idx.static_ip + "</li><li>" + idx.dhcp + "</li><li>" + idx.dhcp_auth + "</li><li>PPPoE</li><li>" + idx.pppoe_dual + "</li><li>PPTP</li><li>L2TP</li><li>" + idx.heartbeat_sig + "</li><li>" + idx.iphone_tether + "</li><li>" + idx.mobile_bb + "</li></ul><br />The connection type can be selected from the dropdown menu labeled <em>" + idx.conn_type + "</em>. The information required and available features will differ depending on what kind of connection type you select.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Some cable providers require a specific MAC address for connection to the Internet. To learn more about this, <a href=\"HWanMAC.asp\">click here to read</a> about the MAC Address Cloning feature.</div></div></dd>";
hidx.ignwandns="<dd>This option plugs a known DNS leak that may occur with some ISP DNS servers. Any ISP DNS servers and alternatively those configured <em>" + idx_static.dns + "</em> are added to the router's <b>/tmp/resolv.dnsmasq</b></dd>";
hidx.rname="<dd>This entry defines how the router is identified on your LAN.</dd>";
hidx.hname="<dd>This entry is necessary for some ISPs and can be provided by them.</dd>";
hidx.dname=hidx.hname;
hidx.mtu="<dd>Maximum Transmission Unit (MTU), specifies the largest packet size permitted for Internet transmission. The default option is <em>" + share.auto + "</em> and is resolved by the router for best results, you can alternatively select <em>" + share.manual + "</em> which has a default value of 1500. You should leave this value in the 1200 to 1500 range.</dd>";
hidx.sfe="<dd>The options are <em>" + share.disable + "</em> <em>SFE</em> or <em>CTF</em>. SFE is a Qualcomm's opensource in Linux kernel IP packet forwarding engine, providing very high speed IP packet forwarding based on IP connection tracking. CTF is a Broadcom's proprietary Cut Through Forwarding software optimization technique to accelerate NAT traffic.</dd>";
hidx.fa="<dd>For Broadcom based routers the available options are <b>Cut Through Forwarding</b> (CTF), on supported devices <b>Cut Through Forwarding & Flow Acceleration</b> (CTF & FA) will also be available and requires two reboots in order to become available.<br /><br /><div class=\"note\"><h4>Note:</h4><div>CTF supports adaptive QoS your mileage may vary, CTF & FA has no QoS support, in addition you may find that PPPoE, STP, port forwarding and parental controls may also not work.<br />Other caveats due to the increased retransmissions caused by these acceleration methods may cause shuttering on some streaming devices including VoIP.<br />SFE can present issues with multiple interfaces e.g. unbridged VAPs / bridges and latency sensitive traffic like VoIP.</div></div></dd>";
hidx.stp="<dd>To build a loop-free logical topology for your network, you canoptionally enable the Spanning Tree Protocol (STP). It prevents bridge loops and the resulting broadcast radiation.</dd>";
hidx.page5="<dd>This is the router's IP Address and Subnet Mask as seen by external users on the Internet (including your ISP). If your Internet connection requires a static IP address, then your ISP should provide you with a Static IP Address and a Subnet Mask.</dd>";
hidx.page6="<dd>Your ISP will provide you with the Gateway IP Address.</dd>";
hidx.page7="<dd>Your ISP will provide you with at least one DNS IP Address.</dd>";
hidx.page8="<dd>Enter the User Name and Password you use when logging onto your ISP through a PPPoE or PPTP connection.</dd>";
hidx.page9="<dd>The PPP Compression provides a method to negotiate and utilize compression protocols over PPP encapsulated links. It is based on the Microsoft Point-to-Point Compression (MPPC) protocol. A protocol wich is designed for transferring compressed datagrams over point-to-point links.</dd>";
hidx.page10="<dd>Microsoft Point-to-Point Encryption (MPPE). A protocol designed for transferring encrypted datagrams over point-to-point links.</dd>";
hidx.page11="<dd>This option schedules the PPPoE reconnection by killing the PPPD daemon and restarting it.</dd>";
hidx.page12="<dd>You can configure the router to disconnect your Internet connection after a specified period of inactivity (Max Idle Time). If your Internet connection has been terminated due to inactivity, Connect on Demand enables the router to automatically re-establish your connection as soon as you attempt to access the Internet again. If you wish to activate Connect on Demand, click the radio button. If you want your Internet connection to remain active at all times, enter 0 in the Max Idle Time field. Otherwise, enter the number of minutes you want to have elapsed before your Internet connection terminates.</dd>";
hidx.page13="<dd>This option keeps you connected to the Internet indefinitely, even when your connection sits idle. To use this option, click the radio button next to <i>Keep Alive</i>. The default Redial Period is 30 seconds (i.e. the router will check the Internet connection every 30 seconds).</dd>";
hidx.page15="<dd>This is the router IP Address and Subnet Mask as seen by the LAN. The default value is 192.168.1.1 for IP Address and 255.255.255.0 for Subnet Mask.</dd>";
hidx.page16="<dd>Keep the default, <em>" + share.enable + "</em>, to enable the router's DHCP server option. If you already have a DHCP server on your network or you do not want a DHCP server, then select <em>" + share.disable + "</em>.</dd>";
hidx.page17="<dd>Enter a numerical value for the DHCP server to start with when issuing IP addresses. Do not start with 192.168.1.1 as this is the router's own IP address.</dd>";
hidx.page18="<dd>Enter the maximum number of computers that you want the DHCP server to assign IP addresses to. The absolute maximum is 253, possible if 192.168.1.2 is your starting IP address.</dd>";
hidx.page19="<dd>The client's Lease Expiration is the amount of time a network device will be allowed a connection to the router with their current dynamic/static IP address. Enter the amount of time, in minutes, that the device will be \"leased\" with this dynamic/static IP address.</dd>";
hidx.page20="<dd>The Domain Name System (DNS) is how the Internet translates domain or website names into Internet addresses or URLs. Your ISP will provide you with at least one DNS Server IP address. If you wish to use another, enter that IP address in one of these fields. You can enter up to three DNS Server IP addresses here. The router will use these for quicker access to functioning DNS servers. Additionally you can enable the <em>" + idx.ignore_wan_dns + "</em> checkbox to ensure the ISP's DNS is ignored.</dd>";
hidx.page21="<dd>The Windows Internet Naming Service (WINS) manages each devices interaction with the Internet. If you use a WINS server, enter that server's IP address here. Otherwise, leave this blank.</dd>";
hidx.dns_dnsmasq="<dd>The dnsmasq's local DNS server resolves all host names known to the router from DHCP (dynamic and static) as well as forwarding and caching DNS entries from remote DNS servers. Enabling dnsmasq for DNS enables DHCP clients on the LAN to resolve local hostnames, including static and dynamic IP assignments.</dd>";
hidx.auth_dnsmasq="<dd>DHCP-Authoritative should be set when DD-WRT is the only DHCP server on the network segment (as in most common setups). The DHCP server will return a <b>NAK</b> response to clients that try to register using IP addresses from another netblock.</dd>";
hidx.force_dnsmasq="<dd>This setting causes all port 53 DNS requests from the LAN to external DNS servers to be redirected to DD-WRT's internal dnsmasq server.</dd>";
hidx.page22="<dd>Select the time zone for your location, or desired location.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. You can test the settings by connecting to the Internet.</dd>";

var hipv6=new Object();
hipv6.right2="IPv6 is a the most recent Internet Protocol standard used by electronic devices to exchange data across a packet-switched network.<br /><br />Changes over IPv4 fall primarily under the following categories:<ul style=\"margin-left: -1.5rem\"><br /><li>Expanded addressing capabilities</li><li>Header format simplification</li><li>Improved support for extensions & options</li><li>Flow labeling capability</li><li>Authentication & privacy capabilities</li></ul>";

// ** DSL ** //
var dsl=new Object();
dsl.status="DSL Status";
dsl.annex=" DSL Annex";
dsl.iface_status="Connection Status";
dsl.datarate="Connection Speed (up/down)";
dsl.snr="DSL Signal (up/down)";

// ** Join.asp **//
var join=new Object();
//sshd.webservices
join.titl="Join";
join.mess1="Successfully joined the following network as a client: ";

// ** Management.asp **//
var management=new Object();
management.titl="Administration";
management.h2="Router Management";
management.changepassword="Your router is currently not protected and uses an unsafe default username and password combination; please change it using the following dialog!";
management.psswd_legend="Login Credentials";
management.pwd_ti_aria="Valid Symbols &#x21; &#x22; &#x23; &#x24; &#x25; &#x26; &#x27; &#x28; &#x29; &#x2a; &#x2b; &#x2c; &#x2d; &#x2e; &#x2f; &#x3b; &#x3c; &#x3d; &#x3e; &#x3f; &#x40; &#x5b; &#x5c; &#x5d; &#x5e; &#x5f; &#x60; &#x7b; &#x7c; &#x7d; &#x7e; &#x20;\nInvalid Symbols &#x3a;";
management.psswd_user="Username";
management.psswd_pass="Password";
management.pass_conf="Re-enter to Confirm";
management.too_short="Too Short";
management.very_weak="Very Weak";
management.weak="Weak";
management.good="Good";
management.strong="Strong";
management.very_strong="Very Strong";
management.score="Score:";
management.complexity="Complexity:";
management.cpbutton="Change Credentials";
management.cpbutton_changed="Changed Credentials";
management.remote_legend="Remote Access";
management.remote_gui="Web UI Management";
management.remote_https="Use HTTPS";
management.remote_guiport="Port";
management.remote_ssh="SSH Management";
management.remote_sshport="Remote Port";
management.remote_telnet="Telnet Management";
management.remote_telnetport="Remote Port";
management.remote_allowanyip="Allow any Remote IP";
management.remote_ip="Allowed Remote IP Range";
management.web_legend="Web Access";
management.web_refresh="Auto Refresh";
management.web_authlimit="Reauthentication Limit";
management.web_sysinfo="Enable Info Site";
management.web_sysinfopass="Info Site Password Protection";
management.web_sysinfomasq="Info Site MAC Masking";
management.boot_legend="Boot Time Recovery";
management.poeswitch="POE Switch";
management.boot_srv="Boot Wait";
management.cron_legend="Cron";
management.cron_srvd="Enable Cron";
management.cron_jobs="Additional Jobs";
management.loop_legend="Loopback";
management.loop_srv="Loopback";
//802.1x was removed in r49200 / r49201
//management.wifi_legend="802.1x";
//management.wifi_srv="802.1x";
management.rst_legend="Reset Button";
management.rst_srv="Enable Button";
// Routing was removed as its obsolete
//management.routing_legend="Routing";
//management.routing_srv="Routing";
management.ipv6_h2="Internet Protocol version 6 (IPv6)";
management.ipv6_legend="Configuration";
management.ipv6_srv="Enable IPv6";
management.ipv6_native="Native IPv6 from ISP";
management.ipv6_px_del="DHCPv6 with Prefix Delegation";
management.ipv6_6in4st="6in4 Static Tunnel";
management.ipv6_typ="Type";
management.ipv6_pf_len="Prefix Length";
management.ipv6_rad_legend="Router Advertisement Daemon (radvd)";
management.ipv6_rad_enable="Enable Daemon";
management.ipv6_rad="Custom Configuration";
management.ipv6_radconf="Configuration";
management.ipv6_dns="Static DNS";
management.ipv6_prefix="Assigned / Routed Prefix";
management.ipv6_addr="Router IPv6 Address";
management.ipv6_dhcp6c_legend="DHCPv6 Client Daemon";
management.ipv6_dhcp6c_norelease="No Release on Reconnect";
management.ipv6_dhcp6c_cust="Custom Configuration";
management.ipv6_dhcp6c_conf="Configuration";
management.ipv6_dhcp6s_legend="DHCPv6 Server Daemon";
management.ipv6_dhcp6s="Enable Daemon";
management.ipv6_dhcp6s_seq_ips="Sequential IPs";
management.ipv6_dhcp6s_hosts="Custom Hosts";
management.ipv6_dhcp6s_cust="Custom Configuration";
management.ipv6_dhcp6s_conf="Configuration";
management.ipv6_tun_end_ipv4="Tunnel Endpoint IPv4 Address";
management.ipv6_tun_client_addr="Tunnel Client IPv6 Address";
management.ipv6_tun_upd_url="Tunnel Update URL";
management.jffs_legend="JFFS2 Support";
management.jffs_srv="Enable Flash Storage";
management.jffs_clean="Wipe Flash Storage";
management.lang_legend="Language Selection";
management.lang_srv="Language";
management.lang_bulgarian="Bulgarian";
management.lang_chinese_traditional="Chinese traditional";
management.lang_chinese_simplified="Chinese simplified";
management.lang_catalan="Catalan";
management.lang_croatian="Croatian";
management.lang_czech="Czech";
management.lang_dutch="Dutch";
management.lang_portuguese_braz="Portuguese (Brazilian)";
management.lang_english="English";
management.lang_polish="Polish";
management.lang_french="French";
management.lang_german="German";
management.lang_turkish="Turkish";
management.lang_italian="Italian";
management.lang_brazilian="Brazilian";
management.lang_russian="Russian";
management.lang_romanian="Romanian";
management.lang_slovenian="Slovenian";
management.lang_spanish="Spanish";
management.lang_serbian="Serbian";
management.lang_swedish="Swedish";
management.lang_japanese="Japanese";
management.lang_hungarian="Hungarian";
management.lang_korean="Korean";
management.lang_latvian="Latvian";
management.net_legend="Network Stack Tuning";
management.net_conctrl="TCP Congestion Control";
management.net_ipcontrkmax="Maximum Connections";
management.net_tcptimeout="TCP Timeout";
management.net_udptimeout="UDP Timeout";
management.clock_legend="Overclocking";
management.clock_frq="Frequency";
management.clock_support="Not Supported";
management.mmc_legend="MMC / SD Card Support";
management.mmc_srv="Enable Device";
management.mmc_gpiosel="GPIO Pins Select";
management.mmc_gpiopins="GPIO Pins";
management.mmc_cardinfo="Card Info";
management.samba_legend="Common Internet File System (CIFS)";
management.samba_srv="CIFS Automount";
management.samba_share="Share";
management.samba_stscript="Start Script";
management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP Port";
management.SIPatH_domain="SIP Domain";
management.gui_style="Web UI Styles";
management.inspired_themes="DD-WRT Inspired Themes";
management.bootconfig="Boot Configuration (Grub)";
management.disable_msi="Disable MSI Interrupt handling.";
management.pci_tuning="PCI / PCI-E Bus Tuning";
management.pcie_aer="PCI-E Advanced Error Reporting";
management.pcie_ari="PCI-E Alternative Routing Interpretation";
management.pci_noacpi="Disable ACPI IRQ Routing";
management.bus_tune_off="Disable Bus Tuning";
management.bus_safe="Safe Bus Tuning";
management.bus_perf="Performance Bus Tuning";
management.bus_peer2peer="Peer2Peer Bus Tuning";
management.nospectre_v1="Spectre V1 Mitigation";
management.nospectre_v2="Spectre V2 Mitigation";
management.mds="MDS Mitigation";
management.srbds="SRBDS Mitigation";
management.l1tf="L1TF Mitigation";
management.nospec_store_bypass_disable="Speculative Store Bypass Mitigation";
management.tsx_async_abort="TSX Async Abort Mitigation";
management.nopti="Page Table Isolation";
management.pstate="Enforce AMD P-State Frequency Driver";
management.bootfail_handling="Bootfail Handling";
management.bootfail="Reset after 5 Bootfails";
management.boot_fail_open="Open WiFi after Bootfail";
management.boot_fail_keepip="Keep IP after Bootfail";

//help container
var hmanagement=new Object();
hmanagement.right1=management.web_refresh + ":";
hmanagement.right2="Adjust the Web interface refresh interval. Enter a value of <b>0</b> to disable this feature.<br/>The default value is <b>3</b> seconds.";
hmanagement.right3=management.jffs_legend + ":";
hmanagement.right4="When you first <b>" + management.jffs_srv + "</b>, it is necessary to enable <b>" + management.jffs_clean + "</b> in order to prepare the flash file system for usage.";
hmanagement.right5=management.net_legend + ":";
hmanagement.right6="Advanced users can use the sysctl tab to further tune the network stack beyond the limited set of options available here. Any settings available on sysctl should be handled with caution, ensure you have a current backup before proceeding in case changes have undesired results.";

//help page
hmanagement.page1="<dd>On this page the router's secure and remote access can be configured as well as other base functions.</dd>";
hmanagement.page2="<dd>The new password must not exceed 63 characters in length or include any spaces. Enter the new password a second time to confirm it.<br /><br /><div class=\"note\"><h4>Note:</h4><div>The default SSH/Telnet username is <tt>root</tt><br />It is strongly recommended that you change the factory default username and password of the router, which is <tt>admin</tt>. At each new session the router's Web interface or the <i>Setup Wizard</i> is accessed, you will be prompted to enter the router's credentials you have previously setup.</div></div></dd>";
hmanagement.page3="<dd>This feature allows you to manage the router from a remote location via the Internet. To disable this feature, keep the default setting, <em>Disable</em>. To enable this feature, select <em>Enable</em>, and use the specified port (default is 8080).<br /><br />To remotely manage the router, enter <tt>http:\/\/xxx.xxx.xxx.xxx:8080</tt> (the x's represent the router's Internet IP address, and :8080 represents the specified port) in your Web browser's address bar. You will be asked to enter the router's password.<br /><br />For HTTPS you need to specify the URL as <tt>https:\/\/xxx.xxx.xxx.xxx:8080</tt> (not supported in all DD-WRT firmware versions without rebuilding with SSL support).<br /><br />You can also enable <em>SSH</em>&nbsp; to remotely access the router via a Secure Shell. The SSH daemon needs to be enabled in <a href=\"HServices.asp\"></a> page.";
hmanagement.page4="<br /><br /><div class=\"note\"><h4>Note:</h4><div>If the router remote access feature is enabled, anyone who knows the router's Internet IP address and password will be able to alter the router's settings.</div></div></dd>";
hmanagement.page5="<dd>This feature allows you to manage the router using either the HTTP or the HTTPS protocol. If you choose to disable this feature, a reboot will be required.<br />The router's information Web page can also be activated. It is now possible to password protect this page using the login credentials currently setup.<br />The Info Site MAC Masking option truncates the first four octets of the MAC addresses on the Info page, this is helpful in case the page is not password protected.<br /><br /><div class=\"note\"><h4>Note:</h4><div>When Info Site MAC Masking is enabled, all the MAC addresses will be shown in the following format: xx:xx:xx:xx:AA:BB. Info Site MAC Masking only applies to the Sys Info page.</div></div></dd>";
hmanagement.page6="<dd>Boot Wait introduces a short delay while booting (5 seconds). During this delay you can initiate a TFTP upload of a new firmware if the current flash ROM contents are damaged. This is only necessary if you can no longer reflash using the Web interface when the installed firmware e.g. will not boot. Form more information see the relevant DD-WRT documentation for your router.<br /><br /><div class=\"note\"><h4>Note:</h4><div>It is recommended that you enable the Boot Wait feature. This will help you recover in the future should you flash your router improperly.</div></div></dd>";
hmanagement.page7="<dd>The cron subsystem schedules execution of Linux commands. You can use the command line or a startup script to extend this functionality.<br /><br /><div class=\"note\"><h4>Note:</h4><div>NTP should be enabled and working, cron and certain system services rely on accurate time to function normally.</div></div></dd>";
hmanagement.page8="<dd>Enable / disable the loopback interface. The loopback interface makes your internal clients appear as if they are external. This is useful for testing things like DynDNS names. The loopback is an option because enabling it will break PPTP and Windows machine browsing by wireless clients.</dd>";
hmanagement.page9="<dd>This feature controls the resetbuttond process. The reset button initiates actions depending on how long you press it.<ul><li>Short press – Reset the router (reboot)</li><li>Long press (&gt;5s) – Reboot and restore the factory default configuration.</li></ul></dd>";
hmanagement.page10="<dd>The TCP Congestion Control algorithm availability is router dependant and provided by the kernel. When the router is the endpoint, and if enabled the Transmission Bitorrent client, Webserver (lighttpd) or VPN connections e.g. PPPoE etc..., can be affected by the selected congestion control algorithm.<br /><br /><div class=\"note\"><h4>Note:</h4><div>The selected congestion control algorithm will benefit mostly slow links or slower low end routers to prevent network congestion occuring in case of traffic overloading and it will <b>not</b> increase link speed.</div></div><br />For peer-to-peer (P2P) applications running on the network/router, dependant on your router hardware capabilities, either increase or decrease the maximum connections number and lower the TCP / UDP timeout. This may help maintain router stability since peer-to-peer applications open many connections and do not close them properly. Consider using these for old low end routers:<ul><li>Maximum Connections: 4096/32768</li><li>TCP Timeout: 300&nbsp;seconds</li><li>UDP Timeout: 60&nbsp;seconds</li></ul></dd><dd>Check all values and click the <em>" + sbutton.save +"</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.reboot + "</em> button to reboot your router immediately.</dd>";

// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymore) *****************************************//
var portserv=new Object();
portserv.titl="Port Services";
portserv.h2="Port Services";

// ** Networking.asp **//
var networking=new Object();
networking.h2="VLAN Tagging";
networking.legend="Tagging";
networking.h22="Bridging";
networking.h2h="Generic Networking and VLAN";
networking.legend2="Create a Bridge";
networking.legend3="Assign to Bridge";
networking.legend4="Current Bridging Table";
networking.brname="Bridge Name";
networking.stp="STP enabled";
networking.iface="Interfaces";
networking.h5="DHCPD";
networking.legend5="Multiple DHCP Servers";
networking.vlan="VLAN";
networking.iface="Interface";
networking.tg_number="Tag Number";
networking.prio="Prio";
networking.pathcost="Path Cost";
networking.hairpin="Hairpin Mode";
networking.bridge="Bridge";
networking.forward_delay="Forward Delay";
networking.max_age="Max Age";
networking.snooping="IGMP Snooping";
networking.unicast="Multicast to Unicast";
networking.assign="Assignment";
networking.bridgeassign="Bridge Assignment";
networking.bonding="Bonding";
networking.bonding_legend="Bonding Configuration";
networking.bondtype="Type";
networking.bondifaces="Interfaces";
networking.bond="Bond";
networking.slave="Slave";
networking.stp="STP";
networking.max="Max";
networking.leasetime="Lease time";
networking.ipvs="IP Virtual Server";
networking.create_ipvs="Create Virtual Server";
networking.ipvs_name="Server Name";
networking.ipvs_sourceip="Source IP";
networking.ipvs_sourceport="Source Port";
networking.ipvs_scheduler="Scheduler";
networking.wrr="Weighted Round Robin";
networking.lc="Least-Connection";
networking.wlc="Weighted Least-Connection";
networking.fo="Weighted Failover";
networking.ovf="Weighted Overflow";
networking.lblc="Locality Least-Connection";
networking.lblcr="Locality Least-Connection / Replication";
networking.dh="Destination Hash";
networking.sh="Source Hash";
networking.sed="Shortest Expected Delay";
networking.mh="Maglev Hashing";
networking.nq="Never Queue";
networking.ipvs_targets="Virtual Server Targets";
networking.ipvs_targetip="Target IP";
networking.ipvs_targetport="Target Port";
networking.ipvs_weight="Weight";
networking.ipvs_role="Role";
networking.ipvs_config="Configuration";
//help container
var hnetworking=new Object();
hnetworking.right1=networking.legend5;
hnetworking.right2="To use multiple DHCP servers, first enable dnsmasq as a DHCP server.";

//help page
hnetworking.page1="<dd>Allows you to transfer different independent network streams by using just one interface. This is done by inserting a small TAG within the Ethernet header. By identifying this tag these Ethernet packets can be split up again on the other side to create new interface out of it. Using this option together with the bridging option allows you to create different transfer networks which can be bridged to a Wireless Interface to separate the Router Management network from the network accessible by the User. This is useful for large ISP networks.</dd>";
hnetworking.page2="<dd>Allows you to create a new VLAN interface out of a standard interface by filtering the interface using a defined TAG number.</dd>";
hnetworking.page3="<dd>Creates a new empty network bridge for later use. STP means Spanning Tree Protocol and with PRIO you are able to set the bridge priority order. The lowest number has the highest priority.</dd>";
hnetworking.page4="<dd>Allows you to assign any valid interface to a network bridge. Consider setting the Wireless Interface options to Bridged if you want to assign any Wireless Interface here. Any system specific bridge setting can be overridden here in this field. </dd><dd>Click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** QoS.asp **//
var qos=new Object();
qos.titl="Quality of Service";
qos.h2="Quality Of Service (QoS)";
qos.legend="QoS Settings";
qos.srv="Start QoS";
qos.type="Packet Scheduler";
qos.aqd="Queuing Discipline";
qos.aqd_sfq="SFQ";
qos.aqd_codel="CODEL";
qos.aqd_fqcodel="FQ_CODEL";
qos.aqd_fqcodel_fast="FQ_CODEL_FAST";
qos.aqd_cake="CAKE";
qos.aqd_pie="PIE";
qos.uplink="Uplink";
qos.dnlink="Downlink";
qos.legend2="Services Priority";
qos.prio_m="Manual";
qos.prio_exempt="Exempt";
qos.prio_x="Maximum";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_b="Bulk";
qos.legend3="Netmask Priority";
qos.ipmask="IP / Mask";
qos.maxrate_b="Max kbit/s";
qos.maxuprate_b="WAN Max Up";
qos.maxdownrate_b="WAN Max Down";
qos.maxlanrate_b="LAN Max";
qos.maxrate_o="Max Rate";
qos.legend4="MAC Priority";
qos.legend5="Ethernet Port Priority";
qos.legend6="Default Bandwidth Level";
qos.legend7="TCP-Packet Priority";
qos.legend8="Interface Priority";
qos.pktdesc="Prioritize small TCP-packets with the following flags:";
qos.pktack="ACK";
qos.pktrst="RST";
qos.pktsyn="SYN";
qos.pktfin="FIN";
qos.pkticmp="ICMP";
qos.enabledefaultlvls="Enable Per User Default Limits";
qos.bandwidth="Bandwidth";
qos.speed="kbit/s";
qos.up="Up";
qos.down="Down";
qos.service="Service";

//help container
var hqos=new Object();
hqos.right1="";
hqos.right2="Uplink:<br />Set this to 85% - 95% (max) of your total upload limit.<br />Downlink:<br />Set this to 85% - 100% (max) of your total download limit.";
hqos.right3="";
hqos.right4="Enable Per User Default Limits:<br />Enable the default level per user or set the level for all users.";
hqos.right6="You may control your data rate with respect to the application that is consuming bandwidth.";
hqos.right8="You may specify priority for all traffic from a given IP address or IP range.";
hqos.right10="You may specify priority for all traffic from a device on your network by assigning it a name, specifying priority and entering the device MAC address.";
hqos.right12="You may control your data rate according to which physical LAN port your device is plugged into. You may assign Priorities accordingly for devices connected on LAN ports 1 through 4.";

//help page
hqos.page1="<dd>Bandwidth management prioritizes the traffic on your router. Interactive traffic (telephony, browsing, telnet, etc.) gets priority and bulk traffic (file transfer, P2P) gets low priority. The main goal is to allow both types to live side-by side without unimportant traffic disturbing more critical things. All of this is more or less automatic.<br /><br />QoS allows control of the bandwidth allocation to different services, netmasks, MAC addresses and the four LAN ports, LAN port availability will vary by router, if it is not displayed, your hardware does not support it. QoS is divided into five bandwidth classes called Maximum, Premium, Express, Standard, and Bulk. Unclassified services will use the Standard bandwidth class.</dd>";
hqos.page2="<dd>You must choose whether to apply QoS to the WAN, or the LAN &amp; WLAN port. (LAN and WLAN ports are bonded internally into a single virtual device). Most should select WAN for this option.</dd>";
hqos.page3="<dd><ul class=\"wide\"><li>HFSC - Hierarchical Fair Service Curve. Queues attached to an interface build a tree, thus each queue can have further child queues. Each queue can have a priority and a bandwidth assigned. Priority controls the time packets take to get sent out, while bandwidth effects throughput. HTB is a little more resource demanding than that of HFSC. </li><li>HTB - Hierarchical Token Bucket, it is a faster replacement for the CBQ qdisc in Linux but is more resource demanding than HFSC. HTB helps in controlling the use of the outbound bandwidth on a given link. HTB allows you to use one physical link to simulate several slower links and to send different kinds of traffic on different simulated links. HTB is useful for limiting a client's download/upload rates, preventing their monopolization of the available bandwidth.</li></ul></dd>";
hqos.page4="<dd>In order to use QoS you must enter bandwidth values for your uplink and downlink. These are generally 85% to 95% of your maximum bandwidth. If you only want QoS to apply to uplink bandwidth, enter 0 (no limit) for downlink. Do not enter 0 for uplink. </dd>";
hqos.page5="<dd>Bandwidth classification based on the four categories will be enabled first on the hardware ports, then on MAC addresses, then netmasks and finally services. For example, if you enable classification based on a MAC address, this will override netmask and service classifications. However, the LAN port based classification will work together with MAC, netmask and service classifications, and will not override them.<ul class=\"wide\"><li>Maximum - (75% - 100%) This class offers maximum priority and should be used sparingly.</li><li>Premium - (50% - 100%) Second highest bandwidth class, by default handshaking and ICMP packets fall into this class. Most VoIP and video services will function good in this class if Express is insufficient.</li><li>Express - (25% - 100%) The Express class is for interactive applications that require bandwidth above standard services so that interactive apps run smoothly.</li><li>Standard - (15% - 100%) All services that are not specifically classed will fall under standard class.</li><li>Bulk - (5% - 100%) The bulk class is only allocated remaining bandwidth when the remaining classes are idle. If the line is full of traffic from other classes, Bulk will only be allocated 1% of total set limit. Use this class for P2P and downloading services like FTP.</li></ul></dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings without taking effect, or click the <em>" + sbutton.apply + "</em> button to permanently save your changes taking effect immediately. Clicking the <em>" + sbutton.cancel + "</em> button will cancel your unsaved changes.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Remember to save your changes before adding another QoS rule.</div></div></dd>";

// ** RouteTable.asp **//
var routetbl=new Object();
routetbl.titl="Routing Table";
routetbl.h2="Routing Table Entry List";
routetbl.h3="Policy Rules Entry List";
routetbl.th1="Destination LAN NET";
routetbl.masquerade="Masquerade Route (NAT)";
routetbl.scope="Scope";
routetbl.advmss="Advertise MSS";
routetbl.table="Table";
routetbl.src="Source";
routetbl.not="Not";
routetbl.from="From";
routetbl.to="To";
routetbl.priority="Priority";
routetbl.tos="ToS / DS Field";
routetbl.fwmark="Fwmark";
routetbl.realms="Realms";
routetbl.table="Table";
routetbl.suppress_prefixlength="Suppress Prefixlength";
routetbl.iif="Incoming IF";
routetbl.oif="Outgoing IF";
routetbl.nat="NAT Address";
routetbl.type="Type";
routetbl.sport="Source Port Range";
routetbl.dport="Destination Port Range";
routetbl.ipproto="IP Protocol";

// ** Routing.asp **//
var route=new Object();
route.rule_name="Policy Name";
route.global="Global";
route.nowhere="Nowhere";
route.host="Host";
route.link="Link";
route.site="Site";
route.unicast="Unicast";
route.blackhole="Blackhole";
route.unreachable="Unreachable";
route.prohibit="Prohibit";
route.nat="NAT";
route.titl="Routing";
route.h2="Advanced Routing";
route.metric="Metric";
route.flags="Flags";
route.mod="Operating Mode";
route.bgp_legend="BGP Settings";
route.bgp_ip="Neighbor IP";
route.bgp_own_as="BGP Own AS#";
route.bgp_as="Neighbor AS#";
route.rip2_mod="RIP2 Router";
route.olsrd_mod="OLSR Router";
route.olsrd_legend="OLSR Routing (Optimized Link State Routing)";
route.olsrd_poll="Poll Rate";
route.olsrd_gateway="Gateway Mode";
route.olsrd_hna="Host Net Announce";
route.olsrd_tc="TC Redundancy";
route.olsrd_mpr="MPR Coverage";
route.olsrd_lqfe="Link Quality Fish Eye";
route.olsrd_lqag="Link Quality Aging";
route.olsrd_lqdmin="Link Quality Dijkstra Min";
route.olsrd_lqdmax="Link Quality Dijkstra Max";
route.olsrd_lqlvl="Link Quality Level";
route.olsrd_hysteresis="Hysteresis";
route.olsrd_newiface="New Interface";
route.olsrd_smartgw="Smart Gateway";
route.zebra_legend="Zebra Configuration";
route.zebra_log="Zebra Log";
route.zebra_copt="Zebra Config Style";
route.bird_legend="Bird Configuration";
route.bird_log="Bird Log";
route.bird_copt="Bird Config Style";
route.ospf_mod="OSPF Router";
route.ospf_legend="OSPF Routing";
route.ospf_conf="OSPF Configuration";
route.ospf_copt="OSPF Config Style";
route.copt_gui="GUI";
route.copt_vtysh="Vtysh";

route.ospf_rip2_mod="OSPF & RIP2-Router";
route.rip2_legend="RIP2 Routing";
route.rip2_conf="RIP2 Configuration";
route.rip2_copt="RIP2 Config Style";

route.gateway_legend="Dynamic Routing";
route.static_legend="Routing Tables";
route.static_setno="Select Route";
route.policy_setno="Select Policy";
route.static_name="Route Name";
route.static_ip="Destination LAN NET";
route.rules="Routing Policies";
route.static_name="Route Name";

//help container
var hroute=new Object();
hroute.right2="If the router is hosting your Internet connection, select <em>Gateway</em> mode. If another router exists on your network, select the <em>Router</em> mode.";
hroute.right4="This is the unique router number; you may set up to 50 routes.";
hroute.right6="Enter the name you would like to assign to this route.";
hroute.right8="This is the remote host to which you would like to assign the static route.";
hroute.right10="Determines the host and the network portion.";

//help page
hroute.page1="<dd>On the Routing page, you can set the routing mode and settings of the router. <i>Gateway</i> mode is recommended for most users.</dd>";
hroute.page2="<dd>Choose the correct working mode. Keep the default setting, Gateway, if the router is hosting your network's connection to the Internet. Select router if the router exists on a network with other routers. In Gateway mode the router performs NAT, while in other modes it does not.</dd>";
hroute.page3="<dd>Dynamic Routing enables the router to automatically adjust to physical changes in the network's layout and exchange routing tables with other routers. The router determines the network packets&#8217; route based on the fewest number of hops between the source and destination.<br /><br />To enable the Dynamic Routing feature for the WAN side, select <i>WAN</i>. To enable this feature for the LAN and wireless side, select <i>LAN &amp; WLAN</i>. To enable the feature for both the WAN and LAN, select <i>Both</i>. To disable the Dynamic Routing feature for all data transmissions, keep the default setting, <em>" + share.disable + "</em>.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Dynamic Routing is not available in Gateway mode.</div></div></dd>";
hroute.page4="<dd>A static route is a pre-determined pathway that network information must travel to reach a specific host or network.<br /><br />To set up a static route between the router and another network:<ol class=\"wide\"><li>Select a number from the Static Routing drop-down list.</li><li>Enter the following data:<ul><li>Destination IP Address – The Destination IP Address is the address of the network or host to which you want to assign a static route.</li><li>Subnet Mask – The Subnet Mask determines which portion of an IP address is the network portion, and which portion is the host portion.</li><li>Gateway – This is the IP address of the gateway device that allows for contact between the router and the network or host.</li></ul></li><li>Depending on where the Destination IP Address is located, select <i>LAN &amp; WLAN</i> or <i>WAN</i> from the Interface drop-down menu. </li><li>Click the <em>" + sbutton.apply + "</em> button to save your changes. To cancel your unsaved changes, click the <em>" + sbutton.cancel + "</em> button. For additional static routes, repeat steps 1-4.</li></ol><br />To delete a static route entry:<ol class=\"wide\"><li>From the Static Routing drop-down list, select the entry number of the static route.</li><li>Click the <i>Delete This Entry</i> button.</li><li>To save a deletion, click the <em>" + sbutton.apply + "</em> button. To cancel a deletion, click the <em>" + sbutton.cancel + "</em> button.</li></ol></dd>";
hroute.page5="<dd>Click the <i>Show Routing Table</i> button to view all of the valid route entries in use. The following data will be displayed for each entry.<ul class=\"wide\"><li>Destination IP Address – The Destination IP Address is the address of the network or host to which the static route is assigned.</li><li>Subnet Mask – The Subnet Mask determines which portion of an IP address is the network portion, and which portion is the host portion.</li><li>Gateway – This is the IP address of the gateway device that allows for contact between the router and the network or host.</li><li>Interface – This interface tells you whether the Destination IP Address is on the LAN &amp; WLAN (internal wired and wireless networks), the WAN (Internet), or Loopback (a dummy network in which one device acts like a network, necessary for certain software programs).</li></ul><br />Click the <i>Refresh</i> button to refresh the data displayed. Click the <em>" + sbutton.clos + "</em> button to return to the Routing screen.</dd>";

// ** Site_Survey.asp **//
var survey=new Object();
survey.titl="Site Survey";
survey.titl2="Channel Survey";
survey.h2="Neighbor's Wireless Networks";
survey.h3="Channel Survey and Qualities";
survey.thjoin="Join Site";

// ** Services.asp **//
var service=new Object();
service.titl="Services";
service.h2="Services Management";

service.apserv_legend="APServ Remote Configuration";
service.apserv="APServ";

//kaid
//service.kaid_legend="Xbox Kaid";
//service.kaid_srv="Start Kaid";
//service.kaid_locdevnum="Number of Local Devices";
//service.kaid_uibind="UI Listening Port";
//service.kaid_orbport="ORB Port";
//service.kaid_orbdeepport="ORB Deep Port";

//DHCPd
service.dhcp_legend="DHCP Client";
service.dhcp_vendor="DHCP Vendor Class";
service.dhcp6c_vendor="DHCP IPv6 Vendor Class";
service.dhcp_reqip="Request IP";
service.dhcp_legend2="DHCP Server Setup";
service.dhcp_srv="DHCP Daemon";
service.dhcp_jffs2="Use JFFS2 for Client Lease DB";
service.dhcp_nvramlease="Use NVRAM for Client Lease DB";
service.dhcp_domain="Used Domain";
service.dhcp_landomain="LAN Domain";
service.dhcp_option="Additional Options";
service.dnsmasq_legend="Dnsmasq Infrastructure";
service.dnsmasq_srv="Enable dnsmasq";
service.dns_crypt="Encrypt DNS";
service.dns_smartdns="SmartDNS Resolver";
service.dns_smartdns_label="Enable Resolver";
service.dns_smartdns_dualstack="Dualstack IP Selection";
service.dns_smartdns_prefetch_domain="Prefetch Domain";
service.dns_smartdns_serve_expired="Serve Expired";
service.dns_smartdns_use_dns="Use Additional Servers Only";
service.dns_smartdns_option="Additional Options";
service.dns_crypt_resolv="DNSCrypt Resolver";
service.dnsmasq_dnssec="Validate DNS Replies (DNSSEC)";
service.dnsmasq_dnssec_proxy="Cache DNSSEC Data";
service.dnsmasq_dnssec_cu="Check Unsigned DNS Replies";
service.dnsmasq_no_dns_rebind="No DNS Rebind";
service.dnsmasq_strict="Query DNS in Strict Order";
service.dnsmasq_add_mac="Add Requestor MAC to DNS Query";
service.dnsmasq_opt="Additional Options";
service.dnsmasq_rc="RFC4039 Rapid Commit Support";
service.dnsmasq_cachesize="Maximum Cached Entries";
service.dnsmasq_forward_max="Maximum Concurrent Requests";
service.tor_legend="The Onion Router Project";
service.tor_srv="Enable Tor";
service.tor_address="DNS Name or External IP";
service.tor_nickname="Nickname / ID";
service.tor_relay="Relay Mode";
service.tor_dir="Directory Mirror";
service.tor_bridge="Tor Bridge Mode";
service.tor_transparent="Transparent Proxy";
service.tor_bwrate="Bandwidth Rate";
service.tor_bwburst="Bandwidth Burst";

//syslog.webservices
service.syslog_legend="System Log";
service.syslog_srv="Syslogd";
service.klog="Klogd";
service.syslog_ip="Remote Server";
service.syslog_jffs2="Store messages on jffs2";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv=nas.proftpd_srv_label;

service.mactelnetd_legend="Mikrotik MAC Telnet";
service.mactelnetd="MAC Telnet";

//pptp.webservices
service.pptp_h2="PPTP Server / Client";
service.pptp_legend="PPTP";
service.pptp_srv="PPTP Server";
service.pptp_option_srv=nas.proftpd_srv_label;
service.pptp_client="Client IP(s)";
service.pptp_chap="CHAP-Secrets";

//pptpd_client.webservices
service.pptpd_legend="PPTP Client";
service.pptpd_lblcli=idx.ntp_client;
service.pptpd_ipdns="Server IP or DNS Name";
service.pptpd_subnet="Remote Subnet";
service.pptpd_subnetmask="Remote Subnet Mask";
service.pptpd_encry="MPPE Encryption";
service.pptpd_mtu="MTU";
service.pptpd_mru="MRU";
service.pptpd_nat="NAT";
service.dns1="DNS1";
service.dns2="DNS2";
service.wins1="WINS1";
service.wins2="WINS2";

//rflow.webservices
service.rflow_legend="RFlow / MACupd";
service.rflow_srv1="RFlow";
service.rflow_srv2="MACupd";

//pppoe-relay.webservices
service.pppoe_legend="PPPoE Relay";
service.pppoe_srv="Enable Relay";

//pppoe-server.webservices
service.pppoesrv_legend="RP-PPPoE Server";
service.pppoesrv_srv=nas.proftpd_srv_label;
service.pppoesrv_interface="Server Interface";
service.pppoesrv_srvopt="RP-PPPoE Server Options";
service.pppoesrv_compr="Compression";
service.pppoesrv_lcpei="LCP Echo Interval";
service.pppoesrv_lcpef="LCP Echo Failure";
service.pppoesrv_limit="Session Limit per MAC";
service.pppoesrv_idlet="Client Idle Time";
service.pppoesrv_auth="Authentication";
service.pppoesrv_radauth="RADIUS Authentication";
service.pppoesrv_radip="Server IP";
service.pppoesrv_radauthport="Authentication Port";
service.pppoesrv_radaccport="Accounting Port";
service.pppoesrv_radkey="Shared Key";
service.pppoesrv_chaps="Local User Management (CHAP-Secrets)";

// portsetup.c line 253
service.wired_8021x_server="Enable Wired 802.1X Server";

//help container
var hpppoesrv=new Object();
hpppoesrv.right2="IP: 0.0.0.0 -> You will serve IP addresses from the pool";
hpppoesrv.right3="You <em><b>must</b></em> set the correct amount of associated clients according to your IP range";

//help page
hpppoesrv.page1="<dd>This is a PPP over Ethernet redirector for PPPD.<br /><b>rp-pppoe</b> is a user-space redirector which permits the use of PPPoE (Point-to-Point Protocol over Ethernet) with Linux.<br />PPPoE is used by many DSL service providers.<br /><div class=\"note\"><h4>Note:</h4><div>Notes to be added</div></div></dd>";
hpppoesrv.page2="<dd>Click the <em>" + sbutton.save + "</em> button to save your settings or click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

//snmp.webservices
service.snmp_legend="Simple Network Management Protocol (SNMP)";
service.snmp_srv="Enable SNMP";
service.snmp_loc="Location";
service.snmp_contact="Contact";
service.snmp_name="Name";
service.snmp_read="RO Community";
service.snmp_write="RW Community";

//openvpn.webvpn
service.vpnd_hlegend1="OpenVPN Server";
service.vpnd_legend="OpenVPN Server / Client";
service.vpnd_srv=nas.proftpd_srv_label;
service.vpnd_starttype="Start Type";
service.vpnd_startWanup="WAN Up";
service.vpnd_startSystem="System";
service.vpnd_gui="GUI (Server)";
service.vpnd_text="Text";
service.vpnd_crl="Certificate Revocation List";
service.vpnd_config="Additional Configuration";
service.vpnd_dhpem="DH PEM";
service.vpnd_tlsauth="TLS Key";
service.vpnd_cert="Public Server Certificate";
service.vpnd_key="Private Server Key";
service.vpnd_pkcs="PKCS12 Key";
service.vpnd_mode="Server Mode";
service.vpnd_netm="IPv4 Network and Mask";
service.vpnd_netmv6="IPv6 Network/Mask";
service.vpnd_startip="Pool start IP";
service.vpnd_endip="Pool end IP";
service.vpnd_cl2cl="Client to Client Connection Allowed";
service.vpnd_switch="Config as";
service.vpnd_dupcn="Allow Duplicate Clients";
service.vpnd_allowcnwan="Allow Clients WAN Access";
service.vpnd_allowcnlan="Bypass LAN Same-Origin Policy";
service.vpnd_proxy="DHCP-Proxy mode";
service.vpnd_clcon="Client Connect Script";
service.vpnd_cldiscon="Client Disconnect Script";
service.vpnd_ccddef="CCD-Dir DEFAULT File";
service.vpnd_dhcpbl="Block DHCP Across the Tunnel";
service.vpnd_blockmulticast="Block Multicast Across the Tunnel";
service.vpnd_dh="Use ECDH Instead of DH.PEM";
service.vpnd_static="Static Key";
service.vpnd_export="Export Client Configuration";
service.vpn_redirgate="Push Client Route";
service.vpn_defgateway="Default Gateway";
service.vpn_srvroute="Servers Subnet";
service.vpn_legend="OpenVPN Client";
service.vpn_mit="CVE-2019-14899 Mitigation";
service.vpn_srv=idx.ntp_client;
service.vpn_mtu="Tunnel MTU Setting";
service.vpn_mss="Tunnel UDP MSS Fix";
service.vpn_fragment="Tunnel UDP Fragment";
service.vpn_compress="Compression";
service.vpn_cl2cl="Allow Client to Client";
service.vpn_tunnel="Tunnel Protocol";
service.vpn_tuntap="Tunnel Device";
service.vpn_srvcert="CA Certificate";
service.vpn_clicert="Public Client Certificate";
service.vpn_certtype="Verify Server Certificate";
service.vpn_clikey="Private Client Key";
service.vpn_nat="NAT";
service.vpn_cipher="Encryption Cipher";
service.vpn_auth="Hash Algorithm";
service.vpn_bridge="Bridge TAP to br0";
service.vpn_adv="Advanced Options";
service.vpn_tlscip="TLS Cipher";
service.vpn_route="Policy based Routing";
service.vpn_scramble="XOR scrambling method";
service.vpn_upauth="User Pass Authentication";
service.vpn_wdog="Watchdog";
service.vpn_fw="Inbound Firewall on TUN";
service.vpnd_server="Server";
service.vpnd_deamon="Daemon";
service.vpnd_lzoyes="Yes";
service.vpnd_lzono="No";
service.vpnd_lzoadaptive="Adaptive";
service.vpnd_lzocompress="Compress";
service.vpn_tls_btn="TLS / Static Key Choice";
service.vpn_tls_crypt="TLS Crypt";
service.vpn_tls_auth="TLS Auth";
service.vpn_dc1="First Data Cipher";
service.vpn_dc2="Second Data Cipher";
service.vpn_dc3="Third Data Cipher";
service.vpn_killswitch="Killswitch";
service.vpn_splitdns="Split DNS";
service.vpn_ipnameport="Server IP / Name : Port";
service.vpn_multiaddress="Set Multiple Servers";
service.vpn_randomsrv="Choose random Server";

// SoftEther
service.softether_h2="SoftEther VPN Server / Client";
service.softether_legend="SoftEther VPN";
service.softether_bridge="Enable Bridge";
service.softether_client=idx.ntp_client;
service.softether_server=nas.proftpd_srv_label;
service.softether_config="Configuration";

//sshd.webservices
service.ssh_legend="Secure Shell (SSH)";
service.ssh_srv="Enable Daemon";
service.ssh_password="Password Login";
service.ssh_key="Authorized Keys";
service.ssh_forwarding="SSH TCP Forwarding";
service.ssh_keylegend="Key Handling";
service.ssh_keygenerate="Generate Key";
service.ssh_keylength="Key Length";
service.ssh_replace="Replace Existing Key(s)";
service.ssh_download="Download Private Key";
service.ssh_keyalert="Be patient, key generation can take up to 10 minutes or more.\nAfter succesful generation the " + service.ssh_download + " button will become available to allow you to download the private key file.";
service.ssh_keydownload="Click the " + sbutton.apply + " button after downloading to restart SSHD!\nThe private key is downloaded in the OpenSSH format.\nIf you wish to use PuTTY, the key must first be converted to a compatible format with PuTTYgen (see help more...).";

//help page
var hservice=new Object();
hservice.right1="Before enabling storage for Client Lease DB into JFFS2 flash storage, JFFS2 support <b>must be enabled</b>. JFFS2 Support is located on the Administration / Management tab.";
hservice.right2="The servers you define within the <b><em>" + service.dns_smartdns_option + "</em></b> below will be used exclusively.";
hservice.sshshelp="To generate a key, choose the desired keysize, then click the <em>" + service.ssh_keygenerate + "</em> button to begin. Click OK and be patient. After key generation has completed you can choose to download the private key in the default OpenSSH format by clicking the <em>" + service.ssh_download + "</em> button. For further information on key usage <span class=\"link\" onclick=\"javascript:openHelpWindow('HServices.asp');\">click here</span>.";

hservice.page1="<dd>The DHCP assigns IP addresses to your local devices. While the main configuration is on the setup page you can program some nifty special functions here.<ul class=\"wide\"><li>Used domain – You can select here which domain the DHCP clients should get as their local domain. This can be the WAN domain set on the Setup screen or the LAN domain which can be set here.</li><li>LAN Domain – You can define here your local LAN domain which is used as local domain for dnsmasq and DHCP service if chosen above.</li><li>Static Leases – If you want to assign certain hosts a specific address then you can define them here. This is also the way to add hosts with a fixed address to the router's local DNS service (dnsmasq).</li></ul><br />There are some extra options you can set by entering them in <em>Additional Options</em>.</dd>";
hservice.page2="<dd>dnsmasq is a local DNS and DHCP server. It will resolve all host names known to the router from DHCP (dynamic and static) as well as forwarding and caching DNS entries from remote DNS servers. dnsmasq will always appear enabled here if either <em>DHCP Server</em> or <em>Use dnsmasq for DNS</em> is enabled on the Basic Setup page.<br />There are some extra options you can set by entering them in <em>Additional Options</em>. For instance:<br /><br /><ul class=\"wide\"><li>Static allocation: dhcp-host=AB:CD:EF:11:22:33,192.168.0.10,myhost,myhost.domain,12h</li><li>Max leases number: dhcp-lease-max=2</li><li>DHCP server IP range: dhcp-range=192.168.0.110,192.168.0.111,12h</li></ul><br /><div class=\"note\"><h4>Note:</h4><div>All options are saved in /tmp/dnsmasq.conf file. The format of this file consists of one option per line.<br />The complete list of available options : <a href=\"http:\/\/thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html\" target=\"_new\">dnsmasq manual</a>.</div></div></dd>";
hservice.dnsmasq_dnssec="<dd>Requests and validates DNSSEC records for domains that provide them, if supported on your router model.</dd>";
hservice.dnsmasq_dnssec_proxy="<dd>Copies the DNSSEC status bit from the upstream server. This option is available on some routers that do not support direct DNSSEC validation, but should only be used when the upstream server is trustworthy. Cache size 0 is recommended when using this option.</dd>";
hservice.dnsmasq_dnssec_cu="<dd>When DNSSEC validation is enabled, also check that unsigned DNS replies are legitimate (they belong to domains that actually do not publish DNSSEC records).</dd>";
hservice.dnsmasq_no_dns_rebind="<dd>Ignore responses in private IP address ranges that are received from upstream (public) DNS servers.</dd>";
hservice.dnsmasq_strict="<dd>Query the upstream servers in the order entered manually or supplied by the WAN connection (i.e. Static DNS 1 first). If disabled, queries can go to any upstream server.</dd>";
hservice.dnsmasq_add_mac="<dd>Adds the internal requestor's MAC address to the query sent to the upstream DNS server. This could be necessary if the upstream server filters requests by MAC.</dd>";
hservice.dnsmasq_rc="<dd>DHCP Rapid Commit removes a round trip of network traffic by immediately returning an address lease in response to a DHCPDISCOVER from a client that also supports Rapid Commit.</dd>";
hservice.dnsmasq_cachesize="<dd>Defines how many cached entries dnsmasq stores (default 1500).</dd>";
// HServices.asp has these page3 through page5 Kaid entries commented out until it returns to DD-WRT
//hservice.page3="<dd>Kai is a means of connecting platform games over the Internet. Enable the service and then add Xbox MAC addresses separated with \";\".<ul class=\"wide\"><li>";
//hservice.page4=" – How many consoles to detect before the engine locks the pcap filter. Setting this to 0, means the engine will never lock - which means you can use any number of consoles, but you will notice a performance hit, if your network is busy with other traffic. The best thing to do here is to set the number to the number of consoles you own - that is why it defaults to 1 - because most people have just 1 console.</li><li>";
//hservice.page5=" – Specifies which IP/port kaid will use to listen for controller UIs.</li></ul><br /><div class=\"note\"><h4>Note:</h4><div>Xbox must be connected directly via one of the Ethernet ports of the router.</div></div></dd>";
hservice.page6="<dd>RFlow Collector is a traffic monitoring and management tool that allows to watch a complete network of DD-WRT routers.<br /><ul class=\"wide\"><li>RFlow Default port is 2055</li><li>MACupd Default port is 2056</li><li>Interval = 10 seems messages will be sent to server each 10 seconds</li><li>Interface : choose which interface to monitor</li></ul><br /><div class=\"note\"><h4>Note:</h4><div>For each RFlow and MACupd server IP : enter the IP address of the listening server (win32 PC with RFlow Collector).</div></div></dd>";
hservice.page7="<dd>Enabling SSHd allows you to access your router's Linux OS with an SSH client (Putty works well on Windows, for example).<ul class=\"wide\"><li>Password login – allow login with the router password (username is <tt>root</tt>)</li><li>SSHd Port – the port number for SSHd (default is 22)</li><li>Authorized Keys – here you paste your public keys to enable key-based login (more secure than a simple password)</li></ul></dd>";
hservice.sshclient="<dd><b>For PuTTY:</b><br />Convert the private key-file with PuttyGen as follows:<ul class=\"wide\"><li>Conversions</li><li>Select <em>Import key</em> from menu then browse for directory where you saved the previusly downloaded id_rsa.ssh key and select it.</li><li>Click <em>Save private key</em> and enter e.g. id_rsa.ppk as a filename, your key is now ready to be used in PuTTY.</li></ul><br /><b>PuTTY key import:</b><ul class=\"wide\"><li>Click SSH to expand options</li><li>Click Auth</li><li>Under Private key file for authentication, click the <em>Browse...</em> button then select the id_rsa.ppk you had previously converted and Save your PuTTY profile.</dd>";
hservice.page8="<dd>Enable Syslogd to capture system messages. By default they will be collected in the local file \/var\/log\/messages. To send them to another system, enter the IP address of a remote syslog server.</dd>";
hservice.page9="<dd>Enable the telnet server to connect to the router with telnet. The username is <tt>root</tt> and the password is the current router's password.<br /><br /><div class=\"note\"><h4>Note:</h4><div>If you are using the router in an untrusted environment e.g. a public hotspot, it is strongly recommended to use SSHd and deactivate telnet.</div></div></dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.reboot + "</em> button to reboot your router immediately.</dd>";

//help container
var hstatus_vpn=new Object();
hstatus_vpn.right1="<b>Policy-based Routing</b>:<br />Add IPs / NETs in the following format 0.0.0.0/0 to force clients to use the tunnel as the default gateway. Enter one IP / NET per line.<br /><b>IP Address / Netmask</b>:<br />Must be set when using DHCP-Proxy mode and local TAP is <b>not</b> bridged</i>";
hstatus_vpn.cfg_xp="Click the <em>" + service.vpnd_export + "</em> button to download your current OpenVPN client settings. DD-WRT cannot generate Client Key / Certificate at this time, you will need to manually edit and insert these in the file. To read documentation on how-to / adjust settings <span class=\"link\" onclick=\"window.open('https:\/\/forum.dd-wrt.com/phpBB2/viewtopic.php&quest;t=327398')\">click here</span>.";
hstatus_vpn.right2="<b>Additional Configuration</b>:<br />To push routes to clients add <i>'push \"route IP mask gateway\"'</i>, to push DNS / WINS add <i>'push \"dhcp-option DNS (or WINS) IP\"'</i> to the config.<br />Client connect directory:<br />When either USB or JFFS2 are mounted to <b>/jffs</b>, scripts will be called from <b>/jffs/etc/openvpn/ccd/</b>";
hstatus_vpn.right3="<b>General</b>:<br />Three auth methods are supported; pkcs12 (+dh on server), static and standard certs. <i>Only enable MSS on one side of the link, fragment on both.</i>";
// common for wireguard and OpenVPN - Bypass LAN Same-Origin Policy
hstatus_vpn.right4="Allows the client to get around personal firewalls on target devices that have a same-origin policy. The inbound traffic is NATed from the VPN as it flows through the local network interface e.g. br0 to make it appear as if it originated from the router's LAN IP, rather than the tunnel's IP network.";

//help page
hstatus_vpn.page1="<dd>A VPN server technology compatible with Microsoft and other remote access vendors, it is implemented in multiple desktop and mobile OSs. Configuring this allows you to access your LAN at home remotely.<ul class=\"wide\"><li>Server IP – The IP address of your router</li><li>Client IP – A list or range of IP addresses for remotely connected devices. This range should not overlap with the DHCP range (e.g. 192.168.0.2,192.168.0.3), a range (192.168.0.1-254 or 192.168.0-255.2) or some combination (192.168.0.2,192.168.0.5-8).</li><li>CHAP-Secrets – A list of usernames and passwords for the VPN login, one user per line (e.g. joe * joespassword *). For more details look up the pppd main page.</li></ul></dd>";
hstatus_vpn.page2="<dd>A VPN client enables you to connect to VPN servers compatible with Microsoft and other remote access vendors. Configuring this allows the router to VPN into a remote network.<ul class=\"wide\"><li>Server IP or DNS Name – The IP address or DNS Name of the VPN server that you would like to connect to (e.g. www.MyServer.com). </li><li>Remote Subnet – Remote Subnet of the network you are connecting to (e.g. 192.168.2.0). </li><li>Remote Subnet Mask – Remote Subnet Mask of the network you are connecting to (e.g 255.255.255.0). </li><li>MPPE Encryption  – The type of security to use for the connection. If you are connecting to another DD-WRT router you need (e.g. mppe required). But if you are connecting to a Windows VPN server you need (e.g. mppe required,no40,no56,stateless) or (e.g. mppe required,no40,no56,stateful) </li><li>MTU – Maximum Transmission Unit (Default: 1436) </li><li>MRU – Maximum Receiving Unit (Default: 1436) </li><li>NAT – Enabling this option will make outbound traffic from inside appear to be coming from router IP, instead of client IP. Enabling this can improve security, but can cause issues in some cases, i.e. when VoIP is used. </li><li>User Name – Enter the username that you will use to connect to the VPN server. If you are connecting to another Linux based PPTP server you just need to enter the username. But if you are connecting to a Windows VPN server you need to enter the servername and username (e.g. DOMAIN\\username). </li><li>Password – Enter the password for the username </li><li>Additional Options – If default options are not working for your setup, you can use this field. If defined, these will replace the default internal options. The options above are still used. </li></ul></dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

//vnc.repeater
service.vncrepeater_legend="Virtual Network Computing (VNC)";
service.vncrepeater="Enable Repeater";

//radiooff.webservices
service.radiooff_legend="SES / AOSS / EZ-SETUP / WPS Button";
service.radiooff_legend_aoss="AOSS Button Function";
service.radiooff_srv="Turn Off Radio";
service.radiooff_srv_aoss="AOSS";
service.radiooff_srv_disabled="Unused";
service.radiooff_bootoff="Turn Radio Off at Boot";

//ses.webservices ====> might replace the above radiooff_button
service.ses_legend="SES / AOSS / EZ-SETUP / WPS Button";
service.ses_srv="Button Action";
service.ses_toggleradio="Toggle Wireless";
service.ses_script="Custom Script";

//hwmon.webservices
service.hwmon_legend="Hardware Monitoring";
service.hwmon_critemp="High Temperature";
service.hwmon_hystemp="Normal Temperature";
service.hwmon_fanon="&nbsp;Fan On";
service.hwmon_fanoff="&nbsp;Fan Off";

//rstat.webservices
service.rstats_legend="Bandwidth Monitoring";
service.rstats_srv="rstats Daemon";
service.rstats_path="Save Bandwidth Data To";
service.rstats_time="Saving Interval";
service.rstats_usrdir="User Directory";

//nstx.webservices
service.nstx_legend="IP over DNS Tunneling";
service.nstx_srv="NSTX Daemon";
service.nstx_ipenable="Bind to this IP Only";
service.nstx_log="Switch on Debug Messages";

//ttraff.webservices
service.ttraff_legend="WAN Traffic Counter";
service.ttraff_daemon="ttraff Daemon";

//notifier.webservices
service.warn_legend="Connection Warning Notifier";
service.warn="Enable Notifier";
service.warn_limit="Connection Limit";
service.warn_server="Email SMTP Server";
service.warn_from="Senders Email Address";
service.warn_fromfull="Senders Full Name";
service.warn_to="Recipient Email Address";
service.warn_domain="Recipient Domain Name";
service.warn_user="SMTP Auth Username";
service.warn_pass="SMTP Auth Password";

//service.chronyd
service.chronyd_legend="Chronyd - NTP Server";
service.chronyd_srv="Chronyd";
service.chronyd_conf="Custom config";

// service samba
service.samba3_srv="Samba";
service.samba3_srvstr="Server String";
service.samba3_connlimit="Connection Limit";
service.samba3_pub="Public Share";
service.samba3_config="Custom Configuration";
service.samba3_workgrp="Workgroup";
service.samba3_interfaces="Interfaces";
service.samba3_dirpath="Path to Files";
service.samba3_usr1="User1";
service.samba3_pass1=" Password1";
service.samba3_usr2="User2";
service.samba3_pass2=" Password2";
service.samba3_pubacl="Readonly";
service.samba3_advanced="Advanced";
service.samba3_custom="Use Custom Configuration";
service.samba3_shares="Shares";
service.samba3_share_path="Path";
service.samba3_share_subdir="Subdir";
service.samba3_share_label="Name";
service.samba3_share_public="Public";
service.samba3_share_access="Access";
service.samba3_users="Users";
service.samba3_username="Username";
service.samba3_password="Password";
service.samba3_user_shares="Access Shares";
service.samba3_min_proto="Minimum Protocol Version";
service.samba3_max_proto="Maximum Protocol Version";
service.samba3_encryption="Encryption";
service.samba3_guest="Access Level";
service.samba3_guest_baduser="Everyone";
service.samba3_guest_never="Restricted";
service.dlna_type_audio="Audio";
service.dlna_type_video="Video";
service.dlna_type_images="Images";
service.nfs="Configuration";
service.nfs_allowed="Allowed Networks";
service.nfs_srv="NFS Server";
service.rsync="rsync";
service.rsync_srv="rsync Daemon";
service.rsync_allowed="Allowed Hosts";

// Zabbix
service.zabbix_legend="Zabbix";
service.zabbix_cl=idx.ntp_client;
service.zabbix_serverip="Server IP";
service.zabbix_usrpara="User Parameters";

// mdns
service.mdns_legend="mDNS / Avahi";
service.mdns_label="Enable Resolver";
service.mdns_domain="Domain Name [local]";
service.mdns_reflector="Reflector";
service.mdns_interfaces="Interfaces";

//TRansmission
service.transmission_legend="Torrent";
service.transmission_srv="Enable Transmission";
service.transmission_dir="Config Directory";
service.transmission_download="Download Directory";
service.transmission_style="Web UI Style";
service.transmission_rpc="Web UI Port";
service.transmission_down="Max global download speed";
service.transmission_up="Max global upload speed";
service.transmission_script="Run script after download complete";
service.transmission_whitelist="Whitelist IPs";

service.plex_legend="Plex Media Server";
service.plex_srv=nas.proftpd_srv_label;
service.plex_appdir="Application Support Directory";

service.antaira_agent_h2="Antaira Quick VPN Agent";
service.antaira_agent_legend="Antaira Agent Configuration";
service.antaira_agent_enable="Enable Agent";
service.antaira_agent_cloud_url="Cloud URL";
service.antaira_agent_connect_url="Connect URL";
service.antaira_agent_configuration_url="Configuration URL";
service.antaira_agent_token=share.token;

// ** eop-tunnel.asp **//
var eoip=new Object();
eoip.titl="Tunnels";
eoip.tunnel="Tunnel";
eoip.legend="Ethernet and IP Tunneling";
eoip.proto="Protocol Type";
eoip.mtik="Mikrotik";
eoip.vxlan="VXLAN";
eoip.mcast="Multicast";
eoip.mcast_group="Multicast Group";
eoip.dport="Destination Port";
eoip.sportrange="Source Port Range";
eoip.ttl="TTL";
eoip.tos="TOS";
eoip.lrn="Forward Database Learning";
eoip.proxy="ARP Proxy";
eoip.rsc="Route Short Circuit";
eoip.l2miss="LL Address Miss Notifications";
eoip.l3miss="IP Address Miss Notifications";
eoip.udpcsum="UDP checksum calculation for IPv4";
eoip.udp6zerocsumtx="Skip IPv6 UDP checksum calculation";
eoip.udp6zerocsumrx="Allow IPv6 UDP packets without checksum";
eoip.ageing="Ageing";
eoip.df="Don't Fragment - Bit";
eoip.flowlabel="Flow label";
eoip.gpb="Group Policy Extensions";
eoip.genkey=service.ssh_keygenerate;
eoip.wireguard="WireGuard";
eoip.wireguard_oet_spbr_ip="Source for PBR";
eoip.wireguard_oet_dpbr_ip="Destination for PBR";
eoip.wireguard_oet_dns="DNS Servers via Tunnel";
eoip.wireguard_oet_natout ="NAT via Tunnel";
eoip.wireguard_showadvanced="Advanced Settings";
eoip.wireguard_rtupscript="Route up Script";
eoip.wireguard_rtdownscript="Route down Script";
eoip.wireguard_fwmark="Firewall Mark";
eoip.wireguard_route_allowedip="Route Allowed IPs via Tunnel";
eoip.wireguard_localport="Listen Port";
eoip.wireguard_ka="Persistent Keepalive";
eoip.wireguard_endpoint="Endpoint";
eoip.wireguard_peer="Endpoint Address";
eoip.wireguard_peerkey="Peer Public Key";
eoip.wireguard_cllegend="Client Config File";
eoip.wireguard_peerip="Peer Tunnel IP/Netmask";
eoip.wireguard_peerdns="Peer Tunnel DNS";
eoip.wireguard_clend="Peer Tunnel Endpoint";
eoip.wireguard_clka="Peer Keepalive";
eoip.wireguard_localkey="Local Public Key";
eoip.wireguard_localprivatekey="Local Private Key";
eoip.wireguard_killswitch="Kill Switch";
eoip.wireguard_dnspbr="Split DNS";
eoip.wireguard_dns4="IPv4 DNS Server";
eoip.wireguard_dns6="IPv6 DNS Server";
eoip.wireguard_spbr="Source Routing (PBR)";
eoip.wireguard_spbr0="Route All Sources via VPN";
eoip.wireguard_spbr1="Route Selected Sources via VPN";
eoip.wireguard_spbr2="Route Selected sources via WAN";
eoip.wireguard_dpbr="Destination Routing";
eoip.wireguard_dpbr0="Route All Destinations via Default Route";
eoip.wireguard_dpbr1="Route Selected Destinations via VPN";
eoip.wireguard_dpbr2="Route Selected Destinations via WAN";
eoip.wireguard_firewallin="Firewall Inbound";
eoip.wireguard_usepsk="Use Pre-shared Key";
eoip.wireguard_genpsk="Generate Pre-shared Key";
eoip.wireguard_oet_status="WireGuard Status (F5 to refresh)";
eoip.wireguard_psk="Pre-Shared Key";
eoip.wireguard_addpeer="Add Peer";
eoip.wireguard_delpeer="Remove Peer";
eoip.wireguard_makeclient="Make Peer Config";
eoip.wireguard_cleanqr="Delete Peer Config";
eoip.wireguard_allowedips="Allowed IPs";
eoip.wireguard_failgrp="Failover Member";
eoip.wireguard_failstate="Fail State (auto)";
eoip.wireguard_standby="Standby";
eoip.wireguard_running="Running";
eoip.wireguard_failed="Failed";
eoip.wireguard_obfuscation="Tunnel Obfuscation";
eoip.wireguard_lanac=service.vpnd_allowcnlan;
eoip.etherip="RFC 3378 EoIP";
eoip.srv="Tunnel";
eoip.remoteIP="Remote IP Address";
eoip.localIP="Local IP Address";
eoip.tunnelID="Tunnel ID";
eoip.comp="Compression";
eoip.passtos="ToS passthrough";
eoip.frag="fragment";
eoip.mssfix="MSS Fix";
eoip.shaper="shaper";
eoip.bridging="Bridging";
eoip.add=" Add Tunnel ";
eoip.del="Delete Tunnel";
eoip.importt="Import Configuration";
eoip.filepicker="Upload / Adapt & Apply";

// help page
var heoip=new Object();
heoip.page1="<dd>Ethernet over IP (EoIP) Tunneling enables you to create an Ethernet tunnel between two routers on top of an IP connection. The EoIP interface appears as an Ethernet interface. When the bridging function of the router is enabled, all Ethernet traffic (all Ethernet protocols) will be bridged just as if there where a physical Ethernet interface and cable between the two routers (with bridging enabled).<br /><br />Network setups with EoIP interfaces : <br /><ul><li>Possibility to bridge LANs over the Internet</li><li>Possibility to bridge LANs over encrypted tunnels</li><li>Possibility to bridge LANs over 802.11b 'ad-hoc' wireless networks</li></ul></dd>";

// ** Sipath.asp + cgi **//
var sipath=new Object();
sipath.titl="SiPath Overview";
sipath.phone_titl="Phonebook";
sipath.status_titl="Status";

// ** Status_Lan.asp **//
var status_lan=new Object();
status_lan.titl="LAN Status";
status_lan.h2="Local Area Network (LAN)";
status_lan.legend="LAN Status";
status_lan.h22=idx.dhcp_legend;
status_lan.legend2="DHCP Status";
status_lan.legend3="DHCP Clients";
status_lan.legend4="Active Clients";
status_lan.legend5="Connected PPTP Clients";
status_lan.legend6="Connected PPPoE Clients";
status_lan.concount="Connections";
status_lan.conratio="Ratio";

//help container
var hstatus_lan=new Object();
hstatus_lan.right2="This is the router's MAC address, as seen on your local Ethernet network.";
hstatus_lan.right4="This shows the router's IP address as it appears on your local Ethernet network.";
hstatus_lan.right6="When the router is using a subnet mask, it is shown here.";
hstatus_lan.right8="If you are using the router as a DHCP server, that will be displayed here.";
hstatus_lan.right10="By clicking on any MAC address, you will obtain the organizationally unique identifier of the network interface (IEEE Standards OUI database search).";

//help page
hstatus_lan.page1="<dd>This page displays the LAN status and configuration. All information is read-only.</dd><dt>MAC Address</dt><dd>The MAC Address of the LAN interface is displayed here.</dd><dt>IP Address and Subnet Mask</dt><dd>The current IP Address and Subnet Mask of the router, as seen by users on your local area network (LAN), are displayed here.</dd><dt>DHCP Server</dt><dd>The status of the router's DHCP server function is displayed here.</dd><dt>Start/End IP Address</dt><dd>The first and the last IP address the DHCP server can hand out to clients.</dd><dt>DHCP Client List</dt><dd>To show the current IP address leases by the DHCP server, click the <i>DHCP Clients Table</i> button.</dd>";

// ** Status_Bandwidth.asp **//
var status_band=new Object();
status_band.titl="Bandwidth Monitoring";
status_band.h2="Bandwidth Monitoring";
status_band.chg_unit="Switch to ";
status_band.chg_scale="Autoscale";
status_band.chg_error="Cannot get data about interface";
status_band.chg_collect_initial="Collecting initial data, please wait...";
status_band.strin="In";
status_band.strout="Out";
status_band.follow="follow";
status_band.up="up";

//help container
var hstatus_band=new Object();
hstatus_band.svg="A browser that supports SVG is required to display bandwidth graphs.";
hstatus_band.right1="Click the label to switch unit (B/s or bit/s).";
hstatus_band.right2="Click the label to choose graph scale type.";

// ** Status_Router.asp **//
var status_router=new Object();
status_router.titl="Router Status";
status_router.h2="Router Information";
status_router.legend="System";
status_router.sys_model="Router Model";
status_router.sys_firmver="Firmware Version";
status_router.sys_time="Current Time";
status_router.sys_up="Uptime";
status_router.sys_load="Load Average";
status_router.sys_kernel="Kernel Version";
status_router.legend2="System Details";
status_router.cpu="CPU Model";
status_router.cores="CPU Cores";
status_router.features="CPU Features";
status_router.clock="CPU Clock";
status_router.legend3="Memory - Available / Total";
status_router.mem_tot="Total";
status_router.mem_free="Free";
status_router.mem_used="Used";
status_router.mem_buf="Buffers";
status_router.mem_cached="Cached";
status_router.mem_active="Active";
status_router.mem_inactive="Inactive";
status_router.mem_hidden="Hidden"; // do not translate this line, this is bogus (BrainSlayer)
status_router.legend4="Network";
status_router.net_ipcontrkmax="Maximum IP Connections";
status_router.net_conntrack="Active IP Connections";
status_router.notavail="Not Available";
status_router.legend6="NVRAM / CIFS / JFFS2 Usage";
status_router.inpvolt="Board Input Voltage";
status_router.cputemp="Temperatures";

//help container
var hstatus_router=new Object();
hstatus_router.right2="This is the specified router name, you had input on the <i>Setup</i> tab under the <i>" + share.routername + "</i> field.";
hstatus_router.right4="This is the router's MAC Address, as seen by your ISP.";
hstatus_router.right6="This is the router's current firmware.";
hstatus_router.right8="This is time received from the NTP server set on the <em>" + bmenu.setup + " | " + bmenu.setupbasic + "</em> tab.";
hstatus_router.right10="This is the measure of the time the router has been \"up\" and running.";
hstatus_router.right12="This is obtained from the three numbers that represent the system load during the last one, five, and fifteen minute periods.";

//help page
hstatus_router.page1="<dd>This status screen displays the router's current status and configuration. All information is read-only.</dd><dt>" + share.routername + "</dt><dd>Shows the configured name of the router.</dd><dt>" + status_router.sys_model + "</dt><dd>Shows the router's vendor, model and other hardware revision details.</dd><dt>" + status_router.sys_firmver + "</dt><dd>The version / revision number and compilation date of the currently installed firmware. It is recommended to visit <a href=\"https:\/\/dd-wrt.com/support/other-downloads/?path=betas\" target=\"_new\">https:\/\/dd-wrt.com/downloads/betas</a> regularly to find updated firmware files.</dd><dt>" + status_router.sys_kernel + "</dt><dd>The current kernel version and build details is displayed here.</dd><dt>" + share.mac + "</dt><dd>The MAC Address of the WAN interface is displayed here.</dd><dt>" + share.hostname + "</dt><dd>The Hostname is the name of the router.</dd><dt>" + share.wandomainname + "</dt><dd>The currently configured WAN domain name is displayed here.</dd><dt>" + share.landomainname + "</dt><dd>The currently configured LAN domain name is displayed here.</dd><dt>" + status_router.sys_time + "</dt><dd>The current date and time is displayed here.</dd><dt>" + status_router.sys_up + "</dt><dd>The total uptime the router has been operating since last reboot is displayed here.</dd><dt>" + status_router.legend2 + "</dt><dd>This section shows various details about your CPU like its model, revision, number of cores, features supported, clock speed, load average and temperature(s) which may include the WiFi radios, if temperature monitoring is supported.</dd><dt>" + status_router.legend3 + "</dt><dd>Shows detailed RAM information starting with the available, free, used, buffer allocated, cached, active and inactive.</dd>";

// ** Status_Internet.asp **//
var status_inet=new Object();
status_inet.titl="WAN Status";
status_inet.h11="WAN";
status_inet.conft="Configuration Type";
status_inet.www_loginstatus="Login Status";
status_inet.wanuptime="Connection Uptime";
status_inet.acname="Access Concentrator Name";
status_inet.sig_status="Signal Status";
status_inet.leasetime="Remaining Lease Time";
status_inet.traff="Traffic";
status_inet.traff_tot="Total Traffic";
status_inet.traff_mon="Traffic by Month";
status_inet.traffin="Incoming";
status_inet.traffout="Outgoing";
status_inet.previous="Previous Month";
status_inet.next="Next Month";
status_inet.dataadmin="Data Administration";
status_inet.delete_confirm="WARNING! This will delete all traffic data. Proceed?";
status_inet.speed="Internet Speed";
status_inet.speedtest="Speedtest";
status_inet.down="Downstream";
status_inet.up="Upstream";
status_inet.server="Server";
status_inet.sponsor="Sponsor";
status_inet.town="Town";
status_inet.country="Country";
status_inet.test="Run Test";

//help container
var hstatus_inet=new Object();
hstatus_inet.right2="This shows the information required by your ISP for connection to the Internet. This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.";
hstatus_inet.right4="This shows your router's Internet traffic since last reboot.";
hstatus_inet.right6="This shows your router's Internet traffic by month. Drag the mouse over graph to see daily data. Data is stored in NVRAM.";
// help page
hstatus_inet.page1="<dt>" + status_inet.conft + "</dt><br /><dt>" + idx.conn_type + "</dt><dd>Will display the selected type of connection:<ul><li>" + share.disabled + "</li><li>" + idx.static_ip + "</li><li>" + idx.dhcp + "</li><li>" + idx.dhcp_auth + "</li><li>PPPoE</li><li>" + idx.pppoe_dual + "</li><li>PPTP</li><li>L2TP</li><li>" + idx.heartbeat_sig + "</li><li>" + idx.iphone_tether + "</li><li>" + idx.mobile_bb + "</li></ul><dt>" + status_inet.wanuptime + "</dt></dd><dd> Will show the current WAN uptime total.</dd><dt>" + share.ipv4 + " and / or " + share.ipv6 + "</dt><dd>The current WAN IP/subnet mask or prefix in case you have IPv6 enabled.</dd><dt>" + share.gateway + "</dt><dd>Shows the router's current IP address.</dd></dt><br /><dt>IPv4 and / or IPv6 DNS 0 / 1 / 2</dt><dd>The Domain Name System (DNS) IP Addresses currently setup and in use by the router are shown here. Up to three DNS IPs are allowed. In most cases, the first available DNS entry is used.</dd><dt>" + status_inet.leasetime + "</dt><dd>The router's remaining lease time assigned to you by the ISP equipment is displayed here.</dd><dt>" + status_inet.traff + "</dt><dd>The router's Internet traffic (total since last reboot or by month).</dd><dt>" + status_inet.traffin + "</dt><dd> The current incoming traffic amount in MiB is displayed here.</dd><dt>" + status_inet.traffout + "</dt><dd> The current outgoing traffic amount in MiB is displayed here.</dd><dt>" + status_inet.traff_mon + "</dt><dd>A graphical representation of incoming / outgoing traffic by month is displayed here, you can view the current or at your choice view the previous or follwoing months when and if this data has already been captured.</dd><dt>" + status_inet.dataadmin + "</dt><dd>This section will allow you to backup / restore or delete the desired captured data.</dd>";

// ** Status_Conntrack.asp **//
var status_conn=new Object();
status_conn.titl="Active IP Connections Table";
status_conn.h2="Active IP Connections";

// ** Status_Wireless.asp **//
var status_wireless=new Object();
status_wireless.titl="Wireless Status";
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
status_wireless.busy="Busy Time";
status_wireless.active="Active Time";
status_wireless.quality="Channel Quality";
status_wireless.rx_time="Receive Time";
status_wireless.tx_time="Transmission Time";
status_wireless.assoc_count="Connected Clients";
status_wireless.chaininfo="Chain Info";
// ** GPS info **//
var status_gpsi=new Object();
status_gpsi.legend="GPS Info";
status_gpsi.status="Status";
status_gpsi.lon="Longitude";
status_gpsi.lat="Latitude";
status_gpsi.alt="Altitude";
status_gpsi.sat="Visible Satellites";
status_gpsi.ant_conn="Antenna Connected";
status_gpsi.ant_disc="Antenna Disconnected";
status_gpsi.na="Unavailable";
//help container
var hstatus_wireless=new Object();
hstatus_wireless.right2="This is the router's MAC address, as seen on your local, wireless network.";
hstatus_wireless.right4="As selected from the wireless tab, this will display the wireless mode (Mixed, G Only, B Only or Disabled) used by the network.";

//help page
hstatus_wireless.page1="<dd>This status screen displays the router's wireless status and configuration. All information is read-only.</dd>";
hstatus_wireless.page2="<dd>The MAC Address of the wireless interface is displayed here.</dd>";
hstatus_wireless.page3="<dd>The Mode of the wireless network is displayed here.</dd>";
hstatus_wireless.page4="<dd>The SSID of the wireless network is displayed here.</dd>";
hstatus_wireless.page5="<dd>The channel of the wireless network is displayed here.</dd>";
hstatus_wireless.page6="<dd>The transfer power of the wireless device is displayed here.</dd>";
hstatus_wireless.page7="<dd>The current wireless transfer rate is displayed here.</dd>";
hstatus_wireless.page8="<dd>The status of the encryption is displayed here.</dd><dd>Click the <i>Survey</i> button to show all wireless networks in your neighbourhood reachable by your router.</dd>";

// ** Status_OpenVPN.asp **//
var status_openvpn=new Object();
status_openvpn.titl="OpenVPN Status";

// ** Status_CWMP.asp **//
var status_cwmp=new Object();
status_cwmp.titl="CWMP (TR-069) Status";

// ** Triggering.asp **//
var trforward=new Object();
trforward.titl="Port Triggering";
trforward.h2="Port Triggering";
trforward.legend="Forwards";
trforward.trrange="Triggered Port Range";
trforward.fwdrange="Forwarded Port Range";
trforward.app="Application";

//help container
var htrforward=new Object();
htrforward.right2="Enter the application name of the trigger.";
htrforward.right4="For each application, list the triggered port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right6="For each application, list the forwarded port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right8="Enter the starting port numbers of the Triggered and Forwarded Ranges.";
htrforward.right10="Enter the ending port numbers of the Triggered and Forwarded Ranges.";

//help page
htrforward.page1="<dd>Port Triggering allows you to do port forwarding without setting a fixed device. By setting Port Triggering rules, you can allow inbound traffic to arrive at a specific LAN host, using ports different than those used for the outbound traffic. This is called port triggering since the outbound traffic triggers to which ports inbound traffic is directed.<br /><br />If you want to forward ports to a device with a static IP address, see <a href=\"HForwardSpec.asp\">";
htrforward.page2="</a> Or <a href=\"Forward_range.asp\">Port Range Forwarding</a>.</dd><dd>To add a new Port Triggering rule, click the <em>" + sbutton.add + "</em> button and fill in the fields below.<br />To remove the last rule, click the <i>remove</i> icon.</dd><dt>Application</dt><dd>Enter the name of the application in the field provided.</dd><dt>Triggered Range</dt><dd>Enter the number of the first and the last port of the range, which should be triggered. If a device sends outbound traffic from those ports, incoming traffic on the <i>Forwarded Range</i> will be forwarded to that device.</dd><dt>Forwarded Range</dt><dd>Enter the number of the first and the last port of the range, which should be forwarded from the Internet to the device, which falls within the <i>Triggered Range</i>.</dd><dt>Enable</dt><dd>Check the <em>" + share.enable + "</em> checkbox to enable port triggering for the application.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Remember to save your changes before adding another triggering rule.</div></div></dd>";

// ** Upgrade.asp **//
var upgrad=new Object();
upgrad.titl="Firmware Upgrade";
upgrad.h2="Firmware and Reset Management";
upgrad.legend="Firmware Upgrade and Reset";
upgrad.info1="After Flashing";
upgrad.resetOff="Do not Reset";
upgrad.resetOn="Reset to Factory Defaults";
upgrad.file="Select a Firmware File";
upgrad.warning="W A R N I N G";
upgrad.mess1="Upgrading the firmware may take a few minutes.<br />Do not turn off the power or press the reset button!";

//help container
var hupgrad=new Object();
hupgrad.right2="Click on the <em>Browse...</em> button to select the firmware file to be uploaded to the router.<br /><br /> Click the <em>Upgrade</em> button to begin the upgrade process. The upgrade must not be interrupted.";

//help page
hupgrad.page1="<dd>New firmware versions are posted at <a href=\"https:\/\/dd-wrt.com/support/other-downloads/?path=betas\" target=\"_new\">https:\/\/dd-wrt.com/downloads/betas</a> and can be downloaded for free.<br />Valid reasons to upgrade to a more recent firmware version include, <b>a new feature, a bug or security fixes</b>.<br /><br /><div class=\"note\"><h4>Note:</h4><div>Ensure you have a backup of your current settings before you upgrade the router's firmware, in case you lose the configuration settings you can restore the backup.</div></div></dd><dd>To upgrade the router's firmware:<ol class=\"wide\"><li>Download the firmware upgrade file from the website.</li><li>Click the <i>Browse...</i> button and choose the firmware upgrade file form the directory you saved the upgrade file to.</li><li>Click the <em>" + sbutton.upgrade + "</em> button and wait until the upgrade has finished and router has rebooted.</li></ol><br /><div class=\"note\"><h4>Note:</h4><div>DO NOT POWER OFF THE ROUTER OR PRESS THE RESET BUTTON WHILE THE FIRMWARE IS BEING UPGRADED.</div></div></dd><dt>Reset Options</dt><dd>If you want to reset the router to the default settings for the firmware version you are upgrading to, select from the dropdown menu the <em>" + upgrad.resetOn + "</em> option.<br /><br /></dd>";

// ** UPnP.asp **//
var upnp=new Object();
upnp.titl="UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Forwards";
upnp.legend2="UPnP Configuration";
upnp.serv="UPnP Service";
upnp.clear="Clear Port Forwards at Startup";
upnp.url="Send presentation URL";
upnp.msg1="Click to delete entry";
upnp.msg2="Delete all entries?";

//help container
var hupnp=new Object();
hupnp.right2="Click the remove icon to delete an individual entry.<br /> Click the <em>" + sbutton.delall + "</em> button to remove all entries.";
hupnp.right4="Allows applications to automatically configure port forwarding.";

//help page
hupnp.page1="<dd>Also known as UPnP it is a set of network protocols used for the automatic configuration of devices. The goals of UPnP are to allow devices to connect seamlessly and to simplify the implementation of home or corporate environment networks. UPnP achieves this by defining and publishing device control protocols built upon open, Internet-based communication standards.</dd>";
hupnp.page2="<dd>The UPnP forwards table shows all open ports forwarded automatically by the UPnP process. You can delete forwards by clicking <i>remove icon</i> or the <em>" + sbutton.delall + "</em> button to clear the undesired entries.</dd>";
hupnp.page3="<dd>Allows applications to automatically setup port forwarding rules.</dd>";
hupnp.page4="<dd>If enabled, all UPnP port forwarding rules are deleted when the router starts up.</dd>";
hupnp.page5="<dd>If enabled, a presentation URL tag is sent with the device description. This allows the router to show up in <em>Windows's My Network Places</em>.<br /><br />div class=\"note\"><h4>Note:</h4><div>When enabling this option you may need to reboot your computer.</div></div></dd><dd>Click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** VPN.asp **//
var vpn=new Object();
vpn.titl="VPN Passthrough";
vpn.h2="Virtual Private Network (VPN)";
vpn.legend="VPN Passthrough";
vpn.ipsec="IPSec Passthrough";
vpn.pptp="PPTP Passthrough";
vpn.l2tp="L2TP Passthrough";

//help container
var hvpn=new Object();
hvpn.right1="You may choose to enable IPSec, PPTP and/or L2TP passthrough to allow your network devices to communicate via VPN.";

//help page
hvpn.page1="<dd>Virtual Private Networking (VPN) is typically used for work-related networking. For VPN tunnels, the router supports IPSec, PPTP and L2TP Passthrough.</dd>";
hvpn.page2="<dd>Internet Protocol Security (IPSec) is a suite of protocols used to implement secure exchange of packets at the IP layer. To allow IPSec tunnels to passthrough the router, IPSec Passthrough is enabled by default. To disable IPSec Passthrough, select <em>" + share.disable + "</em>.</dd>";
hvpn.page3="<dd>Point-to-Point Tunneling Protocol is the method used to enable VPN sessions to PPTP VPN servers. To allow PPTP tunnels to passthrough the router, PPTP Passthrough is enabled by default. To disable PPTP Passthrough, select <em>" + share.disable + "</em>.</dd>";
hvpn.page4="<dd>Layer 2 Tunneling Protocol, an extension to the PPP protocol that enables ISPs to operate VPNs. L2TP merges the best features of two other tunneling protocols: PPTP from Microsoft and L2F from Cisco Systems. To allow L2TP tunnels to passthrough the router, L2TP Passthrough is enabled by default. To disable L2TP Passthrough, select <em>" + share.disable + "</em>.</dd>";

// ** Vlan.asp **//
var vlan=new Object();
vlan.titl="Virtual LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN Configuration";
vlan.vlans_label="Vlans";
vlan.bridge="Assigned To<br />Bridge";
vlan.tagged="Tagged";
vlan.negociate="Autonegotiation";
vlan.aggregation="Link Aggregation<br />on Ports 3 & 4";
vlan.trunk="Trunk";
vlan.linkstatus="Link Status";
vlan.fullspeed="Full Speed";
vlan.fullduplex="Full Duplex";
vlan.gigabit="Gigabit";
vlan.eee="802.3az EEE";
vlan.flow="Flow control";

// ** WEP.asp **//
var wep=new Object();
wep.defkey="Default Transmit Key";
wep.passphrase="Passphrase";
wep.opt_64="64-bit 10 Hex Digits";
wep.opt_128="128-bit 26 Hex Digits";
wep.generate="Generate";

// ** WOL.asp **//
var wol=new Object();
wol.titl="WOL";
wol.h2="Wake-on-LAN (WOL)";
wol.legend="Available Hosts";
wol.legend2="WOL Addresses";
wol.legend3="Output";
wol.legend4="Manual WOL";
wol.enable="Enable WOL";
wol.mac="MAC Address(es)";
wol.broadcast="Net Broadcast";
wol.udp="UDP Port";
wol.msg1="Click to remove WOL host";
wol.h22="Automatic Wake-on-LAN";
wol.legend5="WOL Daemon";
wol.srv="Enable Daemon";
wol.pass="SecureOn Password";

//help container
var hwol=new Object();
hwol.right2="This page allows you to <b><em>" + sbutton.wol + "</em></b> hosts on your local network. You can manually wake up hosts by clicking the <em>" + sbutton.wol + "</em> button or alternatively by programing an automatic wake up schedule provided by the " + wol.legend5 + ".";
hwol.right4="MAC address(es) are entered in the format e.g. 01:23:45:67:89:AB and must be separated by a <em>SPACE</em>.";
hwol.right6="The IP address is typically the broadcast address for the local network, it could also be a remote address when e.g. the target host is not a LAN client.";

//help page
hwol.page1="<dd>This page allows you to <em>" + sbutton.wol + "</em> hosts on your local network.</dd><dt class=\"term\">" + wol.legend + ":</dt><dd class=\"definition\">The " + wol.legend + " section provides a list of current hosts to add or remove from the WOL Addresses section. The " + wol.legend + " is a combination of any defined hosts with static leases or automatically discovered DHCP clients.<div class=\"note\"><h4>Note:</h4><div>This table uses the MAC address, &quot;guesses&quot; the network broadcast address by assuming the host's IP address has the same netmask as the local router (lan_netmask), and uses the " + wol.udp + " specified in the in the " + wol.legend4 + " section -- the default is <b>port 7</b> unless otherwise configured.</div></div></dd>";
hwol.page2="<dd class=\"definition\">The WOL Addresses section allows individual hosts in the WOL list (stored in the <b>wol_hosts</b> NVRAM variable) to be <b>woken up</b>.  The list is a combination of selected (WOL enabled) available Hosts and/or manually added WOL target hosts.</dd>";
hwol.page3="<dd class=\"definition\">The Manual WOL section allows individual or a list of hosts to be woken up by clicking <em>" + sbutton.wol + "</em> button to send it the WOL <i>magic packet</i>.</dd>";
hwol.page4="<dd class=\"definition\">Fill the MAC address(es) (either separated by spaces or one per line) of the computer(s) you would like to wake up.<div class=\"note\"><h4>Note:</h4><div>Each MAC address entered as e.g. xx:xx:xx:xx:xx:xx, where xx is a hexadecimal number between 00 and FF which represents one byte of the address, and is in a network (big endian) byte order.</div></div></dd>";
hwol.page5="<dd class=\"definition\">Broadcast to this IP address or hostname (typically you would want to make this your network's broadcast IP for locally waking up hosts.</dd>";
hwol.page6="<dd class=\"definition\">Broadcast to this " + wol.udp + ".</dd>";
hwol.page7="<dd class=\"definition\">Besides attempting to <b>" + sbutton.wol + "</b> the manually specified host(s), by clicking on the <em>" + sbutton.wol + "</em> button will save the MAC address(es), network broadcast, and " + wol.udp + " values into the respective <b>manual_wol_mac</b>, <b>manual_wol_network</b>, and <b>manual_wol_port</b> NVRAM variables and commits them to memory.</dd>";

// ** WanMAC.asp **//
var wanmac=new Object();
wanmac.titl="MAC Address Clone";
wanmac.h2="MAC Address Clone";
wanmac.legend="MAC Clone";
wanmac.wan="Clone WAN MAC";
wanmac.wlan="Clone Wireless MAC";

//help container
var hwanmac=new Object();
hwanmac.right2="Some ISPs will require you to register your MAC address. If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";

//help page
hwanmac.page1="<dd>The router's MAC address is a 12-digit code assigned to a unique piece of hardware for identification. Some ISPs require that you register the MAC address of your network card/adapter, which was connected to your cable or DSL modem during installation.</dd>";
hwanmac.page2="<dd>To clone your network adapter's MAC address onto the router, select <em>" + share.enable + "</em> and enter your adapter's MAC address in the <i>Clone WAN MAC</i> field or click the <em>" + sbutton.wanmac + "</em> button to automatically fill in the MAC address of the device you are using. Then save your changes by clicking on the <em>" + sbutton.save + "</em> button.<br /><br />To disable MAC address cloning, keep the default setting, <em>" + share.disable + "</em>.</dd><dd>Find your adapter's MAC address by following the instructions for your computers operating system.<br /><br /><b>Microsoft Windows:</b><ol class=\"wide\"><li>Click the Start button, and select Run or the<em>Windows key</em>+<em>R</em> shortcut.</li><li>Type <tt>cmd</tt> in the field provided, and press the OK button.</li><li>At the command prompt, run <tt>ipconfig /all</tt>, and look at your adapter's physical address.</li><li>Write down your adapter's MAC address.</li></ol><br /><b>Linux:</b><ol class=\"wide\"><li>Click <em>CTRL</em>+<em>ALT</em>+<em>T</em>, to open a terminal.</li><li>Type <tt>ifconfig -a</tt>, and press Enter</li><li>Look for the Ethernet adapter you are using, the MAC address is in this format <b>xx:xx:xx:xx:xx:xx</b>.</li><li>Write down your adapter's MAC address.</li></ol><br /></dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

var roaming=new Object();

roaming.debug_level="Debug Level";
roaming.local_mode="Disable Network Communication";
roaming.sta_block_timeout="Station Block Timeout";
roaming.local_sta_timeout="Local Station Timeout";
roaming.local_sta_update="Station Update Interval";
roaming.max_neighbor_reports="Maximum Neighbors";
roaming.max_retry_band="Max Retry Band";
roaming.seen_policy_timeout="Seen Policy Timeout";
roaming.measurement_report_timeout="Measurement Report Timeout";
roaming.load_balancing_threshold="Load Balancing Threshold";
roaming.band_steering_threshold="Band Steering Threshold";
roaming.remote_update_interval="Remote Update Interval";
roaming.remote_node_timeout="Renote Node Timeout";
roaming.assoc_steering="Assoc Steering";
roaming.min_connect_snr="Min Connect SNR";
roaming.min_snr="Min SNR";
roaming.min_snr_kick_delay="Min SNR Kick Delay";
roaming.steer_reject_timeout="Steer Reject Timeout";
roaming.roam_process_timeout="Roam Process Timeout";
roaming.roam_scan_snr="Roam Scan SNR";
roaming.roam_scan_tries="Roam Scan Tries";
roaming.roam_scan_timeout="Roam Scan Timeout";
roaming.roam_scan_interval="Roam Scan Interval";
roaming.roam_trigger_snr="Roam Trigger SNR";
roaming.roam_trigger_interval="Roam Trigger Interval";
roaming.roam_kick_delay="Roam Kick Delay";
roaming.signal_diff_threshold="Signal Diff Threshold";
roaming.initial_connect_delay="Initial Connect Delay";
roaming.load_kick_enabled="Load Kick Enabled";
roaming.load_kick_threshold="Load Kick Threshold";
roaming.load_kick_delay="Load Kick Delay";
roaming.load_kick_min_clients="Load Kick Min Clients";
roaming.load_kick_reason_code="Load Kick Reason Code";
roaming.band_steering_interval="Band Steering Interval";
roaming.band_steering_min_snr="Band Steering Min SNR";
roaming.link_measurement_interval="Link Measurement Interval";
roaming.s80211v="802.11v Support";
roaming.s80211k="802.11k Support";
roaming.wnm_sleep_mode="WNM-Sleep Mode";
roaming.wnm_sleep_mode_no_keys="WNM-Sleep Mode GTK/IGTK workaround";
roaming.bss_transition="BSS Transition Management";
roaming.rrm_neighbor_report="Neighbor report via radio";
roaming.rrm_beacon_report="Beacon report via radio";
roaming.proxy_arp="Proxy ARP";
roaming.time_advertisement="Time advertisement";
roaming.usteer="Band-Steering";
roaming.usteer_options="Band-Steering Options";
roaming.roaming="Roaming Options";
roaming.ft="802.11r Support (FT)";
roaming.nas="NAS Identifier";
roaming.domain="Mobility Domain";
roaming.mbo="Multiband Operation (MBO)";
roaming.mbo_cell_data_conn_pref="Cellular Connection Preference";
roaming.reassociation_deadline="Reassociation Deadline";
roaming.ft_protocol="FT protocol";
roaming.ft_over_ds="FT over DS";
roaming.ft_over_air="FT over the Air";
roaming.budget_5ghz="5 GHz Budget in DBm";
roaming.prefer_5ghz="Prefer 5 GHz";

// ** WL_WPATable.asp / WPA.asp / Radius.asp **//
var wpa=new Object();
wpa.titl="Wireless Security";
wpa.h2="Wireless Security";
wpa.secmode="Security Mode";
wpa.legend="Wireless Encryption";
wpa.auth_mode="Network Authentication";
wpa.mfp="802.11w Management Frame Protection";
wpa.radius="RADIUS";
wpa.wpa_strict_rekey="WPA Strict Rekeying";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval";
wpa.radius_ipaddr="RADIUS Server Address";
wpa.radius_port="RADIUS Server Port";

//portsetup.c line 275 and
wpa.radius_shared_secret="RADIUS Shared Secret";
wpa.session_time="Session Time";

wpa.radius_key="RADIUS Key";
wpa.algorithms="WPA Algorithms";
wpa.shared_key="WPA Shared Key";
wpa.sae_key="SAE Passphrase";
wpa.eapol_key_retries="Disable EAPOL Key Retries";
wpa.ccmp="CCMP-128 (AES)";
wpa.tkip_ccmp="TKIP+CCMP (AES)";
wpa.ccmp_256="CCMP-256";
wpa.tkip="TKIP";
wpa.gcmp_128="GCMP";
wpa.gcmp_256="GCMP-256";
wpa.psk="WPA Personal";
wpa.psk2="WPA2 Personal";
wpa.psk2_sha256="WPA2 Personal with SHA256";
wpa.psk3="WPA3 Personal / SAE";
wpa.wpa="WPA Enterprise";
wpa.wpa2="WPA2 Enterprise";
wpa.wpa2_sha256="WPA2 Enterprise with SHA256";
wpa.wpa3="WPA3 Enterprise";
wpa.wpa3_128="WPA3 Enterprise Suite-B 128-bit";
wpa.wpa3_192="WPA3 Enterprise CNSA Suite-B 192-bit";
wpa.wep_8021x="802.1x / WEP";
wpa.peap="EAP-PEAP";
wpa.leap="EAP-LEAP";
wpa.tls="EAP-TLS";
wpa.ttls="EAP-TTLS";
wpa.owe="OWE Opportunistic Wireless Encryption";
wpa.owe_ifname="OWE Transition Interface";

var aoss=new Object();
aoss.titl="AOSS Security";
aoss.aoss="AOSS";
aoss.service="AOSS Service";
aoss.enable="Enable AOSS";
aoss.start="Start AOSS Negotiation";
aoss.securitymodes="Security Modes";
aoss.wpaaes="WPA AES";
aoss.wpatkip="WPA TKIP";
aoss.wep="WEP 64-bit / 128-bit";
aoss.client_name="Client Name";
aoss.security="Security";
aoss.connectivity="Connectivity";
aoss.clients="AOSS Clients";
aoss.notice="NOTICE";
aoss.ap_mode_notice="NOTICE: AOSS can only be used when the primary radio is configured as AP or WDS AP.";
aoss.wep_notice="WEP security mode is not secure, therefore the use of WEP is not recommended.";
aoss.wep_info="(required for most gaming consoles supporting AOSS)";
aoss.wps="WPS Setup";
aoss.wps_ap_pin="WPS Gateway PIN (Label)";
aoss.wpspin="WPS Client PIN";
aoss.wpsactivate="Activate PIN";
aoss.wpsregister="Register PIN";
aoss.wpsgenerate="Generate PIN";
aoss.pinnotvalid="Invalid PIN, checksum not correct!";
aoss.wpsenable="WPS Button";
aoss.wpsstatus="WPS Status";
aoss.externalregistrar="PIN Method";
aoss.release="Release";
aoss.configure="Configure";

var olupgrade=new Object();
olupgrade.avail_updates="Available Updates";
olupgrade.version="Version";
olupgrade.release="Release";
olupgrade.readme="Readme";
olupgrade.choose="Choose";
olupgrade.retrieve_error="Error retrieving update information";

var nintendo=new Object();
nintendo.titl="Nintendo";
nintendo.spotpass=new Object();
nintendo.spotpass.titl="Nintendo SpotPass";
nintendo.spotpass.enable="Enable Nintendo SpotPass";
nintendo.spotpass.servers="Allow Servers";

var sec80211x=new Object();
sec80211x.xsuptype="Xsupplicant Type";
sec80211x.keyxchng="EAP Key-Management";
sec80211x.servercertif="Public Server Certificate";
sec80211x.clientcertif="Client Certificate";
sec80211x.phase1="Phase1";
sec80211x.phase2="Phase2";
sec80211x.anon="Anonymous Identity";
sec80211x.options="Additional Network Options";
sec80211x.leap="EAP-LEAP Settings";
sec80211x.peap="EAP-PEAP Settings";
sec80211x.tls="EAP-TLS Settings";
sec80211x.ttls="EAP-TTLS Settings";
//help container
var hwpa=new Object();
hwpa.right2="You may choose from; Disabled, WEP, WPA Personal, WPA Enterprise or RADIUS. All devices on your network must use the same security mode. With N-Mode you must use WPA2/AES.";

//help page
hwpa.page1="<dd>The router supports different types of security settings for your network. WiFi Protected Access (WPA), WiFi Protected Access 2 (WPA2), Remote Access Dial In User Service (RADIUS), and Wired Equivalent Privacy (WEP), which can be selected from the list next to Security Mode. To disable security settings, keep the default setting, <i>Disabled</i>.</dd>";
hwpa.page2="<dd>TKIP stands for Temporal Key Integrity Protocol, which utilizes a stronger encryption method than WEP, and incorporates Message Integrity Code (MIC) to provide protection against packet tampering. AES stands for Advanced Encryption System, which utilizes a symmetric 128-bit block data encryption and MIC. You should choose AES if your wireless clients supports it.<br /><br />To use WPA Personal, enter a password in the <i>WPA Shared Key</i> field between 8 and 63 characters long. You may also enter a <i>Group Key Renewal Interval</i> time between 0 and 99,999 seconds.</dd>";
hwpa.page3="<dd>WPA Enterprise uses an external RADIUS server to perform user authentication. To use WPA RADIUS, enter the IP address of the RADIUS server, the RADIUS Port (default is 1812) and the shared secret from the RADIUS server.</dd>";
hwpa.page4="<dd>WPA2 uses 802.11i to provide additional security beyond what is provided in WPA. AES is required under WPA2, and you may need additional updates to your OS and/or wireless drivers for WPA2 support. Please note WPA2/TKIP is not a supported configuration. Aditionally the WPA2 security mode is not supported under WDS.</dd>";
hwpa.page5="<dd>This mode allows for mixing WPA2 and WPA clients. If only some of your clients support WPA2 mode, then you should choose WPA2-PSK/WPA-PSK. For maximum interoperability, you should choose WPA2-PSK/WPA-PSK with TKIP+CCMP (AES).</dd>";
hwpa.page6="<dd>RADIUS utilizes either a RADIUS server for authentication or WEP for data encryption. To utilize RADIUS, enter the IP address of the RADIUS server and the associated shared secret. Select the desired encryption bit (64 or 128) for WEP and enter either a passphrase or a manual WEP key.</dd>";
hwpa.page7="<dd>There are two levels of WEP encryption, 64-bit and 128-bit. To utilize WEP, select the desired encryption bit, and enter a passphrase or up to four WEP key in hexadecimal format. If you are using 64-bit (40-bit), then each key must consist of exactly 10 hexadecimal characters. For 128-bit, each key must consist of exactly 26 hexadecimal characters. Valid hexadecimal characters are \"0\"-\"9\" and \"A\"-\"F\". Check your wireless clients to see which encryption level it supports.<br /><br />Use of WEP is discouraged due to security weaknesses, and one of the WPA modes should be used whenever possible. Only use WEP if you have clients that can only support WEP (usually older, 802.11b-only clients).</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** WL_FilterTable.asp **//
var wl_filter=new Object();
wl_filter.titl="MAC Address Filter List";
wl_filter.h2="MAC Address Filter List";
wl_filter.h3="Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";

// ** WL_ActiveTable.asp **//
var wl_active=new Object();
wl_active.titl="Wireless Active Client MAC List";
wl_active.h2="Wireless Client MAC List";
wl_active.h3="Enable MAC Filter";
wl_active.active="Active Clients";
wl_active.inactive="Inactive Clients";

// ** Wireless_WDS.asp **//
var wds=new Object();
wds.titl="WDS";
wds.h2="Wireless Distribution System";
wds.legend="WDS Settings";
wds.label="Lazy WDS";
wds.label2="WDS Subnet";
wds.wl_mac="Wireless MAC";
wds.lazy_default="Default: Disable";
wds.nat1="WLAN->WDS";
wds.nat2="WDS->WLAN";
wds.subnet="Subnet";
wds.legend2="Extra Options";

//help page
var hwds=new Object();
hwds.page1="<dd>WDS (Wireless Distribution System) is a Wireless Access Point mode that enables wireless bridging in which WDS APs communicate only with each other only (without allowing for wireless clients or stations to access them), and/or wireless repeating in which APs communicate both with each other and with wireless stations (at the expense of half the throughput). This firmware currently supports two types of WDS, LAN and Point to Point.</dd><dt>LAN-type WDS</dt><dd>This is the easiest, and currently most common, type of WDS used for linking LANs. It is very simple to setup and requires no extra routing protocols or knowledge of networking. Simply put, it is pure bridging. A simple example would be extending the range of an existing AP by setting up a 2nd AP and connecting it to the first using LAN-type WDS.<ol class=\"wide\"><li>Make sure you are using the same <a href=\"HWireless.asp\">Wireless Settings</a> on both routers and not any type of <a href=\"HWPA.asp\">" + wpa.titl + "</a>.</li><li>Find a dropdown selection that has <i>Disabled</i> displayed. Click this and select <i>LAN</i>, do the same on the other router.</li><li>On the first router, take the numbers next to <i>Wireless MAC</i> and enter them in to the second router on the same line that you set to \"LAN\".</li><li>Take the Wireless MAC from the second router and enter them on the first router.</li><li>Check for any typing errors and then click the <em>" + sbutton.save + "</em> button.</li><li>Go to the <a href=\"HStatusWireless.asp\">Wireless Status</a> page. You should see <i>WDS Link</i> and the Wireless MAC of the other router listed, with a signal reading. If the signal is \"0dBm\" then there may be something wrong. Check your antenna connections and configuration settings, and try again.</li><li>Once you have a good signal (-70dBm to -30dBm, -70dBm being lowest), you can change the <i>Internet Connection Type</i> on the <a href=\"HSetup.asp\">Basic Setup</a> page of the second router to <i>Disabled</i> and set the <em>Gateway</em> to the LAN IP Address of the first router. You can now run normal tests to check if you are connected (like <tt>ping</tt>).</li></ol><br /><div class=\"note\"><h4>Note:</h4><div>WDS is only available in <em>AP</em> mode. Also Wireless encryption <em>WPA2</em> and Wireless network mode <em>B-Only</em> are not supported under WDS.</div></div></dd>";

// ** Wireless_radauth.asp **//
var radius=new Object();
radius.titl="RADIUS";
radius.h2="Remote Authentication Dial-In User Service";
radius.legend="RADIUS";
radius.retry="Primary Server Retry Limit";
radius.label="Enable MAC Client";
radius.label2="MAC Format";
radius.label3="Auth Server Address";
radius.label4="Auth Server Port";
radius.label7="Auth Shared Secret";

radius.label23="Auth Backup Server Address";
radius.label24="Auth Backup Server Port";
radius.label27="Auth Backup Shared Secret";

radius.label5="Maximum Unauthenticated Users";
radius.label6="Password Format";
radius.label8="Override if Server is Unavailable";
radius.label13="Acct Server Address";
radius.label14="Acct Server Port";
radius.label17="Acct Shared Secret";
radius.label18="Accounting";
radius.local_ip="Force Client IP";

// help page
var hradauth=new Object();
hradauth.page1="<dd>RADIUS is a security service for authenticating and authorizing dial-up users. A typical enterprise network may have an access server attached to a modem pool, along with a RADIUS server to provide authentication services. Remote users dial into the access server, and the access server sends authentication requests to the RADIUS server. The RADIUS server authenticates users and authorizes access to internal network resources. Remote users are clients to the access server and the access server is a client to the RADIUS server.<br /><br /><div class=\"note\"><h4>Note:</h4><div>RADIUS is only available in <em>AP</em> mode.</div></div></dd>";
hradauth.page2="<dd>When sending the authentication request to the RADIUS server, the wireless client use the MAC address as the username. This would be received by the RADIUS server in the following format :<ul class=\"wide\"><li>aabbcc-ddeeff</li><li>aabbccddeeff</li><li>aa-bb-cc-dd-ee-ff</li></ul></dd> ";
hradauth.page3="<dd>The RADIUS server IP address and TCP port.</dd>";
hradauth.page4="<dd>Sets a amount of users which ran access without any valid RADIUS authentication</dd>";
hradauth.page5="<dd>Sets the property which RADIUS password should be used, the shared key or the mac address itself</dd>";
hradauth.page6="<dd>Transactions between the client and RADIUS accounting server are authenticated through the use of a shared secret, which is never sent over the network.</dd>";
hradauth.page7="<dd>If the RADIUS server becomes unavailable then the authentication will be disabled until the server becomes available. This allows wireless remote administration of an Access Point in fail scenarios.</dd>";

// ** Wireless_MAC.asp **//
var wl_mac=new Object();
wl_mac.titl="MAC Filter";
wl_mac.h2="Wireless MAC Filter";
wl_mac.legend="MAC Filter";
wl_mac.label="Enable Filter";
wl_mac.label2="Filter Mode";
wl_mac.deny="Blacklisted Network Clients";
wl_mac.allow="Whitelisted Network Clients";

//help page
var hwl_mac=new Object();
hwl_mac.page1="<dd>The Wireless MAC Filter allows you to control which wireless equipped computers/devices may or may not communicate with the router depending on their MAC addresses. For information how to get MAC addresses from computers, see <a href=\"HWanMAC.asp\">MAC Address Cloning</a> for detailed instructions.</dd><dd>To set up a filter, click <em>" + share.enable + "</em>, and follow these example instructions:<ol class=\"wide\"><li>If you want to block specific wireless-equipped devices from communicating with the router, then keep the default setting, <i>" + wl_mac.deny + "</i>. If you want to allow specific wireless-equipped devices to communicate with the router, then click the radio button next to <i>" + wl_mac.allow + "</i>.</li><li>Click the <em>" + sbutton.filterMac + "</em> button. Enter the appropriate MAC addresses into the MAC fields.<br /><br /><div class=\"note\"><h4>Note:</h4><div>The MAC address should be entered in this format: xx:xx:xx:xx:xx:xx (the x's represent the actual characters of the target MAC address).</div></div></li><li>Click the <em>" + sbutton.save + "</em> button to save your changes. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes. Click the <em>" + sbutton.clos + "</em> button to return to the previous screen without saving changes.</li></ol><br />To disable the Wireless MAC Filter, keep the default setting, <em>" + share.disable + "</em>.</dd>";

// ** WiMAX
var wl_wimax=new Object();
wl_wimax.titl="WiMAX";
wl_wimax.h2="Worldwide Interoperability for Microwave Access";
wl_wimax.downstream="Downstream Frequency";
wl_wimax.upstream="Upstream Frequency";
wl_wimax.width="Channel Width";
wl_wimax.duplex="Duplex Mode";
wl_wimax.mode="Operation Mode";
wl_wimax.mac="Subscriber MAC Address";

// ** Gpio **//
var gpio=new Object();
gpio.titl="GPIO Inputs / Outputs";
gpio.h2="GPIO Inputs / Outputs";
gpio.oplegend="GPIO Outputs";
gpio.iplegend="GPIO Inputs";

// ** FreeRadius.asp **//
var freeradius=new Object();
freeradius.titl="FreeRADIUS";
freeradius.h2="FreeRADIUS";
freeradius.legend="FreeRADIUS Server";
freeradius.srv="Enable Server";
freeradius.port="Port";
freeradius.certificate="Server Certificate";
freeradius.cert="Generate Certificate";
freeradius.certdown="Download";
freeradius.clientcert="Client Certificates";
freeradius.settings="Settings";
freeradius.users="Users";
freeradius.clients="Clients";
freeradius.username="Username";
freeradius.password="Password";
freeradius.downstream="Down Speed";
freeradius.upstream="Up Speed";
freeradius.sharedkey="Shared Key";

freeradius.countrycode="Country Code";
freeradius.state="State or Province";
freeradius.locality="Locality";
freeradius.organisation="Organisation / Company";
freeradius.email="Email Address";
freeradius.common="Common Certificate Name";
freeradius.expiration="Expires (Days)";
freeradius.passphrase="Passphrase";

//freeradius.generate="Generate Certificate";
freeradius.cert_status="Certificate Status";
freeradius.certtbl="Certificate";
freeradius.gencertime="Generating %d%%, this may take a while to complete...";
freeradius.gencerdone="Certicate generation completed!";

//help container
var hfreeradius=new Object();
hfreeradius.right2="Before starting the FreeRADIUS Server, JFFS2 support <b>must be enabled</b>. JFFS2 Support is located on the Administration / Management tab.";

// ** Wireless_Advanced.asp **//
var wl_adv=new Object();
wl_adv.titl="Advanced Wireless Settings";
wl_adv.h2="Advanced Wireless Settings";
wl_adv.legend="Advanced Settings";
wl_adv.legend2="Wireless Multimedia Support Settings";
wl_adv.label="Authentication Type";
wl_adv.label2="Basic Rate";
wl_adv.label3="Transmission Fixed Rate";
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
wl_adv.label16="Afterburner";
wl_adv.label17="Web UI Access";
wl_adv.label18="WMM Support";
wl_adv.label19="No-Acknowledgement";
wl_adv.label20="Shortslot Override";
wl_adv.label21="Transmission Maximum Rate";
wl_adv.label23="Transmission Minimum Rate";
wl_adv.label22="Bluetooth Coexistence Mode";
wl_adv.label24="Antenna Alignment";
wl_adv.label25="Antenna Output";
wl_adv.table1="EDCA AP Parameters (AP to Client)";

wl_adv.txchainmask="TX Antenna Chains";
wl_adv.rxchainmask="RX Antenna Chains";

wl_adv.droplowsignal="Drop Clients with Low Signal";
wl_adv.connect="Minimum Signal for authenticate";
wl_adv.stay="Minimum Signal for connection";
wl_adv.poll_time="Poll Time for signal lookup";
wl_adv.strikes="Amount of allowed low signals";

wl_adv.bgscan="Background Scan";
wl_adv.bgscan_mode="Mode";
wl_adv.bgscan_simple="Simple";
wl_adv.bgscan_learn="Learn";
wl_adv.bgscan_short_int="Short interval";
wl_adv.bgscan_threshold="Signal threshold";
wl_adv.bgscan_long_int="Long interval";

wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";

wl_adv.table3="WMM Tx retry limits, fallback limits and max rate parameters.";
wl_adv.txpcol1="S. Retry";
wl_adv.txpcol2="S. Fallbk";
wl_adv.txpcol3="L. Retry";
wl_adv.txpcol4="L. Fallbk";
wl_adv.txpcol5="Max Rate";
wl_adv.txprow1="AC BE TX Parameters";
wl_adv.txprow2="AC BK TX Parameters";
wl_adv.txprow3="AC VI TX Parameters";
wl_adv.txprow4="AC VO TX Parameters";

wl_adv.col6="Admin Forced";
wl_adv.row1="Background";
wl_adv.row2="Best Effort";
wl_adv.row3="Video";
wl_adv.row4="Voice";
wl_adv.table2="EDCA STA Parameters (Client to AP)";
wl_adv.lng="Long"; 					//************* don't use .long! *************//
wl_adv.shrt="Short"; 				//************* don't use .short! **************//
wl_adv.mbps="Mbit/s";

//help container
var hwl_adv=new Object();
hwl_adv.right2="You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.";

//help page
hwl_adv.page1="<dd>The Wireless Advanced Settings screen allows you to customize data transmission settings. In most cases, the advanced settings on this screen should remain at their default values.</dd>";
hwl_adv.page2="<dd>The default is set to <i>Auto</i>, which allows either Open System or Shared Key authentication to be used. For Open System authentication, the sender and the recipient do NOT use a WEP key for authentication. For Shared Key authentication, the sender and recipient use a WEP key for authentication. If you want to use only Shared Key authentication, then select <i>Shared Key</i>.</dd>";
hwl_adv.page3="<dd>The default value is set to <i>Default</i>. Depending on the wireless mode you have selected, a default set of supported data rates will be selected. The default setting will ensure maximum compatibility with all devices. You may also choose to enable all data rates by selecting <i>ALL</i>. For compatibility with older Wireless-B devices, select <i>1-2&nbsp;Mbit/s</i>.</dd>";
hwl_adv.page4="<dd>The default setting is <i>Auto</i>. The range is from 1 to 54&nbsp;Mbit/s. The rate of data transmission should be set depending on the speed of your wireless network. You can select from a range of transmission speeds, or keep the default setting, <i>Auto</i>, to have the router automatically use the fastest possible data rate and enable the Auto-Fallback feature. Auto-Fallback will negotiate the best possible connection speed between the router and a wireless client.</dd>";
hwl_adv.page5="<dd>The default value is <i>Disabled</i>. When set to <i>Auto</i>, a protection mechanism will ensure that your Wireless-B devices will connect to the Wireless-G router when many Wireless-G devices are present. However, performance of your Wireless-G devices may be decreased.</dd>";
hwl_adv.page6="<dd>The default value is <i>Disabled</i>. Frame burst allows packet bursting which will increase overall network speed though this is only recommended for approx 1-3 wireless clients, Anymore clients and there can be a negative result and throughput will be affected.</dd>";
hwl_adv.page7="<dd>The default value is 100. Enter a value between 1 and 65,535 milliseconds. The Beacon Interval value indicates the frequency interval of the beacon. A beacon is a packet broadcast by the router to synchronize the wireless network. 50 is recommended in poor reception.</dd>";
hwl_adv.page8="<dd>The default value is 1. This value, between 1 and 255, indicates the interval of the Delivery Traffic Indication Message (DTIM). A DTIM field is a countdown field informing clients of the next window for listening to broadcast and multicast messages. When the router has buffered broadcast or multicast messages for associated clients, it sends the next DTIM with a DTIM Interval value. Its clients hear the beacons and awaken to receive the broadcast and multicast messages.</dd>";
hwl_adv.page9="<dd>This value should remain at the default setting of 2346. The range is 256-2346 bytes. It specifies the maximum size for a packet before data is fragmented into multiple packets. If you experience a high packet error rate, you may slightly increase the Fragmentation Threshold. Setting the Fragmentation Threshold too low may result in poor network performance. Only minor modifications of this value are recommended.</dd>";
hwl_adv.page10="<dd>This value should remain at the default setting of 2347. The range is 0-2347 bytes. Should you encounter inconsistent data flow, only minor modifications are recommended. If a network packet is smaller than the preset RTS threshold size, the RTS/CTS mechanism will not be enabled. The router sends Request to Send (RTS) frames to a particular receiving station and negotiates the sending of a data frame. After receiving an RTS, the wireless station responds with a Clear to Send (CTS) frame to acknowledge the right to begin transmission.</dd>";
hwl_adv.page11="<dd>The default value is <i>Off</i>. This setting isolates wireless clients so access to and from other wireless clients are stopped.</dd>";
hwl_adv.page12="<dd>Values are <i>Auto</i>, <i>Left</i>, <i>Right</i>, default value is <i>Auto</i>. This is used in conjunction with external antennas to give them optimum performance. On some router models left and right antennas may be reversed depending on you point of view.</dd>";
hwl_adv.page13="<dd>Values are <i>Long</i> and <i>Short</i>, default value is <i>Long</i>. If your wireless device supports the short preamble and you are having trouble getting it to communicate with other 802.11b devices, make sure that it is set to use the long preamble.</dd>";
hwl_adv.page14="<dd>This value ranges from 1 - 1000 mW. Default txpwr is chosen in order to comply with regulation domains as well as wireless stability. Higher power settings are not recommended for users due to excess heat generated by the radio chipset, which can affect the life of the router.</dd>";
hwl_adv.page15="<dd>The default value is <i>Off</i>. This should only be used with WRT54GS Models and only in conjunction with other Linksys \"GS\" wireless clients that also support Linksys \"Speedbooster\" technology.</dd>";
hwl_adv.page16="<dd>The default value is <i>Enabled</i>. The setting allows access to the routers setup Web interface from wireless clients. Disable this if you wish to block all wireless clients from accessing the setup pages.</dd>";
hwl_adv.page17="<dd>The <em>Radio Times Restriction</em> facility constitutes a time switch for the radio. By default, the time switch is not active and the WLAN is permanently on. Enable the time switch, if you want to turn off the WLAN during some hours of the day. Hours during which the WLAN is on are marked in green, while red indicates that the radio is off. Clicking on the respective hour toggles between on and off.</dd>";
hwl_adv.page18="<dd>Enable support of WiFi Multimedia feature. Configuring QoS options consists of setting parameters on existing queues for different types of wireless traffic. You can configure different minimum and maximum wait times for the transmission of packets in each queue based on the requirements of the media being sent. Queues automatically provide minimum transmission delay for Voice, Video, multimedia, and mission critical applications, and rely on best-effort parameters for traditional IP data.<br /><br /><div class=\"note\"><h4>Note:</h4><div>As an Example, time-sensitive Voice & Video, and multimedia are given effectively higher priority for transmission (lower wait times for channel access), while other applications and traditional IP data which are less time-sensitive but often more data-intensive are expected to tolerate longer wait times.</div></div></dd>";
hwl_adv.page19="<dd>This refers to the acknowledge policy used at the MAC level. Enabling no-acknowledgement can result in more efficient throughput but higher error rates in a noisy Radio Frequency (RF) environment.</dd>";
hwl_adv.page20="<dd>This affects traffic flowing from the access point to the client station.</dd>";
hwl_adv.page21="<dd>This affects traffic flowing from the client station to the access point.</dd>";
hwl_adv.page22="<dd>Priority is low.<br />High throughput. Bulk data that requires maximum throughput and is not time-sensitive is sent to this queue (FTP data, for example).</dd>";
hwl_adv.page23="<dd>Priority is Medium.<br />Medium throughput and delay. Most traditional IP data is sent to this queue.</dd>";
hwl_adv.page24="<dd>Priority is High.<br />Minimum delay. Time-sensitive video data is automatically sent to this queue.</dd>";
hwl_adv.page25="<dd>Minimum Contention Window. This parameter is input to the algorithm that determines the initial random backoff wait time (\"window\") for retry of a transmission. The value specified here in the Minimum Contention Window is the upper limit (in milliseconds) of a range from which the initial random backoff wait time is determined.<br />The first random number generated will be a number between 0 and the number specified here. If the first random backoff wait time expires before the data frame is sent, a retry counter is incremented and the random backoff value (window) is doubled. Doubling will continue until the size of the random backoff value reaches the number defined in the Maximum Contention Window. Valid values for the \"CWmin\" are 1, 3, 7, 15, 31, 63, 127, 255, 511, or 1024. The value for \"CWmin\" must be lower than the value for \"CWmax\".</dd>";
hwl_adv.page26="<dd>Maximum Contention Window. The value specified here in the Maximum Contention Window is the upper limit (in milliseconds) for the doubling of the random backoff value. This doubling continues until either the data frame is sent or the Maximum Contention Window size is reached. Once the Maximum Contention Window size is reached, retries will continue until a maximum number of retries allowed is reached. Valid values for the \"CWmax\" are 1, 3, 7, 15, 31, 63, 127, 255, 511, or 1024. The value for \"CWmax\" must be higher than the value for \"CWmin\".</dd>";
hwl_adv.page27="<dd>The Arbitration Inter-Frame Spacing Number specifies a wait time (in milliseconds) for data frames.</dd>";
hwl_adv.page28="<dd>Transmission Opportunity for \"a\" \"b\" and \"g\" modes is an interval of time when a WME AP has the right to initiate transmissions onto the wireless medium (WM). This value specifies (in milliseconds) the Transmission Opportunity (TXOP) for the AP; that is, the interval of time when the WMM AP has the right to initiate transmissions on the wireless network.</dd><dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your changes. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

// ** Wireless_Basic.asp **//
var wl_basic=new Object();
wl_basic.titl="Wireless";
wl_basic.h2="Wireless";
wl_basic.cardtype="Card Type";
wl_basic.chipset="Chipset";
wl_basic.legend="Basic Settings";
wl_basic.label="Radio Mode";
wl_basic.label2="Network Mode";
wl_basic.ssid="Service Set Identifier (SSID)";
wl_basic.label4="Channel";
wl_basic.bssid="Basic Service Set Identifier (BSSID)";
wl_basic.vht80p80chan="Wireless Channel 2 (VHT80+80)";
wl_basic.label5="SSID Broadcast";
wl_basic.label6="Sensitivity Range / ACK Timing";
wl_basic.label7="802.11n Transmission Mode";
wl_basic.igmpsnooping="Optimize Multicast Traffic";
wl_basic.overlap="Allow Channel Overlapping";
wl_basic.turboqam="TurboQAM (QAM256)";
wl_basic.nitroqam="NitroQAM (QAM1024)";
wl_basic.dwds="Dynamic WDS Mode";
wl_basic.scanlist="ScanList";
wl_basic.duallink="Dual Link";
wl_basic.dualband="Dual-Band";
wl_basic.parent="Parent IP";
wl_basic.masquerade="Masquerade / NAT";
wl_basic.ap="AP";
wl_basic.client="Station";
wl_basic.repeater="Repeater";
wl_basic.repeaterbridge="Repeater Bridge";
wl_basic.clientBridge="Station Bridge";
wl_basic.clientRelayd="Station Bridge (Routed)";
wl_basic.adhoc="Ad-Hoc";
wl_basic.wdssta="Station (WDS)";
wl_basic.wdssta_mtik="Station (Mikrotik WDS)";
wl_basic.wdsap="AP (WDS)";
wl_basic.mesh="Mesh / 802.11s";
wl_basic.fw_type="Firmware Type";
wl_basic.tdma="TDMA";
wl_basic.mixed="Mixed";
wl_basic.bft="Explicit Beamforming";
wl_basic.bfr="Implicit Beamforming";
wl_basic.subf="Single User Beamforming";
wl_basic.mubf="Multi User Beamforming";
wl_basic.atf="Airtime Fairness";
wl_basic.fc="Frame Compression";
wl_basic.shortgi="Short GI";
wl_basic.greenfield="Greenfield";
wl_basic.preamble="Short Preamble";
wl_basic.clientRelaydDefaultGwMode="Default GW Mode";
wl_basic.b="B Only";
wl_basic.a="A Only";
wl_basic.ac="AC Only";
wl_basic.ad="AD Only";
wl_basic.acn="AC / N Mixed";
wl_basic.ax="AX Only";
wl_basic.xacn="AX / AC / N Mixed";
wl_basic.na="N / A Mixed";
wl_basic.ng="N / G Mixed";
wl_basic.n5="N Only - 5 GHz";
wl_basic.n2="N Only - 2.4 GHz";
wl_basic.n="N Only";
wl_basic.g="G Only";
wl_basic.bg="B / G Mixed";
wl_basic.rts="RTS Threshold";
wl_basic.rtsvalue="Threshold";
wl_basic.protmode="Protection Mode";
wl_basic.legend2="Radio Time Restrictions";
wl_basic.radio="Radio Status";
wl_basic.radiotimer="Radio Scheduling";
wl_basic.radio_on="Active";
wl_basic.radio_off="Inactive";
wl_basic.h2_v24="Wireless Interface";
wl_basic.h2_vi="Virtual Interfaces";
wl_basic.regdom="Regulatory Domain";
wl_basic.regdom_label="Domain";
wl_basic.regmode="Mode";
wl_basic.tpcdb="TPC Mitigation Factor";
wl_basic.TXpower="TX Power";
wl_basic.TXpowerFcc="TX Peak Power (FCC)";
wl_basic.power_override="Override Chipset Restrictions";
wl_basic.AntGain="Antenna Gain";
wl_basic.diversity="Diversity";
wl_basic.primary="Primary";
wl_basic.secondary="Secondary";
wl_basic.vertical="Vertical";
wl_basic.horizontal="Horizontal";
wl_basic.adaptive="Adaptive";
wl_basic.internal="Internal";
wl_basic.external="External";
wl_basic.ghz24="2.4 GHz Output";
wl_basic.ghz5="5 GHz Output";
wl_basic.bghz24="2.4 GHz";
wl_basic.bghz5="5 GHz";
wl_basic.network="Network Configuration";
wl_basic.unbridged="Unbridged";
wl_basic.bridged="Bridged";
wl_basic.turbo="Turbo Mode";
wl_basic.extrange="Extended Range";
wl_basic.supergcomp="Super G Compression";
wl_basic.supergff="Super G Fast Framing";
wl_basic.extchannel="Extended Channel Mode";
wl_basic.outband="Outdoor Band";
wl_basic.channel_width="Channel Width";
wl_basic.channel_wide="Extension Channel";
wl_basic.regulatory="SuperChannel";
wl_basic.chanshift="Channel Shifting";
wl_basic.specialmode="2.3 GHz Mode";
wl_basic.wifi_bonding="WiFi Bonding";
wl_basic.sifstime="OFDM SIFS Time";
wl_basic.preambletime="OFDM Preamble Time";
wl_basic.multicast="Multicast Forwarding";
wl_basic.intmit="Noise Immunity";
wl_basic.dynamic_auto_bursting="Dynamic Auto Bursting";
wl_basic.qboost_tdma="QCA Q-Boost / TDMA";
wl_basic.qboost="QCA Q-Boost";
wl_basic.sifs_trigger_time="SIFS Trigger Time";
wl_basic.noise_immunity="Noise Immunity Level";
wl_basic.ofdm_weak_det="OFDM Weak Detection";
wl_basic.radar="Radar Detection";
wl_basic.mtikie="MTik Compatibility";
wl_basic.csma="Carrier Sense";
wl_basic.if_label="Label (optional)";
wl_basic.if_info="Info (optional)";
wl_basic.advanced_options="Advanced Options";
wl_basic.rate_control="Rate Control Algorithm";
wl_basic.ap83_vap_note="Adding more than three virtual interfaces will lead to lower performance with some specific client devices connected to these virtual interfaces.";
wl_basic.isolation="Net Isolation";
wl_basic.tor_anon="Tor Anonymization";
wl_basic.country_settings="Country Settings";
wl_basic.ch_pos_auto="Auto";
wl_basic.ch_pos_upr="Upper";
wl_basic.ch_pos_lwr="Lower";
wl_basic.ch_pos_ll="LL (-6)";
wl_basic.ch_pos_lu="LU (-2)";
wl_basic.ch_pos_ul="UL (+2)";
wl_basic.ch_pos_uu="UU (+6)";
wl_basic.ch_pos_lll="LLL (-14)";
wl_basic.ch_pos_llu="LLU (-10)";
wl_basic.ch_pos_lul="LUL (-6)";
wl_basic.ch_pos_luu="LUU (-2)";
wl_basic.ch_pos_ull="ULL (+2)";
wl_basic.ch_pos_ulu="ULU (+6)";
wl_basic.ch_pos_uul="UUL (+10)";
wl_basic.ch_pos_uuu="UUU (+14)";
wl_basic.ch_pos_lwr="Lower";
wl_basic.ghz="GHz";
wl_basic.mhz="MHz";
wl_basic.tbqam="TurboQAM";
wl_basic.upper="upper";
wl_basic.lower="lower";
wl_basic.ldpc="LDPC (Disable for ESP8266)";
wl_basic.uapsd="U-APSD (Automatic Power Save)";
wl_basic.disassoc_low_ack="Disassoc Low ACK";
wl_basic.smps="Spatial Multiplex Power Save";
wl_basic.add="Add Virtual AP";
wl_basic.airtime_policy="Airtime Policy";
wl_basic.airtime_dynamic="Dynamic Mode";
wl_basic.airtime_limit="Limit Mode";
wl_basic.airtime_weight="Airtime Weight";
wl_basic.airtime_dolimit="Airtime Limit";
wl_basic.mesh_settings="802.11s Mesh Settings";
wl_basic.mesh_fwding="Layer 2 Forwarding";
wl_basic.mesh_retry_timeout="Retry Timeout";
wl_basic.mesh_confirm_timeout="Confirm Timeout";
wl_basic.mesh_holding_timeout="Holding Timeout";
wl_basic.mesh_max_peer_links="Maximum Peer Links";
wl_basic.mesh_max_retries="Maximum Retries";
wl_basic.mesh_ttl="TTL";
wl_basic.mesh_element_ttl="Element TTL";
wl_basic.mesh_auto_open_plinks="Auto Open Plinks";
wl_basic.mesh_hwmp_max_preq_retries="HWMP Maximum Preq Retries";
wl_basic.mesh_path_refresh_time="Path Refresh Time";
wl_basic.mesh_min_discovery_timeout="Minimum Discovery Timeout";
wl_basic.mesh_hwmp_active_path_timeout="HWMP Active Path Timeout";
wl_basic.mesh_hwmp_preq_min_interval="HWMP Preq Minimum Interval";
wl_basic.mesh_hwmp_net_diameter_traversal_time="HWMP Net Diameter Traversal Time";
wl_basic.mesh_hwmp_rootmode="HWMP Rootmode";
wl_basic.mesh_hwmp_rann_interval="HWMP Rann Interval";
wl_basic.mesh_gate_announcements="Gate Announcements";
wl_basic.mesh_sync_offset_max_neighor="Sync Offset Max Neighbor";
wl_basic.mesh_rssi_threshold="RSSI Threshold";
wl_basic.mesh_hwmp_active_path_to_root_timeout="HWMP Active Path to Root Timeout";
wl_basic.mesh_hwmp_root_interval="HWMP Root Interval";
wl_basic.mesh_hwmp_confirmation_interval="HWMP Confirmation Interval";
wl_basic.mesh_power_mode="Power Mode";
wl_basic.mesh_awake_window="Awake Window";
wl_basic.mesh_plink_timeout="Plink Timeout";
wl_basic.mesh_no_root="No Root";
wl_basic.mesh_preq_no_prep="Preq No Prep";
wl_basic.mesh_preq_with_prep="Preq with Prep";
wl_basic.mesh_rann="Rann";
wl_basic.mesh_active="Active";
wl_basic.mesh_deep="Deep Sleep";
wl_basic.mesh_light="Light Sleep";
wl_basic.mesh_nolearn="Avoid Multi-Hop Path Discovery";
wl_basic.mesh_connected_to_gate="Announce Connected to Gate";
wl_basic.mesh_connected_to_as="Announce Connected to Auth-Server";
wl_basic.mesh_header_cache_size="Header Cache Size";
wl_basic.he_bss_color="BSS Color";

//help container
var hwl_basic=new Object();
hwl_basic.right2="<b>Attention:</b> It is recommended that you click the <em>" + sbutton.apply + "</em> button after you change a value in order to update the fields with the corresponding parameters.";
hwl_basic.right3="Sensitivity Range: ";
hwl_basic.right4="Adjusts the ACK timing. <b>0</b> disables ACK timing completely on Broadcom based firmware. On Atheros based firmware, <b>0</b> enables auto ACK timing mode.";
hwl_basic.right6="Click any hour to enable or disable the radio signal (<em>green</em> indicates allowed Wireless access, and <em>red</em> indicates blocked Wireless access)";

//help page
hwl_basic.page1="<dd>The wireless part of your router can run in different modes:<ul class=\"wide\"><li>AP mode – This is the default mode, also called Infrastructure mode. Your router acts as an central connection point, which wireless clients can connect to.</li><li>Client mode – The radio interface is used to connect the Internet-facing side of the router (i.e., the WAN) as a client to a remote access point. NAT or routing are performed between WAN and LAN, like in \"normal\" gateway or router mode. Use this mode, e.g. if your Internet connection is provided by a remote access point, and you want to connect a subnet of your own to it. </li><li>Client Bridged (Routed) mode – The radio interface is used to connect the LAN side of the router to a remote access point. The LAN and the remote AP will be in the same subnet (This is called a \"bridge\" between two network segments). The WAN side of the router is unused and can be disabled. Use this mode, e.g., to make the router act as a \"WLAN adapter\" for a device connected to one of the LAN Ethernet ports.</li><li>Ad-Hoc mode – This is for peer to peer wireless connections. Clients running in Ad-Hoc mode can connect to each other as required without involving central access points.</li></ul><br /><div class=\"note\"><h4>Note:</h4><div>Note that <a href=\"HWDS.asp\">WDS</a> is only available in AP mode.</div></div></dd>";
hwl_basic.page2="<dd><b>2.4 Ghz radio:</b> For a mixed network with 802.11b/g/n devices, keep the default setting <i>Mixed</i>. For 802.11n devices, select <i>N Only</i>. For 802.11g/n devices only, select <i>N /G Mixed</i>. For 802.11g devices only, select <i>G Only</i>. If you want to disable wireless networking, select <em>" + share.disable + "</em>.<br /><br /><div class=\"note\"><h4>Note:</h4><div><i>B Only</i> mode <b>is not supported under WDS.</b></div></dd>";
hwl_basic.page2_5g="<dd><b>5 Ghz radio:</b> For a mixed network with 802.11ac/n/a devices, keep the default setting <i>Mixed</i>. For 802.11a devices, select <i>A Only</i>. For Mixed 802.11a/n devices, select <i>N / A Mixed</i>. For 5 Ghz 802.11n devices, select <i>N Only 5 Ghz</i>. For mixed 802.11ac/n devices, select <i>AC / N Mixed</i>. For 802.11ac devices, select <i>AC Only</i>. If you wish to disable 5 Ghz networking, select <em>" + share.disable + "</em>.</dd>";
hwl_basic.page3="<dd>The SSID is the network name shared among all devices in a wireless network. The SSID must be identical for all devices in the wireless network. It is case-sensitive and must not exceed 32 alphanumeric characters, which may be any keyboard character. Make sure this setting is the same for all devices in your wireless network.<br /><br /><div class=\"note\"><h4>Note:</h4><div>For added security, it is recommended to change the default SSID <tt>dd-wrt</tt> to a unique name of your choice.</div></div></dd>";
hwl_basic.page4="<dd>Select the appropriate channel from the list provided to correspond with your network settings (in North America between channel 1 and 11, in Europe 1 and 13, in Japan all 14 channels). All devices in your wireless network must use the same channel in order to function correctly. Try to avoid conflicts with other wireless networks by choosing a channel where the upper and lower three channels are not in use.</dd>";
hwl_basic.page5="<dd>When wireless clients survey the local area for wireless networks to associate with, they will detect the SSID broadcast by the router. To broadcast the router SSID, keep the default setting, <i>Enable</i>. If you do not want to broadcast the router SSID, then select <em>" + share.disable + "</em>.</dd>";
hwl_basic.page6="<dd>Adjusts the ACK timing based on the maximum distance in meters<br /><div class=\"note\"><h4>Note:</h4><div>On earlier Atheros based DD-WRT firmwares, 0 would enable auto ACK mode instead of disabling it.</div></div><ul class=\"wide\"><li> 0 disables ACK timing completely (0 = No-ACK mode)</li><li> 1 - 999999 adjusts ACK timing</li></ul></dd>";
hwl_basic.page7="<dd>Check all values and click the <em>" + sbutton.save + "</em> button to save your settings. Click the <em>" + sbutton.cancel + "</em> button to cancel your unsaved changes.</dd>";

hwl_basic.antaira="<dt class=\"term\"><b>AP mode</b></dt><dd class=\"definition\">This is the default mode, also called Infrastructure mode</dd><dt class=\"term\"><b>Client mode</b></dt><dd class=\"definition\">The radio is used to connect wirelessly to another AP</dd><dt class=\"term\"><b>Client bridged (Routed no NAT) mode</b></dt><dd class=\"definition\">The radio is used to connect wirelessly to another AP</dd><dt class=\"term\"><b>Ad-Hoc mode</b></dt><dd class=\"definition\">This is for peer to peer wireless connections.</dd><dt class=\"term\"><b>Wireless Channel</b></dt><dd class=\"definition\">Select the appropriate channel top operate on.</dd><dt class=\"term\"><b>SSID Broadcast</b></dt><dd class=\"definition\">When wireless clients survey the local area for wireless networks to associate with, they will detect the SSID broadcast by the router.</dd><dt class=\"term\"><b>Sensitivity Range (ACK Timing)</b></dt><dd class=\"definition\">Adjusts the ACK timing based on the maximum distance in meters.</dd> ";

// SuperChannel **//
var superchan=new Object();
superchan.legend="SuperChannel Activation";
superchan.h2feat="Feature";
superchan.featxt="<p>SuperChannel allows the use of special frequencies on <b>capable devices only</b>.<br />These frequencies range between 2192 Mhz to 2732 Mhz for the 802.11g band and 4915 Mhz to 6100 Mhz for the 802.11a band.<br /><b>This feature is not yet enabled.</b></p>";
superchan.h2disc="Disclaimer";
superchan.lgltxt="<p>Consider that in many countries it is not allowed to use these frequencies. DD-WRT / NewMedia-NET GmbH assumes no liability whatsoever, expressed or implied, for the use of this feature.</p>";
superchan.lsyskey="System Key";
superchan.lactkey="Activation Key";

// ** Fail_s.asp / Fail_u_s.asp / Fail.asp **//
var fail=new Object();
fail.mess1="The values you entered are invalid. Please try again.";
fail.mess2="Upgrade failed.";

// ** Success*.asp / Reboot.asp  **//
var success=new Object();
success.saved="Settings saved.";
success.restore="Settings restored.<br />Unit is rebooting now. Please wait a moment...";
success.upgrade="Upgrade successful.<br />Unit is rebooting now. Please wait a moment...";
success.success_noreboot="Settings were successful.";
success.success_reboot=success.success_noreboot + "<br />Unit is rebooting now. Please wait a moment...";

success.alert_reset="All configuration settings have been restored to their default values.<br /><br />";
success.alert1="Please check the following before connecting again:";
success.alert2="If you have changed your router's IP address, please note that you must release/renew your client(s) address(s) on the network.";
success.alert3="If you are connected via WLAN, please join the network and then click the <em>" + sbutton.continu + "</em> button.";

// ** Logout.asp  **//
var logout=new Object();
logout.message="You have successfully logged out.<br />Thank you for using DD-WRT!";

// ** Setup Assistant **//
var sas=new Object();
sas.title="Setup Assistant";
sas.internet_connection="Internet Connection";
sas.network_settings="Network Settings";
sas.wireless_settings="Wireless Settings";
sas.other_settings="Other Settings";
sas.hwan="WAN Setup";

var hsas=new Object();
hsas.wan="The WAN interface connects your router to the Internet or other networks. If your network is connected to the Internet and you only need an access point then set the WAN mode to \"Disabled\".";
hsas.h_routerip="Router IP";
hsas.routerip="This is the IP address assigned to the router in your internal network. If you change this address you also have to use it to access the Router Management after applying the changes.";
hsas.h_dhcp="DHCP";
hsas.dhcp="Computers and other networked devices can automatically obtain an IP address via DHCP in your local network without requiring manual configuration. In the event there is already a DHCP server running in your local network, please disable this option to avoid conflicts.";
hsas.h_wireless_physical="Wireless Radio Interface";
hsas.wireless_physical="The interface settings allow you to define the router's radio behavior. You can define the main operation mode (Access Point, Client or Repeater), change the wireless network name and change advanced settings like the channel width. In case you did change the wireless channel width from the 20 MHz standard please take care that your wireless clients do support the channel width and are configured properly.";
hsas.h_wireless_security="Wireless Security";
hsas.wireless_security="To make it easier for you to configure client devices you can change the wireless network password. Disabling encryption or changing it to WEP is not recommended for security reasons.";
hsas.h_routername="Router Name";
hsas.routername="This name is communicated to other devices within your network and allows for easier identification.";
hsas.networking="Networking help text";
hsas.wireless="Wireless help text";
hsas.other="Other Settings help text";

// ** AOSS **//
var haoss=new Object();
haoss.basic="The \"AirStation One-Touch Secure System\" (AOSS) allows you to connect AOSS capable clients to your access point without requiring manual configuration.";
haoss.securitymodes="AOSS security modes define what client security modes are accepted for AOSS negotiation. If a client only supports security modes that are not enabled, it cannot connect.";
haoss.wps="WPS enables support for WiFi Protected Setup using the button on your router or the PIN that came with your client device.";

var ias=new Object();
ias.title="Setup";
ias.card_info="Setup Card";
ias.edit_note="Click on any information in the Setup Card to edit it.";
ias.assistant="Run Setup Assistant";
ias.assistant_iptv="Run IPTV Setup";
ias.print_setup_card="Print Setup Card";
ias.print_guest_card="Print Guest Card";
ias.apply_changes="Apply Changes";
ias.wlnetwork="Wireless Network";
ias.wlinfo_2_4_GHz="(2.4 GHz) - 802.11n/g/b Compatible";
ias.wlinfo_5_GHz="(5 GHz) - 802.11n/a Compatible";
ias.hl_setup_card="Setup Card";
ias.hl_client_access="For Client Access";
ias.hl_for_conf="For Configuration";
ias.hl_guest_card="Guest Card";

// ** Speedchecker.asp **//
var speedchecker=new Object();
speedchecker.titl="Speedchecker";
speedchecker.legend="Speedchecker";
speedchecker.server="Speedchecker Service";
speedchecker.regtitle="Please share information with us:";
speedchecker.savemessage="Please save or apply";

// ** feature display boxes **//
speedchecker.nfeath4title="WiFi Speedchecker";
speedchecker.nfeath4txt="Is your WiFi slowing you down? One-click speed test for both Internet and WiFi speed.";
speedchecker.nfeatbutton="&nbsp;&nbsp;Click here to test your speed&nbsp;&nbsp;";

var dnscrypt=new Object();
dnscrypt.nfeath4title="DNSCrypt";
dnscrypt.nfeath4txt="<a href=\"https:\/\/www.dnscrypt.org\">DNSCrypt</a> authenticates communications between a DNS client and a DNS resolver. It prevents DNS spoofing.";
dnscrypt.nfeatbutton="&nbsp;&nbsp;Go to the Services Tab&nbsp;&nbsp;";

var features=new Object();
features.legend="Display New Features";
features.label="Show Features";

var featureshead=new Object();
featureshead.h2title="Check out the new Features!&nbsp;&nbsp;";
featureshead.hidebtn="Hide this Box";
// ************		OLD PAGES 		*******************************//
// *********************** DHCPTable.asp *****************************//
var dhcp=new Object();
dhcp.titl="DHCP Active IP Table";
dhcp.h2="DHCP Active IP Table";
dhcp.server="DHCP Server IP Address :";
dhcp.tclient="Client Hostname";

var donate=new Object();
donate.mb="You may also donate through the Moneybookers account mb@dd-wrt.com";

var reg=new Object();
reg.not_reg="The system is not activated. Please contact your local dealer to obtain a valid Activation Key related to the displayed System Key.";
reg.sys_key="System Key";
reg.act_key="Activation Key";
reg.reg_aok="Activation Completed, the system will now reboot.";
