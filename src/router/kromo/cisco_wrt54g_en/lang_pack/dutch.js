//////////////////////////////////////////////////////////////////////////////////////////////
//		Dutch reference translation file - DD-WRT V23 SP1 by Fabian 22/6/2006				//
//////////////////////////////////////////////////////////////////////////////////////////////

// ******************************************* COMMON SHARE LABEL *******************************************//

lang_charset.set="iso-8859-1";

share.firmware="Firmware";
share.time="Tijd";
share.interipaddr="Internet IP Adres";
share.more="Meer...";
share.help="Help";
share.enable="Activeer";
share.enabled="Geactiveerd";
share.disable="Deactiveer";
share.disabled="Gedeactiveerd";
share.usrname="Gebruikersnaam";
share.passwd="Wachtwoord";
share.hostname="Host Naam";
share.domainname="Domein Naam";
share.statu="Status";
share.start="Start";
share.end="Stop";
share.proto="Protocol";
share.ip="IP Adres";
share.mac="MAC Adres";
share.none="Geen";
share.none2="nee";
share.both="Beide";
share.del="Verwijder";
share.remove="Verwijder";
share.descr="Beschrijving";
share.from="Van";
share.to="Naar";
share.about="Over";
share.everyday="Elke dag";
share.sun="Zondag";
share.sun_s="Zon";
share.sun_s1="Z";
share.mon="Maandag";
share.mon_s="Ma";
share.mon_s1="M";
share.tue="Dinsdag";
share.tue_s="Di";
share.tue_s1="D";
share.wed="Woensdag";
share.wed_s="Woe";
share.wed_s1="W";
share.thu="Donderdag";
share.thu_s="Do";
share.thu_s1="D";
share.fri="Vrijdag";
share.fri_s="Vrij";
share.fri_s1="V";
share.sat="Zaterdag";
share.sat_s="Zat";
share.sat_s1="Z";
share.expires="Verloopt";
share.yes="Ja";
share.no="Nee";
share.allow="Toestaan";
share.deny="Verbieden";
share.range="Bereik";
share.use="Gebruik";
share.mins="Min.";
share.secs="Sec.";
share.routername="Router Naam";
share.manual="Handleiding";
share.port="Poort";
share.ssid="SSID";
share.channel="Kanaal";
share.rssi="Rssi";
share.signal="Signaal";
share.noise="Storing";
share.beacon="beacon";
share.openn="Open";
share.dtim="dtim";
share.rates="Ratio";
share.low="Laag";
share.medium="Medium";
share.high="Hoog";
share.option="Opties";
share.rule="Regel";
share.lan="LAN";
share.point2point="Point to Point";
share.nat="NAT";
share.subnet="Subnet Mask";
share.unmask="Unmask";
share.deflt="Standaard";  //don't use share.default !!!
share.all="Alles";
share.auto="Automatisch";
share.right="Rechts";
share.left="Links";
share.share_key="Gedeelde Sleutel";
share.inter="Interval (in seconden)";
share.srv="Service Naam";
share.port_range="Port Bereik";
share.priority="Prioriteit";
share.gateway="Gateway";
share.intrface="Interface";  //don't use share.interface, Mozilla problem!!!
share.router="Router";
share.static_lease="Statische Lease";
share.srvip="Server IP";
share.localdns="Lokale DNS";
share.minutes="minuten";
share.oui="OUI Zoeken";
share.sttic="Statisch";
share.connecting="Bezig met verbinden";
share.connect="Verbind";
share.connected="Verbonden";
share.disconnect="Onderbreken";
share.disconnected="Onderbroken";
share.info="Informatie";
share.state="Status";
share.mode="Modus";
share.encrypt="Encryptie";
share.key="Sleutel";
share.wireless="Draadloos";
share.dhcp="DHCP";
share.styl="Stijl";
share.err="fout";
share.errs="fouten";

sbutton.save="Bewaar instellingen";
sbutton.saving="Opgeslagen";
sbutton.cmd="Bezig met uitvoeren";
sbutton.cancel="Annuleer Veranderingen";
sbutton.refres="Vernieuw";
sbutton.clos="Sluit";
sbutton.del="Verwijder";
sbutton.continu="Verder";
sbutton.add="Toevoegen";
sbutton.remove="Verwijderen";
sbutton.modify="Aanpassen";
sbutton.deleted="Verwijderd";
sbutton.delall="Verwijder alles";
sbutton.autorefresh="Auto-Refresh is Actief";
sbutton.backup="Backup";
sbutton.restore="Herstel";
sbutton.cptotext="Pas aan";
sbutton.runcmd="Voer Commando Uit";
sbutton.startup="Bewaar Startup";
sbutton.firewall="Bewaar Firewall";
sbutton.wol="Opwekken";
sbutton.add_wol="Voeg Host Toe";
sbutton.manual_wol="Manueel Opwekken";
sbutton.summary="Samenvatting";
sbutton.filterIP="Bewerk Lijst PCs";
sbutton.filterMac="Bewerk MAC Filter Lijst";
sbutton.filterSer="Service Toevoegen/Aanpassen";
sbutton.reboot="Herstart Router";
sbutton.help="   Help  ";
sbutton.wl_client_mac="Draadloze Client MAC Lijst";
sbutton.update_filter="Vernieuw Filter Lijst";
sbutton.join="Join";
sbutton.log_in="Ingaande Log";
sbutton.log_out="Uitgaande Log";
sbutton.apply="Toepassen";
sbutton.edit_srv="Service Toevoegen/Aanpassen";
sbutton.routingtab="Toon Route Tabel";
sbutton.wanmac="Verkrijg Huidig PC MAC Adres";
sbutton.dhcprel="DHCP Release";
sbutton.dhcpren="DHCP Vernieuw";
sbutton.survey="Site Survey";
sbutton.upgrading="Upgrading";
sbutton.upgrade="Upgrade";
sbutton.preview="Voorbeeld";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//

errmsg.err0="U moet een gebruikersnaam invullen.";
errmsg.err1="U  moet een Router Naam invullen.";
errmsg.err2="Buiten het bereik, gelieve het start IP adres aan te passen of het gebruikers nr.";
errmsg.err3="U moet een dag selecteren.."
errmsg.err4="De eindtijd moet groter zijn dan de begintijd.";
errmsg.err5="De lengte van het MAC Adres is niet correct.";
errmsg.err6="U moet een wachtwoord ingeven.";
errmsg.err7="U moet een hostnaam ingeven.";
errmsg.err8="U moet een IP Adres of Domein naam ingeven.";
errmsg.err9="Foutief DMZ IP Adres.";
errmsg.err10="Het bevestigd wachtwoord komt niet overeen met het ingegeven wachtwoord. Gelieve opnieuw te proberen.";
errmsg.err11="Spaties zijn niet toegelaten in het Wachtwoord.";
errmsg.err12="U moet een commando ingeven.";
errmsg.err13="Upgrade is mislukt.";
errmsg.err45="Dit is niet beschikbaar in HTTPS! Gelieve te verbinden in HTTP modus.";
errmsg.err46="Dit is niet beschikbaar in HTTPS";


//common.js error messages
errmsg.err14=" waarde valt buiten het toegelaten bereik [";
errmsg.err15="Het WAN MAC Adres valt buiten de toegelaten waarde [00 - ff].";
errmsg.err16="Het tweede karakter van het MAC adres moet even zijn : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="Het ingegeven MAC Adres is niet correct.";
errmsg.err18="De lengte van het MAC Adres is niet correct.";
errmsg.err19="Het MAC Adres kan niet het broadcast adres zijn."
errmsg.err20="Geef het MAC Adres in (xx:xx:xx:xx:xx:xx) formaat in.";
errmsg.err21="Ongeldig MAC Adres formaat.";
errmsg.err22="Het WAN MAC Adres is ongeldig.";
errmsg.err23="Ongeldige hex waarde ";
errmsg.err24=" gevonden in het MAC adres ";
errmsg.err25="De sleutel waarde is niet correct.";
errmsg.err26="De sleutel lengte is niet correct.";
errmsg.err27="Ongeldige subnet mast.";
errmsg.err28=" heeft ongeldige karakters, verplicht [ 0 - 9 ].";
errmsg.err29=" heeft ongeldige ascii code.";
errmsg.err30=" heeft ongeldige hexadecimale waarden.";
errmsg.err31=" waarde is ongeldig.";
errmsg.err32="IP adres en gateway behoren niet tot hetzelfde subnet mask.";
errmsg.err33="IP adres en gateway kunnen niet gelijk zijn.";
errmsg.err34=" is niet toegelaten om spaties te bevatten.";

//Wol.asp error messages
errmsg.err35="U moet een MAC adres ingeven voor verwerking.";
errmsg.err36="U moet een network broadcast adres ingeven voor verwerking.";
errmsg.err37="U moet een UDP poort ingeven voor verwerking.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Gelieve een gedeelde Sleutel in te geven!";
errmsg.err39="Ongeldige Sleutel, lengte moet tussen 8 en 63 ASCII karakters of 64 hexadecimal karakters liggen"
errmsg.err40="U moet een sleutel ingeven voor Sleutel ";
errmsg.err41="Ongeldige lengte in sleutel ";
errmsg.err43="Interval vernieuwing sleutel";

//config.asp error messages
errmsg.err42="Gelieve een configuratiebestand voor herstelling te selecteren.";

//WL_ActiveTable.asp error messages
errmsg.err44="Het totaal aantal controles overschrijd 128.";

//Site_Survey.asp error messages
errmsg.err47="ongeldige SSID.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS is niet compatibel met de huidige configuratie van de router. Gelieve volgende punten te controleren :\n * Wireless Mode moet ingesteld zijn op AP \n * WPA2 wordt niet ondersteund door WDS \n * Wireless Network B-Only modus wordt niet ondersteund door WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius is enkel beschikbaar in AP modus.";

//Wireless_Basic.asp error messages
errmsg.err50="U moet een SSID ingeven.";

// Management.asp error messages
errmsg.err51="De router gebruikt momenteel het standaard wachtwoord. \
			Ter beveiliging, moet u het wachtwoord veranderen alvorens u controle op afstand kan inschakelen. \
			Klik op OK om uw wachtwoord te veranderen. Klik op Annuleren om controle op afstand ongewijzigd te laten.";
errmsg.err52="Wachtwoord confirmatie komt niet overeen.";

// Port_Services.asp error messages
errmsg.err53="Klik op Toepassen, na voltooiing van alle acties.";
errmsg.err54="U moet een Service Naam ingeven.";
errmsg.err55="De Service Naam bestaat al.";

// QoS.asp error messages
errmsg.err56="Poort waarde is buiten het toegestane bereik [0 - 65535]";

// Routing.asp error messages
errmsg.err57="Gegeven verwijderen ?";

// Status_Lan.asp error messages
errmsg.err58="Klik om de lease te verwijderen";

//Status_Wireless.asp error messages
errmsg.err59="Niet Beschikbaar! Gelieve het draadloos netwerk te activeren.";

//Upgrade.asp error messages
errmsg.err60="Selecteer een bestand voor upgrade.";
errmsg.err61="Ongeldig image bestand.";

//Services.asp error messages
errmsg.err62=" is reeds gedefinieerd als statische lease.";

// *******************************************  COMMON MENU ENTRIES  *******************************************//

bmenu.setup="Setup";
bmenu.setupbasic="Basis Instellingen";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="MAC Adres Clone";
bmenu.setuprouting="Geavanceerde Routing";
bmenu.setupvlan="VLANs";

bmenu.wireless="Draadloos";
bmenu.wirelessBasic="Basis Instellingen";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Draadloze Beveiliging";
bmenu.wirelessMac="MAC Filter";
bmenu.wirelessAdvanced="Geavanceerde Instellingen";
bmenu.wirelessWds="WDS";

bmenu.security="Beveiliging";
bmenu.firwall="Firewall";
bmenu.vpn="VPN";

bmenu.accrestriction="Toegang Beperking";
bmenu.webaccess="Internet Toegang";


bmenu.applications="Applicaties &amp; Spelletjes";
bmenu.applicationsprforwarding="Poorten Forwarden";
bmenu.applicationspforwarding="Poorten Forwarden";
bmenu.applicationsptriggering="Poorten Triggering";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Overzicht";
bmenu.sipathphone="Telefoonboek";
bmenu.sipathstatus="Status";

bmenu.admin="Administratie";
bmenu.adminManagement="Beheer";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Diensten";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Log";
bmenu.adminDiag="Commando's";
bmenu.adminWol="WOL";
bmenu.adminFactory="Fabriek Instellingen";
bmenu.adminUpgrade="Firmware Upgrade";
bmenu.adminBackup="Backup";


bmenu.statu="Status";
bmenu.statuRouter="Router";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Sputnik Agent";
bmenu.statuWLAN="Draadloos";
bmenu.statuSysInfo="Systeem-Info";


// ******************************************* Alive.asp *******************************************//

alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Reboot Invoegen";
alive.sevr1="Reboot Invoegen";
alive.hour="Op een specifiek  moment";
alive.legend2="WDS/Connectie Watchdog";
alive.sevr2="Watchdog Activeren";
alive.IP="IP Adressen";
alive.legend3="Proxy/Connectie Watchdog";
alive.sevr3="Activeer Proxy Watchdog";
alive.IP2="Proxy IP Adres";
alive.port="Proxy Poort";

//help container

halive.right2="Selecteer indien u de router wilt rebooten. Cron moet geactiveerd zijn in de Beheer tab.";
halive.right4="Maximum drie verschillende IPs gescheiden door een <em>SPATIE</em> zijn toegelaten.<BR/>IP Formaat is xxx.xxx.xxx.xxx.";



// ******************************************* config.asp *******************************************//

config.titl=" - Backup & Herstellen";
config.h2="Backup Configuratie";
config.legend="Backup Instellingen";
config.mess1="Klik op de \"" + sbutton.backup + "\" knop om de huidige configuratie op te slaan op je computer.";
config.h22="Herstel Configuratie";
config.legend2="Herstel Instellingen";
config.mess2="Selecteer een te herstellen bestand";
config.mess3="O P G E L E T";
config.mess4="Gelieve enkel bestanden te uploaden die met deze firmware of het zelfde model van router zijn gemaakt.<br />Upload geen bestanden die niet gecreëerd werden door deze interface!";

//help container

hconfig.right2="U kan uw huidige configuratie opslaan in het gevalt dat u uw router terug naar zijn fabriek instellingen moet zetten.<br /><br />Klik op de <em>Backup</em> knop om uw huidige configuratie op ge slaan.";
hconfig.right4="Klik op de <em>Browse...</em> knop om naar een configuratie bestand te zoeken dat zich momenteel op je computer bevind.<br /><br />Klik op de <em>" + sbutton.restore + "</em> knop om alle huidige configuraties te wissen met de gene uit het configuratie bestand.";



// ******************************************* DDNS.asp *******************************************//

ddns.titl=" - Dynamische DNS"
ddns.h2="Dynamisch Domein Naam Systeem (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS Dienst";
ddns.emailaddr="E-mail Adres";
ddns.typ="Type";
ddns.dynamic="Dynamish";
// ddns.static="Static"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.custom="Aangepast";
ddns.wildcard="Wildcard";
ddns.statu="DDNS Status";


ddnsm.dyn_strange="Ongewoon server antwoord, hebt u de juiste server in gegeven?";
ddnsm.dyn_good="DDNS is succesvol geupdate";
ddnsm.dyn_noupdate="Er is geen update noodzakelijk op dit moment";
ddnsm.dyn_nohost="De hostnaam bestaat niet";
ddnsm.dyn_notfqdn="De hostnaam isniet correct";
ddnsm.dyn_yours="De hostnaam kan niet door jou veranderd worden";
ddnsm.dyn_abuse="De hostnaam werd geblokkeerd door misbruik";
ddnsm.dyn_nochg="Het IP Adres is niet veranderd sins de laatste update";
ddnsm.dyn_badauth="Authenticatie fout (gebruikersnaam of wachtwoord)";
ddnsm.dyn_badsys="Ongeldige systeem parameter";
ddnsm.dyn_badagent="Deze user agent werd geblokkeerd";
ddnsm.dyn_numhost="Te veel of te weinig hosts gevonden";
ddnsm.dyn_dnserr="DNS interne fout";
ddnsm.dyn_911="Onverwachte fout 911";
ddnsm.dyn_999="Onverwachte fout 999";
ddnsm.dyn_donator="De gevraagde functie is alleen beschikbaar voor betalende gebruikers.";
ddnsm.dyn_uncode="Onbekend antwoord";

ddnsm.tzo_good="Operatie Volledig";
ddnsm.tzo_noupdate="Geen update nodig";
ddnsm.tzo_notfqdn="Ongeldige Domein Naam";
ddnsm.tzo_notmail="Ongeldig Email";
ddnsm.tzo_notact="Ongeldige Actie";
ddnsm.tzo_notkey="Ongeldige Sleutel";
ddnsm.tzo_notip="Ongeldig IP adres";
ddnsm.tzo_dupfqdn="Domein Naam duplicatie";
ddnsm.tzo_fqdncre="Domein Naam werd reeds aangemaakt voor deze domein naam";
ddnsm.tzo_expired="De account is verlopen";
ddnsm.tzo_error="Een onverwachte server fout";

ddnsm.zone_701="Zone werd niet ingesteld op deze account";
ddnsm.zone_702="Update mislukt";
ddnsm.zone_703="Een of meerdere parameter <em>zones</em> of <em>host</em> zijn vereist";
ddnsm.zone_704="Zone moet een geldige internet naam zijn";
ddnsm.zone_705="Zone kan niet leeg zijn";
ddnsm.zone_707="Meerdere updates voor dezelfde host/ip, verander client instellingen";
ddnsm.zone_201="Er zijn geen updates vereist.";
ddnsm.zone_badauth="Authenticatie mislukt (gebruikersnaam of wachtwoord)";
ddnsm.zone_good="ZoneEdit werd succesvol geupdate";
ddnsm.zone_strange="Onverwacht server antwoord, bent u verbonden op de juiste server ?";

ddnsm.all_closed="DDNS server is niet actief";
ddnsm.all_resolving="Bezig met opzoeken domein naam";
ddnsm.all_errresolv="Opzoeken domein naam mislukt";
ddnsm.all_connecting="Bezig met verbinden naar server";
ddnsm.all_connectfail="Verbinding met server mislukt";
ddnsm.all_disabled="DDNS functie is niet actief";
ddnsm.all_noip="Geen actieve internet verbinding";

//help container

hddns.right2="DDNS biedt toegang tot uw netwerk via een hostnaam in plaats van een IP adres. \
	Deze service houdt uw domein informatie dynamisch up-to-date. \
	U moet hiervoor registreren op volgende site: DynDNS.org, TZO.com of ZoneEdit.com.";



// ******************************************* Diagnostics.asp *******************************************//

diag.titl=" - Diagnostics";
diag.h2="Diagnostics";
diag.legend="Commando Shell";
diag.cmd="Commando's";
diag.startup="Opstarten";
diag.firewall="Firewall";

//help container

hdiag.right2="U kan commando's uitvoeren via de webinterface. Vul uw commando in, in het tekst veld en klik <em>" + sbutton.runcmd + "</em>.";



// ******************************************* DMZ.asp *******************************************//

dmz.titl=" - DMZ";
dmz.h2="Gedemilitariseerde Zone (DMZ)";
dmz.legend="DMZ";
dmz.serv="Gebruik DMZ";
dmz.host="DMZ Host IP Adres";


//help container

hdmz.right2="Indien u deze optie activeerd zal u bepaalde hosts openstellen naar het internet toe. Alle poorten zullen bereikbaar zijn vanuit het internet.";



// ******************************************* Factory_Defaults.asp *******************************************//

factdef.titl=" - Fabricatie Instellingen";
factdef.h2="Fabricatie Instellingen";
factdef.legend="Reset router instellingen";
factdef.restore="Herstel Fabricatie Instellingen";

factdef.mess1="Opgelet! Indien u OK klikt, zal het aparaat naar fabricatie instellingen omgezet worden, alle voorgaande instellingen gaan verloren.";

//help container

hfactdef.right1="Dit zal al uw instellingen herstellen naar fabricatie instellingen. Al uw voorgaande instellingen worden gewist.";



// ******************************************* FilterIP%AC.asp *******************************************//

filterIP.titl=" - Lijst van PCs";
filterIP.h2="Lijst van PCs";
filterIP.h3="Voer het MAC Adres van de PCs in, in dit formaat: xx:xx:xx:xx:xx:xx";
filterIP.h32="Voer het IP Adres van de PCs in";
filterIP.h33="Voer de IP Range van de PCs in";
filterIP.ip_range="IP Range";



// ******************************************* Filter.asp *******************************************//

filter.titl=" - Toegang Beperking";
filter.h2="Toegang Beperking";
filter.legend="Toegangs Beleid";
filter.restore="Herstel Fabricatie Instellingen";
filter.pol="Beleid";
filter.polname="Beleid Naam";
filter.pcs="PCs";
filter.polallow="Internet toegang gedurende volgende dagen en uren.";
filter.legend2="Dagen";
filter.time="Uren";
filter.h24="24 Uur";
filter.legend3="Geblokkeerde Diensten";
filter.catchall="Zoek alle P2P Protocollen";
filter.legend4="Website Verbieden Per URL";
filter.legend5="Website Verbieden Per URL";

filter.mess1="Verwijder het Beleid?";
filter.mess2="U moet een dag selecteren.";
filter.mess3="De eindtijd moet groter dan de begintijd zijn.";

//help container

hfilter.right2="U kan tot 10 toegangs beperkingen aanmaken. Klik op <em>" + sbutton.del + "</em> om een beperking te verwijderen of <em>" + sbutton.summary + "</em> om een overzicht te krijgen.";
hfilter.right4="Activeer of Deactiveer een beperking.";
hfilter.right6="U mag het beleid een naam geven.";
hfilter.right8="Kies een dag van de week waarop u de beperking wilt doen gelden.";
hfilter.right10="Kies het uur van de dag waarop u de beperking wilt doen gelden";
hfilter.right12="U kan toegang tot een bepaalde dienst verbieden. Klik op<em>" + sbutton.filterSer + "</em> om deze instelling te veranderen.";
hfilter.right14="U kan toegang tot een bepaalde website verbieden door hun URL hier in te voeren.";
hfilter.right16="U kan toegang tot een bepaalde website verbieden aan de hand van sleutelwoorden, vermeld op hun site.";



// ******************************************* FilterSummary.asp *******************************************//

filterSum.titl=" - Overzicht Toegangs Beperking";
filterSum.h2="Overzicht Internet Beleid";
filterSum.polnum="Nee.";
filterSum.polday="Tijd en Dag";



// ******************************************* Firewall.asp *******************************************//

firewall.titl=" - Firewall";
firewall.h2="Security";
firewall.legend="Firewall Bescherming";
firewall.firewall="SPI Firewall";
firewall.legend2="Extra Filters";
firewall.proxy="Filter Proxy";
firewall.cookies="Filter Cookies";
firewall.applet="Filter Java Applets";
firewall.activex="Filter ActiveX";
firewall.legend3="Blokkeer WAN Aanvragen";
firewall.ping="Blokkeer Anonieme Internet Aanvragen";
firewall.muticast="Filter Multicast";
filter.nat="Filter Internet NAT Redirection";
filter.port113="Filter IDENT (Port 113)";

//help container

hfirewall.right2="Activeer of deactiveer de SPI firewall.";



// ******************************************* Forward.asp *******************************************//

prforward.titl=" - Poort Bereik Forwarden";
prforward.h2="Poort Bereik Forward";
prforward.legend="Forwards";
prforward.app="Applicatie";

//help container

hprforward.right2="Bepaalde applicaties vereisen bepaalde open poorten zodat ze correct kunnen functioneren. \
	Voorbeelden van deze applicaties houden ook servers en bepaalde online games in. \
	Wanneer een aanvraag voor een bepaalde poort toekomt van het internet, zal de route de data doorsturen naar de computer dat u wenst. \
	Door veiligheids overwegingen, zou u het gebruik van poort forwarding moeten beperken tot de poorten dat u effectief gebruikt, \
	u moet de <em>" + share.enable +"</em> checkbox uit vinken als u klaar bent met instellen.";



// ******************************************* ForwardSpec.asp *******************************************//

pforward.titl=" - Poort Forwarden";
pforward.h2="Poort Forward";
pforward.legend="Forwards";
pforward.app="Applicatie";
pforward.from="Van poort";
pforward.to="Naar poort";

//help container

hpforward.right2="Bepaalde applicaties vereisen bepaalde open poorten zodat ze correct kunnen functioneren. \
Voorbeelden van deze applicaties houden ook servers en bepaalde online games in. \
	Wanneer een aanvraag voor een bepaalde poort toekomt van het internet, zal de route de data doorsturen naar de computer dat u wenst. \
	Door veiligheids overwegingen, zou u het gebruik van poort forwarding moeten beperken tot de poorten dat u effectief gebruikt, \
	u moet de <em>Activeer</em> checkbox uitvinken nadat u klaar bent met instellen.";



// ******************************************* Hotspot.asp *******************************************//

hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot Portaal";
hotspot.legend="Chillispot";
hotspot.nowifibridge="Onderscheid WiFi van het LAN Netwerk";
hotspot.hotspot="Chillispot";
hotspot.pserver="Primary Radius Server IP/DNS";
hotspot.bserver="Backup Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="Redirect URL";
hotspot.dhcp="DHCP Interface";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM Geheim";
hotspot.uamdns="UAM Any DNS";
hotspot.allowuam="UAM Toegelaten";
hotspot.macauth="MACauth";
hotspot.option="Additionele Chillispot Opties";
hotspot.fon_chilli="Chillispot Lokaal Gebruikers Beheer";
hotspot.fon_user="Gebruikers Lijst";
hotspot.http_legend="HTTP Redirect";
hotspot.http_srv="HTTP Redirect";
hotspot.http_ip="HTTP Destinatie IP";
hotspot.http_port="HTTP Destinatie Poort";
hotspot.http_net="HTTP Bron Netwerk";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Gateway Naam";
hotspot.nocat_home="Home Page";
hotspot.nocat_allowweb="Toegelaten Web Hosts";
hotspot.nocat_docroot="Document Root";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="Verbied Poort";
hotspot.nocat_timeout="Login Timeout";
hotspot.nocat_verbose="Verbosity";
hotspot.nocat_route="Enkel Route";
hotspot.smtp_legend="SMTP Redirect";
hotspot.smtp_srv="SMTP Redirect";
hotspot.smtp_ip="SMTP Destinatie IP";
hotspot.smtp_net="SMTP Bron Netwerk";
hotspot.shat_legend="Zero IP Configuratie";
hotspot.shat_srv="Zero IP Configuratie";
hotspot.shat_srv2="Zero IP Configuratie geactiveerd";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik Agent";
hotspot.sputnik_id="Sputnik Server ID";
hotspot.sputnik_instant="Use Sputnik Instant Instelling";
hotspot.sputnik_express="Use SputnikNet Express";
hotspot.sputnik_about="over Sputnik";



// ******************************************* Info.htm *******************************************//

info.titl=" - Info";
info.h2="Systeem Informatie";
info.wlanmac="Wireless MAC";
info.srv="Diensten";
info.ap="Access Point";



// ******************************************* index_heartbeat.asp *******************************************//

idx_h.srv="Heart Beat Server";
idx_h.con_strgy="Connectie Strategie";
idx_h.max_idle="Verbinden op aanvraag: Max Idle Tijd";
idx_h.alive="Keep Alive: Herverbinding Periode";



// ******************************************* index_l2tp.asp *******************************************//

idx_l.srv="L2TP Server";



// ******************************************* index_pppoe.asp *******************************************//

idx_pppoe.use_rp="Gebruik RP PPPoE";



// ******************************************* index_pptp.asp *******************************************//

idx_pptp.srv="Gebruik DHCP";
idx_pptp.wan_ip="Internet IP Adres";
idx_pptp.gateway="Gateway (PPTP Server)";
idx_pptp.encrypt="PPTP Encyptie";



// ******************************************* index_static.asp *******************************************//

idx_static.dns="Statische DNS";



// ******************************************* index.asp *******************************************//

idx.titl=" - Instellen";
idx.h2="Internet Instellen";
idx.h22="Draadloos Instellen";
idx.legend="Internet Connectie Type";
idx.conn_type="Connectie Type";
idx.stp="STP";
idx.stp_mess="(deactiveer voor COMCAST ISP)";
idx.optional="Optionele Instellingen (vereist door sommige ISPs)";
idx.mtu="MTU";
idx.h23="Netwerk Instellingen";
idx.routerip="Router IP";
idx.lanip="Local IP Adres";
idx.legend2="WAN Poort";
idx.wantoswitch="Verbind WAN Poort met Switch";
idx.legend3="Tijd Instellingen";
idx.timeset="Tijd Zone / Zomer Tijd (DST)";
idx.localtime="Gebruik locale Tijd";
idx.static_ip="Statisch IP";
idx.dhcp="Automatische Configuratie - DHCP";
idx.dhcp_legend="Network Adres Server Instellingen (DHCP)";
idx.dhcp_type="DHCP Type";
idx.dhcp_srv="DHCP Server";
idx.dhcp_fwd="DHCP Forwarder";
idx.dhcp_start="Begin IP Adres";
idx.dhcp_end="Eind IP Adres";		//used in Status_Lan.asp
idx.dhcp_maxusers="Maximum DHCP Gebruikers";
idx.dhcp_lease="Client Lease Tijd";
idx.dhcp_dnsmasq="Gebruik DNSMasq voor DHCP";
idx.dns_dnsmasq="Gebruik DNSMasq voor DNS";
idx.auth_dnsmasq="DHCP-Authoritatie";
idx.summt_opt1="none";
idx.summt_opt2="first Sun Apr - last Sun Oct";
idx.summt_opt3="last Sun Mar - last Sun Oct";
idx.summt_opt4="last Sun Oct - last Sun Mar";



//help container

hidx.right2="Deze instellingen worden het meest gebruikt door Cabel operatoren.";
hidx.right4="Voer de hostnaam toegewezen door uw ISP in.";
hidx.right6="Voer de domeinnaam in aangewezen door uw ISP.";
hidx.right8="Dit is het adres van de router.";
hidx.right10="Dit is de subnet mask van de router.";
hidx.right12="Staat toe dat de router uw IP adresen beheerd.";
hidx.right14="Het adres waarmee u zou willen beginnen.";
hidx.right16="U kan het aantal adressen limiteren dat uw router uit deeld.";
hidx.right18="Kies de tijd zone en zomer tijd periode waarin u zich bevind. De router kan dat de locale of UTC tijd gebruiken.";



// ******************************************* Join.asp *******************************************//

//sshd.webservices
join.titl=" - Join";
join.mess1="Succesvol het volgende netwerk als client betreed: ";



// ******************************************* Log_incoming.asp *******************************************//

log_in.titl=" - Inkomende Log Tabel";
log_in.h2="Inkomende Log Tabel";
log_in.th_ip="IP Afkomst";
log_in.th_port="Destinatie Poort Nummer";



// ******************************************* Log_outgoing.asp *******************************************//

log_out.titl=" - Uitgaande Log Tabel";
log_out.h2="Outgoing Log Table";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Destinatie URL/IP";
log_out.th_port="Service/Poort Nummer";



// ******************************************* Log.asp *******************************************//

log.titl=" - Log";
log.h2="Log Beheer";
log.legend="Log";
log.lvl="Log Niveau";
log.drop="Weggelaten";
log.reject="Tegengehouden";
log.accept="Toegelaten";



// ******************************************* Management.asp *******************************************//

management.titl=" - Beheer";
management.h2="Router Beheer";

management.psswd_legend="Router Wachtwoord";
management.psswd_user="Router Gebruikersnaam";
management.psswd_pass="Router Wachtwoord";
management.pass_conf="Vul opnieuw in ter confirmatie";

management.remote_legend="Toegang op Afstand";
management.remote_gui="Web GUI Beheer";
management.remote_https="Gebruik HTTPS";
management.remote_guiport="Web GUI Poort";
management.remote_ssh="SSH Beheer";
management.remote_sshport="SSH Poort";

management.web_legend="Web Toegang";
management.web_refresh="Auto-vernieuwen (in seconds)";
management.web_sysinfo="Activeer Info Site";
management.web_sysinfopass="Info Site Wachtwoord Protectie";
management.web_sysinfomasq="Info Site MAC Verbergen";

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

management.rst_legend="Reset Knop";
management.rst_srv="Reset Knop";

management.routing_legend="Routing";
management.routing_srv="Routing";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="SecureOn Wachtwoord";
management.wol_mac="MAC Adressen<br/>( Formaat: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="IPv6 Support";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd geactiveerd";
management.ipv6_radconf="Radvd configuratie";

management.jffs_legend="JFFS2 Support";
management.jffs_srv="JFFS2";
management.jffs_clean="Clean JFFS2";

management.lang_legend="Taal Selectie";
management.lang_srv="Taal";
management.lang_bulgarian="bulgarian";
management.lang_chinese_traditional="chinese traditional";
management.lang_chinese_simplified="chinese simplified";
management.lang_croatian="croatian";
management.lang_czech="czech";
management.lang_dutch="dutch";
management.lang_portuguese_braz="portuguese (brazilian)";
management.lang_english="english";
management.lang_french="french";
management.lang_german="german";
management.lang_italian="italian";
management.lang_brazilian="brazilian";
management.lang_slovenian="slovenian";
management.lang_spanish="spanish";
management.lang_swedish="swedish";
management.lang_polish="polish";

management.net_legend="IP Filter Instellingen (Pas deze aan voor P2P)";
management.net_port="Maximum Poorten";
management.net_tcptimeout="TCP Timeout (in seconden)";
management.net_udptimeout="UDP Timeout (in seconden)";

management.clock_legend="Overclocking";
management.clock_frq="Frequency";
management.clock_support="Niet mogelijk";

management.mmc_legend="MMC/SD Card Ondersteuning";
management.mmc_srv="MMC Device";

management.samba_legend="Samba FS Automount";
management.samba_srv="SMB Filesystem";
management.samba_share="Delen";
management.samba_stscript="Startscript";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP Poort";
management.SIPatH_domain="SIP Domein";

management.gui_style="Router GUI Stijl";


//help container

hmanagement.right1="Auto-Vernieuwen:";
hmanagement.right2="Past de automatische vernieuwings interval van de Web GUI aan. 0 zet deze functie volledig uit.";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

portserv.titl=" - Poort Diensten";
portserv.h2="Poort Diensten";



// ******************************************* QoS.asp *******************************************//

qos.titl=" - Quality of Service";
qos.h2="Quality Of Service (QoS)";
qos.legend="QoS Instellingen";
qos.srv="Start QoS";
qos.type="Packet Planner";
qos.uplink="Uplink (kbps)";
qos.dnlink="Downlink (kbps)";
qos.gaming="Optimaliseer voor Gaming";
qos.legend2="Diensten Prioriteit";
qos.prio_x="Exempt";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_s="Standaard";
qos.prio_b="Bulk";
qos.legend3="Netmask Prioriteit";
qos.ipmask="IP/Mask";
qos.maxrate_b="Max Kbits";
qos.maxrate_o="Max Snelheid";
qos.legend4="MAC Prioriteit";
qos.legend5="Ethernet Poort Prioriteit";
qos.legend6="Standaard Bandbreedte Level";
qos.bandwith="Bandbreedte in Kbits";

//help container

hqos.right1="Uplink:";
hqos.right2="Stel dit in op 80%-95% (max) van uw totale upload limiet.";
hqos.right3="Downlink:";
hqos.right4="Stel dit in op 80%-100% (max) van uw totale download limiet.";
hqos.right6="U kan uw data snelheid beperken naar gelang de applicatie welke bandbreedte vereist.";
hqos.right8="U kan uw data een prioriteit toekennen voor alle gegeven IP adressen of een IP range.";
hqos.right10="U kan uw data een prioriteit toekennen voor alle informatie afkomstig van een bepaald apparaat, \
			door het een prioriteit en het MAC adres toe te kennen.";
hqos.right12="U kan uw bandbreedte beheren volgens de fysieke LAN poort waarop het apparaat is aangesloten. \
			U kan prioriteiten toewijzen volgens apparaten van poort 1 tot 4.";



// ******************************************* RouteTable.asp *******************************************//

routetbl.titl=" - Route Tabel";
routetbl.h2="Route Tabel Entry Lijst";
routetbl.th1="Bestemming LAN IP";



// ******************************************* Routing.asp *******************************************//

route.titl=" - Route";
route.h2="Geavanceerde Routering";
route.mod="Operatie Modus";
route.bgp_legend="BGP Instellingen";
route.bgp_ip="Buur IP";
route.bgp_as="Buur AS#";
route.rip2_mod="RIP2 Router";
route.ospf_mod="OSPF Router";
route.gateway_legend="Dynamische Routing";
route.static_legend="Statische Routing";
route.static_setno="Selecteer een nummer";
route.static_name="Route Naam";
route.static_ip="Bestemming LAN IP";

//help container

hroute.right2="Indien de router gehost is via uw internet connectie, kies dan <em>Gateway</em> modus. Indien er zich een andere router bevind in het netwerk, selecteer <em>Router</em> modus.";
hroute.right4="Dit is het unieke routerings nummer, je kan tot 20 routes aanmaken.";
hroute.right6="Voer de naam in die u wilt toekennen aan deze route.";
hroute.right8="Dit is de remote host naar waar u de statische route wilt verwijzen.";
hroute.right10="Herkent de host en netwerk grootte.";


// ******************************************* Site_Survey.asp *******************************************//

survey.titl=" - Site Onderzoek";
survey.h2="Neighbor&#39;s Draadloze Netwerken";
survey.thjoin="Join Site";



// ******************************************* Services.asp *******************************************//

service.titl=" - Diensten";
service.h2="Diensten Beheer";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Start Kaid";
service.kaid_mac="Console Macs: (moet eindigen  met;)";

//DHCPd
service.dhcp_legend="DHCP Client";
service.dhcp_vendor="Kies Vendorclass";
service.dhcp_legend2="DHCP Server";
service.dhcp_srv="DHCP Daemon";
service.dhcp_jffs2="Gebruik JFFS2 voor de client lease DB";
service.dhcp_domain="Gebruikte Domein";
service.dhcp_landomain="LAN Domein";
service.dhcp_option="Extra DHCPd Opties";
service.dnsmasq_legend="DNSMasq";
service.dnsmasq_srv="DNSMasq";
service.dnsmasq_loc="Lokale DNS";
service.dnsmasq_opt="Extra DNS Opties";

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
service.pptpd_option="PPTP Client Opties";
service.pptpd_ipdns="Server IP of DNS Naam";
service.pptpd_subnet="Remote Subnet";
service.pptpd_subnetmask="Remote Subnet Mask";
service.pptpd_encry="MPPE Encryptie";
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
service.snmp_loc="Locatie";
service.snmp_contact="Contacteer";
service.snmp_name="Naam";
service.snmp_read="RO Commune";
service.snmp_write="RW Commune";

//openvpn.webservices
service.vpn_legend="OpenVPN Client";
service.vpn_srv="Start OpenVPN";
service.vpn_ipname="Server IP/Naam";
service.vpn_mtu="TUN MTU Instellingen";
service.vpn_mru="TUN MTU Extra";
service.vpn_mss="TCP MSS";
service.vpn_compress="Gebruik LZO Compressie";
service.vpn_tunnel="Tunnel Protocol";
service.vpn_srvcert="Publieke Server Cert";
service.vpn_clicert="Publieke Client Cert";
service.vpn_certtype="nsCertType";
service.vpn_clikey="Private Client Sleutel";

//sshd.webservices
service.ssh_legend="Secure Shell";
service.ssh_srv="SSHd";
service.ssh_password="Wachtwoord Login";
service.ssh_key="Toegelaten Sleutels";



// ******************************************* Sipath.asp + cgi *******************************************//

sipath.titl=" - SiPath Overzicht";
sipath.phone_titl=" - Telefoonboek";
sipath.status_titl=" - Status";



// ******************************************* Status_Lan.asp *******************************************//

status_lan.titl=" - LAN Status";
status_lan.h2="Locaal Netwerk";
status_lan.legend="LAN Status";
status_lan.h22="Dynamic Host Configuratie Protocol";
status_lan.legend2="DHCP Status";
status_lan.legend3="DHCP Clienten";

//help container

hstatus_lan.right2="Dit is het MAC Adres van de router, zoals ingesteld op uw lokaal, Ethernet network.";
hstatus_lan.right4="Dit toont het IP Adres van de router, zoals ingesteld op uw lokaal, Ethernet network.";
hstatus_lan.right6="Wanneer de router een Subnet Mask gebruikt vindt u deze hier.";
hstatus_lan.right8="Indien uw router een DHCP server is, wordt deze hier weergegevevn.";
hstatus_lan.right10="Door op een MAC adres te klikken, zal u de producent van de netwerk interface te weten komen (IEEE Standaard OUI database opzoeking).";



// ******************************************* Status_Router.asp *******************************************//

status_router.titl=" - Router Status";
status_router.h2="Router Informatie";
status_router.legend="Systeem";
status_router.sys_model="Router Model";
status_router.sys_firmver="Firmware Versie";
status_router.sys_time="Huidige Tijd";
status_router.sys_up="Uptime";
status_router.sys_load="Load Gemiddelde";
status_router.legend2="CPU";
status_router.cpu="CPU Model";
status_router.clock="CPU Clock";
status_router.legend3="Geheugen";
status_router.mem_tot="Totaal Beschikbaar";
status_router.mem_free="Vrij";
status_router.mem_used="Gebruikt";
status_router.mem_buf="Buffers";
status_router.mem_cached="Cached";
status_router.mem_active="Actief";
status_router.mem_inactive="Niet Actief";
status_router.legend4="Netwerk";
status_router.net_maxports="IP Filter Maximum Poortens";
status_router.net_conntrack="Actieve IP Connecties";
status_router.h22="Internet";
status_router.legend5="Configuratie Type";
status_router.www_login="Login Type";
status_router.www_loginstatus="Login Status";

//help container

hstatus_router.right2="Dit is de specifieke naam gegeven aan de router, U kan deze veranderen op de <i>Setup</i> tab.";
hstatus_router.right4="Dit is het MAC Adres van de router, zoals uw ISP het zal herkennen.";
hstatus_router.right6="Dit is de huidige firmware van de router.";
hstatus_router.right8="Dit is de tijd verkregen van de ntp server ingesteld op de <em>" + bmenu.admin + " | " + bmenu.adminManagement + "</em> tab.";
hstatus_router.right10="Dit is een berekening van de tijd dat de router \"up and running\" was.";
hstatus_router.right12="Dit is een gegeven van drie nummers dat de belasting van het systeem voorstelt gedurende de laatste één, vijf of vijftien minuten.";
hstatus_router.right14="Dit geeft de informatie die vereist is door uw ISP weer. \
				Deze informatie werd ingegeven op de Setup Tab. U kan uw connectie <em>Verbinden</em> of <em>Verbreken</em> door op de knop te duwen.";



// ******************************************* Status_SputnikAPD.asp *******************************************//


status_sputnik.titl=" - Sputnik Agent Status";
status_sputnik.h2="Sputnik&reg; Agent&trade;";
status_sputnik.manage="Managed By";
status_sputnik.license="SCC License No.";

//help container

hstatus_sputnik.right1="Sputnik Agent Status";
hstatus_sputnik.right2="Dit scherm geeft de status van uw Sputnik Agent weer.";
hstatus_sputnik.right4="Het Sputnik Controle Centrum waarmee uw accesspoint verbonden is.";
hstatus_sputnik.right6="De huidige status van de agent.";
hstatus_sputnik.right8="Het licentie van uw Sputnik Controle Centrum.";



// ******************************************* Status_Wireless.asp *******************************************//

status_wireless.titl=" - Draadloze Status";
status_wireless.h2="Draadloos";
status_wireless.legend="Draadloze Status";
status_wireless.net="Netwerk";
status_wireless.pptp="PPTP Status";
status_wireless.legend2="Pakket Info";
status_wireless.rx="Ontvangen (RX)";
status_wireless.tx="Verzenden (TX)";
status_wireless.h22="Draadloze Nodes";
status_wireless.legend3="Clienten";
status_wireless.signal_qual="Signaal Qualiteit";
status_wireless.wds="WDS Nodes";

//help container

hstatus_wireless.right2="Dit is het MAC Adres van de router, zoals het wordt weergegeven op uw, draadloos netwerk.";
hstatus_wireless.right4="U hebt de draadloze modus ingesteld op de Draadloos tab, dit zal de huidige draadloze modus weergeven (Mixed, G-Enkel, B-Enkel of Niet Geactiveerd).";



// ******************************************* Triggering.asp *******************************************//

trforward.titl=" - Poort Triggering";
trforward.h2="Poort Triggering";
trforward.legend="Forwards";
trforward.trrange="Triggered Poort Bereik";
trforward.fwdrange="Forwarded Poort Bereik";
trforward.app="Applicatie";

//help container

htrforward.right2="Voer de applicatie naam van de trigger in.";
htrforward.right4="Voer voor elke applicatie een lijst van getriggerde poorten in. Controleer de Internet Applicatie documentatie voor het vereiste poort nummer.";
htrforward.right6="Voer voor elke applicatie een lijst van geforwarde poorten in. Controleer de Internet Applicatie documentatie voor het vereiste poort nummer.";
htrforward.right8="Voer het beginnende poort nummer in van de Trigger en Forward Range.";
htrforward.right10="Voer het eindend poort nummer in van de Trigger en Forward Range.";



// ******************************************* Upgrade.asp *******************************************//

upgrad.titl=" - Firmware Upgrade";
upgrad.h2="Firmware Beheer";
upgrad.legend="Firmware Upgrade";
upgrad.info1="Na het flashen, reset op volgende manier";
upgrad.resetOff="Geen reset";
upgrad.resetOn="Standaard instellingen";
upgrad.file="Selecteer een bestand voor de upgrade";
upgrad.warning="O P G E L E T";
upgrad.mess1="Firmware upgraden kan enkele minuten duren.<br />Zet uw router NIET af, reset uw router NIET, gedurende dit proces!";

//help container

hupgrad.right2="Klik op de <em>Browse...</em> knop om het firmware bestand te selecteren dat vereist is voor de upgrade.<br /><br /> \
			Klik op de <em>Upgrade</em> knop om het upgrade proces te starten. De upgrade mag niet onderbroken worden.";



// ******************************************* UPnP.asp *******************************************//

upnp.titl=" - UPnP";
upnp.h2="Universele Plug and Play (UPnP)";
upnp.legend="Forwards";
upnp.legend2="UPnP Configuratie";
upnp.serv="UPnP Dienst";
upnp.clear="Poorten forwarden tijdens het opstarten";
upnp.url="Verzend presentatie URL";
upnp.msg1="Klik om de lease te verwijderen";
upnp.msg2="Verwijder alle waarden?";


//help container

hupnp.right2="Klik op de vuilnisbak om een individuele waarde te verwijderen.";
hupnp.right4="Staat applicaties toe automatisch poort forwards in te stellen.";



// ******************************************* VPN.asp *******************************************//

vpn.titl=" - VPN";
vpn.h2="Virtueel Private Netwerk (VPN)";
vpn.legend="VPN Passthrough";
vpn.ipsec="IPSec Passthrough";
vpn.pptp="PPTP Passthrough";
vpn.l2tp="L2TP Passthrough";

//help container

hvpn.right1="U kan IPSec, PPTP en/of L2TP activeren, zodat uw netwerk aparaten via VPN kunnen communiceren.";


// ******************************************* Vlan.asp *******************************************//

vlan.titl=" - Virtual LAN";
vlan.h2="Virtual Local Area Network (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Toegewezen aan<br />Bridge";
vlan.tagged="Tagged";
vlan.negociate="Auto-Onderhandelen";
vlan.aggregation="Link Aggregatie<br>op Poorten 3 & 4";
vlan.trunk="Trunk";


// ******************************************* WEP.asp *******************************************//

wep.defkey="Standaard Transmit Sleutel";
wep.passphrase="Sleutelzin";



// ******************************************* WOL.asp *******************************************//

wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Beschikbare Hosts";
wol.legend2="WOL Adressen";
wol.legend3="Uitvoer";
wol.legend4="Manuale WOL";
wol.enable="Activeer WOL?";
wol.add_wol="Voeg WOL Host Toe";
wol.restore="Herstel fabricatie instellingen";
wol.mac="MAC Adres(sen)";
wol.broadcast="Net Broadcast";
wol.udp="UDP Poort";
wol.msg1="Klik om een WOL host te verwijderen";

//help container

hwol.right2="Op deze pagina kan u hosts <em>Wakker Maken</em> die verbonden zijn met uw netwerk (bv. locaal verbonden met uw router).";
hwol.right4="MAC Adressen moeten ingevoerd worden volgens het formaat xx:xx:xx:xx:xx:xx (bv. 01:23:45:67:89:AB)";
hwol.right6="Het IP Adres is het broadcast adres van het netwerk, maar kan ook een remote adres zijn indien het doel zich niet in het locale bevind."



// ******************************************* WanMAC.asp *******************************************//

wanmac.titl=" - MAC Adres Clone";
wanmac.h2="MAC Adres Clone";
wanmac.legend="MAC Clone";
wanmac.wan="Clone WAN MAC";
wanmac.wlan="Clone Draadloos MAC";

//help container

hwanmac.right2="Sommige ISP's eisen dat je je MAC adres registreerd. \
			Indien je je MAC adres niet wilt herregistreren, kan je de router je MAC adres laten clonen met hetgene dat je geregistreerd hebt bij je ISP.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

wpa.titl=" - Draadloze Beveiliging";
wpa.h2="Draadloze Beveiliging";
wpa.secmode="Beveiligings Modus";
wpa.legend="Draadloze Encryptie";
wpa.auth_mode="Netwerk Authenticatie";
wpa.psk="WPA Pre-Shared Key";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA Group Rekey Interval";
wpa.rekey="Key Vernieuwings Interval (in seconden)";
wpa.radius_ipaddr="RADIUS Server Adres";
wpa.radius_port="RADIUS Server Poort";
wpa.radius_key="RADIUS Sleutel";
wpa.algorithms="WPA Algorithmes";
wpa.shared_key="WPA Gedeelde Sleutel";
wpa.rekeyInt="rekey interval";

//help container

hwpa.right1="Veligheids Modus:";
hwpa.right2="U hebt keuze uit Deactiveren, WEP, WPA Pre-Shared Key, WPA RADIUS, of RADIUS. Alle apparaten in uw newerk moeten de geselecteerde beveiligingmodus gebruiken.";



// ******************************************* WL_FilterTable.asp *******************************************//

wl_filter.titl=" - MAC Adres Filter Lijst";
wl_filter.h2="MAC Adres Filter Lijst";
wl_filter.h3="Voer het MAC Adres in dit formaat&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx in";



// ******************************************* WL_ActiveTable.asp *******************************************//

wl_active.titl=" - Draadloze Clienten Lijst";
wl_active.h2="Draadloze Clienten MAC Lijst";
wl_active.h3="Activeer MAC Filter";
wl_active.active="Actieve PC";
wl_active.inactive="Inactieve PC";



// ******************************************* Wireless_WDS.asp *******************************************//

wds.titl=" - WDS";
wds.h2="Draadloos Distributie Systeem";
wds.legend="WDS Instellingen";
wds.label="Lazy WDS";
wds.label2="WDS Subnet";
wds.wl_mac="Draadloos MAC";
wds.lazy_default="Standaard: Gedeactiveerd";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="Subnet";
wds.legend2="Extra Opties";



// ******************************************* Wireless_radauth.asp *******************************************//

radius.titl=" - Radius";
radius.h2="Remote Authenticatie Dial-In Gebruikers Dienst";
radius.legend="Radius";
radius.label="MAC Radius Client";
radius.label2="MAC Formaat";
radius.label3="Radius Server IP";
radius.label4="Radius Server Poort";
radius.label5="Maximum Toegelaten Gebruikers";
radius.label6="Wachtwoord Formaat";
radius.label7="RADIUS Gedeeld Geheim";
radius.label8="Overschrijf Radius indien de server niet beschikbaar is";
radius.mac="MAC";



// ******************************************* Wireless_MAC.asp *******************************************//

wl_mac.titl=" - MAC Filter";
wl_mac.h2="Draadloze MAC Filter";
wl_mac.legend="MAC Filter";
wl_mac.label="Gebruik Filter";
wl_mac.label2="Filter Modus";
wl_mac.deny="Verbied toegang tot het draadloos netwerk voor de PC's toegevoegd aan de lijst";
wl_mac.allow="Sta enkel toegang toe aan de PC's die toegevoegd zijn aan de lijst";



// ******************************************* Wireless_Basic.asp *******************************************//

wl_basic.titl=" - Draadloos";
wl_basic.h2="Draadloos";
wl_basic.legend="Basis Instellingen";
wl_basic.label="Draadloze Modus";
wl_basic.label2="Draadloze Network Modus";
wl_basic.label3="Draadloos Netwerk Naam(SSID)";
wl_basic.label4="Draadloos Kanaal";
wl_basic.label5="Draadloos SSID Broadcast";
wl_basic.label6="Gevoeligheid Range (ACK Timing)";
wl_basic.ap="AP";
wl_basic.client="Client";
wl_basic.clientBridge="Client Bridge";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="Mixed";
wl_basic.b="B-Only";
wl_basic.g="G-Only";
wl_basic.sensitivity="Standaard: 20000 meter";
wl_basic.legend2="Radio Time Restrictions";
wl_basic.radio="Radio";
wl_basic.radiotimer="Radio Scheduling";
wl_basic.radio_on="Radio is On";
wl_basic.radio_off="Radio is Off";
wl_basic.radio_unkn="Unknown";

//help container

hwl_basic.right2="Indien u Draadloos-G clienten wilt verbieden, kies dan <em>B-Enkel</em> modus. Indien u draadloze toegang geheel wilt verbieden, kies dan <em>Deactiveer</em>.";
hwl_basic.right3="Gevoeligheid Range: ";
hwl_basic.right4="Pas de ack timing aan. 0 schakelt ack timing volledig uit.";



// ******************************************* Wireless_Advanced.asp *******************************************//

wl_adv.titl=" - Geadvanceerde Draadloze Instellingen";
wl_adv.h2="Geadvanceerde Draadloze Instellingen";
wl_adv.legend="Geadvanceerde Instellingen";
wl_adv.legend2="Draadloze Multimedia Ondersteuning Instellingen";
wl_adv.label="Authenticatie Type";
wl_adv.label2="Basis Rate";
wl_adv.label3="Transmissie Rate";
wl_adv.label4="CTS Protectie Modus";
wl_adv.label5="Frame Burst";
wl_adv.label6="Beacon Interval";
wl_adv.label7="DTIM Interval";
wl_adv.label8="Fragmentatie Drempel";
wl_adv.label9="RTS Drempel";
wl_adv.label10="Max Toegewezen Clienten";
wl_adv.label11="AP Isolatie";
wl_adv.label12="TX Antenne";
wl_adv.label13="RX Antenne";
wl_adv.label14="Preamble";
wl_adv.reference="Storing Voorkeur";
wl_adv.label15="Zend Kracht";
wl_adv.label16="Afterburner";
wl_adv.label17="Draadloze GUI Toegang";
wl_adv.label18="WMM Ondersteuning";
wl_adv.label19="Geen-Bevestiging";
wl_adv.table1="EDCA AP Parameters (AP naar Client)";
wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="Admin Forced";
wl_adv.row1="Achtergrond";
wl_adv.row2="Best Effort";
wl_adv.row3="Video";
wl_adv.row4="Voice";
wl_adv.table2="EDCA STA Parameters (Client to AP)";
wl_adv.lng="Lang"; 					//************* don't use .long ! *************
wl_adv.shrt="Kort"; 				//************* don't use .short ! **************

//help container

hwl_adv.right2="You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.";



// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

fail.mess1="De waarden die u invoerde zijn niet geldig. Probeer nog eens.";
fail.mess2="Upgrade mislukt.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

success.saved="Instellingen opgeslagen.";
success.restore="Instellingen herstelt.<br/>Unit reboot nu. Gelieve even te wachten...";
success.upgrade="Upgrade sucesvol.<br/>Unit reboot nu. Gelieve even te wachten...";
success.success_noreboot="Instellingen zijn ok.";
success.success_reboot=success.success_noreboot + "<br />Unit reboot nu. Gelieven even te wachten...";

success.alert_reset="Alle configuratie instellingen werden naar hun oorspronkelijke waarden herstelt.<br /><br />";
success.alert1="Gelieve het volgende te controleren alvorens te verbinden:";
success.alert2="Indien u uw routers IP adres heeft veranderd, moet u uw client(en) hun adres op het netwerk vernieuwen.";
success.alert3="Indien u verbonden bent via WLAN, sluit u dan aan bij het netwerk en klik op <em>Activeer</em>.";

// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

dhcp.titl=" - DHCP Actieve IP Tabel";
dhcp.h2="DHCP Actieve IP Tabel";
dhcp.server="DHCP Server IP Adres :";
dhcp.tclient="Client Host Naam";


donate.mb="U kan ook geld doneren via de Moneybookers acount mb@dd-wrt.com";
