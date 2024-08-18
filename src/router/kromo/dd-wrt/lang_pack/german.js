//////////////////////////////////////////////////////////////////////////////////////////////
//				German translation DD-WRT
//	initial translation: Sebastian Gottschall <brainslayer@dd-wrt.com>
//	additional works and fixes: sparksofinsanity <sparksofinsanity@users.noreply.github.com>
//////////////////////////////////////////////////////////////////////////////////////////////

// ** COMMON SHARE LABEL **//

lang_charset.set="utf-8";


share.packets="Pakete";
share.annex="Annex Typ";
share.apn="APN";
share.pin="PIN";
share.dial="Wähl Anweisung";
share.mode_3g="Verbindungs Typ";
share.mode_3g_auto="Auto LTE-4G/3G/2G";
share.mode_3g_4g="Erzwinge LTE-4G";
share.mode_3g_3g="Erzwinge 3G";
share.mode_3g_2g="Erzwinge 2G";
share.mode_3g_prefer_3g="Bevorzuge 3G";
share.mode_3g_prefer_2g="Bevorzuge 2G";
share.mode_3g_3g2g="UMTS zuerst, bei Fehler GPRS";
share.firmware="Firmware";
share.time="Zeit";
share.interipaddr="WAN-IP-Adresse";
// choice dd-wrt.c line 1442
share.choice="Bitte Wählen...";
share.more="mehr...";
share.help="Hilfe";
share.enable="Einschalten";
share.enabled="eingeschaltet";
share.disable="Abschalten";
share.disabled="ausgeschaltet";
//not set used in src/router/openvpn/config2/openvpn.webvpn
share.notset="Nicht gesetzt";
share.usrname="Benutzername";
share.passwd="Kennwort";
share.hostname="Hostname";
share.advanced="Erweiterte Einstellungen";
share.vdsl="Erweiterte VLAN Markierung";
share.vdslvlan7="T-Online VLAN 7 Unterstützung";
share.vdslvlan8="T-Online VLAN 8 Unterstützung";
share.vdslbng="T-Online BNG Unterstützung";
share.wan_vlantag="VLAN ID";
share.wan_dualaccess="Dual-Access Mode";
share.compression="PPP Komprimierung (MPPC)";
share.mlppp="Single Line Multi Link";
share.vpi_vci="VPI/VCI";
share.encaps="Kapselung";
share.payload="Typ der Nettodaten";
share.domainname="Domainname";
share.wandomainname="WAN Domain Name";
share.landomainname="LAN Domain Name";
share.statu="Status";
share.start="Start";
share.end="Ende";
share.proto="Protokoll";
share.ip="IP-Adresse";
share.ipaddrmask="IP-Adressen/Netzmaske(CIDR)";
share.ipv4="IPv4-Adresse";
share.ipv6="IPv6-Adresse";
share.localip="Lokale IP";
share.remoteip="Entfernte IP";
share.mac="MAC-Adresse";
share.none="Nichts";
share.none2="Ohne";
share.both="Beides";
share.add="Hinzufügen";
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
share.jan="Januar";
share.feb="Februar";
share.mar="März";
share.apr="April";
share.may="Mai";
share.jun="Juni";
share.jul="Juli";
share.aug="August";
share.sep="September";
share.oct="Oktober";
share.nov="November";
share.dec="Dezember";
share.expires="Läuft ab";
share.yes="Ja";
share.no="Nein";
share.filter="Filtern";
share.deny="Verbieten";
share.range="Von";
share.use="Nutze";
share.mins="min";
share.secs="s";
share.seconds="Sekunden";
share.ms="ms";
share.routername="Router Name";
share.manual="Manuell";
share.port="Port";
// dd-wrt.c line 2873
share.guest_port="Gästenetz";
share.ssid="SSID";
share.channel="Kanal";
share.stations="Stationen";
share.frequency="Frequenz";
share.rssi="Signal";
share.signal="Signal";
share.noise="Rauschen";
share.quality="Qualität";
share.beacon="Beacon";
share.openn="Offen";
share.dtim="DTIM";
share.rates="Raten";
share.rate="Rate";
share.txrate="TX Rate";
share.rxrate="RX Rate";
share.low="Niedrig";
share.medium="Mittel";
share.high="Hoch";
share.option="Optionen";
share.rule="Regel";
share.lan="LAN";
share.point2point="Punkt zu Punkt";
share.nat="NAT";
share.subnet="Netzmaske";
share.unmask="Klartext";
share.deflt="Std";  //don't use share.default!!!
share.reboot="Nach Änderungen neu starten";
share.all="Alles";
share.auto="Auto";
share.desired="Erwünscht";
share.required="Benötigt";
share.requiremts="Anforderungen";
share.right="Rechts";
share.left="Links";
share.share_key="Shared Key";
share.inter="Intervall (in Sekunden)";
share.srv="Dienstname";
share.port_range="Portbereich";
share.priority="Priorität";
share.gateway="Gateway";
share.intrface="Schnittstelle";  //don't use share.interface, Mozilla problem!!!
share.iftbl="IF";
share.input="Rein";
share.output="Raus";
share.total="Total";
share.radioname="Name";
share.ccq="CCQ";
share.pintrface="Physische Schnittstelle";
share.vintrface="Virtuelle Schnittstellen";
share.router="Router";
share.static_lease="Statische Leases";
share.srvip="Server-IP";
share.srvipname="Server-IP/Name";
share.localdns="Lokaler DNS";
share.minutes="Minuten";
share.oui="OUI Suche";
share.sttic="Statisch";
share.off="Aus";
share.on="An";
share.forwarding="Weiterleiten";
share.stp="STP";
share.mstp="MSTP";
share.rstp="RSTP";
share.dynamic="Dynamisch";
share.connecting="Verbinden";
share.connect="Verbinden";
share.connected="Verbunden";
share.disconnect="Trennen";
share.disconnected="Getrennt";
share.info="Information";
share.infotbl="Info";
share.state="Status";
share.mode="Modus";
share.encrypt="Verschlüsselung";
share.key="Schlüssel";
share.wireless="WLAN";
share.dhcp="DHCP";
share.styl="Aussehen";
share.theme="Thema";
share.styl_dark="Dunkles Farbschema";
share.err="Fehler";
share.errs="Fehler";
share.meters="Meter";
share.vht80plus80="80+80 MHz";
share.vht160="160 MHz";
share.vht80="80 MHz";
share.ht40="Weit (40 MHz)";
share.ht20="Voll (20 MHz)";
share.dynamicturbo="Dynamisch (20/40 MHz)";
share.turbo="Turbo (40 MHz)";
share.full="Voll (20 MHz)";
share.half="Halbe (10 MHz)";
share.quarter="Viertel (5 MHz)";
share.subquarter="Achtel (2.5 MHz)";
share.seealso="Siehe auch";
share.never="niemals";
share.unknown="Unbekannt";
share.expired="abgelaufen";
share.logout="Ausloggen";
share.nmounted="Nicht gemounted";
share.fssize="Total / Freier Speicher";
share.src="Lokale Adresse";
share.dst="Zieladresse";
share.name_resolution="Namens Auflösung";
share.timeout="Verfall (s)";
share.detail="Klicke um mehr zu sehen";
share.tmpmem="Temporärer Speicher";
share._1h="Jede Stunde";
share._2h="Alle 2 Stunden";
share._3h="Alle 3 Stunden";
share._4h="Alle 4 Stunden";
share._5h="Alle 5 Stunden";
share._6h="Alle 6 Stunden";
share._12h="Alle 12 Stunden";
share._24h="Jeden Tag";
share._48h="Alle 2 Tage";
share._168h="Jede Woche";
share.days="Tage";
share.from2=share.from;
share.to2=share.to;
share.days_genetive=share.days;
share.standard="Standard";
share.execscript="Skript ausführen";
share.user="Benutzer";
share.privatekey="Privater Schlüssel";
share.bytes="B";
share.kbytes="kB";
share.mbytes="MB";
share.gbytes="GB";
share.preempt="Bevorzugung";
share.acktiming="ACK Zeitplan";
share.broadcast="Broadcast Unterstützung";
share.secondcharacter="s";
share.change="Benutzer Kennwort ändern";
share.copy="Kopieren";
share.paste="Einfügen";
share.bad_request="UNGÜLTIGE ANFORDERUNG";
share.unauthorized="NICHT AUTORISIERT";
share.tcp_error="TCP FEHLER";
share.request_timeout="ZEITÜBERSCHREITUNG DER ANFORDERUNG";
share.not_implemented="NICHT IMPLEMENTIERT";
share.not_found="NICHT GEFUNDEN";
share.file_not_found=" Die Datei %s wurde nicht gefunden.";
share.auth_required=" Autorisierung erforderlich. Falscher Nutzer und/oder Passwort";
share.unexpected_connection_close=" Die Verbindung wurde bei der initialen Anforderung unerwartet geschlossen.";
share.unexpected_connection_close_2=" Verbindung unerwartet geschlossen.";
share.request_timeout_desc=" Zeitüberschreitung beim Warten auf die Anforderung.";
share.cant_parse_no_path=" Anforderung kann nicht ausgewertet werden. (Kein Pfad gefunden)";
share.cant_parse_no_proto=" Anforderung kann nicht ausgewertet werden. (Kein Protokoll gefunden)";
share.method_unimpl=" Die Methode %s ist nicht implementiert.";
share.no_slash=" Falscher Dateiname. (Kein anführender Schrägstrich)";
share.threaten_fs=" Illegaler Dateiname. (Dateiname beeinflusst das lokale Dateisystem)";
share.cross_site=" Webseitenübergreifende Aktion erkannt!";
share.cross_site_ref=" Webseitenübergreifende Aktion erkannt! (Verweis %s)";
share.no_wifi_access=" Die drahtlose Schnittstelle kann nicht für den Zugriff auf die Oberfläche verwendet werden.";
share.sysloglegend="System Log";
share.syslogdisabled="Keine Nachrichten verfügbar! Syslogd ist nicht eingeschaltet!";
share.actiontbl="Aktion";
share.show="Zeige";
share.hide="Verstecke";
share.excluded="Ausgeschlossen";
share.not_prefered="Nicht Bevorzugt";
share.prefered="Bevorzugt";



sbutton.save="Speichern";
sbutton.download_config="Exportiere Partner Konfiguration";
sbutton.savetitle="Nur speichern, ohne bereitstellen";
sbutton.apply="Anwenden";
sbutton.applied="Angewendet";
sbutton.applytitle="Wende Einstellungen unmittelbar an";
sbutton.saving="Gespeichert";
sbutton.cmd="Ausführen";
sbutton.cancel="Einstellungen zurücknehmen";
sbutton.canceltitle="Verwerfe Einstellungen dieser Maske";
sbutton.refres="Auffrischen";
sbutton.clos="Schließen";
sbutton.scrub="Scrub";
sbutton.del="Löschen";
sbutton.continu="Fortfahren";
sbutton.add="Hinzufügen";
sbutton.remove="Entfernen";
sbutton.modify="Verändern";
sbutton.deleted="Gelöscht";
sbutton.delall="Lösche alles";
sbutton.autorefresh="Auto-Refresh ist an";
sbutton.backup="Sichern";
sbutton.restore="Wiederherstellen";
sbutton.cptotext="In den Textbereich kopieren";
sbutton.runcmd="Kommandos ausführen";
sbutton.startup="Startup speichern";
sbutton.shutdown="Shutdown speichern";
sbutton.firewall="Firewall speichern";
sbutton.custom="Benutzerdefiniert speichern";
sbutton.usb="USB speichern";
sbutton.wol="Aufwecken";
sbutton.add_wol="Host hinzufügen";
sbutton.manual_wol="Manuelles Aufwecken";
sbutton.summary="Zusammenfassung";
sbutton.filterIP="Liste der PCs editieren";
sbutton.filterMac="MAC-Filterliste editieren";
sbutton.filterSer="Dienst hinzufügen/editieren";
sbutton.reboot="Router neu starten";
//sbutton.help="   Hilfe  ";
sbutton.wl_client_mac="MAC-Liste WLAN-Clients";
sbutton.update_filter="Filterliste updaten";
sbutton.join="Verbinden";
sbutton.log_in="Eingehendes Log";
sbutton.log_out="Ausgehendes Log";
sbutton.edit_srv="Dienst hinzufügen/editieren";
sbutton.routingtab="Routentabelle anzeigen";
sbutton.policytab="Regeltabelle anzeigen";
sbutton.wanmac="Zeige aktuelle PC-MAC-Adresse";
sbutton.dhcprel="DHCP freigeben";
sbutton.dhcpren="DHCP erneuern";
sbutton.spectral_survey="Spektrum";
sbutton.survey="WLAN-Suche";
sbutton.csurvey="Kanal Analyse";
sbutton.wsurvey="Wiviz Visualisierung";
sbutton.upgrading="Aktualisiere";
sbutton.upgrade="Aktualisierung";
sbutton.preview="Vorschau";
sbutton.allways_on="Alles An";
sbutton.allways_off="Alles Aus";
sbutton.download="Herunterladen";
sbutton.next="Nächstes ›";
sbutton.prev="‹ Vorheriges";

// ** COMMON ERROR MESSAGES  **//

errmsg.err0="Sie müssen einen Benutzernamen angeben.";
errmsg.err1="Sie müssen einen Routernamen angeben.";
errmsg.err2="Außerhalb des zulässigen Bereichs! Bitte ändern Sie die Start-IP-Adresse oder die Anzahl der Benutzer.";
errmsg.err3="Sie müssen einen Tag auswählen.";
errmsg.err4="Die Endzeit muß nach der die Startzeit liegen.";
errmsg.err5="Die MAC-Adressenlänge ist nicht korrekt.";
errmsg.err6="Sie müssen einen Benutzernamen angeben.";
errmsg.err7="Sie müssen einen Hostnamen angeben.";
errmsg.err8="Sie müssen eine IP-Adresse oder einen Domainnamen angeben.";
errmsg.err9="Ungültige DMZ-IP-Adresse.";
errmsg.err10="Das Kennwort zur Bestätigung entspricht nicht dem originalen. Bitte geben Sie es neu ein.";
errmsg.err11="Im Kennwort sind keine Leerzeichen erlaubt";
errmsg.err12="Sie müssen ein Kommando zum Ausführen angeben.";
errmsg.err13="Die Aktualisierung ist fehlgeschlagen.";
errmsg.err45="Diese Funktion ist unter HTTPS nicht verfügbar! Bitte verwenden Sie den HTTP-Modus.";
errmsg.err46="Diese Funktion ist unter HTTPS nicht verfügbar";

//common.js error messages
errmsg.err14=" Wert ist außerhalb des zulässigen Bereichs [";
errmsg.err15="Die WAN-MAC-Adresse ist außerhalb des gültigen Bereichs [00 - ff].";
errmsg.err16="Der zweite Wert der MAC-Adresse muß eine gerade Zahl sein : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="Diese MAC-Adresse ist nicht korrekt.";
errmsg.err18="Die Länge der MAC-Adresse ist nicht korrekt.";
errmsg.err19="Die MAC-Adresse kann nicht die Broadcast-Adresse sein.";
errmsg.err20="Geben Sie die MAC-Adresse im Format (xx:xx:xx:xx:xx:xx) ein.";
errmsg.err21="Ungültiges MAC-Adressen-Format.";
errmsg.err22="Die WAN-MAC-Adresse ist nicht korrekt.";
errmsg.err23="Ungültiger HEX Wert ";
errmsg.err24=" in MAC-Adresse gefunden ";
errmsg.err25="Der Schlüsselwert ist nicht korrekt.";
errmsg.err26="Die Schlüssellänge ist nicht korrekt.";
errmsg.err27="Ungültige Netzmaske.";
errmsg.err28=" enthält ungültige Werte, diese müssen [ 0 - 9 ] sein.";
errmsg.err29=" enthält einen ungültigen ASCII-Wert.";
errmsg.err30=" enthält ungültige hexadezimale Werte.";
errmsg.err31=" ist ungültig.";
errmsg.err32="IP-Adresse und Gateway sind nicht im selben Netzbereich.";
errmsg.err33="IP-Adresse und Gateway können nicht die selben sein.";
errmsg.err34=" darf keine Leerzeichen enthalten.";
errmsg.err110="Die Endnummer muß größer als die Startnummer sein";
errmsg.err111="Ungültige IP Adresse";
errmsg.err112="Ungültige Zeichen \"<invchars>\" in Feld \"<fieldname>\"";
errmsg.err113="Die Mobility Domain muss ein 4 Stelliger Hexadezimalwert sein";

//Wol.asp error messages
errmsg.err35="Sie müssen eine MAC-Adresse angeben um fortzufahren.";
errmsg.err36="Sie müssen eine Netzwerk-Broadcast-Adresse angeben um fortzufahren.";
errmsg.err37="Sie müssen einen UDP-Port angeben um fortzufahren.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Bitte geben sie einen Shared Key an!";
errmsg.err39="Ungültiger Schlüssel, er muss aus 8 bis 63 ASCII-Werten oder 64 hexadezimalen Zahlen bestehen";
errmsg.err40="Sie müssen einen Schlüssel für Key angeben ";
errmsg.err41="Ungültige Schlüssellänge ";
errmsg.err43="Rekey-Intervall";

//config.asp error messages
errmsg.err42="Bitte geben sie eine Konfiguration zum Wiederherstellen an.";

//WL_ActiveTable.asp error messages
errmsg.err44="Die totale Anzahl der Checks überschreitet 256.";

//Site_Survey.asp error messages
errmsg.err47="Ungültige SSID.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS ist nicht kompatibel mit der aktuellen Konfiguration des Routers. Bitte prüfen sie die folgenden Punkte :\n * WLAN-Modus muss AP sein \n * WPA2 wird vom WDS nicht unterstützt \n * WLAN-Netzwerk-Modus Nur-B wird von WDS nicht unterstützt";

//Wireless_radauth.asp error messages
errmsg.err49="RADIUS ist nur im AP-Modus verfügbar.";

//Wireless_Basic.asp error messages
errmsg.err50="Sie müssen eine SSID angeben.";

// Management.asp error messages
errmsg.err51="Der Router ist im Augenblick auf das Standard-Kennwort konfiguriert. Aus Sicherheitsgründen müssen Sie das Kennwort ändern bevor Sie die Remote-Management Funktion aktivieren. Klicken Sie den OK-Button um das Kennwort zu ändern. Klicken Sie den Abbrechen-Button um die Remote-Management-Funktion ausgeschaltet zu lassen.";
errmsg.err52="Eingegebene Passwörter stimmen nicht überein.";

// Port_Services.asp error messages
errmsg.err53="Klicken Sie nach dem Abschluss aller Aktionen den Anwenden-Button um die Einstellungen zu übernehmen.";
errmsg.err54="Sie müssen einen Dienst-Namen angeben.";
errmsg.err55="Der Dienstname existiert bereits.";

// QoS.asp error messages
errmsg.err56="Portwert ist außerhalb des gültigen Bereichs [0 - 65535]";

// Routing.asp error messages
errmsg.err57="Eintrag löschen?";
errmsg.err103=" muß kleiner sein als ";

// Status_Lan.asp error messages
errmsg.err58="Klicken um die Zuweisung zu löschen";
errmsg.err581="Klicken um die Verbindung zu trennen";
errmsg.err582="Klicken um den Eintrag der statischen Lease-Tabelle hinzuzufügen";

//Status_Wireless.asp error messages
errmsg.err59="Nicht verfügbar! Bitte schalten sie das WLAN-Netzwerk an.";

//Upgrade.asp error messages
errmsg.err60="Bitte geben sie eine Datei zur Aktualisierung an.";
errmsg.err61="Ungültiges Dateiformat.";

//Services.asp error messages
errmsg.err62=" wurde bereits als statisches Lease definiert.";

//Saving message
errmsg.err100="In Bearbeitung...<br />Bitte Warten.";
errmsg.err101="Stelle Konfiguration wieder her...<br />Bitte Warten.";
errmsg.err102="Upgrade Firmware...<br />Bitte Warten.";
errmsg.err103="Ungültiges Zeichen";

// **  COMMON MENU ENTRIES  **//

bmenu.setup="Setup";
bmenu.setupbasic="Basis-Setup";
bmenu.setupipv6="IPv6";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC-Adresse klonen";
bmenu.setuprouting="Erweitertes Routing";
bmenu.setupvlan="Switch Konfiguration";
bmenu.setupeop="EoIP Tunnel";
bmenu.networking="Vernetzung";

bmenu.wireless="WLAN";
bmenu.wirelessBasic="Basis-Einstellungen";
bmenu.wirelessRadius="RADIUS";
bmenu.wirelessSuperchannel="SuperChannel";
bmenu.wimax="WiMAX";
bmenu.wirelessSecurity="WLAN-Sicherheit";
bmenu.wirelessAoss="AOSS";
bmenu.wirelessAossWPS="AOSS / WPS";
bmenu.wirelessWPS="WPS";
bmenu.wirelessMac="MAC-Filter";
bmenu.wirelessAdvanced="Erweiterte Einstellungen";
bmenu.wirelessAdvancedwl0="wl0-Erweitert";
bmenu.wirelessAdvancedwl1="wl1-Erweitert";
bmenu.wirelessAdvancedwl2="wl2-Erweitert";
bmenu.wirelessWds="WDS";
bmenu.wirelessWds0="wlan0-WDS";
bmenu.wirelessWds1="wlan1-WDS";
bmenu.wirelessWds2="wlan2-WDS";
bmenu.wirelessWds3="wlan3-WDS";
bmenu.wirelessWdswl0="wl0-WDS";
bmenu.wirelessWdswl1="wl1-WDS";
bmenu.wirelessWdswl2="wl2-WDS";

bmenu.security="Sicherheit";
bmenu.firwall="Firewall";
bmenu.vpn="VPN-Durchleitung";

bmenu.accrestriction="Zugriffsbeschränkung";
bmenu.webaccess="Internet-Zugriff";

bmenu.applications="NAT / QoS";
bmenu.applicationsprforwarding="Port-Bereichs-Weiterleitung";
bmenu.applicationspforwarding="Port-Weiterleitung";
bmenu.applicationsipforwarding="IP-Weiterleitung (1:1 NAT)";
bmenu.applicationsptriggering="Port-Triggering";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";
bmenu.applicationsP2P="P2P";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Übersicht";
bmenu.sipathphone="Telefonbuch";
bmenu.sipathstatus="Status";

bmenu.admin="Administration";
bmenu.adminManagement="Management";
bmenu.adminAlive="Lebenserhaltung";
bmenu.adminSysctl="Sysctl";
bmenu.adminLog="Log";
bmenu.adminDiag="Diagnose";
bmenu.adminWol="WOL";
bmenu.adminFactory="Werkseinstellungen";
bmenu.adminUpgrade="Firmware-Update";
bmenu.adminBackup="Backup";

bmenu.services="Dienste";
bmenu.servicesServices="Dienste";
bmenu.servicesRadius="FreeRADIUS";
bmenu.servicesPppoesrv="PPPoE Server";
bmenu.servicesPptp="VPN";
bmenu.servicesUSB="USB";
bmenu.servicesNAS="NAS";
bmenu.servicesHotspot="Hotspot";
bmenu.servicesNintendo="Nintendo";
bmenu.servicesPrivoxy="Werbeblocker";
bmenu.servicesSpeedchecker="SpeedChecker";

bmenu.statu="Status";
bmenu.statuRouter="Router";
bmenu.statuInet="WAN";
bmenu.statuLAN="LAN";
bmenu.statuWLAN="WLAN";
bmenu.statuVPN="OpenVPN";
bmenu.statuBand="Bandbreite";
bmenu.statuSysInfo="Sys-Info";
bmenu.statuActivate="Aktivieren";
bmenu.statuMyPage="Meine Seite";
bmenu.statuGpio="GPIO E/A";
bmenu.statuSyslog="Syslog";
bmenu.setupnetw="Netzwerk";
bmenu.adminman="Verwaltung";

// sysctl.asp

sysctl.titl="Sysctl Konfiguration";

// ** Alive.asp **//

alive.titl="Lebenserhaltung";
alive.h2="Lebenserhaltung";
alive.legend="Geplanter Neustart";
alive.sevr1="Reboot planen";
alive.hour="Zu einem festen Zeitpunkt";
alive.legend2="WDS/Verbindungsüberwachung";
alive.sevr2="Überwachung Einschalten";
alive.IP="IP-Adressen";
alive.legend3="Proxy/Verbindungsüberwachung";
alive.sevr3="Proxy-Überwachung Einschalten";
alive.IP2="Proxy-IP-Adresse";
alive.port="Proxy-Port";
alive.mode0="Reboot bei einer fehlgeschlagenen IP";
alive.mode1="Reboot wenn alle IPs fehlschlagen";
alive.timeout="Ping timeout";

//help container

halive.right2="Wählen Sie aus, wann der Router neu starten soll. Cron muss hierfür auf dem Management-Tab, eingeschaltet sein.";
halive.right4="<b>" + alive.IP + "</b>: Maximal 3 IP-Adressen (mit einem <em>Leerzeichen</em> getrennt) sind erlaubt.<br />IP-Format ist xxx.xxx.xxx.xxx.";

//help page
halive.page1="<dd>Sie können regelmäßige Neustarts für den Router planen:<ul><li>Regelmäßig nach der eingestellten Anzahl von Sekunden.</li><li>Zu einem bestimmten Zeitpunkt in der Woche oder täglich.</li></ul><br /><div class=\"note\"><h4>Hinweis:</h4><div>Für Datums basierende Reboots muss Cron aktiviert sein. Siehe <a href=\"HManagement.asp\">Management</a> für die Cron Aktivierung.</div></div></dd>";
halive.page2="<dd></dd><dd>Überprüfen Sie alle Werte und klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em> um Ihre Einstellungen zu speichern. Klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em> um Ihre nicht gespeicherten Änderungen zu verwerfen. Klicken Sie auf die Schaltfläche <em>" + sbutton.reboot +"</em> um den Router sofort neu zu starten.</dd>";

// ** config.asp **//

config.titl="Backup & Wiederherstellung";
config.h2="Backup Konfiguration";
config.legend="Backup-Einstellungen";
config.mess1="Klicken Sie den \"" + sbutton.backup + "\" Button um die Backup-Datei zu downloaden.";
config.h22="Konfiguration wiederherstellen";
config.legend2="Einstellungen wiederherstellen";
config.mess2="Datei zur Wiederherstellung";
config.mess3="W A R N U N G";
config.mess4="Laden Sie nur Dateien hoch, welche mit der selben Firmware auf dem selben Routermodell erstellt wurden!";
config.force="Ignoriere Router Modelprüfung";
config.keepip="Router IP behalten";

//help container

hconfig.right2="Sie können hier Ihre aktuelle Konfiguration sichern für den Fall, dass Sie Ihren Router einmal zurücksetzen müssen.<br /><br />Klicken Sie den <em>Backup</em>-Button, um Ihre Konfiguration zu sichern.";
hconfig.right4="Klicken Sie den <em>Durchsuchen...</em>-Button, um eine Konfigurationsdatei auszuwählen, die auf Ihrem PC gespeichert ist.<br /><br />Klicken Sie den <em>" + sbutton.restore + "</em>-Button, um alle aktuellen Konfigurationseinträge mit Ihrer neuen Konfiguration zu überschreiben.";

// help page
hconfig.page1="<dd>Sie können Ihre aktuelle Konfiguration sichern, falls Sie den Router auf die Werkseinstellungen zurücksetzen müssen.</dd><dt>";
hconfig.page2="</dt><dd>Klicken Sie auf die Schaltfläche <em>" + sbutton.backup + "</em> um Ihre aktuelle Konfiguration in einer Datei auf der Festplatte zu sichern.<br /> Tipp: Geben Sie im Terminal <b>nvram show > /tmp/mybackup.txt</b> ein und speichern Sie diese Datei auf dem Desktop, um ein für Menschen lesbares Backup Ihrer aktuellen Konfiguration zu erhalten.</dd>";
hconfig.page3="<dd>Klicken Sie auf die Schaltfläche <i>Datei auswählen...</i> um nach einer Sicherungsdatei zu suchen, die Sie zuvor auf der Festplatte gespeichert haben.<br>Klicken Sie auf die Schaltfläche <em>" + sbutton.restore + "</em> um <b>alle aktuellen Konfigurationseinstellungen</b> mit den Werten aus der Sicherungsdatei zu überschreiben.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div>Laden Sie nur Sicherungsdateien hoch, die mit der DD-WRT Firmware und dem selben Router-Modell erstellt wurden.</dd></div></dd>";

// ** DDNS.asp **//

ddns.titl="Dynamic DNS";
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="Provider";
ddns.emailaddr="E-Mail-Adresse";
ddns.typ="Typ";
ddns.custom="Spezifisch";
ddns.wildcard="Wildcard";
ddns.statu="DDNS-Status";
ddns.system="DYNDNS Server";
ddns.options="Erweiterte DDNS Optionen";
ddns.forceupd="Erzwinge Update Intervall";
ddns.wanip="Externer IP Check";
ddns.hlp="DDNS Service";
ddns.ssl="Nutze SSL";
ddns.ipv6_only="Nur IPv6 Adresse Updaten";

ddnsm.all_closed="DDNS-Server ist aktuell geschlossen";
ddnsm.all_resolving="Domainnamen auflösen";
ddnsm.all_errresolv="Namensauflösung fehlgeschlagen";
ddnsm.all_connecting="Mit Server verbinden";
ddnsm.all_success="Erfolgreich";
ddnsm.all_connectfail="Verbindung mit Server fehlgeschlagen";
ddnsm.all_disabled="DDNS-Funktion ist abgeschaltet";
ddnsm.all_noip="Keine WAN-Verbindung";

//help container

hddns.right2="DDNS erlaubt Ihnen den Zugriff auf Ihr Netzwerk unter Zuhilfenahme eines Domainnamens anstatt einer IP-Adresse. Der Dienst verwaltet und ändert die IP-Adresse sowie die Domaininformation dynamisch. Sie müssen sich für den Dienst bei einem entsprechenden Provider (passend zur Auswahlbox links) anmelden.";
hddns.right4="Geben Sie eine Zahl in der Box an um den Intervall für das erzwungene Update anzugeben (in Tagen). Updates sollten nur automatisch durchgeführt werden wenn sich Ihre IP ändert. Achten Sie darauf das Sie Updates nicht zu oft durchführen, da ihr DDNS Provider Sie sonst sperren könnte.";

//help page
hddns.page1="<dd>Der Router bietet eine DDNS (Dynamic Domain Name System) Funktion. Mit DDNS können Sie einer dynamischen öffentlichen IP-Adresse einen festen Host- und Domainname zuweisen. Dies ist nützlich, wenn Sie Ihre eigene Website hosten oder einen anderen Server hinter dem Router betreiben, der über das Internet zugänglich gemacht werden soll. Um die Funktion nutzen zukönnen, müssen Sie sich bei einem DDNS-Dienst registrieren, z. B. bei <a href=\"http:\/\/www.dyndns.org\" target=\"_new\">www.dyndns.org</a>, einer der verschiedenen DDNS-Anbieter, die von DD-WRT unterstützt werden.</dd>";
hddns.page2="<dd>Um den DDNS-Dienst zu deaktivieren, behalten Sie die Standardeinstellung <em>" + share.disable + "</em> bei. Folgen Sie den Anweisungen um den DDNS-Dienst zu aktivieren:<ol class=\"wide\"><li>Melden Sie sich unter <a href=\"http:\/\/www.dyndns.org\" target=\"_new\">www.dyndns.org</a> für den DDNS-Dienst an, und notieren Sie sich Ihren Benutzernamen, Ihr Kennwort und Ihren Hostnamen.</li><li>Auf der Registerkarte DDNS wählen Sie <i>DynDNS.org</i> aus dem Dropdown-Menü aus.</li><li>Füllen Sie die Felder <em>" + share.usrname + "</em>, <em>" + share.passwd + "</em> und <em>" + share.hostname + "</em> aus.</li><li>Klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em> um Ihre Änderungen zu speichern. Klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em> um nicht gespeicherte Änderungen zu verwerfen.</li></ol><br />Sie können nun vom Internet aus mit der von Ihnen gewählten Domain auf Ihren Router zugreifen.</dd>";
hddns.page3="<dd>Der <em>statische</em> DNS-Dienst ist dem <em>dynamischen</em> DNS-Dienst insoweit ähnlich, das er es ermöglicht, dass ein Hostname wie yourname.dyndns.org auf Ihre IP-Adresse verweist. Im Gegensatz zu einem <em>dynamischen</em> DNS-Host erlischt die Gültigkeit eines <em>statischen</em> DNS-Hosts nicht nach 35 Tagen ohne Aktualisierung, aber es dauert länger, bis sich Aktualisierungen im DNS-System verbreiten.<br />Der DynDNS Service bietet eine verwaltete primäre DNS-Lösung, die Ihnen die vollständige Kontrolle über einen gesamten Domainnamen gibt und einen einheitlichen primären/sekundären DNS-Service bietet. Eine webbasierte Schnittstelle bietet zwei Ebenen der Kontrolle über Ihre Domain, die sowohl für normale als auch für Power-User geeignet ist.</dd>";
hddns.page4="<dd>Wenn Sie die Wildcard-Funktion für Ihren Host aktivieren, wird *.yourhost.dyndns.org als Alias für die gleiche IP-Adresse wie yourhost.dyndns.org verwendet. Diese Funktion ist nützlich, wenn Sie z. B. www.yourhost.dyndns.org verwenden und trotzdem Ihren Hostnamen erreichen möchten.</dd>";
hddns.page5="<dd>Geben Sie eine ganze Zahl in das Feld ein, um das Intervall für die erzwungene Aktualisierung (in Tagen) festzulegen. Eine erzwungene Aktualisierung ist eine Aktualisierung, die durchgeführt wird, obwohl sich Ihre IP-Adresse nicht geändert hat. Eine erzwungene Aktualisierung ist insbesondere für Benutzer erforderlich, die keine Spender von dyndns.org sind, um die Löschung des Hostnamen zu verhindern.</dd>";
hddns.page6="<dd>Hier wird der Status des DDNS-Dienst angezeigt.</dd>";

// ** Diagnostics.asp **//

diag.titl="Diagnose";
diag.h2="Diagnose";
diag.legend="Kommandozeile";
diag.cmd="Kommandos";
diag.output="Ausgabe";
diag.startup="Hochfahren";
diag.shutdown="Runterfahren";
diag.firewall="Firewall";
diag.custom="Benutzerdefiniertes Skript";
diag.usb="USB Skript";
diag.running="läuft";
diag.stopped="gestoppt";

//help container

hdiag.right2="Hier können Sie mit Hilfe des Web-GUI Kommandos auf der Kommandozeile starten. Geben Sie einfach Ihre Kommandos in das Textfeld ein und klicken Sie <em>" + sbutton.runcmd + "</em> um diese auszuführen.";

//help page
hdiag.page1="<dd><b>Empfohlen:</b> Eine Terminal-Verbindung über SSH/Telnet ist für einige Befehle besser geeignet, flexibler, schneller und zuverlässiger.</dd>";
hdiag.page2="<dd>Geben Sie in das Eingabefeld den gewünschten Befehl ein und klicken Sie auf die Schaltfläche <em>" + sbutton.runcmd + "</em> um den Befehl auszuführen.</dd>";
hdiag.page3="<dd>Sie können bestimmte Befehle angeben, die beim Start des Routers ausgeführt werden sollen. Geben Sie die gewünschten Befehle in das Eingabefeld ein (nur ein Befehl pro Zeile) und klicken Sie auf die Schaltfläche <em>" + sbutton.startup + "</em>.</dd>";
hdiag.page4="<dd>Jedes Mal, wenn die Firewall gestartet wird, können benutzerdefinierte Firewall-Regeln zur Filterkette hinzugefügt werden. Füllen Sie das Eingabefeld mit zusätzlichen iptables/ip6tables Befehlen (nur ein Befehl pro Zeile) und klicken Sie auf die Schaltfläche <em>" + sbutton.firewall + "</em>.</dd>";
hdiag.page5="<dd>Ein benutzerdefiniertes Skript wird in <b>/tmp/.rc_custom</b> gespeichert. Sie können es manuell ausführen oder Cron verwenden, um das Skript auszuführen. Füllen Sie das Eingabefeld mit den gewünschten Befehlen (nur ein Befehl pro Zeile) und klicken Sie auf die Schaltfläche <em>" + sbutton.custom + "</em>.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div><ul><li>Startup Befehle werden in der NVRAM-Variable rc_startup gespeichert</li><li>Firewall Befehle werden in der NVRAM-Variable rc_firewall gespeichert</li><li>Benutzerdefinierte Skripte werden in der NVRAM-Variable rc_custom gespeichert</li></ul></div></div></dd>";

// ** DMZ.asp **//

dmz.titl="DMZ";
dmz.h2="Demilitarisierte Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="DMZ verwenden";
dmz.host="IP-Adresse des DMZ-Hosts";

//help container
hdmz.right2="Wenn Sie diese Option aktivieren wird der angegebene Host in das Internet durchgeschaltet. Alle Ports sind dann vom Internet aus erreichbar";

//help page
hdmz.page1="<dd>Mit der DMZ-Hosting-Funktion (Demilitarized Zone) kann ein lokales Gerät mit dem Internet verbunden werden, um einen speziellen Dienst wie Internetspiele oder Videokonferenzen zu nutzen. Beim DMZ-Hosting werden alle Ports gleichzeitig an ein Gerät weitergeleitet. Die Funktion Portweiterleitung ist sicherer, da sie nur die Ports öffnet, die Sie geöffnet haben möchten, während das DMZ-Hosting alle Ports eines Computers öffnet und das Gerät für das Internet sichtbar macht.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div>Jedem Gerät, dessen Port weitergeleitet wird, muss eine statische IP-Adresse zugewiesen werden, da sich die IP-Adresse bei Verwendung der DHCP-Funktion ändern kann.</div></div></dd>";
hdmz.page2="<dd>Um ein Gerät für das Internet zugänglich zu machen, wählen Sie <em>" + share.enable + "</em> und geben Sie die IP-Adresse des Computers in das Feld <em>" + dmz.host + "</em> ein.<br /><br />Um die DMZ zu deaktivieren, behalten Sie die Standardeinstellung <em>" + share.disable + "</em> bei.</dd><dd>Klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em> um Ihre Einstellungen zu speichern, oder klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em> um Ihre nicht gespeicherten Änderungen zu verwerfen.</dd>";

// ** Factory_Defaults.asp **//

factdef.titl="Werkseinstellungen";
factdef.h2="Werkseinstellungen";
factdef.legend="Router-Einstellungen zurücksetzen";
factdef.restore="Werkseinstellungen wiederherstellen";
factdef.mess1="Warnung! Wenn Sie OK klicken wird das Gerät auf die Werkseinstellungen zurückgesetzt und alle getätigten Einstellungen werden gelöscht.";

//help container

hfactdef.right1="Diese Funktion setzt die Konfiguration auf den Auslieferungszustand zurück. Alle Ihre Einstellungen werden gelöscht.";

// help page
hfactdef.page1="<dd>Wenn Sie Probleme mit Ihrem Router haben (die möglicherweise auf eine Änderung bestimmter Einstellungen zurückzuführen sind) dann können Sie die Werkseinstellungen wiederherstellen.</dd>";
hfactdef.page2="<dd>Klicken Sie auf die Schaltfläche <em>" + share.yes + "</em> um alle Konfigurationseinstellungen auf ihre Standardwerte zurückzusetzen. Klicken Sie anschließend auf die Schaltfläche <em>" + sbutton.apply + "</em>.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div>Alle von Ihnen gespeicherten Einstellungen gehen verloren, wenn die Werkseinstellungen wiederhergestellt werden. Nach dem Zurücksetzen auf die Werkseinstellungen wird der Router neu gestartet und ist über die Standard-IP-Adresse 192.168.1.1 erreichbar, und die Eingabe eines neuen Benutzernamens und Kennworts ist erforderlich, bevor Sie weitere Einstellungen vornehmen können.</div></div></dd>";

// ** FilterIPMAC.asp **//

filterIP.titl="Liste der PCs";
filterIP.h2="Liste der PCs";
filterIP.legend1="Geben Sie die MAC-Adresse der PCs in folgendem Format an: xx:xx:xx:xx:xx:xx";
filterIP.legend2="Geben Sie die IP-Adresse des PCs ein";
filterIP.legend3="Geben Sie den IP-Bereich der PCs an";
filterIP.ip_range="IP Bereich";

// ** Filter.asp **//

filter.titl="Zugriffsbeschränkung";
filter.h2="Internetzugriff";
filter.legend="Zugriffsrichtlinie";
filter.pol="Regeln";
filter.polname="Regelnname";
filter.pcs="PCs";
filter.polallow="Internetzugriff während der angegebenen Tage und Stunden.";
filter.legend2="Tage";
filter.time="Zeiten";
filter.h24="24 Stunden";
filter.legend3="Blockierte Dienste";
filter.catchall="Alle P2P-Protokolle abfangen";
filter.legend4="Webseite mit Hilfe einer URL blockieren";
filter.legend5="Webseite mit Hilfe von Schlagworten blockieren";
filter.mess1="Löschen der Richtlinie?";
filter.mess2="Sie müssen mindestens einen Tag angeben.";
filter.mess3="Die Endzeit muss nach der Startzeit liegen.";
filter.none="";
filter.packetcount="Gefilterte Pakete";

// ** FilterSummary.asp **//

filterSum.titl="Zusammenfassung Zugriffsbeschränkung ";
filterSum.h2="Zusammenfassung Internetrichtlinie ";
filterSum.polnum="Nr.";
filterSum.polday="Uhrzeit";

//help container

hfilter.right2="Sie können bis zu 20 Zugriffsregeln definieren. Klicken Sie <em>" + sbutton.del + "</em> um eine Richtlinie zu löschen, oder <em>" + sbutton.summary + "</em> um sich die Zusammenfassung der Richtlinie anzuschauen.";
hfilter.right4="Hier können Sie eine Richtlinie ein- oder ausschalten.";
hfilter.right6="Hier können Sie Ihrer Richtlinie einen Namen zuweisen.";
hfilter.right8="Hier können Sie ein oder mehrere Tage auswählen, an denen die Richtlinie gelten soll.";
hfilter.right10="Hier können Sie die Tageszeit angeben, zu der die Richtlinie gelten soll.";
hfilter.right12="Hier können Sie Dienste auswählen, welche blockiert werden sollen. Klicken Sie <em>" + sbutton.filterSer + "</em> um die Einstellungen zu ändern.";
hfilter.right14="Hier können Sie diverse Webseiten mit Hilfe einer URL blockieren.";
hfilter.right16="Hier können Sie mit Hilfe von Schlagworten Webseiten blockieren, die diese enthalten.";
hfilter.pageintro="<dd>Auf dieser Registerkarte können Sie bestimmte Arten der Internetnutzung sperren oder zulassen. Sie können Internet-Zugangsrichtlinien für bestimmte Computer einrichten und Filter mit Hilfe von Netzwerk-Portnummern einrichten.</dd>";
hfilter.page1="<dd>Diese Funktion ermöglicht es Ihnen, bis zu 20 verschiedene Internet-Zugriffsrichtlinien für bestimmte Geräte festzulegen, die durch ihre IP- oder MAC-Adressen identifiziert werden. Für jedes in der Richtlinie genannte Gerät, während der angegebenen Tage und Zeiträume.<br /><br />Folgen Sie den Anweisungen, um eine Richtlinie zu erstellen oder zu bearbeiten:<ol class=\"wide\"><li>Wählen Sie eine Richtlinien-Nummer (1-20) aus dem Dropdown-Menü aus.</li><li>Geben Sie einen Namen in das Feld <em>" + filter.polname + "</em> ein.</li><li>Klicken Sie auf die Schaltfläche <em>" + sbutton.filterIP + "</em>.</li><li>Geben Sie in dem Fenster <em>" + filterIP.titl + "</em> die gewünschten Computer anhand ihrer IP-Adresse oder MAC-Adresse an. Geben Sie die entsprechenden IP-Adressen in die <i>IP</i> Felder ein. Wenn Sie einen IP-Adressen-Bereich filtern möchten, dann füllen Sie die Felder <i>IP-Bereich</i> aus. Geben Sie die entsprechenden MAC-Adressen in die <i>MAC</i> Felder ein.</li><li>Klicken Sie auf die Schaltfläche <em>" + sbutton.apply + "</em>, um Ihre Änderungen zu übernehmen. Klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em>, um die nicht gespeicherten Änderungen zu verwerfen. Klicken Sie auf die Schaltfläche <em>" + sbutton.clos + "</em>, um zu der Registerkarte <b>"+ bmenu.accrestriction + "</b> zurückzukehren.</li><li>Wenn Sie den Internetzugang für die aufgelisteten Computer während der angegebenen Tage und Uhrzeiten sperren möchten, dann behalten Sie die Standardeinstellung <em>" + share.deny + "</em> bei. Wenn Sie möchten, dass die aufgelisteten Computer während der angegebenen Tage und Uhrzeiten gefiltert auf das Internet zugreifen können, dann klicken Sie auf das Optionsfeld neben <em>" + share.filter + "</em>.</li><li>Legen Sie die Tage fest, an denen der Zugriff gefiltert werden soll. Wählen Sie <em>" + share.everyday + "</em> oder die entsprechenden Wochentage.</li><li>Legen Sie die Zeit fest, zu der der Zugriff gefiltert werden soll. Wählen Sie <em>" + filter.h24 + "</em> oder aktivieren Sie das Kästchen neben <em>" + share.from + "</em> und verwenden Sie die Dropdown-Menüs, um einen bestimmten Zeitraum festzulegen.</li><li>Klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em>, um Ihre Änderungen zu speichern und zu aktivieren.</li><li>Um weitere Richtlinien zu erstellen oder zu bearbeiten, wiederholen Sie die Schritte 1-9.</li></ol><br />Um eine Internet-Zugangsrichtlinie zu löschen, wählen Sie die Richtlinien-Nummer aus, und klicken Sie auf die Schaltfläche <em>" + share.del + "</em>.</dd>";
hfilter.page2="<dd>Um eine Zusammenfassung aller Richtlinien zu sehen, klicken Sie auf die Schaltfläche <em>" + sbutton.summary + "</em>. In dem Fenster <em>" + filterSum.titl + "</em> werden die einzelnen Richtlinien in einer geordneten Liste mit Richtlinien-Nummer, Name, Tagen und Tageszeit angezeigt. Um eine Richtlinie zu löschen, markieren Sie das entsprechende Kästchen, und klicken Sie dann auf die Schaltfläche <em>" + share.del + "</em>. Klicken Sie auf die Schaltfläche <em>" + sbutton.clos + "</em>, um zu der Registerkarte <b>"+ bmenu.accrestriction + "</b> zurückzukehren.</dd>";

// ** Firewall.asp **//

firewall.titl="Firewall";
firewall.h2="Sicherheit";
firewall.legend="Firewall-Schutz";
firewall.firewall="SPI-Firewall";
firewall.legend2="Zusätzliche Filter";
firewall.proxy="Filter-Proxy";
firewall.cookies="Cookies filtern";
firewall.applet="Java-Applets filtern";
firewall.activex="ActiveX filtern";
firewall.legend3="WAN-Zugriffe blockieren";
firewall.legend4="Verhindere WAN DoS/Bruteforce Attacken";
firewall.ping="Anonyme WAN-Zugriffsversuche blockieren";
firewall.muticast="Multicast filtern";
firewall.ssh="SSH Zugriff limitieren";
firewall.telnet="Telnet Zugriff limitieren";
firewall.pptp="PPTP Server Zugriff limitieren";
firewall.ftp="FTP Server Zugriff limitieren";
firewall.arp_spoofing="ARP Spoofing Schutz";
firewall.filter_tos="Filtere ToS/DSCP";
firewall.filter_invalid="Filtere ungültige Packete";

filter.nat="WAN-NAT-Umleitung filtern";
filter.port113="IDENT (Port 113) filtern";
filter.snmp="Block WAN SNMP access";

// ** Log.asp **//

log.label="Log";
log.h2="Log-Verwaltung";
log.legend="Log";
log.lvl="Log-Level";
log.drop="Abgewiesen";
log.reject="Zurückgewiesen";
log.accept="Akzeptiert";

// ** Log_incoming.asp **//

log_in.titl="Eingehende Log-Tabelle";
log_in.h2="Eingehende Log-Tabelle";
log_in.th_ip="Ursprungs-IP";
log_in.th_port="Ziel-Port-Nummer";

// ** Log_outgoing.asp **//

log_out.titl="Ausgehende Log-Tabelle";
log_out.h2="Ausgehende Log-Tabelle";
log_out.th_lanip="LAN-IP";
log_out.th_wanip="Ziel-URL/IP";
log_out.th_port="Dienst-/Port-Nummer";

//help container

hfirewall.right2="Schaltet die SPI-Firewall ein oder aus.";

// help page
hfirewall.page1="<dd>Blockiert HTTP-Anfragen, die die Zeichenkette &quot;<i>Host:</i>&quot; enthalten.</dd>";
hfirewall.page2="<dd>Erkennt HTTP-Anfragen, die die Zeichenfolge &quot;<i>Cookie:</i>&quot; enthalten, und verfälscht das Cookie. Versucht, die Verwendung von Cookies zu verhindern.</dd>";
hfirewall.page3="<dd>Blockiert HTTP-Anfragen, die eine URL mit der Endung &quot;<i>.js</i>&quot; oder &quot;<i>.class</i>&quot; enthalten.</dd>";
hfirewall.page4="<dd>Blockiert HTTP-Anfragen, die eine URL mit der Endung &quot;<i>.ocx</i>&quot; oder &quot;<i>.cab</i>&quot; enthalten.</dd>";
hfirewall.page5="<dd>Verhindert, dass der Router auf &quot;Pings&quot; aus dem WAN antwortet.</dd>";
hfirewall.page6="<dd>Verhindert, dass Multicast-Pakete das LAN erreichen.</dd>";
hfirewall.page7="<dd>Verhindert, dass Hosts im LAN die WAN-Adresse des Routers verwenden, um Server (mit eingerichteter Port-Weiterleitung) im LAN zu kontaktieren.</dd>";
hfirewall.page8="<dd>Verhindert den WAN-Zugriff auf Port 113.</dd>";
hfirewall.page9="<dd>Der Router kann den gesamten eingehenden und ausgehenden Datenverkehr für Ihre Internetverbindung protokollieren.</dd>";
hfirewall.page10="<dd>Um Aktivitätsprotokolle zu erstellen, wählen Sie <em>" + share.enable + "</em>. Um die Protokollierung zu beenden, wählen Sie <em>" + share.disable + "</em>.</dd>";
hfirewall.page11="<dd>Stellen Sie hier die gewünschte Informationsmenge ein. Setzen Sie den <em>" + log.lvl + "</em> höher, um mehr Aktionen zu protokollieren.</dd>";
hfirewall.page12="<dd>Klicken Sie auf die Schaltfläche <em>" + sbutton.log_in + "</em>, um sich ein temporäres Protokoll der letzten eingehenden Verbindungen des Routers anzeigen zulassen.</dd>";
hfirewall.page13="<dd>Klicken Sie auf die Schaltfläche <em>" + sbutton.log_out + "</em>, um sich ein temporäres Protokoll der letzten ausgehenden Verbindungen des Routers anzeigen zulassen.</dd><dd>Überprüfen Sie alle Werte und klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em>, um Ihre Einstellungen zu speichern. Klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em>, um Ihre nicht gespeicherten Änderungen zu verwerfen.</dd>";

// ** Forward.asp **//

prforward.titl="Port-Bereichs-Weiterleitung";
prforward.h2="Port-Bereichs-Weiterleitung";
prforward.legend="Weiterleitungen";
prforward.app="Anwendung";

//help container

hprforward.right2="Einige Anwendungen benötigen offene Ports, damit sie korrekt funktionieren. Beispiele sind Server-Anwendungen oder einige Onlinespiele. Wenn eine Anfrage auf einem dieser Ports aus dem Internet kommt, wird der Router diese auf den entsprechend angegebenen Computer weiterleiten. Aus Sicherheitsgründen sollten Sie die Weiterleitung auf diejenigen Ports beschränken, die Sie benötigen.";

//help page
hprforward.page1="<dd>Die Port-Bereichs-Weiterleitung ermöglicht es Ihnen, öffentliche Dienste in Ihrem Netzwerk einzurichten, z. B. Webserver, FTP-Server, E-Mail-Server oder andere spezielle Internetanwendungen. Spezielle Internetanwendungen sind alle Anwendungen, die den Internetzugang nutzen, um Funktionen wie Videokonferenzen oder Online-Gaming bereitzustellen. Wenn diese Art von Anfragen über das Internet an das Netzwerk gesendet werden, leitet der Router diese Anfragen an das entsprechende Gerät weiter.<br /><br />Wenn Sie nur einen einzelnen Port weiterleiten möchten, lesen Sie bitte den Abschnitt <a href=\"HForward.asp\">Port-Weiterleitung</a>.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div>Jedem Gerät, dessen Port weitergeleitet wird, muss eine statische IP-Adresse zugewiesen werden, da sich die IP-Adresse bei Verwendung der DHCP-Funktion ändern kann.</div></div></dd><dd>Um eine neue Port-Bereichs-Weiterleitungs-Regel zu erstellen, klicken Sie auf die Schaltfläche <em>" + share.add + "</em>, und füllen Sie die unten stehenden Felder aus. Um Regeln zu entfernen, klicken Sie auf das Symbol <i>Löschen</i>.</dd>";
hprforward.page2="<dd>Geben Sie den Namen der Anwendung in das vorgesehene Feld ein.</dd>";
hprforward.page3="<dd>Geben Sie die erste Port-Nummer des Bereichs ein, der für Benutzer aus dem Internet sichtbar sein soll und an Ihre Geräte weitergeleitet werden soll.</dd>";
hprforward.page4="<dd>Geben Sie die letzte Port-Nummer des Bereichs ein, der für Benutzer aus dem Internet sichtbar sein soll und an Ihre Geräte weitergeleitet werden soll.</dd>";
hprforward.page5="<dd>Wählen Sie das richtige Protokoll <i>TCP</i>, <i>UDP</i> oder <i>Beides</i>. Stellen Sie hier ein, was die Anwendung erfordert.</dd>";
hprforward.page6="<dd>Geben Sie die IP-Adresse des Geräts ein, auf dem die Anwendung läuft.</dd>";
hprforward.page7="<dd>Klicken Sie auf das Kästchen <em>" + share.enable + "</em>, um die Portweiterleitung für die Anwendung zu aktivieren.</dd><dd>Überprüfen Sie alle Werte und klicken Sie auf die Schaltfläche <em>" + sbutton.save + "</em>, um Ihre Einstellungen zu speichern. Klicken Sie auf die Schaltfläche <em>" + sbutton.cancel + "</em>, um Ihre nicht gespeicherten Änderungen zu verwerfen.<br /><br /><div class=\"note\"><h4>Hinweis:</h4><div>Denken Sie daran, Ihre Änderungen zu speichern, bevor Sie eine weitere Weiterleitungs-Regel hinzufügen.</div></div></dd>";

// ** P2P.asp **//

p2p.titl="Peer-to-Peer Anwendungen";
p2p.h2="BitTorrent Klient";
p2p.legend="CTorrent";
p2p.ctorrent_srv="Ctorrent Dienst";







// ** ForwardSpec.asp **//

pforward.titl="Port-Weiterleitung";
pforward.h2="Port-Weiterleitung";
pforward.legend="Weiterleitungen";
pforward.app="Anwendung";
pforward.src="Von Netz";
pforward.from="Von Port";
pforward.to="Nach Port";

//help container

hpforward.right2="Um gewisse Programme auf Ihren Rechner nutzen zu können, kann es nötig sein, dass spezielle Ports geöffnet und weitergeleitet werden müssen. Programme welche Serverrollen inne haben, aber auch manche Onlinespiele sind davon betroffen. Aus Sicherheitsgründen sollen nur jene Ports weitergeleitet werden, welche unbedingt nötig sind.";

pforwardip.titl="IP Weiterleitung";
pforwardip.h2="IP Weiterleitung - 1:1 NAT";
pforwardip.legend="Weiterleitungen";
pforwardip.name="Name";
pforwardip.src="Quell IP";
pforwardip.dest="Ziel IP";

// ** USB.asp **//

usb.titl="USB";
usb.usb_legend="USB Unterstützung";
usb.usb_core="USB Unterstützung";
usb.usb_uhci="USB 1.1 Unterstützung (UHCI)";
usb.usb_ohci="USB 1.1 Unterstützung (OHCI)";
usb.usb_ehci="USB 2.0 Unterstützung";
usb.usb_storage="USB Laufwerks Unterstützung";
usb.usb_ip="USB über IP";
usb.usb_printer="USB Drucker Unterstützung";
usb.usb_automnt="Automatisches Einbinden des Laufwerks";
usb.usb_mntpoint="Laufwerks Verzeichnisbindung";
usb.usb_runonmount="Ausführen-beim-einbinden Script Name";
usb.usb_diskinfo="Laufwerks Info";
usb.usb_mntjffs="Diese Partition als /jffs einbinden";
usb.usb_mntopt="Diese Partition als /opt einbinden";
usb.usb_ses_umount="SES Button entfernt Laufwerke";
usb.drive_ra="Laufwerk Vorlesepuffer in Sektoren";

// ** NAS.asp **//

nas.titl="NAS";
nas.proftpd_legend="FTP Server";
nas.proftpd_srv="ProFTPD";
nas.proftpd_port="Server Port";
nas.proftpd_dir="Datei Verzeichnis";
nas.proftpd_passw="Benutzerkennwort Liste";
nas.proftpd_writeen="Schreiberlaubnis";
nas.proftpd_anon="Anonymer Login (Nur-Lesen)";
nas.proftpd_anon_dir="Anonymes Basisverzeichnis";
nas.samba3_legend="Dateifreigabe";
nas.samba3="Samba Server";
// ** DLNA **//
nas.dlna_legend="DLNA Server";
nas.dlna_srv="MiniDLNA";
nas.dlna_merge="Verzeichnisse zusammenführen";
nas.dlna_no_art="Ignoriere Album Grafik";
nas.dlna_thumb="Titelbilder einbinden";
nas.dlna_subtitles="Verwende Untertitel";
nas.dlna_dir="DLNA Verzeichnis";
nas.dlna_cleandb="Datenbank jetzt leeren";
nas.dlna_metadata="Metadaten behalten";
nas.dlna_rescan="Zyklisches neu Scannen";
nas.format="Formatieren";
nas.raidmember="Raid Mitglied";
nas.mirror="Spiegeln";
nas.fs="Dateisystem";
nas.drivemanager="Laufwerksverwaltung";
nas.drive="Laufwerk";
nas.compression="Kompression";
nas.nfs="Network File System Server";
nas.rsync="Fern-synchronisierung";

//** Privoxy.asp **//
privoxy.titl="Werbefilter";
privoxy.server="Privoxy";
privoxy.legend="Proxy Server";
privoxy.pac="Proxy Konfiguration bereitstellen";
privoxy.transp="Transparenter Modus";
privoxy.custom="Eigene Konfiguration";
privoxy.maxclient="Maximale Verbindungen";

hprivoxy.right2="Schaltet Privoxy ein und konfiguriert ihn für ihre Clients.<br /> Proxy IP = <b>Router IP</b> <br />Proxy Port = <b>8118</b>";

//help page
hprivoxy.page1="<dd>Privoxy ermöglicht das Entfernen von Werbung. <br /><ul><br /></ul></dd>";
hprivoxy.page2="<dd><br /><ul><br /><li>Publiziert ein wpad/pac File das dazu dient den Proxy auf Clientcomputern automatisch zu konfigurieren</li><li>Bei manchen Clients ist es nötig die automatische Proxykonfiguration zu aktivieren</li><li>Falls ihr Klient Mitglied einer Domäne ist, müssen sie das Senden der lokalen Domäne aktivieren</li></ul></dd>";
hprivoxy.page3="<dd><br /><ul><li>Verbindungen ins Internet zu Port 80 werden über den Werbefilter geleitet</li><li>Dadurch kann das Filtern für jeden Klient erzwungen werden</li><li>Im transparenten Modus können keine HTTPS Verbindungen gefiltert werden, es sei denn der Klient nutzt die Proxy Auto-Config um die Verbindungen über den Werbefilter zu leiten</li></ul></dd>";
hprivoxy.page4="<dd><br /><ul><li>Erlaubt das Vorgeben eigener Einstellungen</li></ul></dd>";


//** Lighttpd.asp **//

lighttpd.wan="WAN Zugriff";

hnas.right2="Um eine neue Freigabe zu erstellen müssen Pfad und Freigabename eingetragen werden. Anschließend müssen Benutzer und deren Rechte für FTP und Samba Zugriff spezifiziert werden";


// ** Hotspot.asp ** //

hotspot.titl="Hotspot";
hotspot.h2="Hotspot-Portal";
hotspot.legend="CoovaChilli";
hotspot.nowifibridge="Entferne WLAN aus der LAN-Bridge";
hotspot.hotspot="CoovaChilli";
hotspot.pserver="Primärer RADIUS-Server IP/DNS";
hotspot.bserver="Sekundärer RADIUS-Server IP/DNS";
hotspot.dns="DNS-Server IP";
hotspot.url="Umleitungs-URL";
hotspot.dhcp="DHCP-Schnittstelle";
hotspot.radnas="RADIUS NAS ID";
hotspot.net="Netzwerk der Schnittstelle";
hotspot.uam="UAM Secret";
hotspot.uamserver="UAM Server";
hotspot.uamport="UAM Port";
hotspot.uamdns="UAM Any DNS";
hotspot.allowuam="UAM erlaubt (getrennt durch Beistrich)";
hotspot.allowuad="UAM Domänen (getrennt durch Leerzeichen)";
hotspot.macauth="MAC Authentifizierung";
hotspot.macpasswd="MAC Kennwort";
hotspot.sec8021Xauth="802.1X Authentifizierung (EAP)";
hotspot.option="Zusätzliche CoovaChilli-Optionen";
hotspot.fon_chilli="CoovaChilli lokales Benutzermanagement";
hotspot.fon_user="Benutzerliste";
hotspot.http_legend="HTTP-Umleitung";
hotspot.http_srv="HTTP-Umleitung";
hotspot.http_ip="HTTP-Ziel-IP";
hotspot.http_port="HTTP-Ziel-Port";
hotspot.http_net="HTTP-Ursprungsnetzwerk";
hotspot.nodog_legend="NoDogSplash";
hotspot.nodog_srv="NoDogSplash";
hotspot.nodog_gateway="Gateway-Name";
hotspot.nodog_gatewayaddr="Gateway IP Adresse";
hotspot.nodog_gatewayport="Gateway Port";
hotspot.nodog_gatewayiprange="Gateway IP Bereich";
hotspot.nodog_home="Homepage";
hotspot.nodog_extifname="Externe Schnittstelle";
hotspot.nodog_ifname="Interne Schnittstelle";
hotspot.nodog_redirect="Homepage Weiterleitung";
hotspot.nodog_allowweb="Erlaubte Web-Hosts";
hotspot.nodog_docroot="Dokumenten-Pfad";
hotspot.nodog_splash="Splash-URL";
hotspot.nodog_port="Ports ausschließen";
hotspot.nodog_timeout="Login-Timeout";
hotspot.nodog_verbose="Verbosity";
hotspot.nodog_route="Nur Routen";
hotspot.nodog_MAClist="MAC Ausnahmeliste";
hotspot.nodog_maxclients="Maximale Anzahl an Clients";
hotspot.nodog_downloadlimit="Downloadlimit";
hotspot.nodog_uploadlimit="Uploadöimit";
hotspot.smtp_legend="SMTP-Umleitung";
hotspot.smtp_srv="SMTP-Umleitung";
hotspot.smtp_ip="SMTP-Ziel-IP";
hotspot.smtp_net="SMTP-Ursprungs-Netzwerk";
hotspot.shat_legend="Zero IP Config";
hotspot.shat_srv="Zero IP Config";
hotspot.shat_srv2="Zero IP Config eingeschaltet";
hotspot.wifidog_legend="Wifidog";
hotspot.wifidog_srv="Wifidog daemon";
hotspot.wifidog_id="Gateway ID";
hotspot.wifidog_url="Portal's URL";
hotspot.wifidog_port="Port";
hotspot.wifidog_httpdname="Web Server Name";
hotspot.wifidog_httpdconn="Maximale Benutzeranzahl";
hotspot.wifidog_checkinter="Prüfintervall (s)";
hotspot.wifidog_checktimeout="Client Timeout";
hotspot.wifidog_tmaclist="Trusted MAC Liste";
hotspot.wifidog_authsrv="AuthServer Hostname";
hotspot.wifidog_authsrvssl="AuthServer SSL verfügbar";
hotspot.wifidog_authsrvsslport="AuthServer SSL Port";
hotspot.wifidog_authsrvhttpport="AuthServer HTTP Port";
hotspot.wifidog_authsrvpath="AuthServer Path";
hotspot.wifidog_config="Firewall Regeln";
hotspot.wifidog_messagefile="HTML Nachrichten Datei für Wifidog";
hotspot.wifidog_realm="HTTP Server Realm";
hotspot.wifidog_username="HTTP Server Benutzername";
hotspot.wifidog_password="HTTP Server Kennwort";
hotspot.wifidog_auth="HTTP Authentifizierungs Unterstützung";

//help container

hstatus_hots.right1="conup/condown:<br />Wenn USB oder JFFS2 als <b>/jffs</b> eingebunden ist, können Verbindungsskripte in  <b>/jffs/etc/chilli/</b> verwendet werden.<br />Lokale Benutzer:<br />Wenn nur lokale Benutzer verwendet werden, soll der primäre RADIUS-server auf <b>127.0.0.1</b> gesetzt werden.";

// ** Hotspotsystem **//

hotspotsys.legend="Hotspot System";
hotspotsys.nobridge="Entferne WLAN aus der LAN-Bridge";
hotspotsys.uamenable="Spezielle Einstellungen";
hotspotsys.loginonsplash="Login auf Splash Seite";
hotspotsys.allowuam="UAM erlaubt";
hotspotsys.allowuad="UAM Domänen (getrennt mit Leerzeichen)";
hotspotsys.whitelabelproto="White Label Protocol";
hotspotsys.whitelabel="White Label Domain";
hotspotsys.operatorid="Benutzername des Betreibers";
hotspotsys.locationid="Standort Nummer";
hotspotsys.dhcp="DHCP Schnittstelle";
hotspotsys.net="Netzwerk der Schnittstelle";
hotspotsys.customsplash="Angepasste Splash Seite (Walled Garden)";

// **************************************************************** AnchorFree.asp *********************************************************//

//anchorfree.anchorfree="AnchorFree";
//anchorfree.title="Mit Hotspot Geld verdienen";
//anchorfree.anchorfree_revenue="Durch AnchorFree Hotspot mit Werbung, Geld verdienen";
//anchorfree.email="E-Mail für Zahlungsberichte";
//anchorfree.ssid="Nutze alternative SSID";
//anchorfree.ssid_name="SSID";
//anchorfree.address_1="Adresse";
//anchorfree.address_2="Adresse 2";
//anchorfree.city="Stadt";
//anchorfree.zip="Postleitzahl";
//anchorfree.state="Bundesland";
//anchorfree.country="Land";
//anchorfree.category="Kategorie";
//anchorfree.publish="Hotspot auf WiFi Karte anzeigen";
//anchorfree.serviceid="Dienste ID";
//anchorfree.servicestatus="Dienst Status";
//anchorfree.agreement="Nutzungsbestimmungen";
//anchorfree.agree="Einverstanden";
//anchorfree.validaddr="Um den Hotspot korrekt auf der Karte anzeigen zu können, muß eine gültige Adresse angegeben werden!";
//anchorfree.validcity="Um den Hotspot korrekt auf der Karte anzeigen zu können, muß eine gültige Stadt oder Postleitzahl angegeben werden!";
//anchorfree.validcat="Bitte wählen Sie eine Kategorie für ihren Hotspot aus!";
//anchorfree.validcountry="Bitte geben Sie Ihr Land an";
//anchorfree.validterms="Sie müssen den Nutzungsbestimmungen zustimmen!";
//

//hanchorfree.right1="Tritt dem AnchorFree Hotspot Werbenetzwerk bei";
//hanchorfree.right2="AnchorFree betreibt ein Hotspot Werbenetzwerk welches DD-WRT Benutzern wachsende Gewinnmöglichkeiten bietet";
//hanchorfree.right3="Generiere wachsende Gewinne durch die Nutzung von AnchorFree";
//hanchorfree.right4="Durch das Einschalten von AnchorFree und durch das Erstellen des dazugehörigen Benutzerkontos (kostenlos und einfach), wird ein permanenter Werbebereich in die aufgerufenen Webseiten eingefügt, welches zu einer monatlichen Gewinnausschüttung führt.  Beginnend von einem monatlichen Minimum von 25$, wird ihnen ihr Guthaben automatisch von AnchorFree überwiesen.";
//hanchorfree.right5="Für weitere Informationen besuchen sie bitte www.anchorfree.com";
//hanchorfree.right6="Das Einschalten ist einfach";
//hanchorfree.right7="Einmal eingeschaltet, wird ihnen von AnchorFree automatisch eine E-Mail an ihre registrierte E-Mail Adresse geschickt, welche einfache Anweisungen über die Optimierung ihres Hotspots, FAQs und andere wichtige Informationen über das Geld verdienen mit ihrem Router enthält. Durch dieses Konfigurationsmenü wird von AnchorFree ein kleines, nicht zu aufdringliches Werbebanner in die aufgerufenen Webseiten eingefügt.";
//hanchorfree.right8="Kunden Unterstützung";
//hanchorfree.right9="Haben Sie Fragen? Kontaktieren Sie uns durch boxhelp@anchorfree.com";



// ** Info.htm **//

info.titl="Info";
info.h2="System Information";
info.wlanmac="WLAN MAC";
info.srv="Dienste";
info.ap="Access Point";


// ** index_heartbeat.asp **//

idx_h.srv="Heartbeat-Server";
idx_h.con_strgy="Verbindungsstrategie";
idx_h.max_idle="Bei Bedarf verbinden: Maximale Ruhezeit";
idx_h.alive="Lebenserhaltung: Zeitabstand Wiedereinwahl";
idx_h.reconnect="Erzwinge Neuverbindung";



// ** index_l2tp.asp **//

idx_l.srv="L2TP-Server";
idx_l.req_chap="Benötige CHAP";
idx_l.ref_pap="Verweigere PAP";
idx_l.req_auth="Benötige Authentifizierung";


// ** index_pppoe.asp **//

idx_pppoe.use_rp="Nutze RP PPPoE";


// ** index_pptp.asp **//

idx_pptp.srv="Nutze DHCP";
idx_pptp.wan_ip="WAN-IP-Adresse";
idx_pptp.gateway="Gateway (PPTP-Server)";
idx_pptp.encrypt="PPTP-Verschlüsselung";
idx_pptp.reorder="Paket Neuanordung";
idx_pptp.addopt="Zusätzliche PPTP-Optionen";



// ** index_static.asp **//

idx_static.dns="Statischer DNS";


// ** index.asp **//

idx.titl="Einstellungen";
idx.h2="WAN-Einstellungen";
idx.h22="WLAN-Einstellungen";
idx.legend="WAN-Verbindungstyp";
idx.conn_type="Verbindungstyp";
idx.stp="STP";
idx.stp_mess="(Für COMCAST-Provider ausschalten)";
idx.optional="Zusätzliche Einstellungen";
idx.mtu="MTU";
idx.txqlen="TX Warteschlangenlänge";
idx.h23="Netzwerk-Einstellungen";
idx.routerip="Router-IP";
idx.lanip="Lokale IP-Adresse";
idx.legend2="WAN-Port";
idx.wantoswitch="WAN-Port dem Switch zuweisen";
idx.legend3="Uhrzeit-Einstellungen";
idx.timeset="Zeitzone";
idx.static_ip="Statische IP";
idx.dhcp="Automatische Konfiguration - DHCP";
idx.dsl_mdm_bdg="DSL Modem Bridge";
idx.pppoe_dual="PPPoE Dual (MLPPP)";
idx.heartbeat_sig="Heartbeat Signal";
idx.iphone_tether="iPhone Tethering";
idx.mobile_bb="Mobile Broadband";
idx.dhcp_auth="DHCP mit Authentifizierung";
idx.dhcp_legend="Einstellungen Netzwerk-Adress-Server (DHCP)";
idx.dhcp_type="DHCP-Typ";
idx.dhcp_srv="DHCP-Server";
idx.dhcp_fwd="DHCP-Weiterleitung";
idx.dhcp_start="Start-IP-Adresse";
idx.dhcp_end="End-IP-Adresse";		//used in Status_Lan.asp
idx.dhcp_maxusers="Maximale DHCP-Nutzeranzahl";
idx.dhcp_lease="Lease-Ablauf";
idx.dhcp_dnsmasq="Nutze dnsmasq für DHCP";
idx.dns_dnsmasq="Nutze dnsmasq für DNS";
idx.auth_dnsmasq="DHCP-Authoritative";
idx.force_dnsmasq="Erzwungene DNS Umleitung";
idx.force_dnsmasqdot="Erzwungene DoT DNS Umleitung";
idx.recursive_dns="Rekursive DNS Auflösung (Unbound)";
idx.dns_redirect="Optionales DNS Ziel";
idx.summt_opt1="keine";
idx.summt_opt2="erster Son Apr - letzter Son Okt";
idx.summt_opt3="letzter Son Mär - letzter Son Okt";
idx.summt_opt4="letzter Son Okt - letzter Son Mär";
idx.summt_opt5="zweiter Son Mär - erster Son Nov";
idx.summt_opt6="erster Son Okt - dritter Son Mär";
idx.summt_opt7="letzter Son Sep - erster Son Apr";
idx.summt_opt8="dritter Son Okt - dritter Son Mär";
idx.summt_opt9="erster Son Okt - erster Son Apr";
idx.summt_opt10="dritter Son Okt - dritter Son Feb";
idx.portsetup="Port Setup";
idx.wanport="WAN Port Zuweisung";
idx.ntp_client="NTP-Client";
idx.ntp_timer="Updateintervall";
idx.ignore_wan_dns="Ignoriere WAN DNS";

//help container

hidx.right2="Diese Einstellung wird von fast allen Providern genutzt.";
hidx.right4="Geben Sie hier den Hostnamen ein, der Ihnen vom Provider zugewiesen wurde.";
hidx.right6="Geben Sie hier den Domainnamen ein, der Ihnen vom Provider zugewiesen wurde.";
hidx.right8="Dies ist die lokale IP-Adresse Ihres Routers im LAN.";
hidx.right10="Dies ist die Netzmaske Ihres Routers.";
hidx.right12="Erlaubt dem Router, Ihre IP-Adressen zu verwalten.";
hidx.right14="Die erste IP-Adresse die vergeben werden soll.";
hidx.right16="Sie können hier die Anzahl der IP-Adressen einschränken, die der Router maximal vergeben kann. 0 bedeutet das nur vordefinierte statische Leases zur Vergabe verwendet werden.";
hidx.right18="Wählen Sie hier die Zeitzone in der Sie sich befinden. Wenn das Server IP/Name Feld nicht gesetzt ist, wird die interne Adresse für den Zeitserver verwendet (empfohlen)";
hidx.sas="Der Setup Assistent leitet Sie durch die Vereinfachten Einstellungen um ihren Router einzurichten.";

// ** DSL ** //

dsl.status="DSL Status";
dsl.annex=" DSL Annex";
dsl.iface_status="Verbindungsstatus";
dsl.datarate="Verbindungsgeschwindigkeit (up/down)";
dsl.snr="DSL Signal (up/down)";

// ** Join.asp **//

//sshd.webservices
join.titl="Verbinden";
join.mess1="Sie sind nun erfolgreich mit dem folgenden Netzwerk als Client verbunden: ";

// ** Management.asp **//
management.titl="Administration";
management.h2="Router Management";
management.changepassword="Ihr Router ist derzeit nicht gegen unbefugten Zugriff geschützt, bitte ändern Sie ihren Benutzernamen und ihr Kennwort mit Hilfe des folgenden Dialogs!";
management.psswd_legend="Router-Kennwort";
management.psswd_user="Router-Benutzername";
management.psswd_pass="Router-Kennwort";
management.pass_conf="Zum Bestätigen erneut eingeben";
management.remote_legend="Fernzugriff";
management.remote_gui="Web-GUI-Management";
management.remote_https="Nutze HTTPS";
management.remote_guiport="Web-GUI-Port";
management.remote_ssh="SSH-Management";
management.remote_sshport="SSH-Port";
management.remote_telnet="Telnet-Management";
management.remote_telnetport="Telnet-Port";
management.remote_allowanyip="Erlaube alle Remote-IPs";
management.remote_ip="Erlaubter Remote-IP-Bereich";
management.web_legend="Web-Zugriff";
management.web_refresh="Auffrischen";
management.web_sysinfo="Infoseite Einschalten";
management.web_sysinfopass="Kennwortschutz Infoseite";
management.web_sysinfomasq="MAC-Maskierung Infoseite";
management.boot_legend="Beim Booten warten";
management.poeswitch="POE Schalter";
management.boot_srv="Beim Booten warten";
management.cron_legend="Cron";
management.cron_srvd="Cron";
management.cron_jobs="Zusätzliche Cron Jobs";
management.loop_legend="Loopback";
management.loop_srv="Loopback";
//802.1x was removed in r49200 / r49201
//management.wifi_legend="802.1x";
//management.wifi_srv="802.1x";
management.rst_legend="Resetknopf";
management.rst_srv="Resetknopf";
//management.routing_legend="Routing";
//management.routing_srv="Routing";
management.ipv6_h2="Internet Protokoll Version 6 (IPv6)";
management.ipv6_legend="IPv6-Unterstützung";
management.ipv6_srv="IPv6 Einschalten";
management.ipv6_native="Natives IPv6 vom Provider";
management.ipv6_px_del="DHCPv6 mit Prefix Delegation";
management.ipv6_6in4st="6in4 Statischer Tunnel";
management.ipv6_typ="IPv6 Type";
management.ipv6_pf_len="Prefix Länge";
management.ipv6_rad_legend="Router Advertisement Daemon (radvd)";
management.ipv6_rad_enable="Daemon Einschalten";
management.ipv6_rad="Individuelle Konfiguration";
management.ipv6_radconf="Radvd-Konfiguration";
management.ipv6_dns="Statischer DNS";
management.ipv6_prefix="Zugewiesener / Routed Prefix";
management.ipv6_addr="Router IPv6 Addresse";
management.ipv6_dhcp6c_norelease="DHCP6 nicht freigeben";
management.ipv6_dhcp6c_cust="Dhcp6c Individuell";
management.ipv6_dhcp6c_conf="Dhcp6c Konfiguration";
management.ipv6_dhcp6s="Dhcp6s";
management.ipv6_dhcp6s_seq_ips="Aufeinanderfolgende IPs";
management.ipv6_dhcp6s_hosts="Individuelle Hosts";
management.ipv6_dhcp6s_cust="Dhcp6s Individuell";
management.ipv6_dhcp6s_conf="Dhcp6s Konfiguration";
management.ipv6_tun_end_ipv4="Tunnel Endpunkt IPv4 Addresse";
management.ipv6_tun_client_addr="Tunnel Client IPv6 Addresse";
management.ipv6_tun_upd_url="Tunnel Update URL";
management.bootfail_handling="Bootfehler Behandlung";
management.bootfail="Reset nach 5 Bootfehlern";
management.boot_fail_open="Offenes WLAN nach Bootfehler";
management.boot_fail_keepip="IP nach Bootfehler behalten";

management.jffs_legend="JFFS2-Unterstützung";
management.jffs_srv="JFFS2";
management.jffs_clean="JFFS2 löschen";
management.lang_legend="Sprachauswahl";
management.lang_srv="Sprache";
management.lang_bulgarian="Bulgarisch";
management.lang_catalan="Katalanisch";
management.lang_chinese_traditional="Chinesisch traditionell";
management.lang_chinese_simplified="Chinesisch simplified";
management.lang_croatian="Kroatisch";
management.lang_czech="Tschechisch";
management.lang_dutch="Niederländisch";
management.lang_english="Englisch";
management.lang_portuguese_braz="Portugiesisch (Brasil.)";
management.lang_french="Französisch";
management.lang_german="Deutsch";
management.lang_turkish="Türkisch";
management.lang_polish="Polnisch";
management.lang_italian="Italienisch";
management.lang_brazilian="Portugiesisch (Brasil.)";
management.lang_russian="Russisch";
management.lang_romanian="Rumänisch";
management.lang_serbian="Serbisch";
management.lang_slovenian="Slowenisch";
management.lang_spanish="Spanisch";
management.lang_swedish="Schwedisch";
management.lang_japanese="Japanisch";
management.lang_hungarian="Ungarisch";
management.lang_latvian="Lettisch";
management.net_legend="Network Stack Tuning";
management.net_conctrl="TCP Staukontroll-Mechanismus";
management.net_ipcontrkmax="Maximale Verbindungen";
management.net_tcptimeout="TCP-Timeout";
management.net_udptimeout="UDP-Timeout";
management.clock_legend="Übertakten";
management.clock_frq="Frequenz";
management.clock_support="Nicht unterstützt";
management.mmc_legend="MMC/SD-Karten-Unterstützung";
management.mmc_srv="MMC-Gerät";
management.mmc_gpiosel="GPIO Kontakte auswählen";
management.mmc_gpiopins="GPIO Kontakte";
management.mmc_cardinfo="Karten Info";
management.samba_legend="CIFS automatisches Einbinden";
management.samba_srv="Common Internet File System";
management.samba_share="Freigabe";
management.samba_stscript="Startskript";
management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP-Port";
management.SIPatH_domain="SIP-Domain";
management.gui_style="Stil des Router-GUI";
management.too_short="Zu Kurz";
management.very_weak="Sehr Schlecht";
management.weak="Schlecht";
management.good="Gut";
management.strong="Sicher";
management.very_strong="Sehr Sicher";
management.score="Punkte:";
management.complexity="Sicherheit:";
management.bootconfig="Startkonfiguration (Grub)";
management.disable_msi="MSI Interrupt Behandlung abschalten.";
management.pci_tuning="PCI/PCI-E Bus anpassen";
management.pcie_aer="PCI-E Erweitertes Error Reporting (AER)";
management.pcie_ari="PCI-E Alternative Routen Interpretation (ARI)";
management.pci_noacpi="ACPI Irq Routing abschalten";
management.bus_tune_off="Kein Bus Tuning";
management.bus_safe="Sicheres Bus Tuning";
management.bus_perf="Schnelles Bus Tuning";
management.bus_peer2peer="Peer2Peer Bus Tuning";
management.nospectre_v1="Spectre V1 entschärfen";
management.nospectre_v2="Spectre V2 entschärfen";
management.mds="MDS entschärfen";
management.srbds="SRBDS entschärfen";
management.l1tf="L1TF entschärfen";
management.nospec_store_bypass_disable="Speculative Store Bypass entschärfen";
management.tsx_async_abort="TSX Async Abort entschärfen";
management.nopti="Seitentabelle isolieren";
management.pstate="Erzwinge AMD P-State Frequenz-Treiber";

//help container

hmanagement.right1="Automatisches Auffrischen:";
hmanagement.right2="Steuert das automatische Auffrischen diverser GUI-Elemente. 0 schaltet diese Funktion ab.";

// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymore) *****************************************//

portserv.titl="Port-Dienste";
portserv.h2="Port-Dienste";


// ** Networking.asp **//

networking.h2="VLAN Tagging";
networking.legend="Tagging";
networking.h22="Bridging";
networking.h2h="Generic Networking and VLAN";
networking.legend2="Erstelle Bridge";
networking.legend3="Weise Bridge zu";
networking.legend4="Aktuelle Bridging Tabelle";
networking.brname="Bridge Name";
networking.stp="STP";
networking.iface="Schnittstelle";
networking.h5="DHCPD";
networking.legend5="Mehrere DHCP Server";
networking.vlan="VLAN";
networking.vlantype="Typ";
networking.iface="Schnittstelle";
networking.tg_number="Tag Nummer";
networking.prio="Prio";
networking.pathcost="Pfadkosten";
networking.hairpin="Hairpin Modus";
networking.bridge="Bridge";
networking.snooping="IGMP Snooping";
networking.unicast="Multicast nach Unicast";
networking.assign="Zuweisung";
networking.bridgeassign="Bridge Zuweisung";
networking.bonding="Bonding";
networking.bondtype="Bonding Type";
networking.bondifaces="Bonding Schnittstelle";
networking.bond="Bond";
networking.slave="Slave";
networking.max="Max";
networking.leasetime="Verfallszeit";
networking.ipvs="IP Virtueller Server";
networking.create_ipvs="Erstelle Virtuellen Server";
networking.ipvs_name="Servername";
networking.ipvs_sourceip="Quell IP";
networking.ipvs_sourceport="Quell Port";
networking.ipvs_scheduler="Scheduler";
networking.ipvs_targets="Virtueller Server Zieles";
networking.ipvs_targetip="Ziel IP";
networking.ipvs_targetport="Ziel Port";
networking.ipvs_weight="Wichtung";
networking.ipvs_role="Rolle";
networking.ipvs_config="Konfiguration";


//help container

hnetworking.right1="Mehrfach-DHCP";
hnetworking.right2="Aktiviere dnsmasq als DHCP Server um Mehrfach-DHCP zu nutzen.";

// ** QoS.asp **//

qos.titl="Dienstqualität";
qos.h2="Dienstqualität (QoS)";
qos.legend="QoS-Einstellungen";
qos.srv="QoS starten";
qos.type="Packet Scheduler";
qos.aqd="Warteschlangenrichtlinie";
qos.aqd_sfq="SFQ";
qos.aqd_codel="CODEL";
qos.aqd_fqcodel="FQ_CODEL";
qos.uplink="Uplink (kbit/s)";
qos.dnlink="Downlink (kbit/s)";
qos.legend2="Dienst-Priorität";
qos.prio_exempt="Übervorteilt";
qos.prio_m="Manuell";
qos.prio_x="Maximal";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_b="Bulk";
qos.legend3="Netzmasken-Priorität";
qos.ipmask="IP/Maske";
qos.maxrate_b="Max kbit/s";
qos.maxuprate_b="WAN Max Up";
qos.maxdownrate_b="WAN Max Down";
qos.maxlanrate_b="LAN Max";
qos.maxrate_o="Max Rate";
qos.legend4="MAC-Priorität";
qos.legend5="Ethernet-Port-Priorität";
qos.legend6="Standard-Bandbreiten-Zuweisung";
qos.legend7="TCP-Paket Priorität";
qos.legend8="Schnittstellen-Priorität";
qos.pktdesc="Priorisiere kleine TCP-Pakete mit den folgenden Attributen:";
qos.pktack="ACK";
qos.pktrst="RST";
qos.pktsyn="SYN";
qos.pktfin="FIN";
qos.enabledefaultlvls="Benutzer Bandbreiten Management";
qos.bandwidth="Bandbreite";
qos.speed="kbit/s";
qos.service="Dienst";

//help container

hqos.right1="Uplink:";
hqos.right2="Setzen Sie dies auf 80%-95% (max) Ihrer gesamten Upload-Bandbreite.";
hqos.right3="Downlink:";
hqos.right4="Setzen Sie dies auf 80%-100% Ihrer gesamten Download-Bandbreite.";
hqos.right6="Hier können sie die Datenrate unter anwendungsspezifisch kontrollieren.";
hqos.right8="Hier können sie die Priorität des gesamten Datenverkehrs IP-Adressen oder IP-Bereichen zuweisen.";
hqos.right10="Hier können Sie die Priorität für den gesamten Datenverkehr eines Gerätes in Ihrem Netzwerk angeben, indem Sie von dem Gerät die MAC-Adresse, die Priorität und einen Namen angeben";
hqos.right12="Hier können Sie die Datenrate der physischen LAN-Ports an Ihrem Router angeben. Sie können hier Prioritäten für die Geräte angeben die an den Ports 1 - 4 angeschlossen sind";


// ** RouteTable.asp **//

routetbl.titl="Routentabelle";
routetbl.h2="Routentabelle";
routetbl.h3="Routen Regeltabelle";
routetbl.th1="Ziel-LAN-Netz";
routetbl.masquerade="Maskiere Route (NAT)";
routetbl.scope="Bereich";
routetbl.advmss="Bewerbe MSS";
routetbl.table="Tabelle";
routetbl.src="Ursprung";
routetbl.not="Nicht";
routetbl.from="Von";
routetbl.to="Nach";
routetbl.priority="Priorität";
routetbl.tos="ToS / DS Field";
routetbl.fwmark="Fwmark";
routetbl.realms="Bereich";
routetbl.table="Tabelle";
routetbl.suppress_prefixlength="Prefixlänge ausblenden";
routetbl.iif="Eingehendes Interface";
routetbl.oif="Ausgehendes Interface";
routetbl.nat="Nat Adresse";
routetbl.type="Typ";
routetbl.sport="Quell Port Bereich";
routetbl.dport="Ziel Port Bereich";
routetbl.ipproto="IP Protokoll";


// ** Routing.asp **//

route.titl="Routing";
route.rule_name="Regel Name";
route.global="Global";
route.nowhere="Nowhere";
route.host="Host";
route.link="Link";
route.site="Site";
route.unicast="Unicast";
route.blackhole="Blackhole";
route.unreachable="Unerreichbar";
route.prohibit="Verboten";

route.h2="Erweitertes Routing";
route.metric="Metrik";
route.flags="Markierungen";
route.mod="Betriebsmodus";
route.bgp_legend="BGP-Einstellungen";
route.bgp_own_as="BGP Eigner AS#";
route.bgp_ip="Nachbar-IP";
route.bgp_as="Nachbar-AS#";
route.rip2_mod="RIP2-Router";
route.olsrd_mod="OLSR Router";
route.olsrd_legend="OLSR Routing (Optimiertes Link Status Routing)";
route.olsrd_poll="Abfrage Rate";
route.olsrd_gateway="Gateway Modus";
route.olsrd_hna="Host Net Announce";
route.olsrd_tc="TC Redundanz";
route.olsrd_mpr="MPR Abdeckung";
route.olsrd_lqfe="Verbingundsqualität Fish Eye";
route.olsrd_lqag="Verbingundsqualität Aging";
route.olsrd_lqdmin="Verbingundsqualität Dijkstra Min";
route.olsrd_lqdmax="Verbingundsqualität Dijkstra Max";
route.olsrd_lqlvl="Qualitätswert";
route.olsrd_hysteresis="Hysterese";
route.olsrd_newiface="Neue Schnittstelle";
route.olsrd_smartgw="Smart Gateway";
route.zebra_legend="Zebra Konfiguration";
route.zebra_log="Zebra Log";
route.zebra_copt="Zebra Konfigurationsmodus";
route.bird_legend="Bird Konfiguration";
route.bird_log="Bird Log";
route.bird_copt="Bird Konfigurationsmodus";
route.ospf_mod="OSPF-Router";
route.ospf_rip2_mod="OSPF & RIP2-Router";
route.ospf_legend="OSPF Routing";
route.ospf_conf="OSPF Konfiguration";
route.ospf_copt="OSPF Konfigurationsmodus";
route.copt_gui="GUI";
route.copt_vtysh="Vtysh";
route.gateway_legend="Dynamisches Routing";
route.static_legend="Routing Tabelle";
route.static_setno="Wähle Route";
route.static_name="Routenname";
route.static_ip="Ziel-LAN-Netz";
route.rules="Routing Regeln";

//help container

hroute.right2="Wenn Ihr Router die Internetverbindung bereitstellt, benutzen Sie bitte den <em>Gateway</em>-Modus. Wenn ein anderer Router in Ihrem Netzwerk existiert, nutzen Sie den <em>Router</em>-Modus.";
hroute.right4="Dies ist eine eindeutige Routen-Nummer, Sie können bis zu 50 Routen definieren.";
hroute.right6="Geben Sie einen Namen an, den Sie der Route zuweisen möchten.";
hroute.right8="Dies ist die IP-Adresse des Zielnetzes, dem die statische Route zugewiesen wird.";
hroute.right10="Setzt den angewandten Netzwerk-Bereich.";


// ** Site_Survey.asp **//

survey.titl="Netzsuche";
survey.titl2="Kanalsuche";
survey.h2="WLAN-Netzwerke in der Nähe";
survey.h3="Kanäle und Qualitäten";
survey.thjoin="Verbinden";




// ** Services.asp **//

service.titl="Dienste";
service.h2="Dienst-Management";

service.apserv_legend="APServ Fernwartung";
service.apserv="APServ";

//kaid
//service.kaid_legend="XBOX-Kaid";
//service.kaid_srv="Start Kaid";
//service.kaid_locdevnum="Anzahl lokaler Geräte";
//service.kaid_uibind="UI Port";
//service.kaid_orbport="ORB Port";
//service.kaid_orbdeepport="ORB Deep Port";

//DHCPd
service.dhcp_legend="DHCP-Client";
service.dhcp_vendor="Setze Vendorclass";
service.dhcp_121="DHCP Classless Routes (option 121)";
service.dhcp_reqip="Request IP";
service.dhcp_legend2="DHCP-Server";
service.dhcp_srv="DHCP-Daemon";
service.dhcp_jffs2="Nutze JFFS2 für Clientzuweisungs-DB)";
service.dhcp_nvramlease="Nutze NVRAM für Clientzuweisungs-DB";
service.dhcp_domain="Genutzte Domain";
service.dhcp_landomain="LAN-Domain";
service.dhcp_option="Zusätzliche DHCPD-Optionen";
service.dns_crypt="Verschlüssele DNS (DNSCrypt)";
service.dns_crypt_resolv="DNS Crypt Server";
service.dns_smartdns="SmartDNS Auflösung";
service.dns_smartdns_dualstack="Dualstack IP Vorauswahl";
service.dns_smartdns_prefetch_domain="Domain Vorabruf";
service.dns_smartdns_serve_expired="Serve Expired";
service.dns_smartdns_use_dns="Verwende nur zusätzliche Server";
service.dns_smartdns_option="Zusätzliche SmartDNS-Optionen";

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Kennwortanmeldung";
service.ssh_key="Autorisierte Schlüssel";
service.ssh_forwarding="SSH TCP Weiterleitung";
service.ssh_keylegend="Schlüsselbehandlung";
service.ssh_keygenerate="Schlüssel erzeugen";
service.ssh_keylength="SSH Schlüssellänge";
service.ssh_replace="Vorhandene(n) Schlüssel ersetzen";
service.ssh_download="Privaten Schlüssel herunterladen";
service.ssh_keyalert="Das Generieren der Schlüssel kann bis zu 10 Minuten dauern, bitte haben Sie etwas Geduld.\nKlicken Sie nach dem Generieren auf die Schaltfläche Privaten Schlüssel herunterladen.";
service.ssh_keydownload="Klicken Sie nach dem Generieren auf die Schaltfläche Anwenden.\nDer Private Schlüssel wird im OpenSSH Format heruntergeladen.\nFalls Sie PuTTY verwenden möchten, muss der Schlüssel zuerst mit PuTTYgen in ein kompatibles Format konvertiert werden.";

//help page
hservice.right1="Bevor Sie das Speichern der Clientzuweisungs-DB im JFFS2-Flashspeicher aktivieren, muss die JFFS2-Unterstützung <b>aktiviert</b> werden. Die JFFS2-Unterstützung befindet sich auf der Registerkarte Administration / Management.";
hservice.right2="Es werden ausschließlich die Server verwendet, die Sie in den zusätzlichen SmartDNS-Optionen definieren.";

service.dnsmasq_dnssec="Prüfe DNS Antworten (DNSSEC)";
service.dnsmasq_dnssec_proxy="DNSSEC Daten Cachen";
service.dnsmasq_dnssec_cu="Prüfe unsignierte DNS Antworten";

service.dnsmasq_legend="Dnsmasq";
service.dnsmasq_srv="Dnsmasq";
service.dnsmasq_no_dns_rebind="Kein DNS Rebind";
service.dnsmasq_strict="DNS in fester Reihenfolge abfragen";
service.dnsmasq_add_mac="DNS Abfrage mit Anfrage MAC";
service.dnsmasq_opt="Zusätzliche Dnsmasq-Optionen";
service.dnsmasq_cachesize="Maximal gepufferte Einträge";
service.dnsmasq_forward_max="Maximale gleichzeitige Verbindungen";
service.dnsmasq_ipv6="Dnsmasq IPv6 Settings";
service.dnsmasq_ipv6_enabled="IPv6 Router Advertisement (RA)";
service.dnsmasq_ipv6_rastate="DHCP6 RA mode";
service.dnsmasq_ipv6_rastateful="Stateful DHCP6";
service.dnsmasq_ipv6_rastateless="Stateless DHCP6";

//pptp.webservices
service.pptp_h2="PPTP Server / Client";
service.pptp_legend="PPTP";
service.pptp_srv="Enable Server";
service.pptp_client="Client-IP(s)";
service.pptp_chap="CHAP-Secrets";

//syslog.webservices
service.syslog_legend="System-Log";
service.syslog_srv="Syslogd";
service.syslog_ip="Remote Server";
service.syslog_jffs2="Nachrichten auf jffs2 speichern";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//tor

service.tor_legend="The Onion Router Project";
service.tor_srv="Tor";
service.tor_address="DNS Name oder externe IP";
service.tor_nickname="Nickname / ID";
service.tor_relay="Weiterleitungs Modus";
service.tor_dir="Verzeichnisse spiegeln";
service.tor_bridge="Brücken Modus";
service.tor_transparent="Transparenter Proxy";
service.tor_bwrate="Bandbreiten Rate";
service.tor_bwburst="Bandbreitenüberschreitung";
service.tor_strict="Strikte Eigangs und Ausgangsnodes";
service.tor_exit="Land der Ausgangsnode";
service.tor_entry="Land der Eingangsnode";

//pptpd_client.webservices
service.pptpd_legend="PPTP-Client";
service.pptpd_lblcli="Client";
service.pptpd_ipdns="Server-IP oder DNS-Name";
service.pptpd_subnet="Entferntes Netz";
service.pptpd_subnetmask="Entfernte Netzmaske";
service.pptpd_encry="MPPE-Verschlüsselung";
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
service.pppoe_legend="PPPoE-Relay";
service.pppoe_srv="Relay";

//pppoe-server.webservices
service.pppoesrv_legend="PPPoE Server";
service.pppoesrv_srv="RP-PPPoE Server Daemon";
service.pppoesrv_interface="RP-PPPoE Server Schnittstelle";
service.pppoesrv_srvopt="RP-PPPoE Server Optionen";
service.pppoesrv_compr="Kompression";
service.pppoesrv_lcpei="LCP Echo Intervall";
service.pppoesrv_lcpef="LCP Echo Fehler";
service.pppoesrv_limit="Verbindungs Limit pro MAC";
service.pppoesrv_idlet="Idle Time";
service.pppoesrv_radauth="Authentifizierung";
service.pppoesrv_radip="RADIUS Server IP";
service.pppoesrv_radauthport="RADIUS Authentifizierungs Port";
service.pppoesrv_radaccport="RADIUS Accounting Port";
service.pppoesrv_radkey="RADIUS gemeinsamer Schlüssel";
service.pppoesrv_chaps="Lokale Benutzer Verwaltung (CHAP Secrets)";

//help container

hpppoesrv.right2="IP: 0.0.0.0; Ihnen wird eine IP aus dem Pool bereit gestellt";
hpppoesrv.right3="Sie müssen die korrekte Anzahl der beteiligten Geräte passend zum IP-Bereich angeben.";

//snmp.webservices
service.snmp_legend="SNMP";
service.snmp_srv="SNMP";
service.snmp_loc="Standort";
service.snmp_contact="Kontakt";
service.snmp_name="Name";
service.snmp_read="RO-Community";
service.snmp_write="RW-Community";

//openvpn.webservices
service.vpnd_legend="OpenVPN Daemon";
service.vpnd_srv="Starte OpenVPN Daemon";
service.vpnd_starttype="Start Typ";
service.vpnd_startWanup="Mit WAN-Aktivierung starten";
service.vpnd_startSystem="Beim Systemstart";
service.vpnd_gui="GUI(Server)";
service.vpnd_text="Text";
service.vpnd_crl="Zertifikatssperrliste";
service.vpnd_config="OpenVPN Konfiguration";
service.vpnd_dhpem="DH PEM";
service.vpnd_tlsauth="TLS Authentifizierungsschlüssel";
service.vpnd_cert="Öffentliches Server-Zertifikat";
service.vpnd_key="Privater Server-Schüssel";
service.vpnd_pkcs="PKCS12 Schlüssel";
service.vpnd_mode="Server Modus";
service.vpnd_netm="IPv4 Netzwerk und Maske";
service.vpnd_netmv6="IPv6 Netzwerk/Maske";
service.vpnd_startip="Pool Start-IP";
service.vpnd_endip="Pool End-IP";
service.vpnd_cl2cl="Klient zu Klient Verbindung erlaubt";
service.vpnd_switch="Konfiguriere als";
service.vpnd_dupcn="Erlaube doppelte Clients";
service.vpnd_allowcnwan="Erlaube Clients WAN Zugang (Internet)";
service.vpnd_allowcnlan="Erlaube Clients vollen LAN Zugriff";
service.vpnd_proxy="DHCP-Proxy Modus";
service.vpnd_clcon="Client Verbindungsskript";
service.vpnd_ccddef="CCD-Dir DEFAULT file";
service.vpnd_dhcpbl="Blockiere DHCP über den Tunnel";
service.vpnd_blockmulticast="Blockiere multicast über den Tunnel";
service.vpnd_dh="Benutze ECDH anstelle von DH.PEM";
service.vpnd_static="Statischer Schlüssel";
service.vpnd_export="Exportiere Client Konfiguration";
service.vpn_redirgate="Push Client Route";
service.vpn_defgateway="Standardgateway";
service.vpn_srvroute="Servers subnet";
service.vpn_legend="OpenVPN-Client";
service.vpn_mit="CVE-2019-14899 Entschärfung";
service.vpn_srv="Starte OpenVPN";
service.vpn_mtu="TUN-MTU-Einstellungen";
service.vpn_mss="TCP-MSS";
service.vpn_compress="Nutze Kompression";
service.vpn_tunnel="Tunnel-Protokoll";
service.vpn_cl2cl="Erlaube Verbindung zwischen Clients";
service.vpn_tunnel="Tunnelprotokoll";
service.vpn_tuntap="Tunnel-Typ";
service.vpn_srvcert="Öffentliches CA-Zertifikat";
service.vpn_clicert="Öffentliches Client-Zertifikat";
service.vpn_certtype="Überprüfe Server Zert.";
service.vpn_certtype="nsCertType Verifikation";
service.vpn_clikey="Privater Client-Schlüssel";
service.vpn_nat="NAT";
service.vpn_cipher="Verschlüsselung";
service.vpn_auth="Hash Algorithmus";
service.vpn_bridge="Überbrücke TAP und br0";
service.vpn_adv="Erweiterte Einstellungen";
service.vpn_tlscip="TLS Verschlüsselung";
service.vpn_route="Regelbasiertes Routen";
service.vpn_fw="Eingehende Firewall für TUN";
service.vpnd_lzoyes="Ja";
service.vpnd_lzono="Nein";
service.vpnd_lzoadaptive="Adaptiv";
service.vpn_tls_btn="TLS/Statische Schlüsselauswahl";
service.vpn_dc1="Erste Data Cipher";
service.vpn_dc1="Zweite Data Cipher";
service.vpn_dc1="Dritte Data Cipher";
service.vpn_killswitch="Killswitch";
service.vpn_splitdns="Split DNS";
service.vpn_ipnameport="Server-IP/Name : Port";
service.vpn_multiaddress="Weitere Server";
service.vpn_randomsrv="Wähle zufälligen Server";
service.vpn_wdog="Watchdog";


//help container
hstatus_vpn.right1="Regelbasiertes Routing:<br /><i>IP/Netze in der Form 0.0.0.0/0 erzwingen das Verwenden des Tunnels als Standardgateway durch den Client. Eine Zeile pro IP/Netz!<br /><i>IP Adresse/Netzmaske:</i><br />Muss bei der Verwendung eines DHCP-Proxy angegeben werden, wenn das lokale TAP Gerät nicht überbrückt ist.</i>";
hstatus_vpn.right2="Zusätzliche Einstellungen:<br /><i>Folgender Eintrag erlaubt Routen an den Client weiterzugeben: 'push \"route IP mask gateway\"'. Analog kann mit 'push \"dhcp-option DNS (or WINS) IP\"' DNS/WINS Server weitergegeben werden.</i><br />Ordner für Klientenverbindungen:<br /><i>Falls USB oder JFFS2 als /jffs eingehängt sind, werden Skripte aus /jffs/etc/openvpn/ccd/ ausgeführt.</i>";
hstatus_vpn.right3="Allgemein:<br /><i>Drei Methoden zur Authentifizierung sind verfügbar: pkcs12 (+dh auf Server), statisch und Standardzertifikate. MSS darf nur auf einer Seite der Verbindung aktiviert werden, Fragmentierung jedoch auf beiden.</i>";

//vnc.repeater
service.vncrepeater_legend="VNC";
service.vncrepeater="VNC Repeater";

//radiooff.webservices
service.radiooff_legend="SES / AOSS / EZ-SETUP / WPS Druckknopf";
service.radiooff_legend_aoss="AOSS Druckknopf Funktion";
service.radiooff_srv="Wireless Abschalten";
service.radiooff_srv_aoss="AOSS";
service.radiooff_srv_disabled="Unbenutzt";
service.radiooff_bootoff="Schalte Wireless beim Starten ab.";

//ses.webservices ====> might replace the above radiooff_button
service.ses_legend="SES / AOSS / EZ-SETUP / WPS Button";
service.ses_srv="Tastenaktion";
service.ses_toggleradio="Drahtlosnetzwerk umschalten";
service.ses_script="Benutzerdefiniertes Skript";

//hwmon.webservices
service.hwmon_legend="Hardware Überwachung";
service.hwmon_critemp="Kritische Temperatur";
service.hwmon_hystemp="Hysterese Temperatur";
service.hwmon_fanon="&nbsp;Lüfter An";
service.hwmon_fanoff="&nbsp;Lüfter Aus";

//rstat.webservices
service.rstats_legend="Bandbreiten Überwachung";
service.rstats_srv="rstats Daemon";
service.rstats_path="Speichere Daten nach";
service.rstats_time="Speicher Zyklus";
service.rstats_usrdir="Benutzer Verzeichnis";

//nstx.webservices
service.nstx_legend="IP über DNS tunneln";
service.nstx_srv="NSTX Daemon";
service.nstx_ipenable="Nur für diese IP verfügbar machen";
service.nstx_log="Debug-Ausgabe aktivieren";

//ttraff.webservices
service.ttraff_legend="WAN Datenzähler";
service.ttraff_daemon="ttraff Daemon";

//notifier.webservices
service.warn_legend="Verbindungswarner";
service.warn="Verbindungswarner";
service.warn_limit="Verbindungslimit";
service.warn_server="E-Mail SMTP Server";
service.warn_from="E-Mail Adresse des Absenders";
service.warn_fromfull="Voller Name des Absenders";
service.warn_to="E-Mail Adresse des Empfängers";
service.warn_domain="Domain Name des Empfängers";
service.warn_user="SMTP Auth Benutzername";
service.warn_pass="SMTP Auth Kennwort";
service.samba3_srv="Samba";
service.samba3_srvstr="Server String";
service.samba3_pub="Öffentliche Freigabe";
service.samba3_config="Eigene Konfiguration";
service.samba3_workgrp="Arbeitsgruppe";
service.samba3_interfaces="Schnittstellen";
service.samba3_connlimit="Verbindungslimit";
service.samba3_dirpath="Verzeichnis";
service.samba3_usr1="Benutzer1";
service.samba3_pass1=" Kennwort1";
service.samba3_usr2="Benutzer2";
service.samba3_pass2=" Kennwort2";
service.samba3_pubacl="Nur Lesen";
service.samba3_advanced="Erweitert";
service.samba3_custom="Eigene Konfiguration benutzen";
service.samba3_shares="Freigaben";
service.samba3_share_path="Verzeichnis";
service.samba3_share_subdir="Unterverzeichnis";
service.samba3_share_label="Name";
service.samba3_share_public="Öffentlich";
service.samba3_share_access="Zugriff";
service.samba3_users="Benutzer";
service.samba3_username="Benutzer";
service.samba3_password="Kennwort";
service.samba3_user_shares="Zugang zu Freigaben";
service.samba3_min_proto="Minimale Protokoll Version";
service.samba3_max_proto="Maximale Protokoll Version";
service.sambe3_encrypt="Verschlüsselung";
service.samba3_guest="Zugriffsebene";
service.samba3_guest_baduser="Jedermann";
service.samba3_guest_never="Eingeschränkt";
service.dlna_type_video="Filme";
service.dlna_type_audio="Musik";
service.dlna_type_images="Bilder";
service.nfs="NFS";
service.nfs_srv="NFS Server";
service.nfs_allowed="Erlaubte Netze";
service.rsync="rsync";
service.rsync_srv="rsync Dienst";
service.rsync_allowed="Erlaubte Rechner";

// Zabbix
service.zabbix_legend="Zabbix";
service.zabbix_cl="Client";
service.zabbix_serverip="Zabbix Server IP";
service.zabbix_usrpara="User Parameter";

// mdns
service.mdns_legend="mDNS / Avahi";
service.mdns_domain="Domainname [lokal]";
service.mdns_reflector="Reflektor";
service.mdns_interfaces="Schnittstellen";

service.transmission_dir="Transmission Konfigurationsverzeichnis";
service.transmission_download="Transmission Download Verzeichnis";
service.transmission_style="Webinterface Stil";
service.transmission_rpc="Webinterface Port";
service.transmission_down="Maximale Download Geschwindigkeit";
service.transmission_up="Maximale Upload Geschwindigkeit";
service.transmission_script="Starte Script wenn Download beendet";
service.transmission_whitelist="Erlaubte IPs";

service.plex_legend="Medien Server";
service.plex_srv="Plex Medien Server";

// ** eop-tunnel.asp **//

eoip.titl="Tunnel";
eoip.tunnel="Tunnel";
eoip.legend="Ethernet Über IP tunneln";
eoip.srv="EoIP Tunnel";
eoip.genkey="Erzeuge Schlüssel";
eoip.mcast_group="Multicast Gruppe";
eoip.dport="Ziel Port";
eoip.sportrange="Quellportbereich";
eoip.l2miss="LL Adressen Fehlerbenachrichtigung";
eoip.l3miss="IP Adressen Fehlerbenachrichtigung";
eoip.udpcsum="UDP Prüfsummenberechnung für IPv4";
eoip.udp6zerocsumtx="Überspringe IPv6 UDP Prüfsummenberechnung";
eoip.udp6zerocsumrx="Erlaube IPv6 UDP Pakete ohne Prüfsumme";
eoip.wireguard_oet_natout ="NAT über Tunnel";
eoip.wireguard_showadvanced="Erweiterte Einstellungen";
eoip.wireguard_rtupscript="Route up Skript";
eoip.wireguard_rtdownscript="Route down Skript";
eoip.wireguard_fwmark="Firewall Markierung";
eoip.wireguard_route_allowedip="Route erlaubte IPs über Tunnel";
eoip.wireguard_oet_spbr_ip="Quelle für PBR";
eoip.wireguard_oet_dpbr_ip="Ziel für PBR";
eoip.wireguard_oet_dns="DNS Server via Tunnel";
eoip.wireguard_localport="Lokaler Port";
eoip.wireguard_ka="Persistent Keep-Alive";
eoip.wireguard_endpoint="Endpunkt";
eoip.wireguard_peer="Endpunkt Adresse";
eoip.wireguard_peerport="Entfernter Port";
eoip.wireguard_peerkey="Entfernter öffentlicher Schlüssel";
eoip.wireguard_cllegend="Client Config File";
eoip.wireguard_peerip="Peer Tunnel IP/Netzmaske";
eoip.wireguard_peerdns="Peer Tunnel DNS";
eoip.wireguard_clend="Peer Tunnel Endpoint";
eoip.wireguard_clka="Peer Keepalive";
eoip.wireguard_localkey="Lokaler öffentlicher Schlüssel";
eoip.wireguard_localprivatekey="Lokaler privater Schlüssel";
eoip.wireguard_firewallin="Eingehende Firewall";
eoip.wireguard_killswitch="Notschalter";
eoip.wireguard_dnspbr="Teile DNS Tunnel";
eoip.wireguard_dns4="IPv4 DNS Server";
eoip.wireguard_dns6="IPv6 DNS Server";
eoip.wireguard_spbr="Quell Routing (PBR)";
eoip.wireguard_spbr0="Route alle Quellen via VPN";
eoip.wireguard_spbr1="Route definierte Quellen via VPN";
eoip.wireguard_spbr2="Route definierte Quellen via WAN";
eoip.wireguard_dpbr="Ziel Routing";
eoip.wireguard_dpbr0="Route all destinations via default route";
eoip.wireguard_dpbr1="Route definierte Ziele via VPN";
eoip.wireguard_dpbr2="Route definierte Ziele via WAN";
eoip.wireguard_genpsk="Erzeuge Pre-shared Key";
eoip.wireguard_usepsk="Nutze Pre-shared Key";
eoip.wireguard_oet_status="WireGuard Status (F5: Aktualisierung)";
eoip.wireguard_addpeer="Partner Hinzufügen";
eoip.wireguard_cleanqr="Entferne Peer Konfig";
eoip.wireguard_makeclient="Erstelle Peer Konfig";
eoip.wireguard_delpeer="Partner Entfernen";
eoip.wireguard_allowedips="Erlaubte IPs";
eoip.wireguard_failgrp="Failover Mitglied";
eoip.wireguard_failstate="Fehlerstatus (auto)";
eoip.wireguard_standby="In Bereitschaft";
eoip.wireguard_running="Läuft";
eoip.wireguard_failed="Gescheitert";
eoip.wireguard_lanac="Erlaube Clients vollen LAN Zugriff";
eoip.remoteIP="Entfernte IP Adresse";
eoip.localIP="Lokale IP Adresse";
eoip.proto="Protokoll Typ";
eoip.tunnelID="Tunnel Nummer";
eoip.comp="Kompression";
eoip.passtos="ToS Durchleitung";
eoip.frag="Fragment";
eoip.mssfix="MSS Fix";
eoip.shaper="shaper";
eoip.bridging="Brücken";
eoip.add="Tunnel Hinzufügen";
eoip.del="Tunnel Entfernen";
eoip.importt="Konfiguration Importieren";
eoip.filepicker="Lade und Verwende Konfiguration";

// ** Sipath.asp + cgi **//

sipath.titl="SiPath-Übersicht";
sipath.phone_titl="Telefonbuch";
sipath.status_titl="Status";


// ** Status_Lan.asp **//

status_lan.titl="LAN-Status";
status_lan.h2="Lokales Netzwerk";
status_lan.legend="LAN-Status";
status_lan.h22="Dynamic Host Configuration Protokoll";
status_lan.legend2="DHCP-Status";
status_lan.legend3="DHCP-Clients";
status_lan.legend4="Active Clients";
status_lan.legend5="Verbundene PPTP Clients";
status_lan.legend6="Verbundene PPPOE Clients";
status_lan.concount="Verbindungen";
status_lan.conratio="Verhältnis";

//help container

hstatus_lan.right2="Dies ist die Router-MAC-Adresse wie sie in Ihrem lokalen Ethernet-Netzwerk sichtbar ist.";
hstatus_lan.right4="Dies zeigt die Router-IP-Adresse wie sie in Ihrem lokalen Ethernet-Netzwerk sichtbar ist.";
hstatus_lan.right6="Wenn der Router eine Netzmaske nutzt, wird diese hier angezeigt.";
hstatus_lan.right8="Wenn Sie den Router als DHCP-Server nutzen, wird dies hier angezeigt.";
hstatus_lan.right10="Indem Sie auf irgendeine MAC-Adresse klicken, erhalten Sie die OUI ID, welche auf den Hersteller des Geräts rückschließen lässt.";


// ** Status_Bandwidth.asp **//

status_band.titl="Bandbreiten Monitor";
status_band.h2="Bandbreiten Monitor";
status_band.chg_unit="Wechseln zu ";
status_band.chg_scale="Automatische Anpassung";
status_band.chg_error="Die Daten der Schnittstelle sind nicht ermittelbar";
status_band.chg_collect_initial="Initiale Datenerfassung...";
status_band.strin="Eingang";
status_band.strout="Ausgang";
status_band.follow="folgen";
status_band.up="oben";

//help container

hstatus_band.svg="Adobe SVG Erweiterung ist erforderlich, um die Bandbreitengraphen anzuzeigen.";
hstatus_band.right1="Klicken Sie den Text an um die Einheiten zu wechseln (B/s oder bit/s).";
hstatus_band.right2="Klicken Sie den Text an um die Art der Skalierung zu wechseln.";

// ** Status_Router.asp **//

status_router.titl="Router-Status";
status_router.h2="Router-Information";
status_router.legend="System";
status_router.sys_model="Router-Modell";
status_router.sys_firmver="Firmware-Version";
status_router.sys_time="Aktuelle Zeit";
status_router.sys_up="Uptime";
status_router.sys_load="Lastdurchschnitt";
status_router.sys_kernel="Kernel Version";
status_router.legend2="CPU";
status_router.cpu="CPU-Modell";
status_router.clock="Taktfrequenz";
status_router.legend3="Speicher";
status_router.mem_tot="Insgesamt verfügbar";
status_router.mem_free="Frei";
status_router.mem_used="Verwendet";
status_router.mem_buf="Puffer";
status_router.mem_cached="Cached";
status_router.mem_active="Aktiv";
status_router.mem_inactive="Inaktiv";

status_router.legend4="Netzwerk";
status_router.net_ipcontrkmax="IP-Filter: maximale Ports";
status_router.net_conntrack="Aktive IP-Verbindungen";
status_router.notavail="Nicht verfügbar";
status_router.legend6="Verwendung";
status_router.inpvolt="Spannung am Platineneingang";
status_router.cputemp="Sensoren";

//help container

hstatus_router.right2="Dies ist der Name des Routers, wie er unter <i>Setup</i> gesetzt wurde.";
hstatus_router.right4="Dies ist die Router MAC-Adresse, wie sie von Ihrem Anbieter gesehen wird.";
hstatus_router.right6="Dies ist die aktuelle Firmware des Routers.";
hstatus_router.right8="Dies ist die Zeit wie sie unter <i>Setup</i> gesetzt wurde.";
hstatus_router.right10="Dies ist die seit dem Start des Routers gemessene Zeit.";
hstatus_router.right12="Diese wird anhand von 3 Werten dargestellt, welche die Systemauslastung während der letzten ein, fünf, und fünfzehn Minuten angeben.";

// ** Status_Internet.asp **//

status_inet.titl="WAN Status";
status_inet.h11="WAN";
status_inet.conft="Konfigurationstyp";
status_inet.www_loginstatus="Login-Status";
status_inet.wanuptime="Verbindungszeit";
status_inet.leasetime="Verbleibende Lease Zeit";
status_inet.traff="Traffic";
status_inet.traff_tot="Gesamter Traffic";
status_inet.traff_mon="Traffic pro Monat";
status_inet.traffin="Eingehend";
status_inet.traffout="Ausgehend";
status_inet.previous="Vorhergehender Monat";
status_inet.next="Nächster Monat";
status_inet.dataadmin="Daten Administration";
status_inet.delete_confirm="WARNUNG! Dies löscht alle ihre Traffic Daten. Fortfahren??";
status_inet.speed="Internet Geschwindigkeit";
status_inet.speedtest="Speedtest";
status_inet.down="Download Geschwindigkeit";
status_inet.up="Upload Geschwindigkeit";
status_inet.server="Server";
status_inet.sponsor="Sponsor";
status_inet.town="Stadt";
status_inet.country="Land";
status_inet.latency="Latenz";
status_inet.test="Starte Test";


//help container

hstatus_inet.right2="Dies zeigt die Informationen an, welche von Ihrem Anbieter zum Verbinden ins Internet benötigt werden. Diese Informationen wurden auf dem Setup-Tab eingegeben. Sie können hier auch Ihre Verbindung <em>aufbauen</em> oder <em>trennen</em> indem Sie den entsprechenden Button anklicken.";
hstatus_inet.right4="Dies zeigt den Internet Verkehr ihres Routers seit dem letzten Neustart an.";
hstatus_inet.right6="Hier wird die Datenmenge der Routers ins und vom Internet pro Monat angezeigt. Bewegen Sie die Maus über die Grafik um die tägliche Nutzung zu sehen. Die Daten sind im nvram gespeichert.";

// ** Status_Conntrack.asp **//

status_conn.titl="Tabelle aktiver IP Verbindungen";
status_conn.h2="Aktive IP Verbindungen";

// ** Status_Wireless.asp **//

status_wireless.titl="WLAN-Status";
status_wireless.h2="WLAN";
status_wireless.legend="WLAN-Status";
status_wireless.net="Netzwerk";
status_wireless.pptp="PPTP-Status";
status_wireless.legend2="Paket-Info";
status_wireless.rx="Empfangen (RX)";
status_wireless.tx="Übermittelt (TX)";
status_wireless.h22="WLAN-Knoten";
status_wireless.legend3="Clients";
status_wireless.signal_qual="Signalqualität";
status_wireless.wds="WDS-Knotenpunkte";
status_wireless.busy="Belegte Zeit";
status_wireless.active="Aktive Zeit";
status_wireless.quality="Kanal Qualität";
status_wireless.rx_time="Empfangszeit";
status_wireless.tx_time="Sendezeit";
status_wireless.assoc_count="Verbundene Stationen";


// ** GPS info **//

status_gpsi.legend="GPS Information";
status_gpsi.status="Status";
status_gpsi.lon="Längengrad";
status_gpsi.lat="Breitengrad";
status_gpsi.alt="Höhe";
status_gpsi.sat="Sichtbare Satelliten";
status_gpsi.ant_conn="Antenne verbunden";
status_gpsi.ant_disc="Antenna getrennt";
status_gpsi.na="Nicht Verfügbar";

//help container

hstatus_wireless.right2="Dies ist die Router-MAC-Adresse, wie sie in Ihrem lokalen WLAN Netzwerk sichtbar ist.";
hstatus_wireless.right4="Wie auf dem WLAN-Tab angegeben, zeigt Ihnen dies den Modus Ihrer WLAN-Schnittstelle an (Mixed, Nur-G, or ausgeschaltet).";


// ** Status_OpenVPN.asp **//

status_openvpn.titl="OpenVPN Status";


// ** Triggering.asp **//

trforward.titl="Port-Triggering";
trforward.h2="Port-Triggering";
trforward.legend="Weiterleitungen";
trforward.trrange="Ausgelöster Port-Bereich";
trforward.fwdrange="Weitergeleiteter Port-Bereich";
trforward.app="Anwendung";

//help container

htrforward.right2="Geben Sie den Namen der Anwendung für diesen Trigger ein.";
htrforward.right4="Gibt für jede Anwendung den ausgelösten Port-Bereich an. Konsultieren Sie hierzu auch die Dokumentation Ihrer Anwendung.";
htrforward.right6="Gibt für jede Anwendung den weitergeleiteten Port-Bereich an. Konsultieren Sie hierzu auch die Dokumentation Ihrer Anwendung.";
htrforward.right8="Geben Sie den Start-Port des ausgelösten und weitergeleiteten Bereichs an.";
htrforward.right10="Geben Sie den End-Port des ausgelösten und weitergeleiteten Bereichs an.";


// ** Upgrade.asp **//

upgrad.titl="Firmware-Aktualisierung";
upgrad.h2="Firmware-Management";
upgrad.legend="Firmware-Aktualisierung";
upgrad.info1="Nach dem Flashen zurücksetzen auf";
upgrad.resetOff="Kein Zurücksetzen";
upgrad.resetOn="Standardeinstellungen";
upgrad.file="Firmware-Aktualisierungs-Datei";
upgrad.warning="W A R N U N G";
upgrad.mess1="Das Aktualisieren der Firmware kann einige Minuten dauern.<br />Bitte schalten Sie das Gerät nicht ab und drücken Sie auch nicht den Resetknopf!";

//help container

hupgrad.right2="Klicken Sie auf den <em>Durchsuchen...</em>-Button, um eine Firmware auszuwählen die auf das Gerät hochgeladen werden soll.<br /><br /> Klicken Sie den <em>Aktualisieren</em>-Button um den Aktualisierungsprozess zu starten. Der Aktualisierungsprozess darf nicht unterbrochen werden.";


// ** UPnP.asp **//

upnp.titl="UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Weiterleitungen";
upnp.legend2="UPnP-Konfiguration";
upnp.serv="UPnP-Dienst";
upnp.clear="Löscht die Port-Weiterleitungen beim Start";
upnp.url="Sende Präsentations-URL";
upnp.msg1="Klicken Sie hier, um die Lease zu löschen";
upnp.msg2="Alle Einträge löschen?";

//help container

hupnp.right2="Klicken Sie auf den Papierkorb, um einen einzelnen Eintrag zu löschen.";
hupnp.right4="Erlaubt Anwendungen, eine automatische Port-Weiterleitung auf dem Router durchzuführen.";


// ** VPN.asp **//

vpn.titl="VPN-Durchleitung";
vpn.h2="Virtuelles Privates Netzwerk (VPN)";
vpn.legend="VPN-Durchleitung";
vpn.ipsec="IPSec-Durchleitung";
vpn.pptp="PPTP-Durchleitung";
vpn.l2tp="L2TP-Durchleitung";

//help container

hvpn.right1="Sie können hier die IPsec-, PPTP- und/oder L2TP-Durchleitung aktivieren um Ihren Netzwerkgeräten die Kommunikation via VPN zu ermöglichen.";



// ** Vlan.asp **//

vlan.titl="Virtuelles LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN Konfiguration";
vlan.bridge="Der Brücke zuweisen";
vlan.switch_leds="Switch LED's abschaltem";
vlan.tagged="Tagged";
vlan.negociate="Automatisches Aushandeln";
vlan.aggregation="Schnittstellenbündelung<br />auf Port 3 & 4";
vlan.trunk="Trunk";
vlan.linkstatus="Verbindungsstatus";
vlan.flow="Flußsteuerung";

// ** WEP.asp **//

wep.defkey="Standard-Übermittelungsschlüssel";
wep.passphrase="Kennwort";


// ** WOL.asp **//

wol.titl="WOL";
wol.h2="Wake-On-LAN";
wol.legend="Verfügbare Geräte";
wol.legend2="WOL-Adressen";
wol.legend3="Ausgabe";
wol.legend4="Manuelles WOL";
wol.enable="WOL Einschalten?";
wol.mac="MAC-Adresse(n)";
wol.broadcast="Netz-Broadcast";
wol.udp="UDP-Port";
wol.msg1="Klicken, um einen WOL-Host zu entfernen";
wol.h22="Automatic Wake-On-LAN";
wol.legend5="Wake-On-LAN Daemon";
wol.srv="WOL Daemon";
wol.pass="SecureOn Kennwort";

//help container

hwol.right2="Diese Seite erlaubt Ihnen, definierte Geräte in Ihrem Netzwerk aufzuwecken (sofern Sie mit Ihrem Router lokal verbunden sind).";
hwol.right4="Die MAC-Adressen werden in folgendem Format angegeben xx:xx:xx:xx:xx:xx (Bsp. 01:23:45:67:89:AB)";
hwol.right6="IP-Adresse ist typischerweise die Broadcast-Adresse für Ihr lokales Netzwerk. Es kann aber auch eine Remote-Adresse sein wenn der Host nicht in Ihrem lokalen Netzwerk verbunden ist.";


// ** WanMAC.asp **//

wanmac.titl="MAC-Adresse klonen";
wanmac.h2="MAC-Adresse klonen";
wanmac.legend="MAC-Klonen";
wanmac.wan="Klone WAN MAC";
wanmac.wlan="Klone WLAN MAC";

//help container

hwanmac.right2="Manche Anbieter setzen eine Registrierung Ihrer MAC-Adresse vorraus. Wenn Sie Ihre MAC-Adresse nicht neu registrieren möchten, können Sie hier Ihre MAC-Adresse klonen, so wie sie bei Ihrem Anbieter registriert ist.";

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
roaming.usteer_options="Band-Steering Optionen";
roaming.roaming="Roaming Options";
roaming.s80211v="802.11v Unterstützung";
roaming.s80211k="802.11k Unterstützung";
roaming.wnm_sleep_mode="WNM-Sleep Mode";
roaming.wnm_sleep_mode_no_keys="WNM-Sleep Mode GTK/IGTK abhilfe";
roaming.bss_transition="BSS Transition Management";
roaming.rrm_neighbor_report="Neighbor report via radio";
roaming.rrm_beacon_report="Beacon report via radio";
roaming.proxy_arp="Proxy ARP";
roaming.time_advertisement="Time advertisement";
roaming.usteer="Band-Steuerung";
roaming.mbo="Multiband Operation (MBO)";
roaming.mbo_cell_data_conn_pref="Cellular Connection Preference";
roaming.reassociation_deadline="Reassociation Deadline";
roaming.ft_protocol="FT protocol";
roaming.ft_over_ds="FT over DS";
roaming.ft_over_air="FT over the Air";
roaming.ft="802.11r Unterstützung (FT)";
roaming.budget_5ghz="5 GHz Budget in DBm";
roaming.prefer_5ghz="Bevorzuge 5 GHz";
roaming.prefer_he="Bevorzuge 802.11ax";


// ** WL_WPATable.asp / WPA.asp / Radius.asp **//

wpa.titl="WLAN-Sicherheit";
wpa.h2="WLAN-Sicherheit";
wpa.secmode="Sicherheitsmodus";
wpa.legend="WLAN-Verschlüsselung";
wpa.auth_mode="Netzwerk-Authentifizierung";
wpa.mfp="802.11w Management Frame Schutz";
wpa.radius="RADIUS";
wpa.gtk_rekey="WPA Group Rekey Intervall";
wpa.rekey="Schlüssel Erneuerungsintervall";
wpa.radius_ipaddr="RADIUS-Server-IP";
wpa.radius_port="RADIUS-Server-Port";
wpa.radius_key="RADIUS-Key";
wpa.algorithms="WPA-Algorithmus";
wpa.shared_key="Gemeinsamer WPA Schlüssel";
wpa.sae_key="SAE Passwort";
wpa.eapol_key_retries="EAPOL Erneuerung abschalten";
wpa.ccmp="CCMP-128 (AES)";
wpa.ccmp_256="CCMP-256";
wpa.tkip="TKIP";
wpa.gcmp_128="GCMP";
wpa.gcmp_256="GCMP-256";
wpa.psk="WPA Personal";
wpa.psk2="WPA2 Personal";
wpa.psk2_sha256="WPA2 Personal mit SHA256";
wpa.psk3="WPA3 Personal / SAE";
wpa.wpa="WPA Enterprise";
wpa.wpa2="WPA2 Enterprise";
wpa.wpa2_sha256="WPA2 Enterprise mit SHA256";
wpa.wpa2_sha384="WPA2 Enterprise mit SHA384";
wpa.wpa3="WPA3 Enterprise";
wpa.wpa3_128="WPA3 Enterprise Suite-B 128-Bit";
wpa.wpa3_192="WPA3 Enterprise CNSA Suite-B 192-Bit";
wpa.wep_8021x="802.1x / WEP";
wpa.peap="EAP-PEAP";
wpa.leap="EAP-LEAP";
wpa.tls="EAP-TLS";
wpa.ttls="EAP-TTLS";
wpa.owe="OWE Opportunistische WLAN Verschlüsselung";
wpa.owe_ifname="OWE Übergangs Interface";

aoss.titl="AOSS Sicherheit";
aoss.aoss="AOSS";
aoss.service="AOSS Dienst";
aoss.enable="AOSS Einschalten";
aoss.start="Starte AOSS Aushandlung";
aoss.securitymodes="Sicherheits Einstellungen";
aoss.wpaaes="WPA AES";
aoss.wpatkip="WPA TKIP";
aoss.wep="WEP64 / 128";
aoss.client_name="Client Name";
aoss.security="Sicherheit";
aoss.connectivity="Verbindungen";
aoss.clients="AOSS Clients";
aoss.notice="Mitteilung";
aoss.ap_mode_notice="Mitteilung: AOSS kann nur genutzt werden, wenn sich die primäre WLAN Schnittstelle im AP oder WDS-AP Modus befindet.";
aoss.wep_notice="Der WEP Modus ist unsicher! Aus dem Grunde empfehlen wir Ihnen, diesen nicht zu benutzen.";
aoss.wep_info="(wird für einige Spielkonsolen die AOSS unterstützen, in Verbindung mit AOSS benötigt)";
aoss.wps="WPS Einstellungen";
aoss.wps_ap_pin="WPS Gateway PIN (Aufkleber)";
aoss.wpspin="WPS Client PIN";
aoss.wpsactivate="Aktiviere PIN";
aoss.wpsregister="Registriere PIN";
aoss.wpsgenerate="Erzeuge PIN";
aoss.pinnotvalid="Ungültige PIN. Prüfsumme nicht korrekt!";
aoss.wpsenable="WPS Knopf";
aoss.wpsstatus="WPS Status";
aoss.externalregistrar="PIN Methode";
aoss.release="Freigeben";
aoss.configure="Konfigurieren";


olupgrade.avail_updates="Verfügbare Aktualisierungen";
olupgrade.version="Version";
olupgrade.release="Release";
olupgrade.readme="Liesmich";
olupgrade.choose="Wähle";
olupgrade.retrieve_error="Fehler beim empfangen von Aktualisierungsinformationen";


nintendo.titl="Nintendo";

nintendo.spotpass.titl="Nintendo SpotPass";
nintendo.spotpass.enable="Aktiviere Nintendo SpotPass";
nintendo.spotpass.servers="Erlaube Server";


sec80211x.xsuptype="XSupplicant Typ";
sec80211x.keyxchng="EAP Key-Management";
sec80211x.servercertif="Public Server Zertifikat";
sec80211x.clientcertif="Client Zertifikat";
sec80211x.phase2="Phase 2";
sec80211x.anon="Anonyme Identität";
sec80211x.options="Erweiterte Netzwerk Optionen";
sec80211x.leap="EAP-LEAP Einstellungen";
sec80211x.peap="EAP-PEAP Einstellungen";
sec80211x.tls="EAP-TLS Einstellungen";
sec80211x.ttls="EAP-TTLS Einstellungen";


//help container

hwpa.right2="Hier können Sie zwischen deaktiviert, WEP, WPA Personal Key, WPA Enterprise, oder RADIUS wählen. Alle Geräte in Ihrem Netzwerk müssen den selben Modus verwenden.";



// ** WL_FilterTable.asp **//

wl_filter.titl="MAC-Adressen-Filterliste";
wl_filter.h2="MAC-Adressen-Filterliste";
wl_filter.h3="Geben Sie die MAC-Adresse in folgendem Format an:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";


// ** WL_ActiveTable.asp **//

wl_active.titl="MAC-Liste der aktiven WLAN-Clients";
wl_active.h2="WLAN-Client-MAC-Liste";
wl_active.h3="Aktiviere MAC-Filter";
wl_active.active="Aktive PCs";
wl_active.inactive="Inaktive PCs";



// ** Wireless_WDS.asp **//

wds.titl="WDS";
wds.h2="Wireless Distribution System";
wds.legend="WDS-Einstellungen";
wds.label="Lazy WDS";
wds.label2="WDS-Netz";
wds.wl_mac="WLAN-MAC";
wds.lazy_default="Standardwert: Deaktiviert";
wds.nat1="WLAN->WDS";
wds.nat2="WDS->WLAN";
wds.subnet="Netz";
wds.legend2="Erweiterte Optionen";


// ** Wireless_radauth.asp **//

radius.titl="RADIUS";
radius.h2="Fern-Authentifizierungs Einwahl Nutzer Dienst";
radius.legend="RADIUS";
radius.retry="Primäre Server Versuche";
radius.label="RADIUS Client MAC";
radius.label2="MAC-Format";
radius.label3="RADIUS Auth Server IP";
radius.label4="RADIUS Auth Server Port";
radius.label7="RADIUS Auth geteilter Schlüssel";

radius.label23="RADIUS Auth Backup Server IP";
radius.label24="RADIUS Auth Backup Server Port";
radius.label27="Gemeinsames Kennwort";

radius.label5="Erlaubte unauthentifizierte Benutzer";
radius.label6="Kennwort-Format";
radius.label8="Umgehen, wenn Server nicht verfügbar";
radius.label13="RADIUS Acct Server IP";
radius.label14="RADIUS Acct Server Port";
radius.label17="Gemeinsames Kennwort";
radius.label18="RADIUS Accounting";
radius.local_ip="Erzwinge Client IP";

// ** Wireless_MAC.asp **//

wl_mac.titl="MAC-Filter";
wl_mac.h2="WLAN-MAC-Filter";
wl_mac.legend="MAC-Filter";
wl_mac.label="Nutze Filter";
wl_mac.label2="Liste enthält...";
wl_mac.deny="Blacklisted Netzwerkclients";
wl_mac.allow="Whitelisted Netzwerkclients";

// ** WiMAX**//

wl_wimax.titl="WiMAX";
wl_wimax.h2="Worldwide Interoperability for Microwave Access";
wl_wimax.downstream="Downstream Frequenz";
wl_wimax.upstream="Upstream Frequenz";
wl_wimax.width="Kanal Breite";
wl_wimax.duplex="Duplex Modus";
wl_wimax.mode="Betriebsmodus";
wl_wimax.mac="Client MAC Adress";

// ** Gpio.asp **//

gpio.titl="GPIO Eingänge / Ausgänge";
gpio.h2="GPIO Eingänge / Ausgänge";
gpio.outputlegend="GPIO Ausgänge";
gpio.inputlegend="GPIO Eingänge";


// ** FreeRadius.asp **//

freeradius.titl="FreeRADIUS";
freeradius.h2="FreeRADIUS";
freeradius.certificate="Server Zertifikate";
freeradius.cert="Erstelle Zertifikat";
freeradius.certdown="Herunterladen";
freeradius.clientcert="Client Zertifikate";
freeradius.settings="Einstellungen";
freeradius.users="Benutzer";
freeradius.clients="Klienten";
freeradius.username="Benutzername";
freeradius.password="Kennwort";
freeradius.downstream="Downstream";
freeradius.upstream="Upstream";
freeradius.sharedkey="Geteilter Schlüssel";


freeradius.countrycode="Ländercode";
freeradius.state="Staat oder Provinz";
freeradius.locality="Lokalität";
freeradius.organisation="Organisation / Firma";
freeradius.email="E-Mail Adresse";
freeradius.common="Zertifikats Name";
freeradius.expiration="Verfall in Tagen";
freeradius.passphrase="Kennwort";
freeradius.generate="Erzeuge Zertifikate";
freeradius.cert_status="Zertifikat Status";
freeradius.port="RADIUS Port";
freeradius.certtbl="Zertifikat";
freeradius.gencertime="Zertifikatserstellung zu %d%% fertig";
freeradius.gencerdone="Zertifikat fertiggestellt!";

//help container

hfreeradius.right2="Bitte JFFS2 Einschalten, bevor sie RADIUS nutzen.";

// ** Wireless_Advanced.asp **//

wl_adv.titl="Erweiterte WLAN-Einstellungen";
wl_adv.h2="Erweiterte WLAN-Einstellungen";
wl_adv.legend="Erweiterte Einstellungen";
wl_adv.legend2="Einstellungen WLAN-Multimedia";
wl_adv.label="Authentifizierungs-Typ";
wl_adv.label2="Basis-Rate";
wl_adv.label3="Übertragungsrate (Fix)";
wl_adv.label4="CTS Protection Mode";
wl_adv.label5="Frame Burst";
wl_adv.label6="Beacon Intervall";
wl_adv.label7="DTIM Intervall";
wl_adv.label8="Fragmentation Threshold";
wl_adv.label9="RTS Threshold";
wl_adv.label10="Maximal verbundene Clients";
wl_adv.label11="AP Isolation";
wl_adv.label12="TX Antenne";
wl_adv.label13="RX Antenne";
wl_adv.label14="Preamble";
wl_adv.reference="Noise-Referenz";
wl_adv.label16="Afterburner";
wl_adv.label17="WLAN-GUI-Zugriff";
wl_adv.label18="WMM-Unterstützung";
wl_adv.label19="No-Acknowledgement";
wl_adv.label20="Shortslot Umgehung";
wl_adv.label21="Übertragungsrate (Max)";
wl_adv.label23="Übertragungsrate (Min)";
wl_adv.label22="Koexistenzmodus mit Bluetooth";
wl_adv.label24="Antennenausrichtung";
wl_adv.label25="Antennenausgang";
wl_adv.table1="EDCA AP Parameter (AP nach Client)";

wl_adv.txchainmask="TX Antennen Pfade";
wl_adv.rxchainmask="RX Antennen Pfade";



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
wl_adv.table2="EDCA STA Parameter (Client nach AP)";
wl_adv.lng="Lang"; 					//************* don't use .long! *************
wl_adv.shrt="Kurz"; 				//************* don't use .short! **************

wl_adv.droplowsignal="Clients mit niedrigem Signal verwerfen";
wl_adv.connect="Minimum Signal für Authentifizierung";
wl_adv.stay="Minimum Signal für Verbindung";
wl_adv.poll_time="Intervall für Analyse";
wl_adv.strikes="Anzahl erlaubter niedriger Signale";

wl_adv.bgscan="Hintergrund Scan";
wl_adv.bgscan_mode="Modus";
wl_adv.bgscan_simple="Einfach";
wl_adv.bgscan_learn="Lernen";
wl_adv.bgscan_short_int="Kurzer Intervall";
wl_adv.bgscan_threshold="Signal Schwellenwert";
wl_adv.bgscan_long_int="Langer Intervall";

//help container

hwl_adv.right2="Sie können Auto oder Shared Key wählen. Die Shared Key Authentifizierung ist wesentlich sicherer, allerdings muss dazu jedes Gerät in Ihrem Netzwerk diesen Betriebsmodus unterstützen.";

// ** Wireless_Basic.asp **//

wl_basic.titl="WLAN";
wl_basic.h2="WLAN";
wl_basic.cardtype="Karten Typ";
wl_basic.chipset="Chipsatz";
wl_basic.legend="Basis-Einstellungen";
wl_basic.label="Modus";
wl_basic.label2="Netzwerk-Modus";
wl_basic.ssid="Netzwerk-Name (SSID)";
wl_basic.label4="Kanal";
wl_basic.vht80p80chan="Wireless Kanal 2 (VHT80+80)";
wl_basic.label5="SSID-Broadcast";
wl_basic.label6="Empfindlichkeitsbereich (ACK Timing)";
wl_basic.label7="802.11n Übertragungs Modus";
wl_basic.igmpsnooping="Optimiere Multicast Daten";
wl_basic.scanlist="Abfrageliste";
wl_basic.duallink="Dual Link";
wl_basic.parent="Eltern IP";
wl_basic.masquerade="Verschleierung / NAT";
wl_basic.ap="AP";
wl_basic.client="Client";
wl_basic.repeater="Repeater";
wl_basic.repeaterbridge="Repeater Brücke";
wl_basic.clientBridge="Client-Brücke";
wl_basic.clientRelayd="Client-Brücke (Gerouted)";
wl_basic.adhoc="Adhoc";
wl_basic.wdssta="Client (WDS)";
wl_basic.wdsap="AP (WDS)";
wl_basic.apup="AP (WDS + APuP)";
wl_basic.mixed="Gemischt";
wl_basic.bft="Direktes Beamforming";
wl_basic.bfr="Indirektes Beamforming";
wl_basic.overlap="Erlaube Kanalüberlappungen";
wl_basic.twt_required="Target Wake Time Benötigt";
wl_basic.greenfield="Greenfield";
wl_basic.preamble="Short Preamble";
wl_basic.clientRelaydDefaultGwMode="Default GW Mode";
wl_basic.b="Nur-B";
wl_basic.a="Nur-A";
wl_basic.na="NA-Gemischt";
wl_basic.ac="Nur-AC";
wl_basic.acn="AC/N-Gemischt";
wl_basic.ax="Nur-AX";
wl_basic.axg="Nur-AX (2.4 Ghz)";
wl_basic.xacn="AX / AC / N-Gemischt";
wl_basic.ng="NG-Gemischt";
wl_basic.n5="Nur-N (5 GHz)";
wl_basic.n2="Nur-N (2.4 GHz)";
wl_basic.g="Nur-G";
wl_basic.bg="BG-Gemischt";
wl_basic.n="Nur-N";
wl_basic.dwds="Dynamischer WDS Modus";
wl_basic.rts="RTS Schwellwert";
wl_basic.rtsvalue="Schwellwert";
wl_basic.protmode="Schutzmodus";
wl_basic.legend2="WLAN-Zeiteinschränkung";
wl_basic.radio="WLAN";
wl_basic.radiotimer="WLAN Einteilung";
wl_basic.radio_on="ist an";
wl_basic.radio_off="ist aus";
wl_basic.h2_v24="WLAN Schnittstelle";
wl_basic.h2_vi="Virtuelle Schnittstellen";
wl_basic.regdom="Regulationsbereich";
wl_basic.regmode="Regulierungs Modus";
wl_basic.tpcdb="TPC Leistungs Reduzierung";
wl_basic.TXpower="Sendeleistung";
wl_basic.TXpowerFcc="Maximal erlaubte Sendeleistung (FCC)";
wl_basic.power_override="Umgehe Chipsatz Einschränkungen";
wl_basic.AntGain="Antennenleistung";
wl_basic.diversity="Diversität";
wl_basic.primary="Primär";
wl_basic.secondary="Sekundär";
wl_basic.vertical="Vertikal";
wl_basic.horizontal="Horizontal";
wl_basic.adaptive="Adaptiv";
wl_basic.internal="Intern";
wl_basic.external="Extern";
wl_basic.ghz24="2.4 GHz Ausgang";
wl_basic.ghz5="5 GHz Ausgang";
wl_basic.network="Netzwerk Konfiguration";
wl_basic.unbridged="Getrennt";
wl_basic.bridged="Überbrückt";
wl_basic.turbo="Turbo Modus";
wl_basic.extrange="Extended Range (XR)";
wl_basic.supergcomp="Super G Kompression";
wl_basic.supergff="Super G Fast Framing";
wl_basic.extchannel="Erweiterter Kanalmodus";
wl_basic.outband="Outdoor Band";
wl_basic.channel_width="Kanalbreite";
wl_basic.channel_wide="Erweiterer Kanal";
wl_basic.regulatory="SuperKanal";
wl_basic.chanshift="Kanal Verschiebung";
wl_basic.specialmode="2.3 GHz Modus";
wl_basic.wifi_bonding="Wifi Bündelung";
wl_basic.sifstime="OFDM SIFS Time";
wl_basic.preambletime="OFDM Preamble Time";
wl_basic.multicast="Multicast Weiterleitung";
wl_basic.intmit="Rausch Unempfindlichkeit";
wl_basic.noise_immunity="Rausch Unempfindlichkeits Level";
wl_basic.ofdm_weak_det="OFDM Problemerkennung";
wl_basic.radar="Radar Erkennung";
wl_basic.mtikie="MTik Kompatibilität";
wl_basic.csma="Trägererkennung";
wl_basic.if_label="Kennung (optional)";
wl_basic.if_info="Information (optional)";
wl_basic.advanced_options="Erweiterte Optionen";
wl_basic.rate_control="Datenratensteuerung";
wl_basic.ap83_vap_note="Die Verwendung von mehr als drei virtuellen Schnittstellen kann zu Geschwindigkeitseinbusen bei einigen Geräten führen (nur auf den zusätzlichen Schnittstellen).";
wl_basic.fc="Frame Kompression";
wl_basic.isolation="Netz Isolierung";
wl_basic.tor_anon="TOR Anonymisierung";
wl_basic.country_settings="Ländereinstellungen";
wl_basic.ldpc="LDPC (Für ESP8266 Abschalten)";
wl_basic.uapsd="U-APSD (Automatisches Stromsparen)";
wl_basic.disassoc_low_ack="Disassoc Low Ack";
wl_basic.smps="Spatial Multiplex Power Save";
wl_basic.add="Virtuellen AP Hinzufügen";
wl_basic.airtime_policy="Airtime Stategie";
wl_basic.airtime_dynamic="Dynamischer Modus";
wl_basic.airtime_limit="Limit Modus";
wl_basic.airtime_weight="Airtime Wichtung";
wl_basic.airtime_dolimit="Airtime limitieren";	// oder Limit
wl_basic.mesh_settings="802.11s Mesh Einstellungen";
wl_basic.mesh_fwding="Layer 2 Weiterleitung";
wl_basic.mesh_retry_timeout="Retry Timeout";
wl_basic.mesh_confirm_timeout="Confirm Timeout";
wl_basic.mesh_holding_timeout="Holding Timeout";
wl_basic.mesh_max_peer_links="Maximale Peer Anzahl";
wl_basic.mesh_max_retries="Maximum Retries";
wl_basic.mesh_ttl="TTL";
wl_basic.mesh_element_ttl="Element TTL";
wl_basic.mesh_auto_open_plinks="Auto Open Plinks";
wl_basic.mesh_hwmp_max_preq_retries="HWMP Maximum Preq Retries";
wl_basic.mesh_path_refresh_time="Path Refresh Time";
wl_basic.mesh_min_discovery_timeout="Minimum Discovery Timeout";
wl_basic.mesh_hwmp_active_path_timeout="HWMP Active Path Timeout";
wl_basic.mesh_hwmp_preq_min_interval="HWMP Preq Minimum Intervall";
wl_basic.mesh_hwmp_net_diameter_traversal_time="HWMP Net Diameter Traversal Time";
wl_basic.mesh_hwmp_rootmode="HWMP Rootmode";
wl_basic.mesh_hwmp_rann_interval="HWMP Rann Intervall";
wl_basic.mesh_gate_announcements="Gate Announcements";
wl_basic.mesh_sync_offset_max_neighor="Sync Offset Max Neighbor";
wl_basic.mesh_rssi_threshold="Rssi Threshold";
wl_basic.mesh_hwmp_active_path_to_root_timeout="HWMP Active Path To Root Timeout";
wl_basic.mesh_hwmp_root_interval="HWMP Root Intervall";
wl_basic.mesh_hwmp_confirmation_interval="HWMP Confirmation Intervall";
wl_basic.mesh_power_mode="Leistungseinstellungen";
wl_basic.mesh_awake_window="Awake Window";
wl_basic.mesh_plink_timeout="Plink Timeout";
wl_basic.mesh_no_root="No Root";
wl_basic.mesh_preq_no_prep="Preq No Prep";
wl_basic.mesh_preq_with_prep="Preq With Prep";
wl_basic.mesh_rann="Rann";
wl_basic.mesh_active="Aktiv";
wl_basic.mesh_deep="Tiefschlaf";
wl_basic.mesh_light="Leichter Schlaf";
wl_basic.fw_type="Firmware Typ";

//help container

hwl_basic.right2="Wenn Sie G-Clients komplett ausschließen wollen, sollten Sie <em>Nur-B</em> wählen. Wenn Sie den WLAN Zugriff abschalten wollen, wählen Sie <em>abschalten</em>.";
hwl_basic.right3="Empfindlichkeitsbereich: ";
hwl_basic.right4="Setzt das ACK Timing. 0 Schaltet das ACK timing auf Broadcom Geräten komplett ab. Auf Atheros Geräten wechselt eine 0 in den Auto ACK Timing Modus";
hwl_basic.right6="Klick auf irgendeine Stunde um das WLAN Signal an- oder abzuschalten (<em>Grün</em> zeigt den erlaubten Zugriff an, <em>Rot</em> zeigt den blockierten Zugriff an)";

superchan.legend="SuperChannel Activation";
superchan.h2feat="Eigenschaft";
superchan.featxt="<p>SuperChannel erlaubt auf <b>kompatiblen Geräten</b> die Nutzung von speziellen Frequenzen.<br />Der mögliche Frequenzbereich liegt zwischen 2192Mhz und 2732Mhz für 802.11g und 4915Mhz bis 6100Mhz für 802.11a.<br /><b>Diese Eigenschaft ist noch nicht freigeschaltet.</b></p>";
superchan.h2disc="Haftungsausschluss";
superchan.lgltxt="<p>Beachten Sie das es in vielen Ländern nicht gestattet ist diese Frequenzen zu nutzen. DD-WRT / NewMedia-NET GmbH übernimmt keinerlei direkte oder indirekte Verantwortung fünr die Nutzung dieser Eigenschaft.</p>";
superchan.lsyskey="Systemschlüssel";
superchan.lactkey="Aktivierungsschlüssel";

// ** Fail_s.asp / Fail_u_s.asp / Fail.asp **//

fail.mess1="Die angegebenen Werte sind nicht korrekt! Bitte versuchen Sie es erneut.";
fail.mess2="Aktualisierung fehlgeschlagen.";



// ** Success*.asp & Reboot.asp  **//


success.saved="Einstellungen gespeichert.";
success.restore="Einstellungen wiederhergestellt.<br />Das Gerät wird nun neu gestartet. Bitte warten Sie einen Moment...";
success.upgrade="Aktualisierung erfolgreich.<br />Das Gerät wird nun neu gestartet. Bitte warten Sie einen Moment...";
success.success_noreboot="Die Einstellungen wurden erfolgreich gespeichert.";
success.success_reboot=success.success_noreboot + "<br />Das Gerät wird nun neu gestartet. Bitte warten Sie einen Moment...";

success.alert_reset="Alle Konfigurationseinträge wurden auf ihre Standardwerte gesetzt.<br /><br />";
success.alert1="Bitte prüfen Sie nachfolgende Dinge bevor Sie sich neu verbinden:";
success.alert2="Sie haben die IP-Adresse Ihres Routers geändert. Bitte erneuern Sie Ihre Client-Adresse in Ihrem Netzwerk.";
success.alert3="Wenn Sie via WLAN verbunden sind, treten Sie dem Netzwerk erneut bei und klicken Sie auf <em>fortfahren</em>.";

// ** Logout.asp  **//

logout.message="Sie wurden erfolgreich abgemeldet.<br />Danke das Sie DD-WRT benutzen!";

// ** Setup Assistant **//

sas.title="Einstellungens-Assistenten";
sas.internet_connection="Internet Verbindung";
sas.network_settings="Netzwerk Einstellungen";
sas.wireless_settings="WLAN Einstellungen";
sas.other_settings="Andere Einstellungen";
sas.hwan="Internet (WAN) Einstellungen";


hsas.wan="Das WAN Interface verbindet ihren Router mit dem Internet oder anderen Netzwerken. Wenn Ihr Netzwerk bereits mit dem Internet verbunden ist und Sie nur einen Accesspoint benötigen, setzen Sie den WAN Modus auf \"Ausgeschaltet\".";
hsas.h_routerip="Router IP";
hsas.routerip="Dies ist die IP Adresse die dem Router in ihrem internen Netzwerk zugewiesen ist. Wenn Sie diese Adresse ändern, müssen Sie auch diese Verwenden wenn Sie auf die Weboberfläche des Routers zugreifen wollen ";
hsas.h_dhcp="DHCP";
hsas.dhcp="Computer und andere Netzwerk Geräte können mittels DHCP deren IP-Adressen automatisch beziehen ohne das eine manuelle Konfiguration notwendig ist. Für den Fall, dass bereits ein DHCP Server in ihrem Netzwerk läuft, schalten Sie bitte diese Option ab um Konflikte zu vermeiden.";
hsas.h_wireless_physical="WLAN Schnittstelle";
hsas.wireless_physical="Die Schnittstellen Einstellungen erlaubt es Ihnen das Verhalten der WLAN Schnittstelle einzustellen. Sie können den Hauptbetriebsmodus einstellen (Access Point, Client or Repeater), den WLAN Netzwerk Namen ändern und erweiterte Einstellungen vornehmen, wie z.B. die Kanalbreite anpassen. In dem Fall das Sie die Kanalbreite von 20 Mhz auf einen anderen Wert ändern, achten Sie bitte darauf, dass Ihre Geräte diese Kanalbreite auch tatsächlich unterstützten.";
hsas.h_wireless_security="WLAN Sicherheit";
hsas.wireless_security="Damit es ihnen leichter fällt ihren Client zu konfigurieren, können Sie hier ihr Netzwerkkennwort ändern. Die Verschlüsselung abzuschalten oder auf WEP zu setzen wird nicht empfohlen, da dies ihre Sicherheit erheblich beeinträchtigt.";
hsas.h_routername="Router Name";
hsas.routername="Dieser Name wird anderen Geräten in ihrem Netzwerk mitgeteilt und erlaubt die einfachere Identifizierung.";
hsas.networking="Netzwerk Hilfe Text";
hsas.wireless="WLAN Hilfe Text";
hsas.other="Andere Einstellungen Hilfe Text";

// ** AOSS **//
haoss.basic="Das \"AirStation One-Touch Secure System\" (AOSS) erlaubt Ihnen ihre AOSS fähigen Clients mit Ihrem Accesspoint zu verbinden ohne das Sie eine manuelle Konfiguration durchführen müssen";
haoss.securitymodes="Die AOSS Sicherheits Modi definieren welche Sicherheitseinstellungen von Geräten akzeptiert werden. Wenn ein Gerät nur Sicherheitseinstellungen unterstützt die nicht erlaubt sind, kann es sich nicht verbinden.";
haoss.wps="WPS aktiviert die Unterstützung für Wifi Protected Setup unter Zuhilfenahme des WPS / AOSS Knopfes an ihrem Router oder durch Nutzung der PIN die auf ihrem Gerät oder in ihrer Client Anwendung abgebildet ist.";




ias.title="Einstellungen";
ias.card_info="Einstellungskarte";
ias.edit_note="Ein Klick auf eine beliebige Einstellung erlaubt es, diese zu ändern.";
ias.assistant="Starte Assistent";
ias.assistant_iptv="IPTV Setup";
ias.print_setup_card="Einstellungen Drucken";
ias.print_guest_card="Karte drucken";
ias.apply_changes="Übernehmen";
ias.wlnetwork="Drahtloses Netzwerk";
ias.wlinfo_2_4_GHz="(2.4 Ghz) - Kompatibel mit 802.11n/g/b";
ias.wlinfo_5_GHz="(5 Ghz) - Kompatibel mit 802.11n/a";
ias.hl_setup_card="Einstellungskarte";
ias.hl_client_access="Für Gerätezugang";
ias.hl_for_conf="Für Konfiguration";
ias.hl_guest_card="Gästekarte";

// ** Speedchecker.asp **//
speedchecker.titl="Speedchecker";
speedchecker.legend="Speedchecker";
speedchecker.server="Speedchecker Service";
speedchecker.regtitle="Bitte teilen sie Informationen mit uns:";
speedchecker.savemessage="Bitte Speichern oder Wenden Sie an";
// ** feature display boxes **//
speedchecker.nfeath4title="WLAN Speedchecker";
speedchecker.nfeath4txt="Wird deine WLAN Geschwindigkeit langsamer? Testen Sie ihre Geschwindigkeit mit nur einem Klick.";
speedchecker.nfeatbutton="&nbsp;&nbsp;Klicken Sie hier um ihre Geschwindigkeit zu testen&nbsp;&nbsp;";

dnscrypt.nfeath4title="DNSCrypt";
dnscrypt.nfeath4txt="<a href=\"https:\/\/www.dnscrypt.org\">DNSCrypt</a> Authentifiziert die Kommunikation mit ihrem DNS Client und einem DNS Resolver. Dies Verhindert DNS Spoofing.";
dnscrypt.nfeatbutton="&nbsp;&nbsp;Gehen Sie auf den Services Reiter&nbsp;&nbsp;";

featureshead.h2title="Erfahren Sie mehr über unsere neuen Features!&nbsp;&nbsp;";
featureshead.hidebtn="Diese Feld verstecken";


// ************		OLD PAGES 		*******************************//
// *********************** DHCPTable.asp *****************************//

dhcp.titl="DHCP aktive IP-Tabelle";
dhcp.h2="DHCP aktive IP-Tabelle";
dhcp.server="DHCP Server-IP-Adresse :";
dhcp.tclient="Client-Hostname";


donate.mb="Sie können auch durch den Moneybookers-Account mb@dd-wrt.com spenden";

reg.not_reg="Das System ist nicht aktiviert. Bitte Kontaktieren Sie ihren lokalen Händler um einen gültigen Lizenzschlüssel zu erhalten.";
reg.sys_key="Systemschlüssel";
reg.act_key="Lizenzschlüssel";
reg.reg_aok="Aktivierung vollständig. Das System wird nun neu gestartet.";
