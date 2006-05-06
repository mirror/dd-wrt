// Not working in IE & Opera ?
//************ Include the current language file after english.js ************//
//  var Head = document.getElementsByTagName('head')[0];
//  var head_inc = document.createElement('script');
//  head_inc.setAttribute('type','text/javascript');
//  head_inc.setAttribute('src',"lang_pack/language.js");
//  Head.appendChild(head_inc);


// ************************************************************ COMMON SHARE LABEL ***************************************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var share = new Object();
share.firmwarever="Firmware";
share.time="Time ";
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
share.statu="Status";
share.start="Start";
share.end="End";
share.proto="Protocol";
share.ip="IP Address";
share.mac="MAC Address";
share.none="None";
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
sbutton.deleted="Deleted";
sbutton.delall="Delete All";
sbutton.refres="Refresh";
sbutton.autorefresh="Auto-Refresh is On";
sbutton.backup="Backup";
sbutton.restore="Restore";
sbutton.cptotext="Copy to text area";
sbutton.runcmd="Run Commands";
sbutton.startup="Save Startup";
sbutton.firewall="Save Firewall";
sbutton.wol="Wake Up";
sbutton.add_wol="Add Host";
sbutton.manual_wol="Manual Wake Up";
sbutton.summary="Summary";
sbutton.filterIP="Edit List of PCs";
sbutton.filterSer="Add/Edit Service";
sbutton.reboot="Reboot Router";
sbutton.help="   Help  ";
sbutton.wl_client_mac="Wireless Client MAC List";
sbutton.update_filter="Update Filter List";
sbutton.join="Join";
sbutton.log_in="Incoming Log";
sbutton.log_out="Outgoing Log";
sbutton.log_sys="System Log";

// ************************************************************ COMMON ERROR MESSAGES  ***************************************************************//
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
errmsg.err13="Upgrade are failed.";
errmsg.err45="Not available in HTTPS! Please connect in HTTP mode.";
errmsg.err46="Not available in HTTPS";


//common.js error message
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
errmsg.err29=" have illegal ascii code."
errmsg.err30=" have illegal hexadecimal digits.";
errmsg.err31=" value is illegal.";
errmsg.err32="IP address and gateway is not at same subnet mask.";
errmsg.err33="IP address and gateway can't be same.";
errmsg.err34=" is not allowed to contain a space.";

//Wol.asp error message
errmsg.err35="You must input a MAC address to run."
errmsg.err36="You must input a network broadcast address to run.";
errmsg.err37="You must input a UDP port to run.";

//WL_WPATable.asp error message
//WPA.asp error message
errmsg.err38="Please enter a Shared Key!";
errmsg.err39="Invalid Key, must be between 8 and 63 ASCII characters or 64 hexadecimal digits"
errmsg.err40="You have to enter a key for Key ";
errmsg.err41="Invalid Length in key ";
errmsg.err43="rekey interval";

//config.asp error message
errmsg.err42="Please select a configuration file to restore.";

//WL_ActiveTable.asp error message
errmsg.err44="The total checks exceed 128 counts !";

//Site_Survey.asp error message
errmsg.err47=("invalid SSID");

//Wireless_WDS.asp error message
errmsg.err48=("WDS is not compatible with the current configuration of the router. Please check the following points :\n * Wireless Mode must be set to AP \n * WPA2 is not supported under WDS \n * Wireless Network B-Only mode is not supported under WDS");

// **************************************************************  COMMON MENU ENTRIES  **********************************************************//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basic Setup";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Clone";
bmenu.setuprouting="Advanced Routing";
bmenu.setupvlan="VLANs";

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

bmenu.sipath="SIPatH";


bmenu.admin="Administration";
bmenu.adminManagement="Management";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Services";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Log";
bmenu.adminDiag="Diagnostics";
bmenu.adminWol="WOL";
bmenu.adminFactory="Factory Defaults";
bmenu.adminUpgrade="Firmware Upgrade";
bmenu.adminBackup="Backup";


bmenu.statu="Status";


// **************************************************************** DDNS.asp **********************************************************************//

var ddns = new Object();
ddns.titl=" - Dynamic DNS"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS Service";
ddns.emailaddr="E-mail Address";
ddns.typ="Type";
ddns.dynamic="Dynamic";
// ddns.static="Static"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.sttic="Static";
ddns.custom="Custom";
ddns.wildcard="Wildcard";
ddns.statu="DDNS Status";

var ddnsm = new Object();
ddnsm.dyn_good="DDNS is updated successfully";
ddnsm.dyn_noupdate="DDNS is updated successfully";
ddnsm.dyn_nohost="The hostname does not exist";
ddnsm.dyn_notfqdn="Domain Name is not correct";
ddnsm.dyn_yours="Username is not correct";
ddnsm.dyn_abuse="The hostname is blocked by the DDNS server";
ddnsm.dyn_nochg="DDNS is updated successfully";
ddnsm.dyn_badauth="Authorization fails (username or passwords)";
ddnsm.dyn_badsys="The system parameters are invalid";
ddnsm.dyn_badagent="This useragent has been blocked";
ddnsm.dyn_numhost="Too many or too few hosts found";
ddnsm.dyn_dnserr="DNS error encountered";
ddnsm.dyn_911="An unexpected error (1)";
ddnsm.dyn_999="An unexpected error (2)";
ddnsm.dyn_donator="A feature requested is only available to donators, please donate";
ddnsm.dyn_strange="Strange server response, are you connecting to the right server ?";
ddnsm.dyn_uncode="Unknown return code";
ddnsm.tzo_good="Operation Complete";
ddnsm.tzo_noupdate="Operation Complete";
ddnsm.tzo_notfqdn="Invalid Domain Name";
ddnsm.tzo_notmail="Invalis Email";
ddnsm.tzo_notact="Invalid Action";
ddnsm.tzo_notkey="Invalid Key";
ddnsm.tzo_notip="Invalid IP address";
ddnsm.tzo_dupfqdn="Duplicate Domain Name";
ddnsm.tzo_fqdncre="Domain Name has already been created for this domain name";
ddnsm.tzo_expired="The account has expired";
ddnsm.tzo_error="An unexpected server error";
ddnsm.zone_703="one of either parameters 'zones' or 'host' are required";
ddnsm.zone_707="Duplicate updates for the same host/ip, adjust client settings";
ddnsm.zone_707="Too frequent updates for the same host, adjust client settings";
ddnsm.zone_704="Zone must be a valid 'dotted' internet name";
ddnsm.zone_701="Zone is not set up in this account";
ddnsm.zone_200="Update succeeded";
ddnsm.zone_201="No records need updating";
ddnsm.zone_702="Update failed";
ddnsm.zone_705="Zone cannot be empty";
ddnsm.zone_badauth="Authorization fails (username or passwords)";
ddnsm.zone_good="ZoneEdit is updated successfully";
ddnsm.zone_strange="Strange server response, are you connecting to the right server ?";
ddnsm.all_closed="DDNS server is currently closed";
ddnsm.all_resolving="Resolving domain name";
ddnsm.all_errresolv="Domain name resolv fail";
ddnsm.all_connecting="Connecting to server";
ddnsm.all_connectfail="Connect to server fail";
ddnsm.all_disabled="DDNS function is disabled";
ddnsm.all_noip="No Internet connection";
ddnsm.all_connectfail_test="success OK Fine Connect to server fail";

//help container
var hddns = new Object();
hddns.right1="DDNS Service:";
hddns.right2="DDNS allows you to access your network using domain names instead of IP addresses. \
	The service manages changing IP address and updates your domain information dynamically. \
	You must sign up for service through TZO.com or DynDNS.org.";


// **************************************************************** Forward.asp **********************************************************************//

var prforward = new Object();
prforward.titl=" - Port Range Forwarding";
prforward.h2="Port Range Forward";
prforward.legend="Forwards";
prforward.app="Application";

//help container
var hprforward = new Object();
hprforward.right1="Port Range Forward:";
hprforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";



// **************************************************************** ForwardSpec.asp **********************************************************************//

var pforward = new Object();
pforward.titl=" - Port Forwarding";
pforward.h2="Port Forward";
pforward.legend="Forwards";
pforward.app="Application";
pforward.from="Port from";
pforward.to="Port to";

//help container
var hpforward = new Object();
hpforward.right1="Port Forward:";
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>Enable</em> checkbox after you are finished.";


// **************************************************************** Triggering.asp **********************************************************************//

var trforward = new Object();
trforward.titl=" - Port Triggering";
trforward.h2="Port Triggering";
trforward.legend="Forwards";
trforward.trrange="Triggered Port Range";
trforward.fwdrange="Forwarded Port Range";
trforward.app="Application";

//help container
var htrforward = new Object();
htrforward.right1="Application:";
htrforward.right2="Enter the application name of the trigger.";
htrforward.right3="Triggered Range:";
htrforward.right4="For each application, list the triggered port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right5="Forwarded Range:";
htrforward.right6="For each application, list the forwarded port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right7="Start:";
htrforward.right8="Enter the starting port number of the Triggered and Forwarded Range.";
htrforward.right9="End:";
htrforward.right10="Enter the ending port number of the Triggered and Forwarded Range.";


// **************************************************************** UPnP.asp **********************************************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Forwards";
upnp.legend2="UPnP Configuration";
upnp.serv="UPnP Service";
upnp.clear="Clear port forwards at startup";
upnp.url="Send presentation URL";
upnp.msg1="Click to delete lease";
upnp.msg2="Delete all entries?";


//help container
var hupnp = new Object();
hupnp.right1="Forwards:";
hupnp.right2="Click the trash can to delete an individual entry.";
hupnp.right3="UPnP Service:";
hupnp.right4="Allows applications to automatically setup port forwardings.";

// **************************************************************** Alive.asp **********************************************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Schedule Reboot";
alive.sevr1="Schedule Reboot";
alive.time="Time (in seconds)";
alive.hour="At a set Time";
alive.legend2="WDS/Connection Watchdog";
alive.sevr2="Enable Watchdog";
alive.inter="Interval (in seconds)";
alive.IP="IP Addresses";
alive.legend3="Proxy/Connection Watchdog";
alive.sevr3="Enable Proxy Watchdog";
alive.IP2="Proxy IP Address";
alive.port="Proxy Port";

//help container
var halive = new Object();
halive.right1="At a set Time:";
halive.right2="Choose when reboot the router. Cron must be enabled in the managment tab.";
halive.right3="IP Addresses:";
halive.right4="A maximum of three IPs separated by a <em>SPACE</em> is allowed.<BR/>IPs Format is xxx.xxx.xxx.xxx.";

// **************************************************************** config.asp **********************************************************************//

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
hconfig.right1="Backup:";
hconfig.right2="You may backup your current configuration in case you need to reset the router back to its factory default settings.<br /><br />Click the <em>Backup</em> button to backup your current configuration.";
hconfig.right3="Restore:";
hconfig.right4="Click the <em>Browse...</em> button to browse for a configuration file that is currently saved on your PC.<br /><br />Click the <em>" + sbutton.restore + "</em> button to overwrite all current configurations with the ones in the configuration file.";

// **************************************************************** Diagnostics.asp **********************************************************************//

var diag = new Object();
diag.titl=" - Diagnostics";
diag.h2="Diagnostics";
diag.legend="Command Shell";
diag.cmd="Commands";
diag.startup="Startup";
diag.firewall="Firewall";


//help container
var hdiag = new Object();
hdiag.right1="Commands:";
hdiag.right2="You can run command lines via the webinterface. Fill the text area with your command and click <em>" + sbutton.runcmd + "</em> to submit.";


// **************************************************************** DMZ.asp **********************************************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Demilitarized Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="Use DMZ";
dmz.host="DMZ Host IP Address";


//help container
var hdmz = new Object();
hdmz.right1="DMZ:";
hdmz.right2="Enabling this option will expose the specified host to the Internet. All ports will be accessible from the Internet.";

// **************************************************************** Factory_Defaults.asp **********************************************************************//

var factdef = new Object();
factdef.titl=" - Factory Defaults";
factdef.h2="Factory Defaults";
factdef.legend="Reset router settings";
factdef.restore="Restore Factory Defaults";

factdef.mess1="Warning! If you click OK, the device will reset to factory default and all previous settings will be erased.";


//help container
var hfactdef = new Object();
hfactdef.right1="This will reset all settings back to factory defaults. All of your settings will be erased.";



// **************************************************************** Filter.asp **********************************************************************//

var filter = new Object();
filter.titl=" - Access Restrictions";
filter.h2="Internet Access";
filter.legend="Access Policy";
filter.restore="Restore Factory Defaults";
filter.pol="Policy";
filter.polname="Policy Name";
filter.pcs="PCs";
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
hfilter.right1="Internet Access Policy:";
hfilter.right2="You may define up to 10 access policies. Click <em>" + sbutton.del + "</em> to delete a policy or <em>" + sbutton.summary + "</em> to see a summary of the policy.";
hfilter.right3="Status:";
hfilter.right4="Enable or disable a policy.";
hfilter.right5="Policy Name:";
hfilter.right6="You may assign a name to your policy.";
hfilter.right7="Days:";
hfilter.right8="Choose the day of the week you would like your policy to be applied.";
hfilter.right9="Times:";
hfilter.right10="Enter the time of the day you would like your policy to apply.";
hfilter.right11="Blocked Services:";
hfilter.right12="You may choose to block access to certain services. Click <em>" + sbutton.filterSer + "</em> to modify these settings.";
hfilter.right13="Website Blocking by URL:";
hfilter.right14="You can block access to certain websites by entering their URL.";
hfilter.right15="Website Blocking by Keyword:";
hfilter.right16="You can block access to certain website by the keywords contained in their webpage.";


// **************************************************************** FilterIP%AC.asp **********************************************************************//

var filterIP = new Object();
filterIP.titl=" - List of PCs";
filterIP.h2="List of PCs";
filterIP.h3="Enter MAC Address of the PCs in this format: xx:xx:xx:xx:xx:xx";
filterIP.h32="Enter the IP Address of the PCs";
filterIP.h33="Enter the IP Range of the PCs";


// **************************************************************** FilterSummary.asp **********************************************************************//

var filterSum = new Object();
filterSum.titl=" - Access Restrictions Summary";
filterSum.h2="Internet Policy Summary";
filterSum.polnum="No.";
filterSum.polday="Time of Day";

// **************************************************************** Firewall.asp **********************************************************************//

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
firewall.ping="Block Anonymous Internet Requests";
firewall.muticast="Filter Multicast";
filter.nat="Filter Internet NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container
var hfirewall = new Object();
hfirewall.right1="Firewall Protection:";
hfirewall.right2="Enable or disable the SPI firewall.";


// **************************************************************** Hotspot.asp **********************************************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot Portal";
hotspot.legend="Chillispot";
hotspot.hotspot="Chillispot";
hotspot.pserver="Primary Radius Server IP/DNS";
hotspot.bserver="Backup Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="Redirect URL";
hotspot.key="Shared Key";
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
hotspot.smtp_legend="SMTP Redirect";
hotspot.smtp_srv="SMTP Redirect";
hotspot.smtp_ip="SMTP Destination IP";
hotspot.smtp_net="SMTP Source Network";
hotspot.shat_legend="Zero IP Config";
hotspot.shat_srv="Zero IP Config";
hotspot.shat_srv2="Zero IP Config enabled";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik Agent";
hotspot.sputnik_id="Sputnik Server ID";
hotspot.sputnik_instant="Use Sputnik Instant Setup";
hotspot.sputnik_express="Use SputnikNet Express";

// **************************************************************** index_heartbeat.asp *********************************************************//

var idx_h = new Object();
idx_h.srv="Heart Beat Server";
idx_h.con_strgy="Connection Strategy";
idx_h.max_idle="Connect on Demand: Max Idle Time";
idx_h.alive="Keep Alive: Redial Period";

// **************************************************************** index_l2tp.asp *********************************************************//

var idx_l = new Object();
idx_l.srv="L2TP Server";

// **************************************************************** index_pppoe.asp *********************************************************//

var idx_pppoe = new Object();
idx_pppoe.srv="Service Name";
idx_pppoe.use_rp="Use RP PPPoE";

// **************************************************************** index_pptp.asp *********************************************************//

var idx_pptp = new Object();
idx_pptp.srv="Use DHCP";
idx_pptp.wan_ip="Internet IP Address";
idx_pptp.subnet=share.subnet;
idx_pptp.gateway="Gateway (PPTP Server)";
idx_pptp.encrypt="PPTP Encyption";

// **************************************************************** index_static.asp *********************************************************//

var idx_static = new Object();
idx_static.gateway="Gateway";
idx_static.dns="Static DNS";

// **************************************************************** index.asp *********************************************************//

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
idx.localdns="Local DNS";
idx.legend2="WAN Port";
idx.wantoswitch="Assign WAN Port to Switch";
idx.legend3="Time Settings";
idx.timeset="Time Zone / Summer Time (DST)";
idx.localtime="Use local time";


//help container
var hidx = new Object();
hidx.right1="Automatic Configuration - DHCP:";
hidx.right2="This setting is most commonly used by Cable operators.";
hidx.right3="Host Name:";
hidx.right4="Enter the host name provided by your ISP.";
hidx.right5="Domain Name:";
hidx.right6="Enter the domain name provided by your ISP.";
hidx.right7="Local IP Address:";
hidx.right8="This is the address of the router.";
hidx.right9=share.subnet + ":";
hidx.right10="This is the subnet mask of the router.";
hidx.right11="DHCP Server:";
hidx.right12="Allows the router to manage your IP addresses.";
hidx.right13="Starting IP Address:";
hidx.right14="The address you would like to start with.";
hidx.right15="Maximum number of DHCP Users:";
hidx.right16="You may limit the number of addresses your router hands out.";
hidx.right17="Time Setting:";
hidx.right18="Choose the time zone you are in and Summer Time (DST) period. The router can use local time or UTC time.";


// **************************************************************** Join.asp **********************************************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Join";
join.mess1="Successfully joined the following network as a client: ";


// **************************************************************** Log_incoming.asp **********************************************************************//

var log_in = new Object();
log_in.titl=" - Incoming Log Table";
log_in.h2="Incoming Log Table";
log_in.th_ip="Source IP";
log_in.th_port="Destination Port Number";


// **************************************************************** Log_outgoing.asp **********************************************************************//

var log_out = new Object();
log_out.titl=" - Outgoing Log Table";
log_out.h2="Outgoing Log Table";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destination URL/IP";
log_out.th_port="Service/Port Number";


// **************************************************************** Log.asp **********************************************************************//

var log = new Object();
log.titl=" - Log";
log.h2="Log Management";
log.legend="Log";
log.lvl="Log Level";
log.drop="Dropped";
log.reject="Rejected";
log.accept="Accepted";





// **************************************************************** Site_Survey.asp **********************************************************************//

var survey = new Object();
survey.titl=" - Site Survey";
survey.h2="Neighbor&#39;s Wireless Networks";
survey.thjoin="Join Site";


// **************************************************************** Services.asp **********************************************************************//

var service = new Object();

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Password Login";
service.ssh_key="Authorized Keys";


// **************************************************************** WOL.asp **********************************************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Available Hosts";
wol.legend2="WOL Addresses";
wol.legend3="Output";
wol.legend4="Manual WOL";
wol.enable="Enable WOL?";
wol.add_wol="Add WOL Host";
wol.restore="Restore Factory Defaults";
wol.mac="MAC Address(es)";
wol.broadcast="Net Broadcast";
wol.udp="UDP Port";
wol.msg1="Click to remove WOL host";

//help container
var hwol = new Object();
hwol.right1="Local Wake-on-LAN:";
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router).";
hwol.right3="MAC Address(es):";
hwol.right4="MAC Addresses are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB)";
hwol.right5="IP Address:";
hwol.right6="IP Address is typically the broadcast address for the local network, but could be a remote address if the target host is not connected to the router's local network."

// **************************************************************** WL_WPATable.asp **************************************************************//
// **************************************************************** WPA.asp **********************************************************************//

var wpa = new Object();
wpa.titl=" - Wireless Security";
wpa.h2="Wireless Security";
wpa.h3="Wireless Encryption";
wpa.auth_mode="Network Authentication";
wpa.psk="WPA Pre-Shared Key";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval (in seconds)";
wpa.radius_ipaddr="RADIUS Server IP";
wpa.radius_port="RADIUS Server Port";
wpa.radius_key="RADIUS Key";
wpa.algorithms="WPA Algorithms";
wpa.shared_key="WPA Shared Key";


//help container
var hwpa = new Object();
hwpa.right1="Security Mode:";
hwpa.right2="You may choose from Disable, WEP, WPA Pre-Shared Key, WPA RADIUS, or RADIUS. All devices on your network must use the same security mode.";

// **************************************************************** WL_FilterTable.asp **************************************************************//

var wl_filter = new Object();
wl_filter.titl=" - MAC Address Filter List";
wl_filter.h2="MAC Address Filter List";
wl_filter.h3="Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";

// **************************************************************** WL_ActiveTable.asp **************************************************************//

var wl_active = new Object();
wl_active.titl=" - Wireless Active Client MAC List";
wl_active.h2="Wireless Client MAC List";
wl_active.h3="Enable MAC Filter";
wl_active.active="Active PC";
wl_active.inactive="Active PC";

// **************************************************************** Wireless_WDS.asp ***************************************************************//

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

//help container
var hwds = new Object();
hwds.right1=":";
hwds.right2="";






// **************************************************************** Fail_s.asp & Fail_u_s.asp & Fail.asp **********************************************************************//

var fail = new Object();
fail.mess1="The values you entered are invalid. Please try again.";
fail.mess2="Upgrade failed.";


// **************************************************************** Success*.asp & Reboot.asp  **********************************************************************//

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





// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

var dhcp = new Object();
dhcp.titl=" - DHCP Active IP Table";
dhcp.h2="DHCP Active IP Table";
dhcp.server="DHCP Server IP Address :";
dhcp.tclient="Client Host Name";


