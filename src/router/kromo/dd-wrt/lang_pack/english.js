//////////////////////////////////////////////////////////////////////////////////////////////
//		English reference translation file - DD-WRT V23 SP1 by Botho 17/05/2006				//
//////////////////////////////////////////////////////////////////////////////////////////////

// ** COMMON SHARE LABEL **//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var share = new Object();
share.firmware="Firmware";
share.time="Time";
share.interipaddr="WAN IP Address";
share.more="More...";
share.help="Help";
share.enable="Enable";
share.enabled="Enabled";
share.disable="Disable";
share.disabled="Disabled";
share.usrname="User Name";
share.passwd="Password";
share.hostname="Host Name";
share.vdsl="DTAG VDSL Vlan Tagging";
share.wan_vlantag="Vlan Tag ID";
share.compression="PPP Compression";
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
share.ccq="CCQ"; 
share.pintrface="Physical Interface";
share.vintrface="Virtual Interfaces";
share.router="Router";
share.static_lease="Static Leases";
share.srvip="Server IP";
share.srvipname="Server IP/Name";
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
share.turbo="Turbo (40 Mhz)";
share.full="Full (20 Mhz)";
share.half="Half (10 Mhz)";
share.quarter="Quarter (5 Mhz)";
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
share.from2 = share.from;
share.to2 = share.to;
share.days_genetive = share.days;
share.standard="Standard";
share.execscript="Excecute script";
share.user="User";
share.privatekey="Private Key";
share.bytes="bytes";
share.kbytes="KB";
share.mbytes="MB";
share.gbytes="GB";
share.preempt="Preemption";
share.acktiming="ACK Timing";
share.broadcast="Broadcast support";
share.secondcharacter="s";

var sbutton = new Object();
sbutton.save="Save";
sbutton.savetitle="Only save settings without commit";
sbutton.apply="Apply Settings";
sbutton.applytitle="Apply settings immediately";
sbutton.saving="Saved";
sbutton.cmd="Executing";
sbutton.cancel="Cancel Changes";
sbutton.canceltitle="Cancel changes in this form only";
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
sbutton.shutdown="Save Shutdown";
sbutton.firewall="Save Firewall";
sbutton.custom="Save Custom Script";
sbutton.wol="Wake Up";
sbutton.add_wol="Add Host";
sbutton.manual_wol="Manual Wake Up";
sbutton.summary="Summary";
sbutton.filterIP="Edit List of clients";
sbutton.filterMac="Edit MAC Filter List";
sbutton.filterSer="Add/Edit Service";
sbutton.reboot="Reboot Router";
sbutton.help="   Help  ";
sbutton.wl_client_mac="Wireless Client MAC List";
sbutton.update_filter="Update Filter List";
sbutton.join="Join";
sbutton.log_in="Incoming Log";
sbutton.log_out="Outgoing Log";
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


// ** COMMON ERROR MESSAGES  **//
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
errmsg.err110="End number must be bigger then start number";
errmsg.err111="Invalid IP address";

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
errmsg.err103=" must be lower than ";

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
errmsg.err102="Upgrading firmware...<br/>Please, wait";

// **  COMMON MENU ENTRIES  **//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basic Setup";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Clone";
bmenu.setuprouting="Advanced Routing";
bmenu.setupvlan="VLANs";
bmenu.setupeop="EoIP Tunnel";
bmenu.networking="Networking";

bmenu.wireless="Wireless";
bmenu.wirelessBasic="Basic Settings";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSuperchannel="SuperChannel";
bmenu.wimax="WiMAX";
bmenu.wirelessSecurity="Wireless Security";
bmenu.wirelessMac="MAC Filter";
bmenu.wirelessAdvanced="Advanced Settings";
bmenu.wirelessWds="WDS";
bmenu.wirelessWds0="Ath0-WDS";
bmenu.wirelessWds1="Ath1-WDS";
bmenu.wirelessWds2="Ath2-WDS";
bmenu.wirelessWds3="Ath3-WDS";
bmenu.wirelessWdswl0="WL0-WDS";
bmenu.wirelessWdswl1="WL1-WDS";

bmenu.security="Security";
bmenu.firwall="Firewall";
bmenu.vpn="VPN Passthrough";

bmenu.accrestriction="Access Restrictions";
bmenu.webaccess="WAN Access";


bmenu.applications="NAT / QoS";
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
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Log";
bmenu.adminDiag="Commands";
bmenu.adminWol="WOL";
bmenu.adminFactory="Factory Defaults";
bmenu.adminUpgrade="Firmware Upgrade";
bmenu.adminBackup="Backup";

bmenu.services="Services";
bmenu.servicesServices="Services";
bmenu.servicesPppoesrv="PPPoE Server";
bmenu.servicesPptp="VPN";
bmenu.servicesUSB="USB";
bmenu.servicesNAS="NAS";
bmenu.servicesHotspot="Hotspot";
bmenu.servicesMilkfish="Milkfish SIP Router";
bmenu.servicesAnchorFree="My Ad Network";

bmenu.statu="Status";
bmenu.statuRouter="Router";
bmenu.statuInet="WAN";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Sputnik Agent";
bmenu.statuWLAN="Wireless";
bmenu.statuVPN="OpenVPN";
bmenu.statuBand="Bandwidth";
bmenu.statuSysInfo="Sys-Info";
bmenu.statuActivate="Activate";

// ** Alive.asp **//
var alive = new Object();
alive.titl="Keep Alive";
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


// ** config.asp **//
var config = new Object();
config.titl="Backup & Restore";
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


// ** DDNS.asp **//
var ddns = new Object();
ddns.titl="Dynamic DNS"
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
ddnsm.all_errresolv="Domain name resolv failed";
ddnsm.all_connecting="Connecting to server";
ddnsm.all_connectfail="Connect to server failed";
ddnsm.all_disabled="DDNS function is disabled";
ddnsm.all_noip="No WAN connection";

//help container
var hddns = new Object();
hddns.right2="DDNS allows you to access your network using domain names instead of IP addresses. The service manages changing IP addresses and updates your domain information dynamically. You must sign up for service through DynDNS.org, freedns.afraid.org, ZoneEdit.com, No-IP.com or Custom.";
hddns.right4="Type an integer number in the box to set the force update interval (in days). Updates should only be performed automaticaly when your IP address has changed. Beware of your DDNS provider update abuse policy to avoid your hostname or domain to be blocked.";


// ** Diagnostics.asp **//
var diag = new Object();
diag.titl="Diagnostics";
diag.h2="Diagnostics";
diag.legend="Command Shell";
diag.cmd="Commands";
diag.startup="Startup";
diag.shutdown="Shutdown";
diag.firewall="Firewall";
diag.custom="Custom Script";

//help container
var hdiag = new Object();
hdiag.right2="You can run command lines via the web interface. Fill the text area with your command and click <em>" + sbutton.runcmd + "</em> to submit.";


// ** DMZ.asp **//
var dmz = new Object();
dmz.titl="DMZ";
dmz.h2="Demilitarized Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="Use DMZ";
dmz.host="DMZ Host IP Address";

//help container
var hdmz = new Object();
hdmz.right2="Enabling this option will expose the specified host to the Internet. All ports will be accessible from the Internet.";


// ** Factory_Defaults.asp **//
var factdef = new Object();
factdef.titl="Factory Defaults";
factdef.h2="Factory Defaults";
factdef.legend="Reset router settings";
factdef.restore="Restore Factory Defaults";
factdef.mess1="Warning! If you click OK, the device will reset to factory default and all previous settings will be erased.";

//help container
var hfactdef = new Object();
hfactdef.right1="This will reset all settings back to factory defaults. All of your settings will be erased.";


// ** FilterIPMAC.asp **//
var filterIP = new Object();
filterIP.titl="List of clients";
filterIP.h2="List of clients";
filterIP.h3="Enter MAC Address of the clients in this format: xx:xx:xx:xx:xx:xx";
filterIP.h32="Enter the IP Address of the clients";
filterIP.h33="Enter the IP Range of the clients";
filterIP.ip_range="IP Range";


// ** Filter.asp **//
var filter = new Object();
filter.titl="Access Restrictions";
filter.h2="WAN Access";
filter.legend="Access Policy";
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


// ** FilterSummary.asp **//
var filterSum = new Object();
filterSum.titl="Access Restrictions Summary";
filterSum.h2="Internet Policy Summary";
filterSum.polnum="No.";
filterSum.polday="Time of Day";


// ** Firewall.asp **//
var firewall = new Object();
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
firewall.ping="Block Anonymous WAN Requests (ping)";
firewall.muticast="Filter Multicast";
filter.nat="Filter WAN NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container
var hfirewall = new Object();
hfirewall.right2="Enable or disable the SPI firewall.";


// ** Forward.asp **//
var prforward = new Object();
prforward.titl="Port Range Forwarding";
prforward.h2="Port Range Forward";
prforward.legend="Forwards";
prforward.app="Application";

//help container
var hprforward = new Object();
hprforward.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";


// ** P2P.asp **//
var p2p = new Object();
p2p.titl="Peer-to-Peer Apps";
p2p.h2="BitTorrent Client";
p2p.legend="CTorrent";
p2p.ctorrent_srv="Ctorrent Service";


//help container
//var hp2p = new Object();
//hpp2p.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";


// ** ForwardSpec.asp **//
var pforward = new Object();
pforward.titl="Port Forwarding";
pforward.h2="Port Forward";
pforward.legend="Forwards";
pforward.app="Application";
pforward.from="Port from";
pforward.to="Port to";

//help container
var hpforward = new Object();
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. Examples of these applications include servers and certain online games. When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. Due to security concerns, you may want to limit port forwarding to only those ports you are using, and uncheck the <em>Enable</em> checkbox after you are finished.";

// ** USB.asp **//
var usb = new Object();
usb.titl="USB";
usb.usb_legend="USB Support";
usb.usb_core="Core USB Support";
usb.usb_uhci="USB 1.1 Support (UHCI)";
usb.usb_ohci="USB 1.1 Support (OHCI)";
usb.usb_ehci="USB 2.0 Support";
usb.usb_storage="USB Storage Support";
usb.usb_extfs="ext2 / ext3 File System Support";
usb.usb_fatfs="FAT File System Support";
usb.usb_printer="USB Printer Support";
usb.usb_automnt="Automatic Drive Mount";
usb.usb_mntpoint="Disk Mount Point";
usb.usb_runonmount="Run-on-mount Script Name";
usb.usb_diskinfo="Disk Info";

// ** NAS.asp **//
var nas = new Object();
nas.titl="NAS";
nas.proftpd_legend="FTP Server";
nas.proftpd_srv="ProFTPD";
nas.proftpd_port="Server Port";
nas.proftpd_dir="Files Directory";
nas.proftpd_passw="User Password List";
nas.proftpd_writeen="Allow Write";
nas.proftpd_anon="Anonymous Login (Read-only)";
nas.proftpd_anon_subdir="Anonymous Home Sub-directory";
nas.sambasrv_legend="Samba Server";

var hnas = new Object();
hnas.right2="User Password List: Enter one User Password per line. Password can be plain text or MD5 crypt.";

// ** Hotspot.asp **//
var hotspot = new Object();
hotspot.titl="Hotspot";
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
hotspot.net="Remote Network";
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
hotspot.wifidog_checkinter="Check Interval (in sec.)";
hotspot.wifidog_checktimeout="Client Timeout";
hotspot.wifidog_tmaclist="Trusted MAC List";
hotspot.wifidog_authsrv="AuthServer Hostname";
hotspot.wifidog_authsrvssl="AuthServer SSL Available";
hotspot.wifidog_authsrvsslport="AuthServer SSL Port";
hotspot.wifidog_authsrvhttpport="AuthServer HTTP Port";
hotspot.wifidog_authsrvpath="AuthServer Path";
hotspot.wifidog_config="Firewall Ruleset";

var anchorfree = new Object();
anchorfree.anchorfree="AnchorFree";
anchorfree.titl="My Ad Network";
anchorfree.h2="AnchorFree Ad Network";
anchorfree.anchorfree_revenue="Earn revenue by creating ad-supported AnchorFree Hotspot";
anchorfree.email="Email to receive revenue reports";
anchorfree.ssid="Use different SSID";
anchorfree.ssid_name="SSID";
anchorfree.address_1="Street Address";
anchorfree.address_2="Street Address 2";
anchorfree.city="City";
anchorfree.zip="Postal or Zip Code";
anchorfree.state="County/State/Province";
anchorfree.country="Country";
anchorfree.category="Category";
anchorfree.publish="Publish this hotspot on the free WiFi map";
anchorfree.serviceid="Service ID";
anchorfree.servicestatus="Service Status";
anchorfree.agreement="Terms and Conditions";
anchorfree.agree="I ACCEPT AND AGREE";
anchorfree.validaddr="a valid address field must be provided if this hotspot should be published on the wifi hotspot map!";
anchorfree.validcity="a valid city or zip/postal code must be provided if this hotspot should be published on the wifi hotspot map!";
anchorfree.validcat="please select a advertising category for your hotspot";
anchorfree.validcountry="please select a country for your hotspot";
anchorfree.validterms="You must agree to the terms & conditions!";

var hanchorfree = new Object();
hanchorfree.right1="Join AnchorFree's hotspot advertising network";
hanchorfree.right2="AnchorFree operates a hotspot advertising network allowing DD-WRT customers to generate incremental revenues.";
hanchorfree.right3="Generate incremental revenue with advertising from AnchorFree";
hanchorfree.right4="By activating this feature and creating an account with AnchorFree (it's free and easy), a persistent advertising frame is inserted directly into users web browsers, which will earn you a payment every month.  Earn a minimum monthly threshold of $25 and AnchorFree will automatically credit your account with funds.";
hanchorfree.right5="For more information please visit www.anchorfree.com";
hanchorfree.right6="Activation is easy";
hanchorfree.right7="Once you have activated, AnchorFree will send an email to the registered account owner with simple instructions on optimizing your hotspot, FAQs, and other pertinent information on earning money from your router. Through this configuration screen, AnchorFree inserts a thin, non-intrusive advertising frame directly into web browsers accessing the internet from this router.";
hanchorfree.right8="Customer support";
hanchorfree.right9="Have questions? Contact us at boxhelp@anchorfree.com";


// ** Info.htm **//
var info = new Object();
info.titl="Info";
info.h2="System Information";
info.wlanmac="Wireless MAC";
info.srv="Services";
info.ap="Access Point";


// ** index_heartbeat.asp **//
var idx_h = new Object();
idx_h.srv="Heart Beat Server";
idx_h.con_strgy="Connection Strategy";
idx_h.max_idle="Connect on Demand: Max Idle Time";
idx_h.alive="Keep Alive: Redial Period";
idx_h.reconnect="Force reconnect";


// ** index_l2tp.asp **//
var idx_l = new Object();
idx_l.srv="L2TP Server";


// ** index_pppoe.asp **//
var idx_pppoe = new Object();
idx_pppoe.use_rp="Use RP PPPoE";


// ** index_pptp.asp **//
var idx_pptp = new Object();
idx_pptp.srv="Use DHCP";
idx_pptp.wan_ip="WAN IP Address";
idx_pptp.gateway="Gateway (PPTP Server)";
idx_pptp.encrypt="PPTP Encyption";


// ** index_static.asp **//
var idx_static = new Object();
idx_static.dns="Static DNS";


// ** index.asp **//
var idx = new Object();
idx.titl="Setup";
idx.h2="WAN Setup";
idx.h22="Wireless Setup";
idx.legend="WAN Connection Type";
idx.conn_type="Connection Type";
idx.stp="STP";
idx.stp_mess="(disable for COMCAST ISP)";
idx.optional="Optional Settings";
idx.mtu="MTU";
idx.h23="Network Setup";
idx.routerip="Router IP";
idx.lanip="Local IP Address";
idx.legend2="WAN Port";
idx.wantoswitch="Assign WAN Port to Switch";
idx.legend3="Time Settings";
idx.timeset="Time Zone";
idx.dsttime="Summer Time (DST)";
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
idx.summt_opt6="first Sun Oct - 3rd Sun Mar";
idx.summt_opt7="last Sun Sep - first Sun Apr";
idx.summt_opt8="3rd Sun Oct - 3rd Sun Mar";
idx.portsetup="Port Setup";
idx.wanport="WAN Port Assignment";
idx.ntp_client="NTP Client";

//help container
var hidx = new Object();
hidx.right2="This setting is most commonly used by Cable operators.";
hidx.right4="Enter the host name provided by your ISP.";
hidx.right6="Enter the domain name provided by your ISP.";
hidx.right8="This is the address of the router.";
hidx.right10="This is the subnet mask of the router.";
hidx.right12="Allows the router to manage your IP addresses.";
hidx.right14="The address you would like to start with.";
hidx.right16="You may limit the number of addresses your router hands out. 0 means only predefined static leases will be handed out.";
hidx.right18="Choose the time zone you are in and Summer Time (DST) period. The router can use local time or UTC time.";


// ** Join.asp **//
var join = new Object();

//sshd.webservices
join.titl="Join";
join.mess1="Successfully joined the following network as a client: ";


// ** Log_incoming.asp **//
var log_in = new Object();
log_in.titl="Incoming Log Table";
log_in.h2="Incoming Log Table";
log_in.th_ip="Source IP";
log_in.th_port="Destination Port Number";


// ** Log_outgoing.asp **//
var log_out = new Object();
log_out.titl="Outgoing Log Table";
log_out.h2="Outgoing Log Table";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destination URL/IP";
log_out.th_port="Service/Port Number";


// ** Log.asp **//
var log = new Object();
log.titl="Log";
log.h2="Log Management";
log.legend="Log";
log.lvl="Log Level";
log.drop="Dropped";
log.reject="Rejected";
log.accept="Accepted";


// ** Management.asp **//
var management = new Object();
management.titl="Administration";
management.h2="Router Management";
management.changepassword="Your Router is currently not protected and uses an unsafe default username and password combination, please change it using the following dialog!";
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
management.remote_telnet="Telnet Management";
management.remote_telnetport="Telnet Remote Port";
management.remote_allowanyip="Allow Any Remote IP";
management.remote_ip="Allowed Remote IP Range";
management.web_legend="Web Access";
management.web_refresh="Auto-Refresh (in seconds)";
management.web_sysinfo="Enable Info Site";
management.web_sysinfopass="Info Site Password Protection";
management.web_sysinfomasq="Info Site MAC Masking";
management.boot_legend="Boot Wait";
management.boot_srv="Boot Wait";
management.cron_legend="Cron";
management.cron_srvd="Cron";
management.cron_jobs="Additional Cron Jobs";
management.loop_legend="Loopback";
management.loop_srv="Loopback";
management.wifi_legend="802.1x";
management.wifi_srv="802.1x";
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
management.lang_bulgarian="Bulgarian";
management.lang_chinese_traditional="Chinese traditional";
management.lang_chinese_simplified="Chinese simplified";
management.lang_croatian="Croatian";
management.lang_czech="Czech";
management.lang_dutch="Dutch";
management.lang_portuguese_braz="Portuguese (brazilian)";
management.lang_english="English";
management.lang_polish="Polish";
management.lang_french="French";
management.lang_german="German";
management.lang_italian="Italian";
management.lang_brazilian="Brazilian";
management.lang_russian="Russian";
management.lang_slovenian="Slovenian";
management.lang_spanish="Spanish";
management.lang_swedish="Swedish";
management.lang_japanese="Japanese";
management.lang_hungarian="Hungarian";
management.net_legend="IP Filter Settings (adjust these for P2P)";
management.net_port="Maximum Ports";
management.net_tcptimeout="TCP Timeout (in seconds)";
management.net_udptimeout="UDP Timeout (in seconds)";
management.clock_legend="Overclocking";
management.clock_frq="Frequency";
management.clock_support="Not Supported";
management.mmc_legend="MMC/SD Card Support";
management.mmc_srv="MMC Device";
management.mmc_gpiosel="GPIO pins select";
management.mmc_gpiopins="GPIO pins";
management.samba_legend="CIFS Automount";
management.samba_srv="Common Internet File System";
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
portserv.titl="Port Services";
portserv.h2="Port Services";


// ** Networking.asp **//
var networking = new Object();
networking.h2="VLAN Tagging";
networking.legend="Tagging";
networking.h22="Bridging";
networking.legend2="Create Bridge";
networking.legend3="Assign to Bridge";
networking.legend4="Current Bridging Table";
networking.brname="Bridge Name";
networking.stp="STP enabled";
networking.iface="Interfaces";
networking.h5="DHCPD";
networking.legend5="Multiple DHCP Server";


// ** QoS.asp **//
var qos = new Object();
qos.titl="Quality of Service";
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
qos.maxuprate_b="Max Kbits Up";
qos.maxdownrate_b="Max Kbits Down";
qos.maxrate_o="Max Rate";
qos.legend4="MAC Priority";
qos.legend5="Ethernet Port Priority";
qos.legend6="Default Bandwidth Level";
qos.bandwidth="Bandwidth in Kbits";

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


// ** RouteTable.asp **//
var routetbl = new Object();
routetbl.titl="Routing Table";
routetbl.h2="Routing Table Entry List";
routetbl.th1="Destination LAN NET";


// ** Routing.asp **//
var route = new Object();
route.titl="Routing";
route.h2="Advanced Routing";
route.metric="Metric";
route.mod="Operating Mode";
route.bgp_legend="BGP Settings";
route.bgp_ip="Neighbor IP";
route.bgp_own_as="BGP Own AS#";
route.bgp_as="Neighbor AS#";
route.rip2_mod="RIP2 Router";
route.olsrd_mod="OLSR Router";
route.olsrd_legend="OLSR Routing (Optimized Link State Routing)";
route.olsrd_poll="Poll Rate";
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
route.ospf_mod="OSPF Router";
route.ospf_legend="OSPF Routing";
route.ospf_conf="OSPF Configuration";
route.gateway_legend="Dynamic Routing";
route.static_legend="Static Routing";
route.static_setno="Select set number";
route.static_name="Route Name";
route.static_ip="Destination LAN NET";

//help container
var hroute = new Object();
hroute.right2="If the router is hosting your Internet connection, select <em>Gateway</em> mode. If another router exists on your network, select <em>Router</em> mode.";
hroute.right4="This is the unique route number, you may set up to 20 routes.";
hroute.right6="Enter the name you would like to assign to this route.";
hroute.right8="This is the remote host to which you would like to assign the static route.";
hroute.right10="Determines the host and the network portion.";


// ** Site_Survey.asp **//
var survey = new Object();
survey.titl="Site Survey";
survey.h2="Neighbor&#39;s Wireless Networks";
survey.thjoin="Join Site";


// ** Services.asp **//
var service = new Object();
service.titl="Services";
service.h2="Services Management";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Start Kaid";
service.kaid_locdevnum="Number of Local Devices";
service.kaid_uibind="UI Listening Port";
service.kaid_orbport="ORB Port";
service.kaid_orbdeepport="ORB Deep Port";

//DHCPd
service.dhcp_legend="DHCP Client";
service.dhcp_vendor="Set Vendorclass";
service.dhcp_reqip="Request IP";
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
service.dnsmasq_opt="Additional DNSMasq Options";

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
service.pppoe_legend="PPPoE Relay";
service.pppoe_srv="Relay";

//pppoe-server.webservices
service.pppoesrv_legend="PPPoE Server";
service.pppoesrv_srv="RP-PPPoE Server Daemon";
service.pppoesrv_srvopt="RP-PPPoE Server Options";
service.pppoesrv_compr="Compression";
service.pppoesrv_remoteaddr="Remote Start IP";
service.pppoesrv_remotenet="Remote Network IP";
service.pppoesrv_remotemask="Remote Network Mask";
service.pppoesrv_lcpei="LCP Echo Interval";
service.pppoesrv_lcpef="LCP Echo Failure";
service.pppoesrv_idlet="Idle Time";
service.pppoesrv_auth="Authentication";
service.pppoesrv_radip="Radius Server IP";
service.pppoesrv_radauthport="Radius Authentication Port";
service.pppoesrv_radaccport="Radius Accounting Port";
service.pppoesrv_radkey="Radius Shared Key";
service.pppoesrv_chaps="Local User Management (CHAP Secrets)";

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
service.vpn_tuntap="Tunnel Device";
service.vpn_srvcert="Public Server Cert";
service.vpn_clicert="Public Client Cert";
service.vpn_certtype="nsCertType";
service.vpn_clikey="Private Client Key";

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Password Login";
service.ssh_key="Authorized Keys";
service.ssh_forwarding="SSH TCP Forwarding";

//radiooff.webservices
service.radiooff_legend="SES / AOSS / EZ-SETUP / WPS Button";
service.radiooff_srv="Use this button for turning off radio";
service.radiooff_bootoff="Turn radio off at boot";

//ses.webservices ====> might replace the above radiooff_button
service.ses_legend="SES / AOSS / EZ-SETUP / WPS Button";
service.ses_srv="Button action";
service.ses_toggleradio="Toggle Wireless";
service.ses_script="Custom Script";

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

//nstx.webservices
service.nstx_legend="IP over DNS Tunneling";
service.nstx_srv="nstx Daemon";
service.nstx_ipenable="Bind to this IP only";
service.nstx_log="Switch on debug messages";

//ttraff.webservices
service.ttraff_legend="WAN Traffic Counter";
service.ttraff_daemon="ttraff Daemon";

//milkfish.webservices
service.milkfish_siprouter="Milkfish SIP Router";
service.milkfish_alias="Alias";
service.milkfish_uri="SIP URI";
service.milkfish_mainswitch="Main Switch";
service.milkfish_fromswitch="From-Substitution";
service.milkfish_fromdomain="From-Domain";
service.milkfish_username="Milkfish Username";
service.milkfish_password="Milkfish Password";
service.milkfish_audit="Milkfish Audit";
service.milkfish_siptrace="SIP Trace";
service.milkfish_subscribers="Local Subscribers";
service.milkfish_aliases="Local Aliases";
service.milkfish_dynsip="Dynamic SIP";
service.milkfish_status="SIP Status";
service.milkfish_database="SIP Database";
service.milkfish_messaging="SIP Messaging";
service.milkfish_phonebook="SIP Phonebook";
service.milkfish_dynsipdomain="DynSIP Domain";
service.milkfish_dynsipurl="DynSIP Update URL";
service.milkfish_dsusername="DynSIP Username";
service.milkfish_dspassword="DynSIP Password";
service.milkfish_sipmessage="SIP Message";
service.milkfish_destination="SIP Destination";
service.milkfish_contact="Contact";
service.milkfish_agent="User Agent";
service.milkfish_registrations="Active Registrations";
//service.milkfish_="";//
service.hmilkfish_right2="Enables/Disables your Milkfish SIP Router.";
service.hmilkfish_right4="Enables/Disables From:-Headerfield substitution of your WAN IP in outgoing SIP messages. This setting should be enabled, if you want to allow callees to call you back if your WAN IP has changed";
service.hmilkfish_right6="Callee calls back this Domain (instead of your WAN IP) when you send it as your From:-Domain instead of your WAN IP. From-Substitution needs to be enabled for this setting to take effect";
service.hmilkfish_right8="Here goes your Milkfish Community Forum Username - Registration allocates you yourname.homesip.net";
service.hmilkfish_right10="Here goes your Milkfish Community Forum Password - Registration allocates you yourname.homesip.net";
service.hmilkfish_right12="Enables/Disables basic SIP Tracing on your router";
service.hmilkfish_right14="Local SIP Subscribers are locally managed SIP Accounts";
service.hmilkfish_right16="Local SIP Aliases are SIP Forwards similar to Email Forwards. Can be used to forward alphanumeric to numeric phone user accounts or vice versa";
service.hmilkfish_right18="Enables/Disables Dynamic SIP (i.e. Homesip.net Service) - Username and Password need to be set";
service.hmilkfish_right20="not implemented yet - leave empty";
service.hmilkfish_right22="not implemented yet - leave empty";
service.hmilkfish_right24="not implemented yet - leave empty";
service.hmilkfish_right26="not implemented yet - leave empty";
//service.hmilkfish_="";//


// ** eop-tunnel.asp **//
var eoip = new Object();
eoip.titl="EoIP Tunnel";
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


// ** Sipath.asp + cgi **//
var sipath = new Object();
sipath.titl="SiPath Overview";
sipath.phone_titl="Phonebook";
sipath.status_titl="Status";


// ** Status_Lan.asp **//
var status_lan = new Object();
status_lan.titl="LAN Status";
status_lan.h2="Local Network";
status_lan.legend="LAN Status";
status_lan.h22="Dynamic Host Configuration Protocol";
status_lan.legend2="DHCP Status";
status_lan.legend3="DHCP Clients";
status_lan.legend4="Active Clients";
status_lan.concount="Conn. Count";
status_lan.conratio="Ratio";

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="This is the Router's MAC Address, as seen on your local, Ethernet network.";
hstatus_lan.right4="This shows the Router's IP Address, as it appears on your local, Ethernet network.";
hstatus_lan.right6="When the Router is using a Subnet Mask, it is shown here.";
hstatus_lan.right8="If you are using the Router as a DHCP server, that will be displayed here.";
hstatus_lan.right10="By clicking on any MAC address, you will obtain the Organizationally Unique Identifier of the network interface (IEEE Standards OUI database search).";


// ** Status_Bandwidth.asp **//
var status_band = new Object();
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
var hstatus_band = new Object();
hstatus_band.svg="The Adobe's SVG plugin is required to display bandwidth graphs.";
hstatus_band.right1="Click the label to switch unit (bytes/s or bits/s).";
hstatus_band.right2="Click the label to choose graph scale type.";

// ** Status_Router.asp **//
var status_router = new Object();
status_router.titl="Router Status";
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
status_router.mem_hidden="Hidden"; // do not translate this line, this is bogus (BrainSlayer)
status_router.legend4="Network";
status_router.net_maxports="IP Filter Maximum Ports";
status_router.net_conntrack="Active IP Connections";
status_router.notavail="Not available";
status_router.legend6="Space Usage";
status_router.inpvolt="Board Input Voltage";
status_router.cputemp="CPU Temperature";

//help container
var hstatus_router = new Object();
hstatus_router.right2="This is the specific name for the router, which you set on the <i>Setup</i> tab.";
hstatus_router.right4="This is the router's MAC Address, as seen by your ISP.";
hstatus_router.right6="This is the router's current firmware.";
hstatus_router.right8="This is time received from the ntp server set on the <em>" + bmenu.setup + " | " + bmenu.setupbasic + "</em> tab.";
hstatus_router.right10="This is a measure of the time the router has been \"up\" and running.";
hstatus_router.right12="This is given as three numbers that represent the system load during the last one, five, and fifteen minute periods.";

// ** Status_Internet.asp **//
var status_inet = new Object();
status_inet.titl="WAN Status";
status_inet.h11="WAN";
status_inet.conft="Configuration Type";
status_inet.www_loginstatus="Login Status";
status_inet.wanuptime="Connection Uptime";
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


//help container
var hstatus_inet = new Object();
hstatus_inet.right2="This shows the information required by your ISP for connection to the Internet. This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.";
hstatus_inet.right4="This shows your router's Internet traffic since last reboot.";
hstatus_inet.right6="This shows your router's Internet traffic by month. Drag the mouse over graph to see daily data. Data is stored in nvram.";


// ** Status_Conntrack.asp **//
var status_conn = new Object();
status_conn.titl="Active IP Connections Table";
status_conn.h2="Active IP Connections";


// ** Status_SputnikAPD.asp **//
var status_sputnik = new Object();
status_sputnik.titl="Sputnik Agent Status";
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


// ** Status_Wireless.asp **//
var status_wireless = new Object();
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

// ** GPS info **//
var status_gpsi = new Object();
status_gpsi.legend="GPS Info";
status_gpsi.status="Status";
status_gpsi.lon="Longitude";
status_gpsi.lat="Latitude";
status_gpsi.alt="Altitude";
status_gpsi.sat="Visible Satelites";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="This is the Router's MAC Address, as seen on your local, wireless network.";
hstatus_wireless.right4="As selected from the Wireless tab, this will display the wireless mode (Mixed, G-Only, B-Only or Disabled) used by the network.";


// ** Status_OpenVPN.asp **//
var status_openvpn = new Object();
status_openvpn.titl="OpenVPN Status";


// ** Triggering.asp **//
var trforward = new Object();
trforward.titl="Port Triggering";
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


// ** Upgrade.asp **//
var upgrad = new Object();
upgrad.titl="Firmware Upgrade";
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


// ** UPnP.asp **//
var upnp = new Object();
upnp.titl="UPnP";
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


// ** VPN.asp **//
var vpn = new Object();
vpn.titl="VPN Passthrough";
vpn.h2="Virtual Private Network (VPN)";
vpn.legend="VPN Passthrough";
vpn.ipsec="IPSec Passthrough";
vpn.pptp="PPTP Passthrough";
vpn.l2tp="L2TP Passthrough";

//help container
var hvpn = new Object();
hvpn.right1="You may choose to enable IPSec, PPTP and/or L2TP passthrough to allow your network devices to communicate via VPN.";


// ** Vlan.asp **//
var vlan = new Object();
vlan.titl="Virtual LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Assigned To<br />Bridge";
vlan.tagged="Tagged";
vlan.negociate="Auto-Negotiate";
vlan.aggregation="Link Aggregation<br>on Ports 3 & 4";
vlan.trunk="Trunk";


// ** WEP.asp **//
var wep = new Object();
wep.defkey="Default Transmit Key";
wep.passphrase="Passphrase";


// ** WOL.asp **//
var wol = new Object();
wol.titl="WOL";
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
wol.legend5="Wake-On-LAN daemon";
wol.srv="WOL daemon";
wol.pass="SecureOn Password";

//help container
var hwol = new Object();
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router). You can manually wake up hosts by clicking the <em>"+sbutton.wol+"</em> or you can program an automatic schedule wake up thanks to the "+wol.srv+".";
hwol.right4="MAC Address(es) are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB) and must be separated by a <em>SPACE</em>";
hwol.right6="IP Address is typically the broadcast address for the local network, but could be a remote address if the target host is not connected to the router's local network."


// ** WanMAC.asp **//
var wanmac = new Object();
wanmac.titl="MAC Address Clone";
wanmac.h2="MAC Address Clone";
wanmac.legend="MAC Clone";
wanmac.wan="Clone WAN MAC";
wanmac.wlan="Clone Wireless MAC";

//help container
var hwanmac = new Object();
hwanmac.right2="Some ISPs will require you to register your MAC address. If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";


// ** WL_WPATable.asp / WPA.asp / Radius.asp **//
var wpa = new Object();
wpa.titl="Wireless Security";
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

var sec80211x = new Object();
sec80211x.xsuptype="XSupplicant Type";
sec80211x.servercertif="Public Server Certificate";
sec80211x.clientcertif="Client Certificate";
sec80211x.phase2="Phase2 Authentication";
sec80211x.anon="Anonymous Identity";

//help container
var hwpa = new Object();
hwpa.right2="You may choose from Disable, WEP, WPA Personal, WPA Enterprise, or RADIUS. All devices on your network must use the same security mode.";


// ** WL_FilterTable.asp **//
var wl_filter = new Object();
wl_filter.titl="MAC Address Filter List";
wl_filter.h2="MAC Address Filter List";
wl_filter.h3="Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";


// ** WL_ActiveTable.asp **//
var wl_active = new Object();
wl_active.titl="Wireless Active Client MAC List";
wl_active.h2="Wireless Client MAC List";
wl_active.h3="Enable MAC Filter";
wl_active.active="Active clients";
wl_active.inactive="Inactive clients";


// ** Wireless_WDS.asp **//
var wds = new Object();
wds.titl="WDS";
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


// ** Wireless_radauth.asp **//
var radius = new Object();
radius.titl="Radius";
radius.h2="Remote Authentication Dial-In User Service";
radius.legend="Radius";
radius.label="MAC Radius Client";
radius.label2="MAC Format";
radius.label3="Radius Auth Server Address";
radius.label4="Radius Auth Server Port";
radius.label5="Maximum Unauthenticated Users";
radius.label6="Password Format";
radius.label7="Radius Auth Shared Secret";
radius.label8="Override Radius if server is unavailable";
radius.label13="Radius Acct Server Address";
radius.label14="Radius Acct Server Port";
radius.label17="Radius Acct Shared Secret";
radius.label18="Radius Accounting";

// ** Wireless_MAC.asp **//
var wl_mac = new Object();
wl_mac.titl="MAC Filter";
wl_mac.h2="Wireless MAC Filter";
wl_mac.legend="MAC Filter";
wl_mac.label="Use Filter";
wl_mac.label2="Filter Mode";
wl_mac.deny="Prevent clients listed from accessing the wireless network";
wl_mac.allow="Permit only clients listed to access the wireless network";

// ** WiMAX
var wl_wimax = new Object();
wl_wimax.titl="WiMAX";
wl_wimax.h2="Worldwide Interoperability for Microwave Access";
wl_wimax.downstream="Downstream Frequency";
wl_wimax.upstream="Upstream Frequency";
wl_wimax.width="Channel Width";
wl_wimax.duplex="Duplex Mode";
wl_wimax.mode="Operation Mode";
wl_wimax.mac="Subscriber MAC Address";

// ** Wireless_Advanced.asp **//
var wl_adv = new Object();
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
wl_adv.label17="Wireless GUI Access";
wl_adv.label18="WMM Support";
wl_adv.label19="No-Acknowledgement";
wl_adv.label20="Shortslot Override";
wl_adv.label21="Transmission Maximum Rate";
wl_adv.label23="Transmission Minimum Rate";
wl_adv.label22="Bluetooth Coexistence Mode";
wl_adv.label24="Antenna Alignment";
wl_adv.table1="EDCA AP Parameters (AP to Client)";

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
wl_adv.lng="Long"; 					//************* don't use .long ! *************
wl_adv.shrt="Short"; 				//************* don't use .short ! **************

//help container
var hwl_adv = new Object();
hwl_adv.right2="You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.";

// ** Wireless_Basic.asp **//
var wl_basic = new Object();
wl_basic.titl="Wireless";
wl_basic.h2="Wireless";
wl_basic.legend="Basic Settings";
wl_basic.label="Wireless Mode";
wl_basic.label2="Wireless Network Mode";
wl_basic.label3="Wireless Network Name (SSID)";
wl_basic.label4="Wireless Channel";
wl_basic.label5="Wireless SSID Broadcast";
wl_basic.label6="Sensitivity Range (ACK Timing)";
wl_basic.label7="802.11n Transmission Mode";
wl_basic.scanlist="ScanList";
wl_basic.duallink="Dual Link";
wl_basic.parent="Parent IP";
wl_basic.ap="AP";
wl_basic.client="Client";
wl_basic.repeater="Repeater";
wl_basic.repeaterbridge="Repeater Bridge";
wl_basic.clientBridge="Client Bridge";
wl_basic.adhoc="Adhoc";
wl_basic.wdssta="WDS Station";
wl_basic.wdsap="WDS AP";
wl_basic.mixed="Mixed";
wl_basic.greenfield="Greenfield";
wl_basic.preamble="Short Preamble";
wl_basic.b="B-Only";
wl_basic.a="A-Only";
wl_basic.na="NA-Only";
wl_basic.g="G-Only";
wl_basic.bg="BG-Mixed";
wl_basic.n="N-Only";
wl_basic.rts="RTS Threshold";
wl_basic.rtsvalue="Threshold";
wl_basic.protmode="Protection Mode";
wl_basic.legend2="Radio Time Restrictions";
wl_basic.radio="Radio";
wl_basic.radiotimer="Radio Scheduling";
wl_basic.radio_on="Radio is On";
wl_basic.radio_off="Radio is Off";
wl_basic.h2_v24="Wireless Physical Interface";
wl_basic.h2_vi="Virtual Interfaces";
wl_basic.regdom="Regulatory Domain";
wl_basic.TXpower="TX Power";
wl_basic.AntGain="Antenna Gain";
wl_basic.diversity="Diversity";
wl_basic.primary="Primary";
wl_basic.secondary="Secondary";
wl_basic.vertical="Vertical";
wl_basic.horizontal="Horizontal";
wl_basic.adaptive="Adaptive";
wl_basic.external="External";
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
wl_basic.channel_wide="Wide Channel";
wl_basic.regulatory="SuperChannel";
wl_basic.chanshift="Channel Shifting";
wl_basic.specialmode="2.3 Ghz Mode";
wl_basic.wifi_bonding="Wifi Bonding";
wl_basic.sifstime="OFDM SIFS Time";
wl_basic.preambletime="OFDM Preamble Time";
wl_basic.multicast="Multicast forwarding";
wl_basic.intmit="Noise Immunity";
wl_basic.noise_immunity="Noise Immunity Level";
wl_basic.ofdm_weak_det="OFDM Weak Detection";
wl_basic.radar="Radar Detection";
wl_basic.mtikie="MT Compatibility";
wl_basic.csma="Carrier Sense";
//help container
var hwl_basic = new Object();
hwl_basic.right2="If you wish to exclude Wireless-G clients, choose <em>B-Only</em> mode. If you would like to disable wireless access, choose <em>Disable</em>.<br/><b>Note :</b> when changing wireless mode, some advanced parameters are succeptible to be modified (\"" + wl_adv.label16 + "\", \"" + wl_adv.label2 + "\" or \"" + wl_adv.label5 + "\").";
hwl_basic.right3="Sensitivity Range: ";
hwl_basic.right4="Adjusts the ack timing. 0 disables ack timing completely for broadcom firmwares. On Atheros based firmwares it will turn into auto ack timing mode";
hwl_basic.right6="Click any hour to enable or disable the radio signal (<em>green</em> indicates allowed Wireless access, and <em>red</em> indicates blocked Wireless access)";

// ** Fail_s.asp / Fail_u_s.asp / Fail.asp **//
var fail = new Object();
fail.mess1="The values you entered are invalid. Please try again.";
fail.mess2="Upgrade failed.";


// ** Success*.asp / Reboot.asp  **//
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

// ** Logout.asp  **//
var logout = new Object();
logout.message="You have successfully logged out.<br />Thank you for using DD-WRT !";


// ************		OLD PAGES 		*******************************//
// *********************** DHCPTable.asp *****************************//
var dhcp = new Object();
dhcp.titl="DHCP Active IP Table";
dhcp.h2="DHCP Active IP Table";
dhcp.server="DHCP Server IP Address :";
dhcp.tclient="Client Host Name";

var donate = new Object();
donate.mb="You may also donate through the Moneybookers account mb@dd-wrt.com";
