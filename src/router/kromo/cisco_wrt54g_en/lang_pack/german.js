// Not working in IE & Opera ?08.05.2006 15:1
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
share.time="Zeit";
share.interipaddr="Internet IP Addresse";
share.more="Mehr...";
share.help="Hilfe";
share.enable="Einschalten";
share.enabled="An";
share.disable="Abschalten";
share.disabled="Aus";
share.usrname="Nutzername";
share.passwd="Passwort";
share.hostname="Host Name";
share.domainname="Domain Name";
share.statu="Status";
share.start="Start";
share.end="Ende";
share.proto="Protokol";
share.ip="IP Address";
share.mac="MAC Addresse";
share.none="Nichts";
share.both="Beides";
share.del="Löschen";
share.remove="Entfernen";
share.descr="Beschreibung";
share.from="Von";
share.to="Nach";
share.about="Über";
share.everyday="Jeden Tag";
share.sun="Sonntag";
share.sun_s="Son";
share.sun_s1="S";
share.mon="Montag";
share.mon_s="Mon";
share.mon_s1="M";
share.tue="Dienstag";
share.tue_s="Die";
share.tue_s1="D";
share.wed="Mittwoch";
share.wed_s="Mit";
share.wed_s1="W";
share.thu="Donnerstag";
share.thu_s="Don";
share.thu_s1="D";
share.fri="Freitag";
share.fri_s="Fre";
share.fri_s1="F";
share.sat="Samstag";
share.sat_s="Sam";
share.sat_s1="S";
share.expires="Läuft ab";
share.yes="Ja";
share.no="Nein";
share.allow="Erlauben";
share.deny="Verbieten";
share.range="Bereich";
share.use="Nutze";
share.mins="Min.";
share.secs="Sek.";
share.routername="Router Name";
share.manual="Manuell";
share.port="Port";
share.ssid="SSID";
share.channel="Kanal";
share.rssi="Signal";
share.noise="Rauschen";
share.beacon="beacon";
share.openn="Offen";
share.dtim="dtim";
share.rates="Rate";
share.low="Niedrig";
share.medium="Mittel";
share.high="Hoch";
share.option="Optionen";
share.rule="Regel";
share.lan="LAN";
share.point2point="Punkt zu Punkt";
share.nat="NAT";
share.subnet="Subnet Mask";
share.unmask="Unmask";
share.deflt="Standard";  //don't use share.default !!!
share.all="Alles";
share.auto="Auto";
share.right="Rechts";
share.left="Links";
share.share_key="Shared Key";
share.inter="Interval (in sekunden)";
share.srv="Service Name";
share.port_range="Port Bereich";



var sbutton = new Object();
sbutton.save="Einstellungen Speichern";
sbutton.saving="Gespeichert";
sbutton.cmd="Ausführen";
sbutton.cancel="Einstellungen zurücknehmen";
sbutton.refres="Auffrischen";
sbutton.clos="Schließen";
sbutton.del="Löschen";
sbutton.continu="Fortfahren";
sbutton.add="Hinzufügen";
sbutton.remove="Entfernen";
sbutton.modify="Verändern";
sbutton.deleted="Gelöscht";
sbutton.delall="Lösche Alles";
sbutton.refres="Auffrischen";
sbutton.autorefresh="Auto-Refresh ist An";
sbutton.backup="Sichern";
sbutton.restore="Wiederherstellen";
sbutton.cptotext="In den Text bereich kopieren";
sbutton.runcmd="Kommandos Ausführen";
sbutton.startup="Startup Speichern";
sbutton.firewall="Firewall Speichern";
sbutton.wol="Aufwecken";
sbutton.add_wol="Host hinzufügen";
sbutton.manual_wol="Manuelles Aufwecken";
sbutton.summary="Zusammenfassung";
sbutton.filterIP="Liste der PCs editieren";
sbutton.filterMac="MAC Filter Liste editieren";
sbutton.filterSer="Hinzufügen/Editieren Service";
sbutton.reboot="Router Neustarten";
sbutton.help="   Hilfe  ";
sbutton.wl_client_mac="Wireless Client MAC Liste";
sbutton.update_filter="Filter Liste updaten";
sbutton.join="Beitreten";
sbutton.log_in="Eingehendes Log";
sbutton.log_out="Ausgehendes Log";
sbutton.apply="Anwenden";



// ************************************************************ COMMON ERROR MESSAGES  ***************************************************************//
var errmsg = new Object();
errmsg.err0="Sie müssen einen Nutzernamen angeben.";
errmsg.err1="Sie müssen einen Routernamen angeben.";
errmsg.err2="Auserhalb des zulässigen Bereichs, bitte verändern Sie die Start IP Addresse oder die anzahl der Nutzer.";
errmsg.err3="Sie müssen einen Tag auswählen."
errmsg.err4="Die End Zeit muß größer als die Startzeit sein.";
errmsg.err5="Die MAC Addressen länge ist nicht korrekt.";
errmsg.err6="Sie müssen einen Nutzernamen angeben.";
errmsg.err7="Sie müssen einen Hostnamen angeben.";
errmsg.err8="Sie müssen eine IP Adresse oder einen Domainnamen angeben.";
errmsg.err9="Illegale DMZ IP Addresse.";
errmsg.err10="Das Passwort zur Bestätigung entspricht nicht dem originalen. Bitte geben Sie es neu ein.";
errmsg.err11="In einem Passwort sind keine Leerzeichen erlaubt";
errmsg.err12="Sie müssen ein Kommando zum Ausführen angeben.";
errmsg.err13="Das Upgrade ist fehlgeschlagen.";
errmsg.err45="Diese Funktion ist nicht unter HTTPS verfügbar! Bitte verwenden Sie den HTTP modus.";
errmsg.err46="Diese Funktion ist nicht unter HTTPS verfügbar";


//common.js error messages
errmsg.err14=" wert ist außerhalb des zulässigen Bereichs [";
errmsg.err15="Die WAN MAC Addresse ist außerhalb des gültigen Bereichs [00 - ff].";
errmsg.err16="Der zweite Wert der MAC Addresse muß eine gerade Zahl sein : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="Diese MAC Addresse ist nicht korrekt.";
errmsg.err18="Die länger der MAC Addresse ist nicht korrekt.";
errmsg.err19="Die MAC Adresse kann nicht die Broadcast Addresse sein."
errmsg.err20="Geben Sie die MAC Adresse im Format (xx:xx:xx:xx:xx:xx) ein.";
errmsg.err21="Ungültiges MAC Addressen Format.";
errmsg.err22="Die WAN MAC Addresse ist nicht korrekt.";
errmsg.err23="Ungültiger HEX Wert ";
errmsg.err24=" in MAC Adresse gefunden ";
errmsg.err25="Der Schlüssel Wert ist nicht korrekt.";
errmsg.err26="Die Schlüssel länge ist nicht korrekt.";
errmsg.err27="Ungültige Subnetz Maske.";
errmsg.err28=" enthält ungültige Werte, diese müssen [ 0 - 9 ] sein.";
errmsg.err29=" enthält einen ungülten Ascii Wert."
errmsg.err30=" enthält illegale hexadezimale Werte.";
errmsg.err31=" ist ungültig.";
errmsg.err32="IP Addresse und Gateway sind nicht im selben Netzbereich.";
errmsg.err33="IP Address und Gateway können nicht die selben sein.";
errmsg.err34=" dark keine leerzeichen enthalten.";

//Wol.asp error messages
errmsg.err35="Sie müssen eine MAC Addresse zum fortfahren eingeben."
errmsg.err36="Sie müssen eine Netzwerk Broadcast Addresse angeben um fortzufahren.";
errmsg.err37="Sie müssen einen UDP Port angeben um fortzufahren.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Bitte geben sie einen Shared Key an!";
errmsg.err39="Ungültiger Schlüssel, er muß aus 8 bis 63 ASCII Werten oder 64 hexadezimalen Zahlen bestehen"
errmsg.err40="Sie müssen einen Schlüssel für Key angeben ";
errmsg.err41="Ungültige Schlüssen länge ";
errmsg.err43="Rekey Interval";

//config.asp error messages
errmsg.err42="Bitte geben sie eine Konfiguration zum wiederherstellen an.";

//WL_ActiveTable.asp error messages
errmsg.err44="Die totale Anzahl der Checks überschreitet 128.";

//Site_Survey.asp error messages
errmsg.err47=("Ungültige SSID.");

//Wireless_WDS.asp error messages
errmsg.err48="WDS ist nicht kompatibel mit der aktuellen Konfiguration des Routers. Bitte prüfen sie die folgenden Punkte :\n * Wireless Modus muß AP sein \n * WPA2 wird vom WDS nicht unterstützt \n * Wireless Netzwerk Modus Nur-B wird von WDS nicht unterstützt";

//Wireless_radauth.asp error messages
errmsg.err49="Radius ist nur im AP Modus verfügbar.";

//Wireless_Basic.asp error messages
errmsg.err50="Sie müssen eine SSID angeben.";

// Management.asp error messages
errmsg.err51="Der Router ist im Augenblick auf das Standard Kennwort konfiguriert. \
			Aus Sicherheitsgründen müssen sie das Kennwort ändern befor sie das Remote Management feature aktivieren. \
			Klicken Sie den OK Button um das Kennwort zu ändern. Klicken Sie den Abbrechen Button um die Remote Management Funktion ausgeschaltet zu lassen.";
errmsg.err52="Passwort überprüfung stimmt nicht überein.";

// Port_Services.asp error messages
errmsg.err53="Klicken Sie nach dem abschluss aller Aktionen den Anwenden Button um die Einstellungen zu übernehmen.";
errmsg.err54="Sie müssen einen Service Namen angeben.";
errmsg.err55="Der Service Name existiert bereits.";


// **************************************************************  COMMON MENU ENTRIES  **********************************************************//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Basis Setup";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Address Klonen";
bmenu.setuprouting="Erweitertes Routing";
bmenu.setupvlan="VLANs";

bmenu.wireless="Wireless";
bmenu.wirelessBasic="Basis Einstellungen";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Wireless Sicherheit";
bmenu.wirelessMac="MAC Filter";
bmenu.wirelessAdvanced="Erweiterte Einstellungen";
bmenu.wirelessWds="WDS";

bmenu.security="Sicherheit";
bmenu.firwall="Firewall";
bmenu.vpn="VPN";

bmenu.accrestriction="Zugriffs Beschränkung";
bmenu.webaccess="Internet Zugriff";


bmenu.applications="Applikationen &amp; Spiele";
bmenu.applicationsprforwarding="Port Bereichs Weiterleitung";
bmenu.applicationspforwarding="Port Weiterleitung";
bmenu.applicationsptriggering="Port Triggering";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";


bmenu.admin="Administration";
bmenu.adminManagement="Management";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Services";
bmenu.adminAlive="Lebenserhaltung";
bmenu.adminLog="Log";
bmenu.adminDiag="Diagnose";
bmenu.adminWol="WOL";
bmenu.adminFactory="Fabrik Einstellungen";
bmenu.adminUpgrade="Firmware Update";
bmenu.adminBackup="Backup";


bmenu.statu="Status";



// **************************************************************** DDNS.asp **********************************************************************//

var ddns = new Object();
ddns.titl=" - Dynamic DNS"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS Service";
ddns.emailaddr="E-mail Addresse";
ddns.typ="Type";
ddns.dynamic="Dynamisch";
// ddns.static="Static"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.sttic="Statisch";
ddns.custom="Spezifisch";
ddns.wildcard="Wildcard";
ddns.statu="DDNS Status";

var ddnsm = new Object();
ddnsm.dyn_strange="Ungewöhnliche Server antwort, ist dies der Richtige Server ?";
ddnsm.dyn_good="DDNS erfolgreich geupdated";
ddnsm.dyn_noupdate="Diesmal ist kein update erforderlich";
ddnsm.dyn_nohost="Der hostname existiert nicht";
ddnsm.dyn_notfqdn="Der Host Name ist nicht korrekt";
ddnsm.dyn_yours="Der Host gehört ihnen nicht";
ddnsm.dyn_abuse="Der Host wurde wegen mißbrauchs blockiert";
ddnsm.dyn_nochg="Die IP addresse hat sich seit dem letzten update nicht geändert";
ddnsm.dyn_badauth="Authentizifierung fehlgeschlagen (Nutzername oder Passwort)";
ddnsm.dyn_badsys="Ungüötiger System parameter";
ddnsm.dyn_badagent="Dieser Nutzeragent wurde blockiert";
ddnsm.dyn_numhost="Zu viele oder zu wenige Hosts gefunden";
ddnsm.dyn_dnserr="DNS interner fehler";
ddnsm.dyn_911="Unerwarteter Fehler 911";
ddnsm.dyn_999="Unerwarteter Fehler 999";
ddnsm.dyn_donator="Dieses Features ist nur für Spender verfügbar, bitte spenden sie";
ddnsm.dyn_uncode="Unbekannter Rückgabewert";

ddnsm.tzo_good="Operation erfolgreich ausgeführt ";
ddnsm.tzo_noupdate="Diesmal ist kein update notwendig";
ddnsm.tzo_notfqdn="Ungültiger Domain Name";
ddnsm.tzo_notmail="Ungültige Email";
ddnsm.tzo_notact="Ungültige Aktion";
ddnsm.tzo_notkey="Ungültiger Schlüssel";
ddnsm.tzo_notip="Ungültige IP Addresse";
ddnsm.tzo_dupfqdn="Duplizierter Domain Name";
ddnsm.tzo_fqdncre="Domain Name wurde schon erstellt";
ddnsm.tzo_expired="Der Account ist abgelaufen";
ddnsm.tzo_error="Ein unerwarteter Server fehler";

ddnsm.zone_701="Die Zone ist nicht in diesem Account eingestellt";
ddnsm.zone_702="Update fehlgeschlagen";
ddnsm.zone_703="Einer der Parameter <em>zones</em> oder <em>host</em> wird benötigt";
ddnsm.zone_704="Die Zone muß eine gültiger <em>dotted</em> Internet Name sein";
ddnsm.zone_705="Zone darf nicht leer sein";
ddnsm.zone_707="Doppeltes Update for den selben host/ip, bitte verändern sie die client einstellungen";
ddnsm.zone_201="Keiner der Einträge erfordert ein update";
ddnsm.zone_badauth="Authorisation fehlgeschlagen (Nutzername oder Passwort)";
ddnsm.zone_good="ZoneEdit wurde erfolgreich geupdatet";
ddnsm.zone_strange="Eigenartige Server Antwort! Sind Sie sicher das sie mit dem richtigen Server verbunden sind ?";

ddnsm.all_closed="DDNS server ist aktuelle geschlossen";
ddnsm.all_resolving="domain name Auflösen";
ddnsm.all_errresolv="Domain name auflösung fehlgeschlagen";
ddnsm.all_connecting="Mit Server verbinden";
ddnsm.all_connectfail="Verbindung mit Server fehlgeschlagen";
ddnsm.all_disabled="DDNS funktion ist abgeschaltet";
ddnsm.all_noip="Keine Internet Verbindung";

//help container
var hddns = new Object();
hddns.right1="DDNS Service:";
hddns.right2="DDNS erlaubt ihnen den Zugriff auf ihr Netzwerk unter zuhilfename eines Domain Namens anstatt einer IP Addresse. \
	Der Service verwaltet und ändert die IP Adresse sowie die domain information dynamisch.\
	Yie müssen sich für den Service bei TZO.com order DynDNS.ORG anmelden.";



// **************************************************************** Forward.asp **********************************************************************//

var prforward = new Object();
prforward.titl=" - Port Bereichs Weiterleitung";
prforward.h2="Port Bereichs Weiterleitung";
prforward.legend="Weiterleitungen";
prforward.app="Applikation";

//help container
var hprforward = new Object();
hprforward.right1="Port Bereichs Weiterleitung:";
hprforward.right2="Einige Applikationen benötigen manchmal offene Ports damit diese korrekt funktionieren. \
	Beispiele für diese Applikationen sind Server oder einige Online Spiele. \
	Wenn eine Anfrage auf einem dieser Ports aus dem Internet kommt, wird der Router diese auf den entsprechend angegebenen Computer weiterleiten. \
	Aus Sicherheitsgründen, sollten die Weiterleitung nur auf Ports beschränken die Sie benötigen.";
	


// **************************************************************** ForwardSpec.asp **********************************************************************//

var pforward = new Object();
pforward.titl=" - Port Weiterleitung";
pforward.h2="Port Weiterleitung";
pforward.legend="Weiterleitungen";
pforward.app="Applikation";
pforward.from="Von Port";
pforward.to="Nach Port";

//help container
var hpforward = new Object();
hpforward.right1="Port Weiterleitung:";
hpforward.right2="Einige Applikationen benötigen manchmal offene Ports damit diese korrekt funktionieren. \
	Beispiele für diese Applikationen sind Server oder einige Online Spiele. \
	Wenn eine Anfrage auf einem dieser Ports aus dem Internet kommt, wird der Router diese auf den entsprechend angegebenen Computer weiterleiten. \
	Aus Sicherheitsgründen, sollten die Weiterleitung nur auf Ports beschränken die Sie benötigen.";





// **************************************************************** Triggering.asp **********************************************************************//

var trforward = new Object();
trforward.titl=" - Port Triggering";
trforward.h2="Port Triggering";
trforward.legend="Weiterleitungen";
trforward.trrange="Getriggerter Port Bereich";
trforward.fwdrange="Weitergeleiteter Port Bereich";
trforward.app="Applikation";

//help container
var htrforward = new Object();
htrforward.right1="Applikation:";
htrforward.right2="Geben Sie den Applikationsnamen des Triggers ein.";
htrforward.right3="Getriggerter Bereich:";
htrforward.right4="For each application, list the triggered port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right5="Weitergeleiteter Bereich:";
htrforward.right6="For each application, list the forwarded port number range. Check with the Internet application documentation for the port number(s) needed.";
htrforward.right7="Start:";
htrforward.right8="Geben Sie den Start Port des Getriggerten und Weitergeleiteten Bereichs an.";
htrforward.right9="Ende:";
htrforward.right10="Geben Sie den End Port des Getriggertern und Weitergeleiteten Bereichs an.";



// **************************************************************** UPnP.asp **********************************************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug und Play (UPnP)";
upnp.legend="Weiterleitungen";
upnp.legend2="UPnP Konfiguration";
upnp.serv="UPnP Service";
upnp.clear="Löscht die Port Weiterleitungen beim Start";
upnp.url="Sende Präsentations URL";
upnp.msg1="Klicken sie um das Lease zu Löschen";
upnp.msg2="Alle Einträge löschen?";


//help container
var hupnp = new Object();
hupnp.right1="Weiterleitungen:";
hupnp.right2="Klicken sie auf den Papierkorb um einen individuellen Eintrag zu löschen.";
hupnp.right3="UPnP Service:";
hupnp.right4="Erlaubt Applikationen eine Automatische Port weiterleitung auf dem Router durchzuführen.";



// **************************************************************** Alive.asp **********************************************************************//

var alive = new Object();
alive.titl=" - Lebenserhaltung";
alive.h2="Lebenserhaltung";
alive.legend="Geplanter Neustart";
alive.sevr1="Reboot Planen";
alive.time="Zeit (in sekundenj)";
alive.hour="Zu einem festen Zeitpunkt";
alive.legend2="WDS/Verbindungsüberwachung";
alive.sevr2="Überwachung Einschalten";
alive.IP="IP Addressen";
alive.legend3="Proxy/Verbindungsüberwachung";
alive.sevr3="Proxy Überwachung Einschalten";
alive.IP2="Proxy IP Addresse";
alive.port="Proxy Port";

//help container
var halive = new Object();
halive.right1="Zu einem festen Zeitpunkt:";
halive.right2="Wählen Sie aus wann der Router neu starten soll. Cron muß hierfür auf dem Management Tab, eingeschaltet sein.";
halive.right3="IP Addressen:";
halive.right4="Maximal 3 IP's (mit einem <em>Lehrzeichen</em> getrennt) sind erlaubt.<BR/>IP Format ist xxx.xxx.xxx.xxx.";



// **************************************************************** config.asp **********************************************************************//

var config = new Object();
config.titl=" - Backup & Wiederherstellung";
config.h2="Backup Konfiguration";
config.legend="Backup Einstellungen";
config.mess1="Klickem Sie den \"" + sbutton.backup + "\" Button um die Backup Datei zu downloaden.";
config.h22="Konfiguration Wiederherstellen";
config.legend2="Einstellungen Wiederherstellen";
config.mess2="Bitte geben Sie eine Datei zur Wiederherstellung an";
config.mess3="W A R N U N G";
config.mess4="Uploaden Sie nur Dateien die auch mit dieser Firmware auf dem selben Routermodell erstellt wurden.!";

//help container
var hconfig = new Object();
hconfig.right1="Backup:";
hconfig.right2="Sie können hier Ihre aktuelle Konfiguration Backupen für den Fall das sie Ihren Router einmal Reseten müssen.<br /><br />Klicken Sie den <em>Backup</em> Button um ihre Konfiguration zu sichern.";
hconfig.right3="Wiederherstellung:";
hconfig.right4="Klicken Sie den <em>Durchsuchen...</em> Button um eine Konfigurations Datei zu auszuwählen die auf ihrem PC gespeichert ist.<br /><br />Klicken Sie den <em>" + sbutton.restore + "</em> Button um alle aktuellen Konfigurationseinträge mit ihrer neuen Konfiguration zu überschreiben.";



// **************************************************************** Diagnostics.asp **********************************************************************//

var diag = new Object();
diag.titl=" - Diagnose";
diag.h2="Diagnose";
diag.legend="Kommandozeile";
diag.cmd="Kommandos";
diag.startup="Startup";
diag.firewall="Firewall";

//help container
var hdiag = new Object();
hdiag.right1="Kommandos:";
hdiag.right2="Hier können Sie mit Hilfe des Web interfaces Kommandos auf der Kommandozeile starten. Geben Sie einfach ihre Kommandos in das Textfeld ein und Klicken Sie <em>" + sbutton.runcmd + "</em> um diese Auszuführen.";



// **************************************************************** DMZ.asp **********************************************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Demilitarisierte Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="DMZ Nutzen";
dmz.host="DMZ Host IP Addresse";


//help container
var hdmz = new Object();
hdmz.right1="DMZ:";
hdmz.right2="Wenn Sie diese Option aktivieren wird der angegebende Host in das Internet durchgeschaltet. Alle Ports sind dann vom Internet aus erreichbar";



// **************************************************************** Factory_Defaults.asp **********************************************************************//

var factdef = new Object();
factdef.titl=" - Werks Einstellungen";
factdef.h2="Werks Einstellungen";
factdef.legend="Router Einstellungen zurück setzen";
factdef.restore="Werkseinstellungen Wiederherstellen";

factdef.mess1="Warnung! Wenn Sie OK Clicken wird das Gerät auf Werks Einstellungen zurücl gesetzt und alle vorherigen einstellunge gelöscht.";

//help container
var hfactdef = new Object();
hfactdef.right1="Diese Funktion setzt alle Einstellungen auf Werkszustand zurück. Alle ihre Konfigurationen werden gelöscht.";



// **************************************************************** Filter.asp **********************************************************************//

var filter = new Object();
filter.titl=" - Zugriffs Beschränkung";
filter.h2="Internet Zugriff";
filter.legend="Zugriffs Richtlinie";
filter.restore="Werkseinstellungen Wiederherstellen";
filter.pol="Richtlinie";
filter.polname="Richtlinien Name";
filter.pcs="PCs";
filter.polallow="Internet zugriff wärend der angegebenen Tage und Stunden.";
filter.legend2="Tage";
filter.time="Zeiten";
filter.h24="24 Stunden";
filter.legend3="Blockierte Services";
filter.catchall="Alle P2P Protokolle wegfangen";
filter.legend4="Webseite mit Hilfe einer URL blockieren";
filter.legend5="Webseite mit Hilfe von Schlagworten blockieren";

filter.mess1="Löschen der Richtlinie?";
filter.mess2="Sie müssen mindestens einen Tag angeben.";
filter.mess3="Die Endzeit muß größer sein als die Startzeit.";

//help container
var hfilter = new Object();
hfilter.right1="Internet Zugriffs Richtlinie:";
hfilter.right2="Sie können bis zu 10 Zugriffs Richtlinien definieren. Klicken Sie <em>" + sbutton.del + "</em> um eine Richtlinie zu löschen, oder <em>" + sbutton.summary + "</em> um sich die zusammenfassung der Richtlinie anzuschauen.";
hfilter.right3="Status:";
hfilter.right4="Hier könnnen Sie eine Richtlinie Ein oder Ausschalten.";
hfilter.right5="Richtlinien Name:";
hfilter.right6="Hier können Sie ihrer Richtlinie einen Namen zuweisen.";
hfilter.right7="Tage:";
hfilter.right8="Hier können Sie ein oder mehere Tage selektieren an dem die Richtlinie gültig ist.";
hfilter.right9="Zeiten:";
hfilter.right10="Hier können Sie die Tageszeit angeben an dem die Richtlinie gültig ist.";
hfilter.right11="Blockierte Services:";
hfilter.right12="Hier können Sie Service auswählen die Blockiert werden sollen. Klicken Sie <em>" + sbutton.filterSer + "</em> um die Einstellungen zu ändern.";
hfilter.right13="Webseite mit Hilfe einer URL blockieren:";
hfilter.right14="Hier können Sie diverse Webseiten mit Hilfe einer URL Blockieren.";
hfilter.right15="Webseite mit Hilfe von Schlagworten blockieren:";
hfilter.right16="Hier können Sie mit hilfe von Schlagworten Webseiten blockieren die diese enthalten.";



// **************************************************************** FilterIP%AC.asp **********************************************************************//

var filterIP = new Object();
filterIP.titl=" - Liste der PCs";
filterIP.h2="Liste der PCs";
filterIP.h3="Geben Sie die MAC Adresse der PC in folgendem Format an: xx:xx:xx:xx:xx:xx";
filterIP.h32="Geben Sie die IP Adresse des PC's ein";
filterIP.h33="Geben Sie den IP Bereich der PC's an";
filterIP.ip_range="IP Bereich";



// **************************************************************** FilterSummary.asp **********************************************************************//

var filterSum = new Object();
filterSum.titl=" - Zugriffs Beschränkung Zusammenfassung";
filterSum.h2="Internet Richtlinie Zusammenfassung";
filterSum.polnum="Nr.";
filterSum.polday="Uhrzeit";



// **************************************************************** Firewall.asp **********************************************************************//

var firewall = new Object();
firewall.titl=" - Firewall";
firewall.h2="Sicherheit";
firewall.legend="Firewall Schutz";
firewall.firewall="SPI Firewall";
firewall.legend2="Zusätzliche Filter";
firewall.proxy="Filter Proxy";
firewall.cookies="Filter Cookies";
firewall.applet="Filter Java Applets";
firewall.activex="Filter ActiveX";
firewall.legend3="WAN Zugriffe Blockieren";
firewall.ping="Anonyme Internet Zugriffsversuche blockieren";
firewall.muticast="Filter Multicast";
filter.nat="Filter Internet NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container
var hfirewall = new Object();
hfirewall.right1="Firewall Schutz:";
hfirewall.right2="Schaltet die SPI firewall ein oder aus.";



// **************************************************************** Hotspot.asp **********************************************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot Portal";
hotspot.legend="Chillispot";
hotspot.hotspot="Chillispot";
hotspot.pserver="Primärer Radius Server IP/DNS";
hotspot.bserver="Secundärer Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="Umleitungs URL";
hotspot.dhcp="DHCP Interface";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM Secret";
hotspot.uamdns="UAM Any DNS";
hotspot.allowuam="UAM Allowed";
hotspot.macauth="MACauth";
hotspot.option="Zusätzliche Chillispot Optionen";
hotspot.fon_chilli="Chillispot Lokales Nutzer Management";
hotspot.fon_user="Nutzer Liste";
hotspot.http_legend="HTTP Umleitung";
hotspot.http_srv="HTTP Umleitung";
hotspot.http_ip="HTTP Ziel IP";
hotspot.http_port="HTTP Ziel Port";
hotspot.http_net="HTTP Ursprungs Netzwerk";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Gateway Name";
hotspot.nocat_home="Home Page";
hotspot.nocat_allowweb="Erlaubte Web Hosts";
hotspot.nocat_docroot="Dokumenten Pfad";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="Ports Ausschließen";
hotspot.nocat_timeout="Login Timeout";
hotspot.nocat_verbose="Verbosity";
hotspot.nocat_route="Nur Routen";
hotspot.smtp_legend="SMTP Umleitung";
hotspot.smtp_srv="SMTP Umleitung";
hotspot.smtp_ip="SMTP Ziel IP";
hotspot.smtp_net="SMTP Ursprungs Netzwerk";
hotspot.shat_legend="Zero IP Config";
hotspot.shat_srv="Zero IP Config";
hotspot.shat_srv2="Zero IP Config eingeschaltet";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik Agent";
hotspot.sputnik_id="Sputnik Server ID";
hotspot.sputnik_instant="Nutze Sputnik Instant Setup";
hotspot.sputnik_express="Nutze SputnikNet Express";



// **************************************************************** index_heartbeat.asp *********************************************************//

var idx_h = new Object();
idx_h.srv="Heart Beat Server";
idx_h.con_strgy="Connection Strategie";
idx_h.max_idle="Auf bedarf Verbinden: Maximale Ruhezeit";
idx_h.alive="Lebenserhaltung: Wiederwahl Periode";



// **************************************************************** index_l2tp.asp *********************************************************//

var idx_l = new Object();
idx_l.srv="L2TP Server";



// **************************************************************** index_pppoe.asp *********************************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="Nutze RP PPPoE";



// **************************************************************** index_pptp.asp *********************************************************//

var idx_pptp = new Object();
idx_pptp.srv="Nutze DHCP";
idx_pptp.wan_ip="Internet IP Addresse";
idx_pptp.gateway="Gateway (PPTP Server)";
idx_pptp.encrypt="PPTP Verschlüsselung";



// **************************************************************** index_static.asp *********************************************************//

var idx_static = new Object();
idx_static.gateway="Gateway";
idx_static.dns="Statischer DNS";



// **************************************************************** index.asp *********************************************************//

var idx = new Object();
idx.titl=" - Einstellungen";
idx.h2="Internet Einstellungen";
idx.h22="Wireless Einstellungen";
idx.legend="Internet Verbindungs Typ";
idx.conn_type="Verbindungs Type";
idx.stp="STP";
idx.stp_mess="(Für COMCAST Provider ausschalten)";
idx.optional="Zusätzliche Einstellungen (Wird von einigen Providern benötigt)";
idx.mtu="MTU";
idx.h23="Netzwerk Einstellungen";
idx.routerip="Router IP";
idx.lanip="Lokale IP Addresse";
idx.localdns="Lokaler DNS";
idx.legend2="WAN Port";
idx.wantoswitch="WAN Port dem Switch zuweisen";
idx.legend3="Zeit Einstellungen";
idx.timeset="Zeitzone / Sommerzeit (DST)";
idx.localtime="Nutze Lokale Zeit";

//help container
var hidx = new Object();
hidx.right1="Automatische Konfiguration - DHCP:";
hidx.right2="Diese Einstellung wird von fast allen Providern genutzt.";
hidx.right3="Host Name:";
hidx.right4="Geben Sie den Host Namen an der von ihrem Provider angegeben ist.";
hidx.right5="Domain Name:";
hidx.right6="Geben Sie hier den Domainnamen ein der ihnen von ihrem Provider zugewiesen wurde.";
hidx.right7="Lokale IP Addresse:";
hidx.right8="Dies ist die Adresse ihres Routers.";
hidx.right9=share.subnet + ":";
hidx.right10="Dies ist die Subnetz Maske ihres Routers.";
hidx.right11="DHCP Server:";
hidx.right12="Erlaubt dem Router ihre IP Adressen zu verwalten.";
hidx.right13="Start IP Addresse:";
hidx.right14="Die Adresse bei der sie Anfangen wollen.";
hidx.right15="Maximale Anzahl der DHCP Nutzer:";
hidx.right16="Sie können hier die Anzahl der Adressen einschränken die der Router erzeilt.";
hidx.right17="Zeit Einstellungen:";
hidx.right18="Wählen Sie hier ihre Zeitzone und die Sommerzeit Einstellungen. Der Router kann auch die Lokalzeit nutzen.";



// **************************************************************** Join.asp **********************************************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Beitreten";
join.mess1="Sie sind nun erfolgreich dem folgendem Netzerk als Client beigetreten: ";



// **************************************************************** Log_incoming.asp **********************************************************************//

var log_in = new Object();
log_in.titl=" - Eingehende Log Tabelle";
log_in.h2="Eingehende Log Tabelle";
log_in.th_ip="Ursprungs IP";
log_in.th_port="Ziel Port Nummer";



// **************************************************************** Log_outgoing.asp **********************************************************************//

var log_out = new Object();
log_out.titl=" - Ausgehende Log Tabelle";
log_out.h2="Ausgehende Log Tabelle";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Ziel URL/IP";
log_out.th_port="Service/Port Nummer";



// **************************************************************** Log.asp **********************************************************************//

var log = new Object();
log.titl=" - Log";
log.h2="Log Verwaltung";
log.legend="Log";
log.lvl="Log Level";
log.drop="Abgewiesen";
log.reject="Zurückgewiesen";
log.accept="Akzeptiert";



// **************************************************************** Management.asp **********************************************************************//

var management = new Object();
management.titl=" - Administration";
management.h2="Router Management";

management.psswd_legend="Router Password";
management.psswd_user="Router Nutzername";
management.psswd_pass="Router Passwort";
management.pass_conf="Zur bestätigen Wiedereingeben";

management.remote_legend="Fernzugriff";
management.remote_gui="Web GUI Management";
management.remote_https="Nutze HTTPS";
management.remote_guiport="Web GUI Port";
management.remote_ssh="SSH Management";
management.remote_sshport="SSH Port";

management.web_legend="Web Zugriff";
management.web_proto="Protokoll";
management.web_refresh="Automatisches Auffrischen (in seconds)";
management.web_sysinfo="Info Seite einschalten";
management.web_sysinfopass="Info Seite Passwort Schutz";
management.web_sysinfomasq="Info Seite MAC Maskierung";

management.boot_legend="Beim Booten Warten";
management.boot_srv="Beim Booten Warten";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.dsn_legend="DNS Masq";
management.dsn_srv="DNS Masq";
management.dsn_loc="Lokaler DNS";
management.dsn_opt="Zusätzliche DNS Optionen";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="NTP Client";
management.ntp_srv="NTP";
management.ntp_server="Server IP";

management.rst_legend="Reset Knopf";
management.rst_srv="Reset Knopf";

management.routing_legend="Routing";
management.routing_srv="Routing";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="SecureOn Passwort";
management.wol_mac="MAC Addressen<br/>( Format: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="IPv6 Unterstützung";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd eingeschaltet";
management.ipv6_radconf="Radvd Konfiguration";

management.jffs_legend="JFFS2 Unterstützung";
management.jffs_srv="JFFS2";
management.jffs_clean="JFFS2 Löschen";

management.lang_legend="Sprach Auswahl";
management.lang_srv="Sprache";
management.lang_bulgarian="bulgarian";
management.lang_tradchinese="tradchinese";
management.lang_croatian="croatian";
management.lang_czech="czech";
management.lang_dutch="dutch";
management.lang_english="english";
management.lang_french="french";
management.lang_german="german";
management.lang_italian="italian";
management.lang_brazilian="brazilian";
management.lang_slovenian="slovenian";
management.lang_spanish="spanish";
management.lang_swedish="swedish";

management.net_legend="IP Filter Einstellungen (Für P2P bitte anpassen)";
management.net_port="Maximale Ports";
management.net_tcptimeout="TCP Timeout (in Sekunden)";
management.net_udptimeout="UDP Timeout (in Sekunden)";

management.clock_legend="Übertakten";
management.clock_frq="Frequenz";
management.clock_support="Nicht unterstützt";

management.mmc_legend="MMC/SD Karten unterstützung";
management.mmc_srv="MMC Gerät";

management.samba_legend="Samba FS Automount";
management.samba_srv="SMB Filesystem";
management.samba_share="Share";
management.samba_stscript="Startscript";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP Port";
management.SIPatH_domain="SIP Domain";


//help container
var hmanagement = new Object();
hmanagement.right1="Automatisches Auffrischen:";
hmanagement.right2="Justiert das automatische auffrischen diverser GUI Elemente. 0 Schaltet dieses Feature ab.";



// **************************************************************** Port_Services.asp **********************************************************************//

var portserv = new Object();
portserv.titl=" - Port Services";
portserv.h2="Port Services";



// **************************************************************** QoS.asp **********************************************************************//

var qos = new Object();
qos.titl=" - Quality of Service";
qos.h2="Quality Of Service (QoS )";
qos.legend="QoS Settings";
qos.srv="Start QoS";
qos.uplink="Uplink (kbps)";
qos.dnlink="Downlink (kbps)";
qos.gaming="Optimize for Gaming";
qos.legend2="Services Priority";
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
hqos.right5="Application Priority:";
hqos.right6="You may control your data rate with respect to the application that is consuming bandwidth.";
hqos.right7="Netmask Priority:";
hqos.right8="You may specify priority for all traffic from a given IP address or IP Range.";
hqos.right9="MAC Priority:";
hqos.right10="You may specify priority for all traffic from a device on your network by giving the device a Device Name, \
			specifying priority and entering its MAC address.";
hqos.right11="Ethernet Port Priority:";
hqos.right12="You may control your data rate according to which physical LAN port your device is plugged into. \
			You may assign Priorities accordingly for devices connected on LAN ports 1 through 4.";



// **************************************************************** RouteTable.asp **********************************************************************//

var routetbl = new Object();
routetbl.titl=" - Routing Table";
routetbl.h2="Routing Table Entry List";
routetbl.th1="Destination LAN IP";
routetbl.eth_lan="LAN &amp; Wireless";
routetbl.eth_wan="WAN (Internet)";



// **************************************************************** Routing.asp **********************************************************************//

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




// **************************************************************** Site_Survey.asp **********************************************************************//

var survey = new Object();
survey.titl=" - Netzsuche";
survey.h2="Nachbarliche Wireless Netzwerke";
survey.thjoin="Netz Betreten";



// **************************************************************** Services.asp **********************************************************************//

var service = new Object();

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Passwort Login";
service.ssh_key="Authorized Keys";



// **************************************************************** WOL.asp **********************************************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Verfügbare Hosts";
wol.legend2="WOL Addressen";
wol.legend3="Ausgabe";
wol.legend4="Manuelles WOL";
wol.enable="WOL Einschalten?";
wol.add_wol="WOL Host Hinzufügen";
wol.restore="Werks Einstellungen Wiederherstellen";
wol.mac="MAC Addresse(n)";
wol.broadcast="Netz Broadcast";
wol.udp="UDP Port";
wol.msg1="Klicken um einen WOL Host zu entfernen";

//help container
var hwol = new Object();
hwol.right1="Local Wake-on-LAN:";
hwol.right2="This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your router).";
hwol.right3="MAC Address(es):";
hwol.right4="MAC Addresses are entered in the format xx:xx:xx:xx:xx:xx (i.e. 01:23:45:67:89:AB)";
hwol.right5="IP Address:";
hwol.right6="IP Address is typically the broadcast address for the local network, but could be a remote address if the target host is not connected to the router's local network."

// **************************************************************** WanMAC.asp **********************************************************************//

var wanmac = new Object();
wanmac.titl=" - MAC Address Clone";
wanmac.h2="MAC Address Clone";
wanmac.legend="MAC Clone";
wanmac.wan="Clone WAN MAC";
wanmac.wlan="Clone Wireless MAC";

//help container
var hwanmac = new Object();
hwanmac.right1="MAC Address Clone:";
hwanmac.right2="Some ISP will require you to register your MAC address. \
			If you do not wish to re-register your MAC address, you can have the router clone the MAC address that is registered with your ISP.";


// **************************************************************** WL_WPATable.asp **************************************************************//
// **************************************************************** WPA.asp **********************************************************************//

var wpa = new Object();
wpa.titl=" - Wireless Sicherheit";
wpa.h2="Wireless Sicherheit";
wpa.h3="Wireless Verschlüsselung";
wpa.auth_mode="Network Authentication";
wpa.psk="WPA Pre-Shared Key";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Renewal Interval (in sekunden)";
wpa.radius_ipaddr="RADIUS Server IP";
wpa.radius_port="RADIUS Server Port";
wpa.radius_key="RADIUS Key";
wpa.algorithms="WPA Algorithmus";
wpa.shared_key="WPA Shared Key";
wpa.rekeyInt="rekey interval";

//help container
var hwpa = new Object();
hwpa.right1="Sicherheits Modus:";
hwpa.right2="Hier können Sie zwischen Deaktiviert, WEP, WPA Pre-Shared Key, WPA RADIUS, oder RADIUS wählen. Alle Geräte in ihrem Netzwerk müssen den selben Modus verwenden.";



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
wl_active.inactive="Inactive PC";



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



// **************************************************************** Wireless_radauth.asp ***************************************************************//

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



// **************************************************************** Wireless_MAC.asp ***************************************************************//

var wl_mac = new Object();
wl_mac.titl=" - MAC Filter";
wl_mac.h2="Wireless MAC Filter";
wl_mac.legend="MAC Filter";
wl_mac.label="Use Filter";
wl_mac.label2="Filter Mode";
wl_mac.deny="Prevent PCs listed from accessing the wireless network";
wl_mac.allow="Permit only PCs listed to access the wireless network";



// **************************************************************** Wireless_Basic.asp ***************************************************************//

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
hwl_basic.right1="Wireless Network Mode: ";
hwl_basic.right2="If you wish to exclude Wireless-G clients, choose <em>B-Only</em> mode. If you would like to disable wireless access, choose <em>Disable</em>.";
hwl_basic.right3="Sensitivity Range: ";
hwl_basic.right4="Adjusts the ack timing. 0 disables ack timing completely.";



// **************************************************************** Wireless_Advanced.asp ***************************************************************//

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


