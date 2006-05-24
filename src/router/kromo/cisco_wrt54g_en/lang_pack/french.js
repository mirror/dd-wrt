//////////////////////////////////////////////////////////////////////////////////////////////
//				French translation DD-WRT V23 SP1 by Botho 17/05/2006						//
//////////////////////////////////////////////////////////////////////////////////////////////


// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var donate = new Object();
donate.mb="Vous pouvez également faire un don sur le compte Moneybookers : mb@dd-wrt.com";

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
share.none2="aucune";
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
share.deflt="Défaut";
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
share.intrface="Interface";
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
share.err="erreur";
share.errs="erreurs";


var sbutton = new Object();
sbutton.save="Enregistrer";
sbutton.saving="Enregistré";
sbutton.cmd="Patientez ...";
sbutton.cancel="Annuler";
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
sbutton.runcmd="Exécutez les commandes";
sbutton.startup="Sauver Démarrage";
sbutton.firewall="Sauver Pare-feu";
sbutton.wol="Réveiller";
sbutton.add_wol="Ajouter l'hôte";
sbutton.manual_wol="Réveil manuel";
sbutton.summary="Résumé";
sbutton.filterIP="Liste des PC";
sbutton.filterMac="Liste de filtrage MAC";
sbutton.filterSer="Ajouter/Editer un Service";
sbutton.reboot="Redémarrer";
sbutton.help="Aide";
sbutton.wl_client_mac="Liste des adresses MAC";
sbutton.update_filter="Liste des filtres";
sbutton.join="Rejoindre";
sbutton.log_in="Connexions entrantes";
sbutton.log_out="Connexions sortantes";
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
errmsg.err0="Vous devez saisir un nom d'utilisateur.";
errmsg.err1="Vous devez saisir un nom de Routeur.";
errmsg.err2="Hors de l'intervalle, merci d'ajuster l'adresse IP de début ou le nombre d'utilisateurs.";
errmsg.err3="Vous devez selectionner au moins un jour."
errmsg.err4="L'heure de fin doit être supérieure à l'heure de départ.";
errmsg.err5="La longueur de l'adresse MAC est incorrecte.";
errmsg.err6="Vous devez saisir un mot de passe.";
errmsg.err7="Vous devez saisir un nom d'hôte.";
errmsg.err8="Vous devez saisir une adresse IP ou un nom de domaine.";
errmsg.err9="Adresse IP invalide.";
errmsg.err10="La confirmation du mot de passe ne correspond pas. Merci de saisir à nouveau le mot de passe.";
errmsg.err11="Aucun espace n'est permis dans le mot de passe";
errmsg.err12="Vous devez saisir une commande à exécuter.";
errmsg.err13="La mise à jour a échoué.";
errmsg.err45="Non disponible en HTTPS ! Merci de vous connecter en mode HTTP.";
errmsg.err46="Non disponible en HTTPS.";


//common.js error messages
errmsg.err14=" la valeur est en dehors de l'intervalle [";
errmsg.err15="L'adresse MAC WAN est en dehors de l'intervalle [00 - ff].";
errmsg.err16="Le deuxième caractère de l'adresse MAC doit être un nombre pair : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="L'adresse MAC est incorrecte.";
errmsg.err18="La longueur de l'adresse MAC est incorrecte.";
errmsg.err19="L'adresse MAC ne peut pas être identique à l'adresse de diffusion."
errmsg.err20="Sairir l'adresse MAC au format (xx:xx:xx:xx:xx:xx).";
errmsg.err21="Format d'adresse MAC invalide.";
errmsg.err22="L'adresse MAC WAN est incorrecte.";
errmsg.err23="Valeur hexadécimale invalide ";
errmsg.err24=" trouvée dans l'adresse MAC ";
errmsg.err25="La valeur de la clé est incorrecte.";
errmsg.err26="La longueur de la clé est incorrecte.";
errmsg.err27="Masque de sous-réseau incorrect.";
errmsg.err28=" a un caractère illégal, doit être [ 0 - 9 ].";
errmsg.err29=" a un code ascii illégal."
errmsg.err30=" a des nombres hexadécimaux illégaux.";
errmsg.err31=" a une valeur illégale.";
errmsg.err32="L'adresse IP et la passerelle n'utilisent pas le même masque de sous réseau.";
errmsg.err33="L'adresse IP et la passerelle ne peuvent pas être identiques.";
errmsg.err34=" ne doit pas contenir un espace.";

//Wol.asp error messages
errmsg.err35="Vous devez saisir une adresse MAC."
errmsg.err36="Vous devez saisir une adresse réseau de diffusion.";
errmsg.err37="Vous devez saisir un port UDP.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Merci de saisir une clé partagée !";
errmsg.err39="Clé invalide, doit contenir entre 8 et 63 caractères ASCII ou 64 nombres hexadécimaux"
errmsg.err40="Vous devez saisir une clé pour la Clé ";
errmsg.err41="Longueur de clé invalide ";
errmsg.err43="Saisissez de nouveau l'intervalle.";

//config.asp error messages
errmsg.err42="Merci de sélectionner une fichier de configuration à restaurer.";

//WL_ActiveTable.asp error messages
errmsg.err44="Le nombre total d'adresses MAC filtrées ne peut pas dépasser 128.";

//Site_Survey.asp error messages
errmsg.err47="SSID invalide.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS n'est pas compatible avec la configuration courante du routeur. \
	Merci de vérifier les points suivants :\n * Le routeur doit fonctionner en mode AP \n * WPA2 n'est pas supporté sous WDS \n * Le mode sans fil B-Only n'est pas supporté sous WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius n'est seulement diponible qu'en mode AP.";

//Wireless_Basic.asp error messages
errmsg.err50="Vous devez saisir un SSID.";

// Management.asp error messages
errmsg.err51="Le routeur dispose actuellement de son mot de passe par défaut. \
	Par mesure de sécurité, vous devez changer le mot de passe avant de pouvoir activer la fonctionnalité de gestion distante. \
	Cliquez sur le bouton OK pour changer le mot de passe. Cliquez sur le bouton Annuler pour laisser la fonctionnalité de gestion distante désactivée.";
errmsg.err52="Le mot de passe de confirmation ne correspond pas.";

// Port_Services.asp error messages
errmsg.err53="Après avoir terminé toutes les actions, cliquez sur Valider pour enregistrer les modifications.";
errmsg.err54="Vous devez saisir un nom de service.";
errmsg.err55="Ce nom de service existe déjà.";

// QoS.asp error messages
errmsg.err56="La valeur du port est en dehors de l'intervalle [0 - 65535].";

// Routing.asp error messages
errmsg.err57="Effacer cette entrée ?";

// Status_Lan.asp error messages
errmsg.err58="Cliquez pour effacer le bail.";

//Status_Wireless.asp error messages
errmsg.err59="Non disponible ! Merci d'activer le réseau sans fil.";

//Upgrade.asp error messages
errmsg.err60="Veuillez sélectionner le fichier de mise à jour du routeur.";
errmsg.err61="Fichier incorrect.";

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
bmenu.adminDiag="Shell";
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
alive.titl=" - Maintenir Actif";
alive.h2="Maintenir Actif";
alive.legend="Redémarrage Programmé du Routeur";
alive.sevr1="Redémarrage programmé";
alive.hour="A une heure définie";
alive.legend2="WDS/Connection Watchdog";
alive.sevr2="Watchdog";
alive.IP="Adresses IP";
alive.legend3="Proxy/Connection Watchdog";
alive.sevr3="Proxy Watchdog";
alive.IP2="Adresse IP du proxy";
alive.port="Port du proxy";

//help container
var halive = new Object();
halive.right2="Choisissez quand le routeur doit redémarrer. <em>Cron</em> doit être activé dans l'onglet gestion.";
halive.right4="Un maximum de 3 IPs séparées par un <em>ESPACE</em> est autorisé.<BR/>Le format des IPs est xxx.xxx.xxx.xxx.";



// ******************************************* config.asp *******************************************//

var config = new Object();
config.titl=" - Sauvegarde & Restauration";
config.h2="Sauvegarde";
config.legend="Sauvergarder la Configuration";
config.mess1="Cliquez sur le bouton <em>\"" + sbutton.backup + "\"</em> pour enregistrer dans un fichier sur votre ordinateur les paramètres de configuration du routeur.";
config.h22="Restauration";
config.legend2="Restaurer les Réglages";
config.mess2="Veuillez sélectionner un fichier de sauvegarde à restaurer";
config.mess3="A T T E N T I O N";
config.mess4="Ne restaurer que des fichiers sauvegardés par cette version de micrologiciel.<br />Ne restaurer pas un fichier qui n'aurait pas été créé par cette interface !";

//help container
var hconfig = new Object();
hconfig.right2="Vous devriez sauvegarder votre configuration courante au cas où vous auriez besoin de réinitialiser votre routeur à ses paramètres usine.<br /><br />Cliquez sur le bouton <em>\"" + sbutton.backup + "\"</em> pour enregistrer la configuration actuelle.";
hconfig.right4="Cliquez sur le bouton <em>\"Parcourir...\"</em> pour sélectionner le fichier de configuration à restaurer.<br /><br />Cliquez sur le bouton <em>\"" + sbutton.restore + "\"</em> pour écraser la configuration courante par celle du fichier de configuration.";



// ******************************************* DDNS.asp *******************************************//

var ddns = new Object();
ddns.titl=" - DNS Dynamique"
ddns.h2="Dynamic Domain Name System (DDNS)";
ddns.legend="DNS Dynamique";
ddns.srv="Service DDNS";
ddns.emailaddr="Adresse E-mail";
ddns.typ="Type";
ddns.dynamic="Dynamique";
ddns.custom="Personnalisé";
ddns.wildcard="Wildcard";
ddns.statu="Etat";

var ddnsm = new Object();
ddnsm.dyn_strange="Erreur inconnue. Vérifier que vous êtes connecté au bon serveur.";
ddnsm.dyn_good="DDNS a été mis à jour avec succès.";
ddnsm.dyn_noupdate="Aucune mise à jour nécessaire pour le moment.";
ddnsm.dyn_nohost="Nom d&#39;hôte inconnu.";
ddnsm.dyn_notfqdn="Nom d&#39;hôte incorrect.";
ddnsm.dyn_yours="Ce nom d&#39;hôte ne vous appartient pas.";
ddnsm.dyn_abuse="Hôte bloqué pour abus.";
ddnsm.dyn_nochg="Adresse IP inchangée depuis la dernière mise à jour.";
ddnsm.dyn_badauth="L&#39;authentification a échoué (nom d&#39;utilisateur ou mot de passe incorrect).";
ddnsm.dyn_badsys="Paramètre système incorrect.";
ddnsm.dyn_badagent="Utilisateur bloqué.";
ddnsm.dyn_numhost="Trop ou pas assez d&#39;hôte trouvés";
ddnsm.dyn_dnserr="Erreur interne DNS.";
ddnsm.dyn_911="Erreur inattendue 911.";
ddnsm.dyn_999="Erreur inattendue 999.";
ddnsm.dyn_donator="A feature requested is only available to donators, please donate.";
ddnsm.dyn_uncode="Erreur inconnue.";

ddnsm.tzo_good="Mise à jour terminée avec succès.";
ddnsm.tzo_noupdate="Aucune mise à jour nécessaire pour le moment.";
ddnsm.tzo_notfqdn="Nom d&#39;hôte incorrect.";
ddnsm.tzo_notmail="Adresse E-mail invalide.";
ddnsm.tzo_notact="Action invalide.";
ddnsm.tzo_notkey="Clé invalide.";
ddnsm.tzo_notip="Adresse IP invalide.";
ddnsm.tzo_dupfqdn="Nom d&#39;hôte incorrect.";
ddnsm.tzo_fqdncre="Ce nom d&#39;hôte ne vous appartient pas.";
ddnsm.tzo_expired="Le compte utilisateur est expiré.";
ddnsm.tzo_error="Erreur inconnue.";

ddnsm.zone_701="Zone is not set up in this account";
ddnsm.zone_702="Echec de la mise à jour.";
ddnsm.zone_703="One of either parameters <em>zones</em> or <em>host</em> are required";
ddnsm.zone_704="Zone must be a valid <em>dotted</em> internet name";
ddnsm.zone_705="Zone cannot be empty";
ddnsm.zone_707="Duplicate updates for the same host/ip, adjust client settings";
ddnsm.zone_201="No records need updating";
ddnsm.zone_badauth="Authorization fails (username or passwords)";
ddnsm.zone_good="ZoneEdit is updated successfully";
ddnsm.zone_strange="Strange server response, are you connecting to the right server ?";

ddnsm.all_closed="Le serveur DDNS est actuellement fermé.";
ddnsm.all_resolving="Résolution du nom de domaine en cours ...";
ddnsm.all_errresolv="La résolution du nom de domaine a échoué.";
ddnsm.all_connecting="Connection en cours...";
ddnsm.all_connectfail="La connection au serveur a échoué";
ddnsm.all_disabled="DDNS est désactivé";
ddnsm.all_noip="Connection Internet non détectée";

//help container
var hddns = new Object();
hddns.right2="DDNS vous permet d'attribuer un nom de domaine et d'hôte fixe à une adresse IP Internet dynamique.\
	Cela peut s'avérer utile si vous hébergez votre propre site Web, un serveur FTP ou tout autre type de serveur \
	derrière le routeur. <br /><br />Avant d'opter pour cette fonctionnalité, vous devez souscrire à un service DDNS auprès de fournisseurs spécialisés, \
	tels que DynDNS.org, TZO.com ou ZoneEdit.com.";



// ******************************************* Diagnostics.asp *******************************************//

var diag = new Object();
diag.titl=" - Shell";
diag.h2="Commandes Shell";
diag.legend="Invite de commandes";
diag.cmd="Commande(s)";
diag.startup="Démarrage";
diag.firewall="Pare-feu";

//help container
var hdiag = new Object();
hdiag.right2="Vous pouvez lancer des lignes de commandes par le biais de l'interface web. \
	Saisissez dans la zone de texte les commandes à exécuter et cliquez sur le bouton <em>\"" + sbutton.runcmd + "\"</em> pour les soumettre.";



// ******************************************* DMZ.asp *******************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Zone démilitarisée (DMZ)";
dmz.legend="DMZ";
dmz.serv="DMZ";
dmz.host="Adresse IP de l'hôte de la DMZ";


//help container
var hdmz = new Object();
hdmz.right2="L'activation de cette option va exposer l'hôte à Internet. Tous les ports vont être accessibles depuis Internet et redirigés vers cette adresse IP (non recommandé).";



// ******************************************* Factory_Defaults.asp *******************************************//

var factdef = new Object();
factdef.titl=" - Paramètres usine";
factdef.h2="Paramètres usine";
factdef.legend="Réinitialiser les réglages du routeur";
factdef.restore="Réinitialiser les paramètres usine";

factdef.mess1="Attention ! Si vous cliquez sur OK, le routeur va réinitialiser les paramètres usine et tous les réglages seront effacés.";

//help container
var hfactdef = new Object();
hfactdef.right1="Cliquez sur <em>Oui</em> pour rétablir les valeurs par défaut de tous les paramètres de configuration, puis \
	cliquez sur <em>\"" + sbutton.save + "\"</em>. Tous les paramètres enregistrés précédemment seront \
	perdus une fois les paramètres usine restaurés. Par défaut, cette fonctionnalité est désactivée.";



// ******************************************* FilterIP%AC.asp *******************************************//

var filterIP = new Object();
filterIP.titl=" - Liste des Ordinateurs";
filterIP.h2="Liste des Ordinateurs";
filterIP.h3="Saisissez l'adresse MAC des ordinateurs au format: xx:xx:xx:xx:xx:xx";
filterIP.h32="Saisissez l'adresse IP des ordinateurs";
filterIP.h33="Saisissez l'intervalle d'adresses IP des ordinateurs";
filterIP.ip_range="Intervalle IP";



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
firewall.titl=" - Pare-Feu";
firewall.h2="Sécurité";
firewall.legend="Protection par Pare-Feu";
firewall.firewall="Pare-Feu SPI";
firewall.legend2="Filtres Supplémentaires";
firewall.proxy="Filtre Proxy";
firewall.cookies="Filtre Cookies";
firewall.applet="Filtre Applets Java";
firewall.activex="Filtre ActiveX";
firewall.legend3="Blocage des requêtes WAN";
firewall.ping="Bloquer les requêtes Internet anonymes";
firewall.muticast="Filtre Multidiffusion";
filter.nat="Filtre de redirection NAT Internet";
filter.port113="Filtre IDENT (port 113)";

//help container
var hfirewall = new Object();
hfirewall.right1="Firewall Protection:";
hfirewall.right2="Activez cette fonctionnalité pour utiliser la technologie SPI (Stateful Packet Inspection) \
		et procéder à une vérification plus poussée des paquets de données infiltrant votre environnement réseau.";



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
info.ap="Points d'Access";


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
idx.dns_dnsmasq="Utiliser DNSMasq pour DNS";
idx.auth_dnsmasq="Autorité DHCP";




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
management.lang_chinese_traditional="chinois traditionnel";
management.lang_chinese_simplified="chinois simplifié";
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
qos.titl=" - Qualité de Service";
qos.h2="Qualité de Service (QoS)";
qos.legend="Configuration QoS";
qos.srv="QoS";
qos.type="Ordonanceur de Packet";
qos.uplink="Débit montant (kbps)";
qos.dnlink="Débit descendant (kbps)";
qos.gaming="Optimiser pour le jeu";
qos.legend2="Priorité des Services";
qos.prio_x="Exempt";
qos.prio_p="Premium";
qos.prio_e="Express";
qos.prio_s="Standart";
qos.prio_b="Bulk";
qos.legend3="Priorité Sous-réseau";
qos.ipmask="IP/Masque";
qos.maxrate_b="Max Kbits";
qos.maxrate_o="Max Rate";
qos.legend4="Priorité MAC";
qos.legend5="Priorité de Port Ethernet";
qos.legend6="Default Bandwith Level";
qos.bandwith="Bandwith in Kbits";

//help container
var hqos = new Object();
hqos.right1="Débit montant:";
hqos.right2="Set this to 80%-95% (max) of your total upload limit.";
hqos.right3="Débit descendant:";
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
survey.titl=" - Réseaux Sans Fil à Portée";
survey.h2="Réseaux Sans Fil à Portée";
survey.thjoin="Rejoindre le réseau";



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
service.vpn_certtype="nsCertType";
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
status_wireless.titl=" - Etat Sans Fil";
status_wireless.h2="Sans Fil";
status_wireless.legend="Etat Sans Fil";
status_wireless.net="Réseau";
status_wireless.pptp="Etat PPTP";
status_wireless.legend2="Information Paquets";
status_wireless.rx="Recus (RX)";
status_wireless.tx="Transmis (TX)";
status_wireless.h22="Noeuds Sans fil";
status_wireless.legend3="Clients Sans fil";
status_wireless.signal_qual="Qualité du Signal";
status_wireless.wds="Noueds WDS";

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
upgrad.mess1="Upgrading firmware may take a few minutes.<br />Do not turn off the power or press the reset button !";

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



// ******************************************* VPN.asp *******************************************//

var vpn = new Object();
vpn.titl=" - VPN";
vpn.h2="Réseau Privé Virtuel (VPN)";
vpn.legend="Intercommunication VPN";
vpn.ipsec="Interconnexion IPSec";
vpn.pptp="Interconnexion PPTP";
vpn.l2tp="Interconnexion L2TP";

//help container
var hvpn = new Object();
hvpn.right1="Utilisez les paramètres de cet onglet pour permettre à des tunnels VPN utilisant les protocoles IPSec, PPTP ou L2TP de traverser le pare-feu du routeur.";



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
fail.mess1="Les paramètres sont incorrects. Veuillez essayer de nouveau.";
fail.mess2="La mise à jour a échoué.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="Les paramètres ont été enregistrés avec succès.";
success.restore="Les paramètres ont été restaurés avec succès.<br/>Le routeur redémarre. Patientez un moment SVP ...";
success.upgrade="Mise à jour réussie.<br/>Le routeur redémarre. Patientez un moment SVP ...";
success.success_noreboot="Les paramètres ont été enregistrés avec succès.";
success.success_reboot=success.success_noreboot + "<br />Le routeur redémarre. Patientez un moment SVP ...";

success.alert_reset="Tous les paramètres ont été initialisés à leur valeur par défaut.<br /><br />";
success.alert1="Veuillez vérifier les points suivants avant de vous connecter de nouveau :";
success.alert2="Si l'adresse IP du routeur a changé, veuillez SVP renouveler les adresses IP de tous les clients connectés.";
success.alert3="Si vous êtes connecté au routeur par une connexion sans fil, veuillez SVP vous connecter de nouveau avant de cliquer sur le bouton <em>Continuer</em>.";
