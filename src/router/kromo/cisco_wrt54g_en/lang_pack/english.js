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
share.none="None";
share.both="Both";


var sbutton = new Object();
sbutton.save="Save Settings";
sbutton.save.value="Saved";
sbutton.cancel="Cancel Changes";
sbutton.refres="Refresh";
sbutton.clos="Close";
sbutton.del="Delete";
sbutton.continu="Continue";
sbutton.add="Add";
sbutton.remove="Remove";



// ************************************************************ COMMON ERROR MESSAGES  ***************************************************************//
var errmsg = new Object();
errmsg.err0="You must input a username.";
errmsg.err1="You must input a Router Name.";
errmsg.err2="Out of range, please adjust start IP address or user's numbers.";
errmsg.err3="You must at least select a day."
errmsg.err4="The end time must be bigger than start time.";
errmsg.err5="The MAC Address length is not correct.";
errmsg.err6="You must input a password.";
errmsg.err7="You must input a hostname.";
errmsg.err8="You must input an IP Address or Domain Name.";
errmsg.err9="Illegal DMZ IP Address.";
errmsg.err10="Confirmed password did not match Entered Password. Please re-enter password.";
errmsg.err11="No spaces are allowed in Password";

errmsg.err12="Confirmed password did not match Entered Password.  Please re-enter password";

errmsg.err13="Upgrade are failed.";
//common.js error message
errmsg.err14=" value is out of range ";
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
errmsg.err34="IP";
errmsg.err35="Gateway";
errmsg.err36="DNS";
errmsg.err37="netmask";
errmsg.err38="Idle time";
errmsg.err39="Redial period";
errmsg.err40="DHCP starting IP";
errmsg.err41="Number of DHCP users";
errmsg.err42="DHCP Lease Time";
errmsg.err43="URL";
errmsg.err44="Keyword";
errmsg.err45="Name";
errmsg.err46="Port";
errmsg.err47="Port%20number";



// **************************************************************  COMMON MENU ENTRIES  **********************************************************//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basic Setup";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Clone";
bmenu.setuprouting="Advanced Routing";
bmenu.setupvlan="VLANs";

bmenu.wireless="Wireless";


bmenu.security="Security";


bmenu.accrestriction="Access Restrictions";


bmenu.applications="Applications &amp; Gaming";
bmenu.applicationsprforwarding="Port Range Forwarding";
bmenu.applicationspforwarding="Port Forwarding";
bmenu.applicationsptriggering="Port Triggering";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";


bmenu.admin="Administration";


bmenu.statu="Status";


// **************************************************************** DDNS.asp **********************************************************************//

var ddns = new Object();
ddns.titl=" - Dynamic DNS"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS Service";
ddns.dyndns="DynDNS.org";
ddns.tzo="TZO.com";
ddns.zone="ZoneEdit.com";
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
hddns.right1="DDNS Service :";
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
hprforward.right1="Port Range Forward :";
hprforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>Enable</em> checkbox after you are finished.";



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
hpforward.right1="Port Forward :";
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>Enable</em> checkbox after you are finished.";

