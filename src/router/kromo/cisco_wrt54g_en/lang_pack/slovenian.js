
// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-2";

var share = new Object();
share.firmware="Strojna programska oprema";
share.time="Èas";
share.interipaddr="Internetni naslov IP";
share.more="Veè...";
share.help="Pomoè";
share.enable="Omogoèi";
share.enabled="Omogoèeno";
share.disable="Onemogoèi";
share.disabled="Onemogoèeno";
share.usrname="Uporabniško ime";
share.passwd="Geslo";
share.hostname="Ime gostitelja";
share.domainname="Domensko ime";
share.statu="Status";
share.start="Zaèetek";
share.end="Konec";
share.proto="Protokol";
share.ip="Naslov IP";
share.mac="Naslov MAC";
share.none="Nobeden";
share.both="Oboje";
share.del="Izbriši";
share.remove="Odstrani";
share.descr="Opis";
share.from="Od";
share.to="Do";
share.about="Vizitka";
share.everyday="Vsak dan";
share.sun="Nedelja";
share.sun_s="Ned";
share.sun_s1="N";
share.mon="Ponedeljek";
share.mon_s="Pon";
share.mon_s1="P";
share.tue="Torek";
share.tue_s="Tor";
share.tue_s1="T";
share.wed="Sreda";
share.wed_s="Sre";
share.wed_s1="S";
share.thu="Èetrtek";
share.thu_s="Èet";
share.thu_s1="È";
share.fri="Petek";
share.fri_s="Pet";
share.fri_s1="P";
share.sat="Sobota";
share.sat_s="Sob";
share.sat_s1="S";
share.expires="Poteèe";
share.yes="Da";
share.no="Ne";
share.allow="Dovoli";
share.deny="Prepreèi";
share.range="Razpon";
share.use="Uporabi";
share.mins="Min.";
share.secs="Sek.";
share.routername="Ime usmerjalnika";
share.manual="Roèno";
share.port="Vrata";
share.ssid="SSID";
share.channel="Kanal";
share.rssi="Rssi";
share.signal="Signal";
share.noise="Šum";
share.beacon="beacon";
share.openn="Odprto";
share.dtim="dtim";
share.rates="Hitrost";
share.low="Nizko";
share.medium="Srednje";
share.high="Visoko";
share.option="Možnosti";
share.rule="Pravilo";
share.lan="LAN";
share.point2point="Povezava med toèkama";
share.nat="NAT";
share.subnet="Podmrežna maska";
share.unmask="Odkrij";
share.deflt="Privzeto";  //don't use share.default !!!
share.all="Vse";
share.auto="Samodejno";
share.right="Dasno";
share.left="Levo";
share.share_key="Skupni kljuè";
share.inter="Interval (v sekundah)";
share.srv="Ime storitve";
share.port_range="Razpon vrat";
share.priority="Prioriteta";
share.gateway="Prehod";
share.intrface="Vmesnik";  //don't use share.interface, Mozilla problem!!!
share.router="Usmerjalnik";
share.static_lease="Statièni zakupi";
share.srvip="IP strežnika";
share.localdns="Lokalni DNS";
share.minutes="minut";
share.oui="Iskanje OUI";
share.sttic="Statièno";
share.connecting="Povezujem";
share.connect="Povezati";
share.connected="Povezano";
share.disconnect="Prekini";
share.disconnected="Prekinjeno";
share.info="Informacija";
share.state="Status";
share.mode="Naèin";
share.encrypt="Šifriranje";
share.key="Kljuè";
share.wireless="Brezžièno";
share.dhcp="DHCP";
share.styl="Stil";


var sbutton = new Object();
sbutton.save="Shrani nastavitve";
sbutton.saving="Shranjeno";
sbutton.cmd="Izvajam";
sbutton.cancel="Preklièi spremembe";
sbutton.refres="Osveži";
sbutton.clos="Zapri";
sbutton.del="Izbriši";
sbutton.continu="Nadaljuj";
sbutton.add="Dodaj";
sbutton.remove="Odstrani";
sbutton.modify="Spremeni";
sbutton.deleted="Izbrisano";
sbutton.delall="Izbriši vse";
sbutton.autorefresh="Samodejno osveževanje je vkljuèeno";
sbutton.backup="Varnostno kopiranje";
sbutton.restore="Obnovitev";
sbutton.cptotext="Kopiraj v polje z besedilom";
sbutton.runcmd="Zaženi ukaze";
sbutton.startup="Shrani zagon";
sbutton.firewall="Shrani požarni zid";
sbutton.wol="Zbujanje";
sbutton.add_wol="Dodaj gostitelja";
sbutton.manual_wol="Roèno zbujanje";
sbutton.summary="Povzetek";
sbutton.filterIP="Uredi seznam PCjev";
sbutton.filterMac="Uredi seznam MAC filtrov";
sbutton.filterSer="Dodaj/uredi storitev";
sbutton.reboot="Ponovni zagon usmerjalnika";
sbutton.help="   Pomoè  ";
sbutton.wl_client_mac="MAC seznam brezžiènih odjamalcev";
sbutton.update_filter="Posodobi seznam filtrov";
sbutton.join="Poveži";
sbutton.log_in="Vhodni dnevnik";
sbutton.log_out="Izhodni dnevnik";
sbutton.apply="Uporabi";
sbutton.edit_srv="Dodaj/uredi storitev";
sbutton.routingtab="Pokaži usmerjalno tabelo";
sbutton.wanmac="Prikaži trenutni PCjev MAC naslov";
sbutton.dhcprel="Sprosti DHCP";
sbutton.dhcpren="Obnovi DHCP";
sbutton.survey="Pregled podroèja";
sbutton.upgrading="Posodabljanje";
sbutton.upgrade="Posodobi";
sbutton.preview="Predogled";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//
var errmsg = new Object();
errmsg.err0="Vnesti morate uporabniško ime.";
errmsg.err1="Vnesti morate ime usmerjalnika.";
errmsg.err2="Izven dovoljenega obmoèja, prosim popravite zaèetni IP naslov ali število uporabnikov.";
errmsg.err3="Izbrati morate vsaj dan."
errmsg.err4="Konèni èas mora biti veèji od zaèetnega èasa.";
errmsg.err5="Dolžina naslova MAC ni pravilna.";
errmsg.err6="Vnesti morate geslo.";
errmsg.err7="Vnesti morate ime gostitelja.";
errmsg.err8="Vnesti morate naslov IP ali domensko ime.";
errmsg.err9="Nepravilni DMZ naslov IP.";
errmsg.err10="Potrditveno geslo se ne ujema z vnešenim geslom. Prosim ponovite vnos.";
errmsg.err11="V geslu presledki niso dovoljeni";
errmsg.err12="Vnesti morate ukaz.";
errmsg.err13="Posodabljanje ni uspelo.";
errmsg.err45="Ni na voljo v HTTPS naèinu! Prosim, uporabite naèin HTTP.";
errmsg.err46="Ni na voljo v HTTPS naèinu";


//common.js error messages
errmsg.err14=" vrednost je izven dovoljenih meja [";
errmsg.err15="WAN MAC je izven dovoljenih meja [00 - ff].";
errmsg.err16="drugi znak v MAC mora biti sodo število : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="MAC naslov ni pravilen.";
errmsg.err18="Dolžina MAC naslova ni pravilna.";
errmsg.err19="MAC naslov ne more biti broadcast naslov."
errmsg.err20="Vnesite MAC naslov v (xx:xx:xx:xx:xx:xx) formatu.";
errmsg.err21="Nepravilni format MAC naslova.";
errmsg.err22="WAN MAC naslov ni pravilen.";
errmsg.err23="Nepravilna hex vrednost ";
errmsg.err24=" najdena v MAC naslovu ";
errmsg.err25="Vrednost kljuèa ni pravilna.";
errmsg.err26="Dožina kljuèa ni pravilna.";
errmsg.err27="Nepravina podmrežna maska.";
errmsg.err28=" ima nepravilni znak, biti mora [ 0 - 9 ].";
errmsg.err29=" ima nepravilno ascii kodo."
errmsg.err30=" ima nepravino hexadecimalno številko.";
errmsg.err31=" vrednost je nepravilna.";
errmsg.err32="naslov IP in prehod nista v istem podmrežju.";
errmsg.err33="naslov IP in prehod ne moreta biti enaka.";
errmsg.err34=" ne sme vsebovati presledka.";

//Wol.asp error messages
errmsg.err35="Vnesti morate MAC naslov za zagon."
errmsg.err36="Vnesti morate mrežni broadcast naslov za zagon.";
errmsg.err37="Vnesti morate UDP vrata za zagon.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Prosim vnesite skupni kljuè!";
errmsg.err39="Nepravilen kljuè, biti mora med 8 in 63 ASCII znaki ali 64 hexadecimalnih znakov"
errmsg.err40="Vnesti morate kljuè za Kljuè ";
errmsg.err41="Nepravilna dolžina kljuèa ";
errmsg.err43="Interval za izmenjavo kljuèa";

//config.asp error messages
errmsg.err42="Prosim, izberite datoteko za obnovitev.";

//WL_ActiveTable.asp error messages
errmsg.err44="Skupno število je preseglo 128.";

//Site_Survey.asp error messages
errmsg.err47=("nepravilen SSID.");

//Wireless_WDS.asp error messages
errmsg.err48="WDS ni združljiv s trenutno konfiguracijo usmerjalnika. Prosim, preverite naslednje :\n * Brezžièni naèin mora biti AP \n * WPA2 ni podprta pod WDS \n * Brežièno omrežje v naèinu samo-B ni podprto pod WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius je na volju samo v naèinu AP.";

//Wireless_Basic.asp error messages
errmsg.err50="Vnesti morate SSID.";

// Management.asp error messages
errmsg.err51="Usmerjalnik trenutno uporablja privzeto geslo. \
			Zaradi varnosti morate spremeniti geslo preden omogoèite oddaljeni dostop. \
			Kliknite gumb OK za spremembo gesla. Kliknite gumb Preklièi za onemogoèen oddaljeni dostop.";
errmsg.err52="Geslo se ne ujema.";

// Port_Services.asp error messages
errmsg.err53="Ko konèate, kliknite gumb Uporabi, da shranite nastavitve.";
errmsg.err54="Vnesti morate ime storitve.";
errmsg.err55="Ime storitve že obstaja.";

// QoS.asp error messages
errmsg.err56="Vrednost vrat je izven obmoèja [0 - 65535]";

// Routing.asp error messages
errmsg.err57="Zbrišem vnos ?";

// Status_Lan.asp error messages
errmsg.err58="Kliknite za brisane zakupa";

//Status_Wireless.asp error messages
errmsg.err59="Ni na voljo! Prosim, omogoèite brezžièno omrežje.";

//Upgrade.asp error messages
errmsg.err60="Prosim, izberite datoteko za posodobitev.";
errmsg.err61="Nepravilna datoteka.";

// *******************************************  COMMON MENU ENTRIES  *******************************************//
var bmenu= new Object();
bmenu.setup="Namestitev";
bmenu.setupbasic="Osnovna namestitev";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="Kloniranje MAC naslova";
bmenu.setuprouting="Napredno usmerjanje";
bmenu.setupvlan="VLAN";

bmenu.wireless="Brezžièno";
bmenu.wirelessBasic="Osnovne nastavitve";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Brezžièna zašèita";
bmenu.wirelessMac="Filter MAC";
bmenu.wirelessAdvanced="Napredne namestitve";
bmenu.wirelessWds="WDS";

bmenu.security="Zašèita";
bmenu.firwall="Požarni zid";
bmenu.vpn="VPN";

bmenu.accrestriction="Omejitve dostopa";
bmenu.webaccess="Dostop do medmrežja";


bmenu.applications="Programi &amp; Igre";
bmenu.applicationsprforwarding="Posredovanje razpona vrat";
bmenu.applicationspforwarding="Posredovanje vrat";
bmenu.applicationsptriggering="Proženje vrat";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Pregled";
bmenu.sipathphone="Imenik";
bmenu.sipathstatus="Status";

bmenu.admin="Administracija";
bmenu.adminManagement="Upravljanje";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Storitve";
bmenu.adminAlive="Keep Alive";
bmenu.adminLog="Dnevnik";
bmenu.adminDiag="Diagnostika";
bmenu.adminWol="WOL";
bmenu.adminFactory="Privzete nast.";
bmenu.adminUpgrade="Posodabljanje";
bmenu.adminBackup="Varnostno kopiranje";


bmenu.statu="Status";
bmenu.statuRouter="Usmerjalnik";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Agent Sputnik";
bmenu.statuWLAN="Brezžièno";
bmenu.statuSysInfo="Sis-Info";


// ******************************************* Alive.asp *******************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Razpored ponovnega zagona";
alive.sevr1="Razpored ponovnega zagona";
alive.time="Èas (v sekundah)";
alive.hour="Ob nastavljenem èasu";
alive.legend2="WDS/Stražni mehanizem";
alive.sevr2="Omogoèi stražni mehanizem";
alive.IP="IP naslovi";
alive.legend3="Proksi/Stražni mehanizem";
alive.sevr3="Omogoèi proksi stražni mehanizem";
alive.IP2="Proksi IP naslov";
alive.port="Proksi vrata";

//help container
var halive = new Object();
halive.right2="Izbira ponovnega zagona usmerjalnika. Cron mora biti omogoèen pod zavihkom Upravljanje.";
halive.right4="Dovoljeni so najveè trije IP naslovi loèeni s <em>PRESLEDEK</em>.<BR/>Format IP je xxx.xxx.xxx.xxx.";



// ******************************************* config.asp *******************************************//

var config = new Object();
config.titl=" - Varnostno kopiranje & Obnovitev";
config.h2="Konfiguracija varnostnega kopiranja";
config.legend="Varnostno kopiranje nastavitev";
config.mess1="Kliknite na gumb \"" + sbutton.backup + "\" za pronos varnostne kopije na Vaš raèunalnik.";
config.h22="Obnovi konfiguracijo";
config.legend2="Obnovi nastavitve";
config.mess2="Izberite datoteko za obnovitev";
config.mess3="O P O Z O R I L O";
config.mess4="Obnovitev je možna samo z datoteko ustvarjeno na istem modelu usmerjalnika in isto verzijo strojne programske opreme.<br />Ne nalagajte datotek, ki niso bile ustvarjene z tem uporabniškim vmesnikom!";

//help container
var hconfig = new Object();
hconfig.right2="Varnostno kopijo lahko naredite v primeru, da morate usmerjalnik ponastaviti na tovarniško privzete nastavitve.<br /><br />Kliknite gumb <em>Varnostno kopiranje</em> za varnostno kopiranje trenutne konfiguracije.";
hconfig.right4="Kliknite gumb <em>Prebrskaj...</em> za iskanje varnostne kopije, ki je trenutno na Vašem PCju.<br /><br />Kliknite gumb <em>" + sbutton.restore + "</em> za nalaganje varnstne kopije. To bo nadomestilo trenutno konfiguracijo s konfiguracijo shranjeno v varnostni kopiji.";



// ******************************************* DDNS.asp *******************************************//

var ddns = new Object();
ddns.titl=" - Dinamièni DNS"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DDNS";
ddns.srv="DDNS servis";
ddns.emailaddr="email naslov";
ddns.typ="Tip";
ddns.dynamic="Dinamièni";
// ddns.static="Static"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.custom="Prirejen";
ddns.wildcard="Nadomestni znak";
ddns.statu="DDNS status";

var ddnsm = new Object();
ddnsm.dyn_strange="Neprièakovani odziv strežnika; ali se prikljuèujete na pravilni strežnik ?";
ddnsm.dyn_good="DDNS posodobitev je uspela";
ddnsm.dyn_noupdate="Posodobitev trenutno ni potrebna";
ddnsm.dyn_nohost="Ime gostitelja ne obstaja";
ddnsm.dyn_notfqdn="Ime gostitelja ni pravilno";
ddnsm.dyn_yours="Ime gostitelja ni vaša last";
ddnsm.dyn_abuse="Gostitelj je bil blokiran zaradi zlorabe";
ddnsm.dyn_nochg="IP naslov se ni spremenil od zadnje posodobitve";
ddnsm.dyn_badauth="Overjanje ni uspelo (uporabniško ime ali geslo)";
ddnsm.dyn_badsys="Neveljavni sistemski parameter";
ddnsm.dyn_badagent="Ta uporabniški agent je bil blokiran";
ddnsm.dyn_numhost="Najdeno preveè ali premalo gostiteljev";
ddnsm.dyn_dnserr="DNS notranja napaka";
ddnsm.dyn_911="Neprièakovana napaka 911";
ddnsm.dyn_999="Neprièakovana napaka 999";
ddnsm.dyn_donator="Zahtevana funkcija je na voljo samo za donatorje, prosim darujte";
ddnsm.dyn_uncode="Neznana povratna koda";

ddnsm.tzo_good="Operacija zakljuèena";
ddnsm.tzo_noupdate="Posodobitev trenutno ni potrebna";
ddnsm.tzo_notfqdn="Napaèno domensko ime";
ddnsm.tzo_notmail="Napaèen email";
ddnsm.tzo_notact="Napaèno dejanje";
ddnsm.tzo_notkey="Napaèen kljuè";
ddnsm.tzo_notip="Nepravilen IP naslov";
ddnsm.tzo_dupfqdn="Podvojeno domensko ime";
ddnsm.tzo_fqdncre="Domensko ime je že bilo ustvarjeno za to domensko ime";
ddnsm.tzo_expired="Raèun je potekel";
ddnsm.tzo_error="Neprièakovana napaka na strežniku";

ddnsm.zone_701="Cona ni nastavljena za ta raèun";
ddnsm.zone_702="Posodobitev ni uspela";
ddnsm.zone_703="Potreben vsaj en parameter: <em>cona</em> ali <em>gostitelj</em>";
ddnsm.zone_704="Cona mora biti veljaveno internetno ime s <em>pikami</em>";
ddnsm.zone_705="Cona ne more biti prazna";
ddnsm.zone_707="Podvojena posodobitev za istega gostitelja/IP, prilagodi odjemalèeve nastavitve";
ddnsm.zone_201="Zapis ne potrebuje posodobitve";
ddnsm.zone_badauth="Overjanje ni uspelo (uporabniško ime ali geslo)";
ddnsm.zone_good="ZoneEdit je uspešno posodobljen";
ddnsm.zone_strange="Neprièakovani odziv strežnika; ali se prikljuèujete na pravilni strežnik ?";

ddnsm.all_closed="DDNS strežnik je trenutno zaprt";
ddnsm.all_resolving="Rezreševanje domenskega imena";
ddnsm.all_errresolv="Rezreševanje domenskega imena ni uspelo";
ddnsm.all_connecting="Prikljuèevanje na strežnik";
ddnsm.all_connectfail="Prikljuèevanje na strežnik ni uspelo";
ddnsm.all_disabled="DDNS funkcija je onemogoèena";
ddnsm.all_noip="Ni medmrežne povezave";

//help container
var hddns = new Object();
hddns.right2="DDNS omogoèa dostop do Vaše mreže z domenskim imenom namesto z naslovom IP. \
	Ta storitev upravlja s spremembami Vašega IP naslova in dinamièno posodobi podatke o Vaši domeni. \
	Prijaviti se morate pri DynDNS.org, TZO.com ali ZoneEdit.com.";



// ******************************************* Diagnostics.asp *******************************************//

var diag = new Object();
diag.titl=" - Diagnostika";
diag.h2="Diagnostika";
diag.legend="Ukazna lupina";
diag.cmd="Ukazi";
diag.startup="Zagon";
diag.firewall="Požarni zid";

//help container
var hdiag = new Object();
hdiag.right2="Ukaze lahko zaženete preko uporabniškega vmesnika. Vnesite ukaze v textovno polje in kliknite <em>" + sbutton.runcmd + "</em>.";



// ******************************************* DMZ.asp *******************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Demilitarizirano obmocèje (DMZ)";
dmz.legend="DMZ";
dmz.serv="Uporabi DMZ";
dmz.host="DMZ gostiteljev IP naslov";


//help container
var hdmz = new Object();
hdmz.right2="Omogoèanje te opcije bo izpostavilo gostitelja medmrežju. Vsa vrata bodo dostopna z medmrežja.";



// ******************************************* Factory_Defaults.asp *******************************************//

var factdef = new Object();
factdef.titl=" - Privzete nastavitve";
factdef.h2="Privzete nastavitve";
factdef.legend="Ponastavi nastavitve usmerjalnika";
factdef.restore="Obnovi privzete nastavitve";

factdef.mess1="Opozorilo! Èe kliknete OK, bodo nastavljene privzete nastavitve in vse prejšnje nastavitve bodo izbrisane.";

//help container
var hfactdef = new Object();
hfactdef.right1="To bo ponastavilo privzete nastavitve. Vse Vaše nastavitve bodo izbrisane.";



// ******************************************* FilterIP%AC.asp *******************************************//

var filterIP = new Object();
filterIP.titl=" - Seznam PCjev";
filterIP.h2="Seznam PCjev";
filterIP.h3="Vnesi PCjev MAC v tem formatu formatu: xx:xx:xx:xx:xx:xx";
filterIP.h32="Vnesi PCjev IP naslov";
filterIP.h33="Vnesi PCjev razpon IP naslovov";
filterIP.ip_range="IP razpon";



// ******************************************* Filter.asp *******************************************//

var filter = new Object();
filter.titl=" - Omejitve dostopa";
filter.h2="Dostop do medmrežja";
filter.legend="Smernica dostopa";
filter.restore="Obnovi privzete nastavitve";
filter.pol="Smernica";
filter.polname="Ime smernice";
filter.pcs="PCji";
filter.polallow="Dostop do medmrežja med izbranimi dnevi in urami.";
filter.legend2="Dnevi";
filter.time="Èasi";
filter.h24="24 ur";
filter.legend3="Zaprte storitve ";
filter.catchall="Ujemi vse protokole P2P";
filter.legend4="Zapiranje spletišè z URL naslovom";
filter.legend5="Zapiranje spletišè s kljuèno besedo";

filter.mess1="Zbriši smernico?";
filter.mess2="Izbrati morate vsaj dan.";
filter.mess3="Konèni èas mora biti veèji od zaèetnega.";

//help container
var hfilter = new Object();
hfilter.right1="Smernice dostopa do medmrežja:";
hfilter.right2="Doloèite lahko do 10 smernic dostopa. Kliknite <em>" + sbutton.del + "</em>, da zbrišete smernico, ali <em>" + sbutton.summary + "</em> za povzetek smernice.";
hfilter.right3="Status:";
hfilter.right4="Omogoèi ali onemogoèi smernico.";
hfilter.right5="Ime smernice:";
hfilter.right6="Smernici lahko doloèite ime.";
hfilter.right7="Dnevi:";
hfilter.right8="Doloèite dan v tednu, ko bo smernica uporabljena.";
hfilter.right9="Èasi:";
hfilter.right10="Vnesite èas, ko bo smernica uporabljena.";
hfilter.right11="Zaprte storitve:";
hfilter.right12="Zaprete lahko doloèene storitve. Kliknite <em>" + sbutton.filterSer + "</em> za spremembo nastavitev.";
hfilter.right13="Zapiranje spletišè z URL naslovom:";
hfilter.right14="Zaprete lahko doloèena spletišèa tako, da vnesete njihov URL.";
hfilter.right15="Zapiranje spletišè s kljuèno besedo:";
hfilter.right16="Zaprete lahko doloèena spletišèa tako, da vnesete kljuèno besedo vsebovano v njihovi spletni starni.";



// ******************************************* FilterSummary.asp *******************************************//

var filterSum = new Object();
filterSum.titl=" - Povtetek omejitev dostopa";
filterSum.h2="Povzetek smernice dostopa do medmrežja";
filterSum.polnum="Št.";
filterSum.polday="Èas dneva";



// ******************************************* Firewall.asp *******************************************//

var firewall = new Object();
firewall.titl=" - Požarni zid";
firewall.h2="Varnost";
firewall.legend="Zašèita z požarnim zidom";
firewall.firewall="SPI požarni zid";
firewall.legend2="Dodatni filtri";
firewall.proxy="Filter za proksi";
firewall.cookies="Filter za piškotke";
firewall.applet="Filter za Java aplete";
firewall.activex="Filter za ActiveX";
firewall.legend3="Prepreèi WAN zahtevo";
firewall.ping="Prepreèi anonimno zahtevo z medmrežja";
firewall.muticast="Filter za multicast";
filter.nat="Filter za NAT preusmeritev";
filter.port113="Filter za IDENT (vrata 113)";

//help container
var hfirewall = new Object();
hfirewall.right2="Omogoèi ali onemogoèi zašèito s SPI požarnim zidom.";



// ******************************************* Forward.asp *******************************************//

var prforward = new Object();
prforward.titl=" - Posredovanje razpona vrat";
prforward.h2="Posredovanje razpona vrat";
prforward.legend="Posredovanje";
prforward.app="Aplikacija";

//help container
var hprforward = new Object();
hprforward.right2="Nekatere aplikacije zahtevajo, da so doloèena vrata odprta za njihovo pravilno delovanje. \
	Primeri takih aplikacij so strežniki in nekatere igre. \
	Ko z medmrežja pride zahteva za doloèena vrata, bo usmerjalnik usmeril podatke do doloèenega raèunalnika. \
	Zaradi varnosti omejite posredovanje vrat samo natista vrata, ki ji uporabljate. ";


// ******************************************* ForwardSpec.asp *******************************************//

var pforward = new Object();
pforward.titl=" - Posredovanje vrat";
pforward.h2="Posredovanje vrat";
pforward.legend="Posredovanje";
pforward.app="Aplikacija";
pforward.from="Od vrat";
pforward.to="Do vrat";

//help container
var hpforward = new Object();
pforward.right2="Nekatere aplikacije zahtevajo, da so doloèena vrata odprta za njihovo pravilno delovanje. \
	Primeri takih aplikacij so strežniki in nekatere igre. \
	Ko z medmrežja pride zahteva za doloèena vrata, bo usmerjalnik usmeril podatke do doloèenega raèunalnika. \
	Zaradi varnosti omejite posredovanje vrat samo natista vrata, ki ji uporabljate. ";



// ******************************************* Hotspot.asp *******************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Hotspot portal";
hotspot.legend="Chillispot";
hotspot.hotspot="Chillispot";
hotspot.pserver="Glavni Radius Server IP/DNS";
hotspot.bserver="Nadomestni Radius Server IP/DNS";
hotspot.dns="DNS IP";
hotspot.url="Preusmerjeni URL";
hotspot.dhcp="DHCP vmesnik";
hotspot.radnas="Radius NAS ID";
hotspot.uam="UAM skrivnost";
hotspot.uamdns="UAM vsak DNS";
hotspot.allowuam="UAM dopušèeni";
hotspot.macauth="MACauth";
hotspot.option="Dodatne Chillispot možnosti";
hotspot.fon_chilli="Chillispot lokalno uporabniško upravljanje";
hotspot.fon_user="Seznam uporabnikov";
hotspot.http_legend="HTTP preusmeritev";
hotspot.http_srv="HTTP preusmeritev";
hotspot.http_ip="HTTP ciljni IP";
hotspot.http_port="HTTP ciljna vrata";
hotspot.http_net="HTTP izvorna mreža";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Ime prohoda";
hotspot.nocat_home="Damaèa stran";
hotspot.nocat_allowweb="Dovoljeni gostitelji svetovnega spleta";
hotspot.nocat_docroot="Korenski imenik dokumenta";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="Izloèi vrata";
hotspot.nocat_timeout="Èasovna omejitev prijave";
hotspot.nocat_verbose="Obširnost";
hotspot.nocat_route="Samo usmerjanje";
hotspot.smtp_legend="Preusmeritev SMTP";
hotspot.smtp_srv="Preusmeritev SMTP";
hotspot.smtp_ip="SMTP ciljni IP";
hotspot.smtp_net="SMTP izvorna mreža";
hotspot.shat_legend="Zero IP nastavitev";
hotspot.shat_srv="Zero IP nastavitev";
hotspot.shat_srv2="Zero IP nastavitev omogoèena";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik agent";
hotspot.sputnik_id="Sputnik server ID";
hotspot.sputnik_instant="Uporabi Sputnik Instant Setup";
hotspot.sputnik_express="Uporabi SputnikNet Express";
hotspot.sputnik_about="o Sputniku";



// ******************************************* Info.htm *******************************************//

var info = new Object();
info.titl=" - Info";
info.h2="Sistemske informacije";
info.wlanmac="Brezžièni MAC";
info.srv="Storitve";


// ******************************************* index_heartbeat.asp *******************************************//

var idx_h = new Object();
idx_h.srv="Heart Beat strežnik";
idx_h.con_strgy="Strategija povezovanja";
idx_h.max_idle="Poveži po potrebi: najveèji nedejavni èas";
idx_h.alive="Ohrani zvezo: èas ponovnega klicanja";



// ******************************************* index_l2tp.asp *******************************************//

var idx_l = new Object();
idx_l.srv="L2TP strežnik";



// ******************************************* index_pppoe.asp *******************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="Uporabi RP PPPoE";



// ******************************************* index_pptp.asp *******************************************//

var idx_pptp = new Object();
idx_pptp.srv="Uporabi DHCP";
idx_pptp.wan_ip="IP naslov";
idx_pptp.gateway="Prehod (PPTP strežnik)";
idx_pptp.encrypt="PPTP šifrirnje";



// ******************************************* index_static.asp *******************************************//

var idx_static = new Object();
idx_static.dns="Statièni DNS";



// ******************************************* index.asp *******************************************//

var idx = new Object();
idx.titl=" - Namestitev";
idx.h2="Namestitev medmrežja";
idx.h22="Namestitev brezžiènega omrežja";
idx.legend="Naèin dostopa do medmrežja";
idx.conn_type="Naèin dostopa";
idx.stp="STP";
idx.stp_mess="(onemogoèi za COMCAST ISP)";
idx.optional="Dodatne nastavitve (potrebno za nekatere ISPje)";
idx.mtu="MTU";
idx.h23="Nastavitev mreže";
idx.routerip="IP usmerjalnika";
idx.lanip="Lokalni IP naslov";
idx.legend2="WAN vrata";
idx.wantoswitch="dodeli WAN vrata k stikalu";
idx.legend3="Nastavitev èasa";
idx.timeset="Èasovna cona / Poletni èas";
idx.localtime="Uporabi krajevni èas";
idx.static_ip="Statièni IP";
idx.dhcp="Avtomatièna namestitev - DHCP";
idx.dhcp_legend="Nastavitev strežnika mrežnih naslovov (DHCP)";
idx.dhcp_type="Tip DHCP";
idx.dhcp_srv="DHCP strežnik";
idx.dhcp_fwd="DHCP posredovalnik";
idx.dhcp_start="Zaèetni IP naslov";
idx.dhcp_end="Konèni IP naslov";		//used in Status_Lan.asp
idx.dhcp_maxusers="Najveèje št. DHCP uporabnikov";
idx.dhcp_lease="Odjemalèev èas zakupa";
idx.dhcp_dnsmasq="Uporabi DNSMasq za DHCP";




//help container
var hidx = new Object();
hidx.right2="Ta nastavitev se pogosto uporablja za kabelske ponudnike (ISP).";
hidx.right4="Vnesi uporabniško ime dodeljeno od Vašega ISP.";
hidx.right6="Vnesi domensko ime dodeljeno od Vašega ISP.";
hidx.right8="To je naslov usmerjalnika.";
hidx.right10="To je maska podmrežja usmerjalnika.";
hidx.right12="Dovoli usmerjalniku, da upravlja z Vašimi IP naslovi.";
hidx.right14="Željeni zaèetni naslov.";
hidx.right16="Omejite lahko število naslovov, ki ji dodeli Vaš usmerjalnik.";
hidx.right18="Izberite Vašo èasovno cono in obdobje poletnega èasa. Usmerjalnik lahko uparablja lokalni èas ali èas UTC.";



// ******************************************* Join.asp *******************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Poveži";
join.mess1="Kot odjemalec ste se uspešno povezali s sledeèo mrežo: ";



// ******************************************* Log_incoming.asp *******************************************//

var log_in = new Object();
log_in.titl=" - Tabela vhodnega dnevnika";
log_in.h2="Tabela vhodnega dnevnika";
log_in.th_ip="Izvorni IP";
log_in.th_port="Št. naslovnih vrat";



// ******************************************* Log_outgoing.asp *******************************************//

var log_out = new Object();
log_out.titl=" - Tabela izhodnega dnevnika";
log_out.h2="Tabela izhodnega dnevnika";
log_out.th_lanip="LAN IP";
log_out.th_wanip="Naslovni URL/IP";
log_out.th_port="Storitev/št. vrat";



// ******************************************* Log.asp *******************************************//

var log = new Object();
log.titl=" - Dnevnik";
log.h2="Upravljanje dnevnika";
log.legend="Dnevnik";
log.lvl="Nivo dnevnika";
log.drop="Izvrženi";
log.reject="Zavrnjeni";
log.accept="Sprejeti";



// ******************************************* Management.asp *******************************************//

var management = new Object();
management.titl=" - Administracija";
management.h2="Upravljanje usmerjalnika";

management.psswd_legend="Geslo usmerjalnika";
management.psswd_user="Uporabniško ime usmerjalnika";
management.psswd_pass="Geslo usmerjalnika";
management.pass_conf="Ponovno vnesi za potrditev";

management.remote_legend="Oddaljeni dostop";
management.remote_gui="Upravljanje uporabniškega vmesnika";
management.remote_https="Uporabi HTTPS";
management.remote_guiport="Vrata uporabniškega vmesnika";
management.remote_ssh="Upravljanje SSH";
management.remote_sshport="SSH vrata";

management.web_legend="Dostop do uporabniškega vmesnika";
management.web_refresh="Samodejno osveževanje (v sekundah)";
management.web_sysinfo="Omogoèi Info stran";
management.web_sysinfopass="Info stran zašèitena z geslom";
management.web_sysinfomasq="Info stran: skrij MAC naslove";

management.boot_legend="Zagonska pavza";
management.boot_srv="Zagonska pavza";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.dsn_legend="DNSMasq";
management.dsn_srv="DNSMasq";
management.dsn_loc="Lokalni DNS";
management.dsn_opt="Dodatne DNS možnosti";

management.loop_legend="Povratna zanka";
management.loop_srv="Povratna zanka";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="NTP odjemalec";
management.ntp_srv="NTP";

management.rst_legend="Gumb za ponastavitev";
management.rst_srv="Gumb za ponastavitev";

management.routing_legend="Usmerjanje";
management.routing_srv="Usmerjanje";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="SecureOn geslo";
management.wol_mac="MAC naslovi<br/>( format: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="IPv6 podpora";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd omogoèen";
management.ipv6_radconf="Radvd nastavitev";

management.jffs_legend="JFFS2 podpora";
management.jffs_srv="JFFS2";
management.jffs_clean="Oèisti JFFS2";

management.lang_legend="Izbira jezika";
management.lang_srv="Jezik";
management.lang_bulgarian="bulgaršèina";
management.lang_tradchinese="trad.kitajšèina";
management.lang_croatian="hrvašèina";
management.lang_czech="èešèina";
management.lang_dutch="nizozemšèina";
management.lang_english="anglešèina";
management.lang_french="francošèina";
management.lang_german="nemšèina";
management.lang_italian="italijanšèina";
management.lang_brazilian="braziljšèina";
management.lang_slovenian="slovenšèina";
management.lang_spanish="španšèina";
management.lang_swedish="švedšèina";

management.net_legend="Nastavitve IP filtra (nastavite za P2P)";
management.net_port="Najveèje št. vrat";
management.net_tcptimeout="TCP èasovna omejitev (v sekundah)";
management.net_udptimeout="UDP èasovna omejitev (v sekundah)";

management.clock_legend="Overclocking";
management.clock_frq="Frekvenca";
management.clock_support="Ni podprto";

management.mmc_legend="podpora za MMC/SD kartice";
management.mmc_srv="MMC naprava";

management.samba_legend="Samba FS avtomatski priklop";
management.samba_srv="SMB datoteèni sistemm";
management.samba_share="Skupni imenik";
management.samba_stscript="Zaèetni skript";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="SIP vrata";
management.SIPatH_domain="SIP domena";

management.gui_style="Stil usmerjalnikovega uporabniškega vmesnika";



//help container
var hmanagement = new Object();
hmanagement.right1="Samodejno osveževanje:";
hmanagement.right2="Nastavite interval samodejnega osveževanja. 0 popolnoma onemogoèi samodejno osveževanje.";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

var portserv = new Object();
portserv.titl=" - Storitve vrat";
portserv.h2="Storitve vrat";



// ******************************************* QoS.asp *******************************************//

var qos = new Object();
qos.titl=" - Kakovost storitev (QoS)";
qos.h2="Kakovost storitev (QoS)";
qos.legend="QoS nastavitve";
qos.srv="Zaženi QoS";
qos.type="Paketni naèrt";
qos.uplink="Hitrost prenosa od vas (kbps)";
qos.dnlink="Hitrost prenosa k vam (kbps)";
qos.gaming="Optimiziraj za igre";
qos.legend2="Prioriteta storitev";
qos.prio_x="Exempt";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_s="Standard";
qos.prio_b="Bulk";
qos.legend3="Prioriteta maske omrežja";
qos.ipmask="IP/maska";
qos.maxrate_b="Maks. Kbits";
qos.maxrate_o="Maks. hitrost";
qos.legend4="MAC prioriteta";
qos.legend5="Prioriteta ethernet vrat";
qos.legend6="Privzeti nivo pasovne širine";
qos.bandwith="Pasovna širina v Kbits";

//help container
var hqos = new Object();
hqos.right1="Hitrost prenosa od vas:";
hqos.right2="Nastavite na 80%-95% (maks.) vaše skupnege hitrosti prenosa od vas.";
hqos.right3="Hitrost prenosa k vam:";
hqos.right4="Nastavite na 80%-100% (maks.) vaše skupne hitrosti prenosa k vam.";
hqos.right6="Nadzirate lahko hitrost prenosa glede na aplikacijo, ki uporablja pasovno širino.";
hqos.right8="Doloèite lahko prioriteto za ves promet z doloèenega IPja ali razpona IPjev.";
hqos.right10="Doloèite lahko hitrost za ves promet z doloèene naprave na Vaši mreži tako, da doloèite ime naprave, \
			doloèite prioriteto in vnesete MAC naslov.";
hqos.right12="Nadzirate lahko hitrost za ves promet glede na to na kateri fižièni port je vaša naprava prikljuèena. \
			Doloèite lahko prioriteto za naprave prikljuèene na LAN vrata 1 do 4";



// ******************************************* RouteTable.asp *******************************************//

var routetbl = new Object();
routetbl.titl=" - Usmerjalna tabela";
routetbl.h2="Seznam vnosov v usmerjalno tabelo";
routetbl.th1="Ciljni LAN IP";



// ******************************************* Routing.asp *******************************************//

var route = new Object();
route.titl=" - Usmerjanje";
route.h2="Napredno usmerjanje";
route.mod="Naèin";
route.bgp_legend="BGP nastavitve";
route.bgp_ip="Sosednji IP";
route.bgp_as="Sosednji AS#";
route.rip2_mod="RIP2 usmerjalnik";
route.ospf_mod="OSPF usmerjalnik";
route.gateway_legend="Dinamièno usmerjanje";
route.static_legend="Statièno usmerjanje";
route.static_setno="Izberi št. niza";
route.static_name="Ime smeri";
route.static_ip="Ciljni LAN IP";

//help container
var hroute = new Object();
hroute.right1="Naèin:";
hroute.right2="Èe usmerjalnik gosti Vašo povezavo v medmerežje, izberite naèin <em>Prehod</em>. Èe na Vaši mreži obstaja kak drug usmerjevalnik, izberite naèin <em>Usmerjevalnik</em>.";
hroute.right3="Izberite št. niza:";
hroute.right4="To je edinstvena številka, doloèite lahko do 20 smeri.";
hroute.right5="Ime smeri:";
hroute.right6="Vnesite ime, ki ga doloèite za smer.";
hroute.right7="Ciljni LAN IP:";
hroute.right8="To je oddaljeni gostitelj do katerage doloèate statièno smer.";
hroute.right9="Maska podmrežja:";
hroute.right10="Doloèi gostitelja in mrežo.";


// ******************************************* Site_Survey.asp *******************************************//

var survey = new Object();
survey.titl=" - Pregled podroèja";
survey.h2="Sosednja brezžièna omrežja";
survey.thjoin="Poveži";



// ******************************************* Services.asp *******************************************//

var service = new Object();

service.titl=" - Storitve";
service.h2="Upravljanje storitev";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Zaèni Kaid";
service.kaid_mac="MAC konzole: (konèati se mora z ;)";

//DHCPd
service.dhcp_legend="DHCP odjemalec";
service.dhcp_vendor="Nastavitev Vendorclass";
service.dhcp_legend2="DHCP strežnik";
service.dhcp_srv="DHCP demon";
service.dhcp_jffs2="Uporabi JFFS2 za bazo zakupov";
service.dhcp_domain="Uporabljena domena";
service.dhcp_landomain="LAN domena";
service.dhcp_option="Dodatne DHCPd opcije";

//pptp.webservices
service.pptp_legend="PPTP";
service.pptp_srv="PPTP strežnik";
service.pptp_client="IPji odjemalcev";
service.pptp_chap="CHAP-skrivnosti";

//syslog.webservices
service.syslog_legend="Sistemski dnevnik";
service.syslog_srv="Syslogd";
service.syslog_ip="Oddaljeni strežnik";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//pptpd_client.webservices
service.pptpd_legend="PPTP odjemalec";
service.pptpd_option="PPTP opcije odjemalca";
service.pptpd_ipdns="Strežnikov IP ali ime DNS";
service.pptpd_subnet="Oddaljena podmreža";
service.pptpd_subnetmask="Oddaljena maska podmreže";
service.pptpd_encry="MPPE šifriranje";
service.pptpd_mtu="MTU";
service.pptpd_mru="MRU";
service.pptpd_nat="NAT";

//rflow.webservices
service.rflow_legend="RFlow / MACupd";
service.rflow_srv1="RFlow";
service.rflow_srv2="MACupd";

//pppoe-relay.webservices
service.pppoe_legend="PPPOE posredovanje";
service.pppoe_srv="posredovanje";

//snmp.webservices
service.snmp_legend="SNMP";
service.snmp_srv="SNMP";
service.snmp_loc="Lokacija";
service.snmp_contact="Zveza";
service.snmp_name="Ime";
service.snmp_read="RO skupnost";
service.snmp_write="RW skupnost";

//openvpn.webservices
service.vpn_legend="OpenVPN odjemalec";
service.vpn_srv="Zaèni OpenVPN";
service.vpn_ipname="Strežnikov IP/Ime";
service.vpn_mtu="TUN MTU nastavitev";
service.vpn_mru="TUN MTU dodatno";
service.vpn_mss="TCP MSS";
service.vpn_compress="Uporabi LZO stiskanje";
service.vpn_tunnel="Tunelski protokol";
service.vpn_srvcert="Javno strežniško potrdilo";
service.vpn_clicert="Javno odjemalèevo potrdilo";
service.vpn_clikey="Zasebni odjemalèev kljuè";

//sshd.webservices
service.ssh_legend="Varna ukazna lupina";
service.ssh_srv="SSHd";
service.ssh_password="Prijava z geslom";
service.ssh_key="Pooblašèeni kljuèi";



// ******************************************* Sipath.asp + cgi *******************************************//

var sipath = new Object();
sipath.titl=" - SiPath pregled";
sipath.phone_titl=" - Imenik";
sipath.status_titl=" - Status";



// ******************************************* Status_Lan.asp *******************************************//

var status_lan = new Object();
status_lan.titl=" - LAN status";
status_lan.h2="Lokalna mreža";
status_lan.legend="LAN status";
status_lan.h22="DHCP - Protokol za dinamicènego konfiguracijo gostitelja";
status_lan.legend2="DHCP status";
status_lan.legend3="DHCP odjemalci";

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="To je usmerjalnikov MAC naslov, kot je viden v Vaši lokalni mreži.";
hstatus_lan.right4="To je usmerjalnikov IP naslov, kot je viden v Vaši lokalni mreži.";
hstatus_lan.right6="To je maska podmreže, kadar jo usmerjalnik uporablja.";
hstatus_lan.right8="Kadar uporabljate usmerjalnik kot DHCP strežnik, je to vidno tukaj.";
hstatus_lan.right10="S klikom na MAC naslov boste dobili ime iz Organizationally Unique Identifier of the network interface (IEEE Standards OUI baze podatkov).";



// ******************************************* Status_Router.asp *******************************************//

var status_router = new Object();
status_router.titl=" - Status usmerjalnika";
status_router.h2="Informacije usmerjalnika";
status_router.legend="Sistem";
status_router.sys_model="Model usmerjalnika";
status_router.sys_firmver="Verzija strojne programske opreme";
status_router.sys_time="Trenutni èas";
status_router.sys_up="Neprekinjeno delovanje";
status_router.sys_load="Povpreèna obremenitev";
status_router.legend2="Procesor";
status_router.cpu="Model procesorja";
status_router.clock="Ura procesorja";
status_router.legend3="Spomin";
status_router.mem_tot="Skupaj na voljo";
status_router.mem_free="Prosto";
status_router.mem_used="Uporabljeno";
status_router.mem_buf="Medpomnilniki";
status_router.mem_cached="Predpmnilnik";
status_router.mem_active="Dejavno";
status_router.mem_inactive="Nedajvno";
status_router.legend4="Mreža";
status_router.net_maxports="IP filter najveèje število vrat";
status_router.net_conntrack="Aktivne IP povezave";
status_router.h22="Medmrežje";
status_router.legend5="Naèin povezave";
status_router.www_login="Naèin prijave";
status_router.www_loginstatus="Status prijave";

//help container
var hstatus_router = new Object();
hstatus_router.right2="To je ime usmerjalnika, kot ste ga doloèili v zavihku <i>Nemestitev</i>.";
hstatus_router.right4="To je usmerjalnikov MAC naslov, kot ga vidi Vaš ponudnik medmrežja.";
hstatus_router.right6="To je usmerjalnikova trenutna strojna programska oprema.";
hstatus_router.right8="To je trenutni èas.";
hstatus_router.right10="To je èas neprekinjenega delovanja.";
hstatus_router.right12="To so povpreène obremenitve v zadnji 1, 5 in 15 minutah.";
hstatus_router.right14="To kaže podatke, potrebne za prikljuèitev v medmrežje. \
				Te podatke ste vnesli v zavihku <em>Namestitev</em>. Tu lahko s klikom na <em>Poveži</em> ali <em>Prekini</em> povežete ali prekinete Vašo povezavo.";



// ******************************************* Status_SputnikAPD.asp *******************************************//

var status_sputnik = new Object();
status_sputnik.titl=" - Status Sputnik agenta";
status_sputnik.h2="Sputnik&reg; agent&trade;";
status_sputnik.manage="Upravljan od";
status_sputnik.license="SCC št. licence.";

//help container
var hstatus_sputnik = new Object();
hstatus_sputnik.right1="Status Sputnik agenta";
hstatus_sputnik.right2="Ta zaslon prikazuje status Sputnik agenta.";
hstatus_sputnik.right4="Sputnikov kontrolni center na katerega je ta usmerjalnik povezan.";
hstatus_sputnik.right6="Trenutni status Sputnik agenta.";
hstatus_sputnik.right8="Št. licence vašega Sputnik kontrolnega centra.";



// ******************************************* Status_Wireless.asp *******************************************//

var status_wireless = new Object();
status_wireless.titl=" - Brezžièni status";
status_wireless.h2="Brezžièno";
status_wireless.legend="Brezžièni status";
status_wireless.net="Mreža";
status_wireless.pptp="PPTP status";
status_wireless.legend2="Info o paketih";
status_wireless.rx="Sprejeti (RX)";
status_wireless.tx="Oddani (TX)";
status_wireless.h22="Brezžièna vozlišèa";
status_wireless.legend3="Brezžièni odjemalci";
status_wireless.signal_qual="kakovost signala";
status_wireless.wds="WDS vozlišèa";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="To je usmerjalnikov MAC naslov, kot je viden v Vaši lokalni brezžièni mreži..";
hstatus_wireless.right4="Tu je prikazan brezžièni naèin, kot je izbran v zavihku Brezžièno (Mešano, Samo-G, Samo-B ali Onemogoèeno) used by the network.";



// ******************************************* Triggering.asp *******************************************//

var trforward = new Object();
trforward.titl=" - Proženje vrat";
trforward.h2="Proženje vrat";
trforward.legend="Posredovanja";
trforward.trrange="Razpon proženja vrat";
trforward.fwdrange="Razpon posredovanja vrat";
trforward.app="Aplikacija";

//help container
var htrforward = new Object();
htrforward.right2="Vnesite ime aplikacije za proženje.";
htrforward.right4="Za vsako aplikacijo vnesite razpon proženja vrat. Preverite dokumentacijo za potrebne številke vrat.";
htrforward.right6="Za vsako aplikacijo vnesite razpon posredovanja vrat. Preverite dokumentacijo za potrebne številke vrat.";
htrforward.right8="Vnesite zaèetna vrata za razpon proženja ali posredovanja vrat.";
htrforward.right10="Vnesite konèna vrata za razpon proženja ali posredovanja vrat.";



// ******************************************* Upgrade.asp *******************************************//

var upgrad = new Object();
upgrad.titl=" - Posodabljanje strojne programske opreme";
upgrad.h2="Upravljanje s strojno programsko opremo";
upgrad.legend="Posodabljanje strojne programske opreme";
upgrad.info1="Po posodobitvi ponastavi na";
upgrad.resetOff="Brez ponastavitve";
upgrad.resetOn="Privzete nastavitve";
upgrad.file="Prosim izberite datoteko za posodobitev";
upgrad.warning="O P O Z O R I L O";
upgrad.mess1="Posodabljanje lahko traja nekaj minut.<br />Ne izljuèujte usmerjalnika in ne pritiskajte na gumb za ponastavitev!";

//help container
var hupgrad = new Object();
hupgrad.right2="Kliknite na gumb <em>Prebrskaj...</em> za izbiro datoteke za posodobitev.<br /><br /> \
			Kliknite na gumb <em>Posodobi</em> za zaèetek posodabljanja. Posodobitve ne smete prekiniti.";



// ******************************************* UPnP.asp *******************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Posredovanja";
upnp.legend2="Konfiguracija UPnP";
upnp.serv="UPnP storitev";
upnp.clear="Ob zagonu izbriši posredovanja";
upnp.url="Pošlji predstavitveni URL";
upnp.msg1="Kliknite za izbris zakupov";
upnp.msg2="Izbriši vse vnose?";


//help container
var hupnp = new Object();
hupnp.right2="Kliknite na koš za izbris posameznega vnosa.";
hupnp.right4="Dovoli aplikacijam, da samostojno nastavijo posredovanja.";



// ******************************************* VPN.asp *******************************************//

var vpn = new Object();
vpn.titl=" - VPN";
vpn.h2="Navidezno zasebno omrežje (VPN)";
vpn.legend="VPN prehajanje";
vpn.ipsec="IPSec prehajanje";
vpn.pptp="PPTP prehajanje";
vpn.l2tp="L2TP prehajanje";

//help container
var hvpn = new Object();
hvpn.right1="Izberete lahko IPSec, PPTP in/ali L2TP prehajanje, da se lahko Vaše naprave povežejo v VPN.";


// ******************************************* Vlan.asp *******************************************//

var vlan = new Object();
vlan.titl=" - Virtualni LAN";
vlan.h2="Virtualno loaklno omrežje (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Dodeli k<br />Mostu";
vlan.tagged="Znaèka";
vlan.negociate="Samodejno pogajanje";
vlan.aggregation="Združitev zveze<br>na vratih 3 & 4";
vlan.trunk="Povezava";


// ******************************************* WEP.asp *******************************************//

var wep = new Object();
wep.defkey="Privzeti oddajni kljuè";
wep.passphrase="Geslo";



// ******************************************* WOL.asp *******************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Razpoložljivi gostitelji";
wol.legend2="WOL naslovi";
wol.legend3="Izhod";
wol.legend4="Roèni WOL";
wol.enable="Omogoèi WOL?";
wol.add_wol="Dodaj WOL gostitelja";
wol.restore="Obnovi privzete nastavitve";
wol.mac="MAC naslov(i)";
wol.broadcast="Mrežni broadcast";
wol.udp="UDP vrata";
wol.msg1="Kliknite za odstranitev WOL gostitelja";

//help container
var hwol = new Object();
hwol.right2="Ta stran omogoèa da <em>zbudite</em> na vaši lokalni mreži (t.j. prikljuèene na Vaš usmerjalnik).";
hwol.right4="MAC naslove vnesite v formatu xx:xx:xx:xx:xx:xx (t.j. 01:23:45:67:89:AB)";
hwol.right6="IP naslov je ponavadi broadcast naslov za lokalno mrežo.Lahko pa je oddaljeni naslov, èe gostitelj ni prikljuèenj na VAšo lokalno mrežo."



// ******************************************* WanMAC.asp *******************************************//

var wanmac = new Object();
wanmac.titl=" - Kloniranje MAC naslova";
wanmac.h2="Kloniranje MAC naslova";
wanmac.legend="Kloniranje MAC";
wanmac.wan="Kloniraj WAN MAC";
wanmac.wlan="Kloniraj brezžièni MAC";

//help container
var hwanmac = new Object();
hwanmac.right2="nekateri ponudniki medmerežja zahtevajo, da prijavite Vaš MAC naslov. \
			Èe ne želite prijaviti novega MAC naslova lahko klonirate naslov, ki je že prijavljen pri Vašem ponudniku medmerežja.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

var wpa = new Object();
wpa.titl=" - Brezžièna zašèita";
wpa.h2="Brezžièna zašèita";
wpa.legend="Brezžièno šifriranje";
wpa.auth_mode="Mrežno overovljenje";
wpa.psk="WPA predhodno deljeni kljuè";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="WPA interval izmenjave kljuèev";
wpa.rekey="Interval izmenjave kljuèev (v sekundaj)";
wpa.radius_ipaddr="RADIUS naslov strežnika";
wpa.radius_port="RADIUS vrata strežnika";
wpa.radius_key="RADIUS kljuè";
wpa.algorithms="WPA algoritm";
wpa.shared_key="WPA deljeni kljuè";
wpa.rekeyInt="Interval izmenjave kljuèev";

//help container
var hwpa = new Object();
hwpa.right1="Varnostni naèin:";
hwpa.right2="Izberete lahko med: Onemogoèeno, WEP, WPA predhodno deljeni kljuè, WPA RADIUS, or RADIUS. Vse naprave na Vaši mreži morajo uporabljanti isti naèin.";



// ******************************************* WL_FilterTable.asp *******************************************//

var wl_filter = new Object();
wl_filter.titl=" - Seznam filtra MAC naslovov";
wl_filter.h2="Seznam filtra MAC naslovov";
wl_filter.h3="Vnesi MAC naslov v tem formatu&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";



// ******************************************* WL_ActiveTable.asp *******************************************//

var wl_active = new Object();
wl_active.titl=" - Seznam MAC aktivnih brezžiènih odjemalcev";
wl_active.h2="Seznam MAC brezžiènih odjemalcev";
wl_active.h3="Omogoèi MAC filter";
wl_active.active="Aktivni PCji";
wl_active.inactive="Neaktivni PCji";



// ******************************************* Wireless_WDS.asp *******************************************//

var wds = new Object();
wds.titl=" - WDS";
wds.h2="Brezžièni distribucijski sistem (WDS)";
wds.legend="WDS nastavitve";
wds.label="Leni WDS";
wds.label2="WDS podmreža";
wds.wl_mac="Brezžièni MAC";
wds.lazy_default="Privzeto: Onemogoèeno";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="Podmreža";
wds.legend2="Dodatne opcije";



// ******************************************* Wireless_radauth.asp *******************************************//

var radius = new Object();
radius.titl=" - Radius";
radius.h2="Remote Authentication Dial-In User Service";
radius.legend="Radius";
radius.label="MAC Radius odjemalec";
radius.label2="MAC format";
radius.label3="Radius strežnikov IP";
radius.label4="Radius odjemalèeva vrata";
radius.label5="Najveèje št. neoverovljenih uporabnikov";
radius.label6="Format gesla";
radius.label7="RADIUS deljena skrivnost";
radius.label8="razveljavi Radius, èe strežnik ni na voljo";
radius.mac="MAC";



// ******************************************* Wireless_MAC.asp *******************************************//

var wl_mac = new Object();
wl_mac.titl=" - MAC filter";
wl_mac.h2="Brezžièni filter MAC";
wl_mac.legend="MAC filter";
wl_mac.label="Uporabi filter";
wl_mac.label2="Naèin filtra";
wl_mac.deny="Prepreèi PCjem na seznamu dostop do brezžiènega omrežja";
wl_mac.allow="Dovoli PCjem na seznamu dostop do brezžiènega omrežja";



// ******************************************* Wireless_Basic.asp *******************************************//

var wl_basic = new Object();
wl_basic.titl=" - Brezžièno";
wl_basic.h2="Brezžièno";
wl_basic.legend="Osnovne nastavitve";
wl_basic.label="Brezžièni naèin";
wl_basic.label2="Naèin brezžiène mreže";
wl_basic.label3="Ime brezžiène mreže (SSID)";
wl_basic.label4="Brezžièni kanal";
wl_basic.label5="Brezžièno oddajanje SSID";
wl_basic.label6="Obmoèje obèutljivosti (ACK uèasenje)";
wl_basic.ap="AP";
wl_basic.client="Odjemalec";
wl_basic.clientBridge="Odjemalec-most";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="Mešano";
wl_basic.b="Samo-B";
wl_basic.g="Samo-G";
wl_basic.sensitivity="Privzeto: 20000 metrov";

//help container
var hwl_basic = new Object();
hwl_basic.right2="Èe želite prepovedati vstop odjemalcem G, izberite <em>Samo-B</em> naèin. Èe želite izkljuèiti brezžièno omrežje, izberite <em>Onemogoèie</em>.";
hwl_basic.right3="Obmoèje obèutljivosti: ";
hwl_basic.right4="Prilagodi ACK uèasenje. 0 popolnoma onemogoèi ACK uèasenje.";



// ******************************************* Wireless_Advanced.asp *******************************************//

var wl_adv = new Object();
wl_adv.titl=" - Napredne brezžiène nastavitve";
wl_adv.h2="Napredne brezžiène nastavitve";
wl_adv.legend="Napredne nastavitve";
wl_adv.legend2="Nastavitve za podporo brezžiènim multimedijem";
wl_adv.label="Naèin overovljanja";
wl_adv.label2="Osnovna hitrost";
wl_adv.label3="Hitrost oddajanja";
wl_adv.label4="CTS naèin zašèite";
wl_adv.label5="Frame Burst";
wl_adv.label6="Signalni interval";
wl_adv.label7="DTIM interval";
wl_adv.label8="Prag drobitve";
wl_adv.label9="RTS prag";
wl_adv.label10="Najveèje št. pridruženih odjemalcev";
wl_adv.label11="AP osamitev";
wl_adv.label12="Oddajna antena";
wl_adv.label13="Sprejemna antena";
wl_adv.label14="Preambula";
wl_adv.reference="Referenca šuma";
wl_adv.label15="Oddajna moè";
wl_adv.label16="Afterburner";
wl_adv.label17="Brezžièni dostop do uporabniškega vmesnika";
wl_adv.label18="WMM podpora";
wl_adv.label19="Brez potrditve";
wl_adv.table1="EDCA AP parameteri (AP do odjemalca)";
wl_adv.col1="CWmin";
wl_adv.col2="CWmaks";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="Upravniško prisiljen";
wl_adv.row1="Ozadje";
wl_adv.row2="Najboljši trud";
wl_adv.row3="Slika";
wl_adv.row4="Glas";
wl_adv.table2="EDCA STA parametri (odjemalec do AP)";
wl_adv.lng="Dolga"; 					//************* don't use .long ! *************
wl_adv.shrt="Kratka"; 				//************* don't use .short ! **************

//help container
var hwl_adv = new Object();
hwl_adv.right1="Naèin overovljanja:";
hwl_adv.right2="Izberete lahko med Avto ali Deljeni kjluè. Overovljanje z deljenim kjluèem je bolj varno, toda vse naprave na Vaši mreži morajo podpirati ta naèin.";



// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

var fail = new Object();
fail.mess1="Vnešene vrednosti so neveljavne. Prosim poskusite znova.";
fail.mess2="Posodabljanje ni uspelo.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="Nastavitve so shranjene.";
success.restore="Nastavitve so obnovljene.<br/>Ponovni zagon naprave. Prosim, poèakajte trenutek...";
success.upgrade="Posodabljanje je uspelo.<br/>Ponovni zagon naprave. Prosim, poèakajte trenutek...";
success.success_noreboot="Nastavitve so uspele.";
success.success_reboot=success.success_noreboot + "<br />Ponovni zagon naprave. Prosim, poèakajte trenutek...";

success.alert_reset="Vse nastavitve so bile nastavljene na privezete vrednosti.<br /><br />";
success.alert1="Pred ponovnim priklopom prosim preverite nasledje:";
success.alert2="Èe ste spremenili usmerjalnikov IP naslov, morate tudi sprostiti/obnoviti odjemalèeve IP naslove.";
success.alert3="Èe ste povezani brezžièno, se morate ponovno povezati v omrežje, in nato klikniti <em>Nadaljuj</em>.";







// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

var dhcp = new Object();
dhcp.titl=" - DHCP tabela aktivnih IP";
dhcp.h2="DHCP tabela aktivnih IP";
dhcp.server="DHCP strežnikov IP naslov :";
dhcp.tclient="Odjemalèevo gostiteljevo ime";

var donate = new Object();
donate.mb="Darujete lahko tudi preko Moneybookerjevega raèuna mb@dd-wrt.com";
