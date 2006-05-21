//////////////////////////////////////////////////////////////////////////////////////////////
//				French translation DD-WRT V23 SP1 by Botho 17/05/2006															//
//////////////////////////////////////////////////////////////////////////////////////////////


// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var share = new Object();
share.firmware="Micrologiciel";
share.time="Heure";
share.interipaddr="Adresse IP Internet";
share.more="Plus...";
share.help="Aide";
share.enable="Activer";
share.enabled="Activé";
share.disable="Désactiver";
share.disabled="Désactivé";
share.usrname="Nom d'utilisateur";
share.passwd="Mot de passe";
share.hostname="Nom d'hôte";
share.domainname="Nom de domaine";
share.statu="Etat";
share.start="Début";
share.end="Fin";
share.proto="Protocole";
share.ip="Adresse IP";
share.mac="Adresse MAC";
share.none="Aucun";
share.both="Les deux";
share.del="Supprimer";
share.remove="Enlever";
share.descr="Description";
share.from="De";
share.to="A";
share.about="A propos";
share.everyday="Tous les jours";
share.sun="Dimanche";
share.sun_s="Dim";
share.sun_s1="D";
share.mon="Lundi";
share.mon_s="Lun";
share.mon_s1="L";
share.tue="Mardi";
share.tue_s="Mar";
share.tue_s1="M";
share.wed="Mercredi";
share.wed_s="Mer";
share.wed_s1="M";
share.thu="Jeudi";
share.thu_s="Jeu";
share.thu_s1="J";
share.fri="Vendredi";
share.fri_s="Ven";
share.fri_s1="V";
share.sat="Samedi";
share.sat_s="Sam";
share.sat_s1="S";
share.expires="Expiration";
share.yes="Oui";
share.no="Non";
share.allow="Autoriser";
share.deny="Interdire";
share.range="Plage";
share.use="Utiliser";
share.mins="Min.";
share.secs="Sec.";
share.routername="Nom du routeur";
share.manual="Manuel";
share.port="Port";
share.ssid="SSID";
share.channel="Canal";
share.rssi="Rssi";
share.signal="Signal";
share.noise="Bruit";
share.beacon="beacon";
share.openn="Open";
share.dtim="dtim";
share.rates="Vitesse";
share.low="Faible";
share.medium="Moyen";
share.high="Haut";
share.option="Options";
share.rule="Règle";
share.lan="LAN";
share.point2point="Point à Point";
share.nat="NAT";
share.subnet="Masque sous-réseau";
share.unmask="Afficher";
share.deflt="Défaut";					//don't use share.default
share.all="Tous";
share.auto="Auto";
share.right="Droite";
share.left="Gauche";
share.share_key="Clé partagée";
share.inter="Intervalle (secondes)";
share.srv="Nom du service";
share.port_range="Plage de ports";
share.priority="Priorité";
share.gateway="Passerelle";
share.intrface="Interface";				//don't use share.interface, Mozilla problem
share.router="Routeur";
share.static_lease="Réservation d'adresse";
share.srvip="IP Serveur";
share.localdns="DNS Local";
share.minutes="minutes";
share.oui="Recherche OUI";
share.sttic="Statique";
share.connecting="Connection en cours";
share.connect="Connecter";
share.connected="Connecté";
share.disconnect="Déconnecter";
share.disconnected="Déconnecté";
share.info="Information";
share.state="Etat";
share.mode="Mode";
share.encrypt="Cryptage";
share.key="Clé";
share.wireless="Sans fil";
share.dhcp="DHCP";
share.styl="Skin";


var sbutton = new Object();
sbutton.save="Enregistrer les paramètres";
sbutton.saving="Enregistré";
sbutton.cmd="Patientez ...";
sbutton.cancel="Annuler les modifications";
sbutton.refres="Actualiser";
sbutton.clos="Fermer";
sbutton.del="Supprimer";
sbutton.continu="Continuer";
sbutton.add="Ajouter";
sbutton.remove="Enlever";
sbutton.modify="Modifier";
sbutton.deleted="Supprimé";
sbutton.delall="Supprimer tout";
sbutton.autorefresh="Auto-Actualisation On";
sbutton.backup="Sauvegarder";
sbutton.restore="Restaurer";
sbutton.cptotext="Copier dans la zone de texte";
sbutton.runcmd="Exécutez la commande";
sbutton.startup="Sauver Démarrage";
sbutton.firewall="Sauver Pare-feu";
sbutton.wol="Réveiller";
sbutton.add_wol="Ajouter l'hôte";
sbutton.manual_wol="Réveil manuel";
sbutton.summary="Résumé";
sbutton.filterIP="Liste des PC";
sbutton.filterMac="Liste de filtrage MAC";
sbutton.filterSer="Ajouter/Editer un Service";
sbutton.reboot="Redémarrer le Routeur";
sbutton.help="Aide";
sbutton.wl_client_mac="Liste des adresses MAC";
sbutton.update_filter="Liste des filtres";
sbutton.join="Rejoindre";
sbutton.log_in="Journal des connexions entrantes";
sbutton.log_out="Journal des connexions sortantes";
sbutton.apply="Valider";
sbutton.edit_srv="Ajouter/Editer Service";
sbutton.routingtab="Table de routage";
sbutton.wanmac="Obtenir l'adresse MAC du PC";
sbutton.dhcprel="Libérer DHCP";
sbutton.dhcpren="Renouveler DHCP";
sbutton.survey="Réseaux sans fil à portée";
sbutton.upgrading="Mise à jour ...";
sbutton.upgrade="Mettre à jour";
sbutton.preview="Visualiser";


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
errmsg.err13="Upgrade are failed.";
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
errmsg.err29=" have illegal ascii code."
errmsg.err30=" have illegal hexadecimal digits.";
errmsg.err31=" value is illegal.";
errmsg.err32="IP address and gateway is not at same subnet mask.";
errmsg.err33="IP address and gateway can't be same.";
errmsg.err34=" is not allowed to contain a space.";

//Wol.asp error messages
errmsg.err35="You must input a MAC address to run."
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
errmsg.err47=("invalid SSID.");

//Wireless_WDS.asp error messages
errmsg.err48="WDS is not compatible with the current configuration of the router. Please check the following points :\n * Wireless Mode must be set to AP \n * WPA2 is not supported under WDS \n * Wireless Network B-Only mode is not supported under WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius is only available in AP mode.";

//Wireless_Basic.asp error messages
errmsg.err50="You must input a SSID.";

// Management.asp error messages
errmsg.err51="The Router is currently set to its default password. \
			As a security measure, you must change the password before the Remote Management feature can be enabled. \
			Click the OK button to change your password. Click the Cancel button to leave the Remote Management feature disabled.";
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

// *******************************************  COMMON MENU ENTRIES  *******************************************//
var bmenu= new Object();
bmenu.setup="Configuration";
bmenu.setupbasic="Configuration de base";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="Adresse MAC dupliquée";
bmenu.setuprouting="Routage avancé";
bmenu.setupvlan="LAN Virtuel";

bmenu.wireless="Sans fil";
bmenu.wirelessBasic="Paramètres de base";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Sécurité";
bmenu.wirelessMac="Filtrage MAC";
bmenu.wirelessAdvanced="Paramètres avancés";
bmenu.wirelessWds="WDS";

bmenu.security="Sécurité";
bmenu.firwall="Pare-feu";
bmenu.vpn="VPN";

bmenu.accrestriction="Restrictions d'accès";
bmenu.webaccess="Accès Internet";


bmenu.applications="Applications &amp; Jeux";
bmenu.applicationsprforwarding="Redirection plage de port";
bmenu.applicationspforwarding="Redirection de port";
bmenu.applicationsptriggering="Déclenchement de connexion";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Vue d'ensemble";
bmenu.sipathphone="Carnet d'adresses";
bmenu.sipathstatus="Etat";

bmenu.admin="Administration";
bmenu.adminManagement="Gestion";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Services";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Journal";
bmenu.adminDiag="Diagnostics";
bmenu.adminWol="WOL";
bmenu.adminFactory="Paramètres usine";
bmenu.adminUpgrade="Mise à niveau";
bmenu.adminBackup="Sauvegarde";


bmenu.statu="Etat";
bmenu.statuRouter="Routeur";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Agent Sputnik";
bmenu.statuWLAN="Sans fil";
bmenu.statuSysInfo="Système";


// ******************************************* Alive.asp *******************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Schedule Reboot";
alive.sevr1="Schedule Reboot";
alive.time="Time (in seconds)";
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
ddns.titl=" - DNS Dynamique"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="Service DDNS";
ddns.emailaddr="Adresse E-mail";
ddns.typ="Type";
ddns.dynamic="Dynamique";
ddns.custom="Personnalisé";
ddns.wildcard="Wildcard";
ddns.statu="Statut";

var ddnsm = new Object();
ddnsm.dyn_strange="Erreur inconnue. Vérifier que vous êtes connecté au bon serveur";
ddnsm.dyn_good="DDNS a été mis à jour avec succès";
ddnsm.dyn_noupdate="Aucune mise à jour nécessaire pour le moment";
ddnsm.dyn_nohost="Nom d'hôte inconnu";
ddnsm.dyn_notfqdn="Nom d'hôte incorrect";
ddnsm.dyn_yours="Ce nom d'hôte ne vous appartient pas";
ddnsm.dyn_abuse="Hôte bloqué pour abus";
ddnsm.dyn_nochg="Adresse IP  has not changed since the last update";
ddnsm.dyn_badauth="Authentication failure (username or passwords)";
ddnsm.dyn_badsys="Invalid system parameter";
ddnsm.dyn_badagent="This useragent has been blocked";
ddnsm.dyn_numhost="Too many or too few hosts found";
ddnsm.dyn_dnserr="DNS internal error";
ddnsm.dyn_911="Unexpected error 911";
ddnsm.dyn_999="Unexpected error 999";
ddnsm.dyn_donator="A feature requested is only available to donators, please donate";
ddnsm.dyn_uncode="Unknown return code";

ddnsm.tzo_good="Operation Complete";
ddnsm.tzo_noupdate="No update needed at this time";
ddnsm.tzo_notfqdn="Invalid Domain Name";
ddnsm.tzo_notmail="Invalis Email";
ddnsm.tzo_notact="Invalid Action";
ddnsm.tzo_notkey="Invalid Key";
ddnsm.tzo_notip="Invalid IP address";
ddnsm.tzo_dupfqdn="Duplicate Domain Name";
ddnsm.tzo_fqdncre="Domain Name has already been created for this domain name";
ddnsm.tzo_expired="The account has expired";
ddnsm.tzo_error="An unexpected server error";

ddnsm.zone_701="Zone is not set up in this account";
ddnsm.zone_702="Update failed";
ddnsm.zone_703="One of either parameters <em>zones</em> or <em>host</em> are required";
ddnsm.zone_704="Zone must be a valid <em>dotted</em> internet name";
ddnsm.zone_705="Zone cannot be empty";
ddnsm.zone_707="Duplicate updates for the same host/ip, adjust client settings";
ddnsm.zone_201="No records need updating";
ddnsm.zone_badauth="Authorization fails (username or passwords)";
ddnsm.zone_good="ZoneEdit is updated successfully";
ddnsm.zone_strange="Strange server response, are you connecting to the right server ?";

ddnsm.all_closed="Le serveur DDNS est actuellement fermé";
ddnsm.all_resolving="Nom de domaine Resolving domain name";
ddnsm.all_errresolv="Domain name resolv fail";
ddnsm.all_connecting="Connection en cours...";
ddnsm.all_connectfail="La connection au serveur a échouée";
ddnsm.all_disabled="DDNS est désactivé";
ddnsm.all_noip="Connection Internet non détectée";

//help container
var hddns = new Object();
hddns.right2="DDNS vous permet d'attribuer un nom de domaine et d'hôte fixe à une adresse IP Internet dynamique.\
		Cela peut s'avérer utile si vous hébergez votre propre site Web, un serveur FTP ou tout autre type de serveur \
		derrière le routeur. Avant d'opter pour cette fonctionnalité, vous devez souscrire à un service DDNS auprès de fournisseurs spécialisés, tels que DynDNS.org, TZO.com ou ZoneEdit.com.";



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
hdiag.right2="You can run command lines via the webinterface. Fill the text area with your command and click <em>" + sbutton.runcmd + "</em> to submit.";



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



// ******************************************* FilterIP%AC.asp *******************************************//

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
firewall.ping="Block Anonymous Internet Requests";
firewall.muticast="Filter Multicast";
filter.nat="Filter Internet NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container
var hfirewall = new Object();
hfirewall.right1="Firewall Protection:";
hfirewall.right2="Enable or disable the SPI firewall.";



// ******************************************* Forward.asp *******************************************//

var prforward = new Object();
prforward.titl=" - Port Range Forwarding";
prforward.h2="Port Range Forward";
prforward.legend="Forwards";
prforward.app="Application";

//help container
var hprforward = new Object();
hprforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>" + share.enable +"</em> checkbox after you are finished.";



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
hpforward.right2="Certain applications may require to open specific ports in order for it to function correctly. \
	Examples of these applications include servers and certain online games. \
	When a request for a certain port comes in from the Internet, the router will route the data to the computer you specify. \
	Due to security concerns, you may want to limit port forwarding to only those ports you are using, \
	and uncheck the <em>Enable</em> checkbox after you are finished.";



// ******************************************* Hotspot.asp *******************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot Portal";
hotspot.legend="Chillispot";
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
hotspot.sputnik_about="about Sputnik";



// ******************************************* Info.htm *******************************************//

var info = new Object();
info.titl=" - Information";
info.h2="Information du Système";
info.wlanmac="WLAN MAC";
info.srv="Services";



// ******************************************* index_heartbeat.asp *******************************************//

var idx_h = new Object();
idx_h.srv="Serveur Heart Beat";
idx_h.con_strgy="Strategie de connexion";
idx_h.max_idle="Connexion à la demande : délai d'inactivité maximal";
idx_h.alive="Maintenir la connexion : rappel après";



// ******************************************* index_l2tp.asp *******************************************//

var idx_l = new Object();
idx_l.srv="Serveur L2TP";



// ******************************************* index_pppoe.asp *******************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="Utiliser RP-PPPoE";



// ******************************************* index_pptp.asp *******************************************//

var idx_pptp = new Object();
idx_pptp.srv="Utiliser DHCP";
idx_pptp.wan_ip="Adresse IP Internet";
idx_pptp.gateway="Passerelle";
idx_pptp.encrypt="Cryptage PPTP";



// ******************************************* index_static.asp *******************************************//

var idx_static = new Object();
idx_static.dns="DNS Statique";



// ******************************************* index.asp *******************************************//

var idx = new Object();
idx.titl=" - Configuration";
idx.h2="Configuration Internet";
idx.h22="Configuration Sans fil";
idx.legend="Type de connexion Internet";
idx.conn_type="Type de connexion";
idx.stp="STP";
idx.stp_mess="(Désactiver pour le FAI COMCAST)";
idx.optional="Paramètres facultatifs (exigés par certains FAI)";
idx.mtu="MTU";
idx.h23="Configuration réseau";
idx.routerip="Adresse IP du routeur";
idx.lanip="Adresse IP";
idx.legend2="Port WAN";
idx.wantoswitch="Ajouter le port WAN au Switch";
idx.legend3="Réglage de l'heure";
idx.timeset="Fuseau horaire / Heure d'été";
idx.localtime="Utiliser l'heure locale";
idx.static_ip="Adresse IP statique";
idx.dhcp="Configuration automatique - DHCP";
idx.dhcp_legend="Paramètres du serveur d'adresse de réseau (DHCP)";
idx.dhcp_type="Type de DHCP";
idx.dhcp_srv="Serveur DHCP";
idx.dhcp_fwd="Transfert de DHCP";
idx.dhcp_start="Adresse IP de début";
idx.dhcp_end="Adresse IP de fin";				//used in Status_Lan.asp
idx.dhcp_maxusers="Nombre maximal d'utilisateurs DHCP";
idx.dhcp_lease="Durée de connexion du client";
idx.dhcp_dnsmasq="Utiliser DNSMasq pour DHCP";




//help container
var hidx = new Object();
hidx.right2="Vous devez conserver cette option uniquement si votre FAI prend en charge le protocole DHCP ou si vous vous connectez via une adresse IP dynamique (cas des câblo-opérateurs).";
hidx.right4="Saisissez le nom d'hôte du routeur qui vous est fourni pour votre FAI. Dans la plupart des cas, vous pourrez laisser ce champ vide.";
hidx.right6="Saisissez le nom de domaine du routeur qui vous est fourni pour votre FAI. Dans la plupart des cas, vous pourrez laisser ce champ vide.";
hidx.right8="Saisissez l'dresse IP du routeur.";
hidx.right10="Saisissez le masque de sous-réseau du routeur.";
hidx.right12="Vous pouvez utiliser le routeur en tant que serveur DHCP de votre réseau. Un serveur DHCP permet d'attribuer automatiquement une adresse IP à chaque ordinateur de votre réseau.";
hidx.right14="Saisissez une valeur de début pour la publication d'adresses IP sur le serveur DHCP.";
hidx.right16="Saisissez le nombre maximal d'ordinateurs auxquels le serveur DHCP doit attribuer des adresses IP. Ce nombre ne peut être supérieur à 253.";
hidx.right18="Changez le fuseau horaire de votre réseau à partir du menu déroulant. Le routeur peut utiliser l'heure UTC ou l'heure locale.";



// ******************************************* Join.asp *******************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Rejoindre";
join.mess1="Vous avez rejoint avec succès le réseau sans fil suivant : ";



// ******************************************* Log_incoming.asp *******************************************//

var log_in = new Object();
log_in.titl=" - Journal des connexions entrantes";
log_in.h2="Journal des connexions entrantes";
log_in.th_ip="Adresse IP source";
log_in.th_port="Port de destination";



// ******************************************* Log_outgoing.asp *******************************************//

var log_out = new Object();
log_out.titl=" - Journal des connexions sortantes";
log_out.h2="Journal des connexions sortantes";
log_out.th_lanip="Adresse IP locale";
log_out.th_wanip="URL/IP destination";
log_out.th_port="Service/Port";



// ******************************************* Log.asp *******************************************//

var log = new Object();
log.titl=" - Journal";
log.h2="Gestion du journal";
log.legend="Journal";
log.lvl="Niveau de détail";
log.drop="Ignoré";
log.reject="Rejeté";
log.accept="Accepté";



// ******************************************* Management.asp *******************************************//

var management = new Object();
management.titl=" - Administration";
management.h2="Administration du Routeur";

management.psswd_legend="Administrateur";
management.psswd_user="Nom d'utilisateur";
management.psswd_pass="Mot de passe";
management.pass_conf="Confirmation du mot de passe";

management.remote_legend="Accès Distant au Routeur";
management.remote_gui="Accès à la console";
management.remote_https="Utilisation de HTTPS";
management.remote_guiport="Port de la console";
management.remote_ssh="Accès SSH";
management.remote_sshport="Port SSH";

management.web_legend="Console d'Administration";
management.web_refresh="Auto-Actualisation (en secondes)";
management.web_sysinfo="Enable Info Site";
management.web_sysinfopass="Info Site Password Protection";
management.web_sysinfomasq="Info Site MAC Masquerading";

management.boot_legend="Temporisation de Démarrage";
management.boot_srv="Boot Wait";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.dsn_legend="DNSMasq";
management.dsn_srv="DNSMasq";
management.dsn_loc="DNS Local";
management.dsn_opt="Options DNS";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="Client NTP";
management.ntp_srv="NTP";

management.rst_legend="Bouton d'initialisation";
management.rst_srv="Bouton d'initialisation";

management.routing_legend="Routage";
management.routing_srv="Routage";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="mot de passe SecureOn";
management.wol_mac="Adresses MAC<br/>( Format: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="Support IPv6";
management.ipv6_srv="IPv6";
management.ipv6_rad="Activation Radvd";
management.ipv6_radconf="Configuration Radvd";

management.jffs_legend="Support JFFS2";
management.jffs_srv="JFFS2";
management.jffs_clean="Effacer JFFS2";

management.lang_legend="Langue";
management.lang_srv="Langue";
management.lang_bulgarian="bulgare";
management.lang_tradchinese="chinois traditionnel";
management.lang_croatian="croate";
management.lang_czech="tchèque";
management.lang_dutch="hollandais";
management.lang_english="anglais";
management.lang_french="francais";
management.lang_german="allemand";
management.lang_italian="italien";
management.lang_brazilian="brésilien";
management.lang_slovenian="slovène";
management.lang_spanish="espagnol";
management.lang_swedish="suédois";

management.net_legend="Filtrage IP (modifier pour une utilisation P2P)";
management.net_port="Nombre de connexions maximum";
management.net_tcptimeout="TCP Timeout (en secondes)";
management.net_udptimeout="UDP Timeout (en secondes)";

management.clock_legend="Overclocking";
management.clock_frq="Fréquence";
management.clock_support="Non supporté";

management.mmc_legend="Support de carte MMC/SD";
management.mmc_srv="Dispositif MMC";

management.samba_legend="Système de fichier Samba";
management.samba_srv="Samba Filesystem";
management.samba_share="Partage";
management.samba_stscript="Script de démarrage";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="Port SIP";
management.SIPatH_domain="Domaine SIP";

management.gui_style="Skin de la console";


//help container
var hmanagement = new Object();
hmanagement.right1="Auto-Actualisation:";
hmanagement.right2="Saisissez l'intervalle de rafraichissement automatique de la console d'administration (seules certaines pages bénéficient de cette fonction). La valeur 0 désactive cette fonctionnalité.";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

var portserv = new Object();
portserv.titl=" - Port Services";
portserv.h2="Port Services";



// ******************************************* QoS.asp *******************************************//

var qos = new Object();
qos.titl=" - Quality of Service";
qos.h2="Quality Of Service (QoS )";
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
qos.prio_s="Standard";
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
hqos.right10="You may specify priority for all traffic from a device on your network by giving the device a Device Name, \
			specifying priority and entering its MAC address.";
hqos.right12="You may control your data rate according to which physical LAN port your device is plugged into. \
			You may assign Priorities accordingly for devices connected on LAN ports 1 through 4.";



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
hroute.right1="Operating Mode:";
hroute.right2="If the router is hosting your Internet connection, select <em>Gateway</em> mode. If another router exists on your network, select <em>Router</em> mode.";
hroute.right3="Select Set Number:";
hroute.right4="This is the unique route number, you may set up to 20 routes.";
hroute.right5="Route Name:";
hroute.right6="Enter the name you would like to assign to this route.";
hroute.right7="Destination LAN IP:";
hroute.right8="This is the remote host to which you would like to assign the static route.";
hroute.right9="Subnet Mask:";
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
service.dhcp_domain="Used Domain";
service.dhcp_landomain="LAN Domain";
service.dhcp_option="Additional DHCPd Options";

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
service.vpn_clikey="Private Client Key";

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Password Login";
service.ssh_key="Authorized Keys";



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

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="This is the Router's MAC Address, as seen on your local, Ethernet network.";
hstatus_lan.right4="This shows the Router's IP Address, as it appears on your local, Ethernet network.";
hstatus_lan.right6="When the Router is using a Subnet Mask, it is shown here.";
hstatus_lan.right8="If you are using the Router as a DHCP server, that will be displayed here.";
hstatus_lan.right10="By clicking on any MAC address, you will obtain the Organizationally Unique Identifier of the network interface (IEEE Standards OUI database search).";



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
status_router.www_login="Login Type";
status_router.www_loginstatus="Login Status";

//help container
var hstatus_router = new Object();
hstatus_router.right2="This is the specific name for the router, which you set on the <i>Setup</i> tab.";
hstatus_router.right4="This is the router's MAC Address, as seen by your ISP.";
hstatus_router.right6="This is the router's current firmware.";
hstatus_router.right8="This is the time, as you set on the Setup Tab.";
hstatus_router.right10="This is a measure of the time the router has been \"up\" and running.";
hstatus_router.right12="This is given as three numbers that represent the system load during the last one, five, and fifteen minute periods.";
hstatus_router.right14="This shows the information required by your ISP for connection to the Internet. \
				This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.";



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
status_wireless.legend2="Packet Info";
status_wireless.rx="Received (RX)";
status_wireless.tx="Transmitted (TX)";
status_wireless.h22="Wireless Nodes";
status_wireless.legend3="Clients";
status_wireless.signal_qual="Signal Quality";
status_wireless.wds="WDS Nodes";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="This is the Router's MAC Address, as seen on your local, wireless network.";
hstatus_wireless.right4="As selected from the Wireless tab, this will display the wireless mode (Mixed, G-Only, or Disabled) used by the network.";



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
upgrad.resetOff="No reset";
upgrad.resetOn="Default settings";
upgrad.file="Please select a file to upgrade";
upgrad.warning="W A R N I N G";
upgrad.mess1="Upgrading firmware may take a few minutes.<br />Do not turn off the power or press the reset button!";

//help container
var hupgrad = new Object();
hupgrad.right2="Click on the <em>Browse...</em> button to select the firmware file to be uploaded to the router.<br /><br /> \
			Click the <em>Upgrade</em> button to begin the upgrade process. Upgrade must not be interrupted.";



// ******************************************* UPnP.asp *******************************************//

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
hupnp.right2="Click the trash can to delete an individual entry.";
hupnp.right4="Allows applications to automatically setup port forwardings.";



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
wol.add_wol="Add WOL Host";
wol.restore="Restore Factory Defaults";
wol.mac="MAC Address(es)";
wol.broadcast="Net Broadcast";
wol.udp="UDP Port";
wol.msg1="Click to remove WOL host";

//help container
var hwol = new Object();
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router).";
hwol.right4="MAC Addresses are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB)";
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
hwanmac.right2="Some ISP will require you to register your MAC address. \
			If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

var wpa = new Object();
wpa.titl=" - Wireless Security";
wpa.h2="Wireless Security";
wpa.legend="Wireless Encryption";
wpa.auth_mode="Network Authentication";
wpa.psk="WPA Pre-Shared Key";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval (in seconds)";
wpa.radius_ipaddr="RADIUS Server Address";
wpa.radius_port="RADIUS Server Port";
wpa.radius_key="RADIUS Key";
wpa.algorithms="WPA Algorithms";
wpa.shared_key="WPA Shared Key";
wpa.rekeyInt="rekey interval";

//help container
var hwpa = new Object();
hwpa.right1="Security Mode:";
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
radius.label3="Radius Server IP";
radius.label4="Radius Server Port";
radius.label5="Maximum Unauthenticated Users";
radius.label6="Password Format";
radius.label7="RADIUS Shared Secret";
radius.label8="Override Radius if Server is unavailable";
radius.mac="MAC";



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
wl_basic.clientBridge="Client Bridge";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="Mixed";
wl_basic.b="B-Only";
wl_basic.g="G-Only";
wl_basic.sensitivity="Default: 20000 meters";

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
hwl_adv.right1="Authentication Type:";
hwl_adv.right2="You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.";



// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

var fail = new Object();
fail.mess1="The values you entered are invalid. Please try again.";
fail.mess2="Upgrade failed.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="Settings saved.";
success.restore="Settings restored.<br/>Le routeur redémarre. Patientez un moment SVP ...";
success.upgrade="Upgrade successful.<br/>Le routeur redémarre. Patientez un moment SVP ...";
success.success_noreboot="Settings are successful.";
success.success_reboot=success.success_noreboot + "<br />Le routeur redémarre. Patientez un moment SVP ...";

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


var donate = new Object();
donate.mb="You may dontate through the Moneybookers account mb@dd-wrt.com too";
