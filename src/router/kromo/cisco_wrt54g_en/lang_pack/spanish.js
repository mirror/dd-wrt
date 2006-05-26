// Not working in IE & Opera ?
//************ Include the current language file after english.js ************//
//  var Head = document.getElementsByTagName('head')[0];
//  var head_inc = document.createElement('script');
//  head_inc.setAttribute('type','text/javascript');
//  head_inc.setAttribute('src',"lang_pack/language.js");
//  Head.appendChild(head_inc);

//////////////////////////////////////////////////////////////////////////////////////////////
//		English reference translation file - DD-WRT V23 SP1 by Botho 17/05/2006				//
//		Traducci&#243;n Espa&#241;ola del archivo de referencia - DD-WRT V23 SP1 por SuperWebon 25/05/2006	//
//////////////////////////////////////////////////////////////////////////////////////////////


// ******************************************* COMMON SHARE LABEL *******************************************//
var lang_charset = new Object();
lang_charset.set="iso-8859-1";

var share = new Object();
share.firmware="Firmware";
share.time="Hora";
share.interipaddr="Direc. IP de Internet";
share.more="M&#225;s...";
share.help="Ayuda";
share.enable="Enable";
share.enabled="Activado";
share.disable="Desactivar";
share.disabled="Desactivado";
share.usrname="Nombre Usuario";
share.passwd="Clave";
share.hostname="Host Name";
share.domainname="Nombre Dominio";
share.statu="Estado";
share.start="Inicio";
share.end="Fin";
share.proto="Protocolo";
share.ip="Direcci&#243;n IP";
share.mac="Direcci&#243;n MAC";
share.none="Ninguno";
share.none2="no";
share.both="Ambos";
share.del="Borrar";
share.remove="Eliminar";
share.descr="Descripci&#243;n";
share.from="Desde";
share.to="Hasta";
share.about="Acerca de";
share.everyday="Cada D&#237;a";
share.sun="Domingo";
share.sun_s="Dom";
share.sun_s1="D";
share.mon="Lunes";
share.mon_s="Lun";
share.mon_s1="L";
share.tue="Martes";
share.tue_s="Mar";
share.tue_s1="M";
share.wed="Mi&#233;rcoles";
share.wed_s="Mie";
share.wed_s1="X";
share.thu="Jueves";
share.thu_s="Jue";
share.thu_s1="J";
share.fri="Viernes";
share.fri_s="Vie";
share.fri_s1="V";
share.sat="S&#225;bado";
share.sat_s="Sab";
share.sat_s1="S";
share.expires="Expira";
share.yes="S&#237;";
share.no="No";
share.allow="Permitir";
share.deny="Denegar";
share.range="Rango";
share.use="Uso";
share.mins="Min.";
share.secs="Seg.";
share.routername="Nombre Router";
share.manual="Manual";
share.port="Puerto";
share.ssid="SSID";
share.channel="Canal";
share.rssi="Rssi";
share.signal="Se&#241;al";
share.noise="Ru&#237;do";
share.beacon="Radiofaro";
share.openn="Abierto";
share.dtim="DTIM";
share.rates="Tasa";
share.low="Bajo";
share.medium="Medio";
share.high="Alto";
share.option="Opciones";
share.rule="Regla";
share.lan="LAN";
share.point2point="Punto a Punto";
share.nat="NAT";
share.subnet="M&#225;scara Subred";
share.unmask="Desenmascarar";
share.deflt="Defecto";  //don't use share.default !!!
share.all="Todos";
share.auto="Auto";
share.right="Dcha";
share.left="Izda";
share.share_key="Clave Compartida";
share.inter="Intervalo (en segundos)";
share.srv="Nombre Servicio";
share.port_range="Rango de Puertos";
share.priority="Prioridad";
share.gateway="Puerta de Enlace";
share.intrface="Interfaz";  //don't use share.interface, Mozilla problem!!!
share.router="Router";
share.static_lease="Leases Est&#225;ticas";
share.srvip="IP Servidor";
share.localdns="DNS Local";
share.minutes="minutos";
share.oui="Busqueda OUI";
share.sttic="Est&#225;tico";
share.connecting="Conectando";
share.connect="Conectar";
share.connected="Conectado";
share.disconnect="Desconectar";
share.disconnected="Desconectado";
share.info="Informaci&#243;n";
share.state="Estado";
share.mode="Modo";
share.encrypt="Encriptaci&#243;n";
share.key="Clave";
share.wireless="Wi-Fi";
share.dhcp="DHCP";
share.styl="Estilo";
share.err="error";
share.errs="errores";


var sbutton = new Object();
sbutton.save="Grabar Config.";
sbutton.saving="Grabado";
sbutton.cmd="Ejecutando";
sbutton.cancel="Cancelar Cambios";
sbutton.refres="Actualizar";
sbutton.clos="Cerrar";
sbutton.del="Borrar";
sbutton.continu="Continuar";
sbutton.add="A&#241;adir";
sbutton.remove="Eliminar";
sbutton.modify="Modificar";
sbutton.deleted="Borrado";
sbutton.delall="Borrar Todos";
sbutton.autorefresh="Auto-Actualizar est&#225; Activado";
sbutton.backup="Backup";
sbutton.restore="Restaurar";
sbutton.cptotext="Copiar a &#225;rea de texto";
sbutton.runcmd="Ejecutar Comandos";
sbutton.startup="Grabar Arranque";
sbutton.firewall="Grabar Firewall";
sbutton.wol="Despertar";
sbutton.add_wol="A&#241;adir Host";
sbutton.manual_wol="Despertado Manual";
sbutton.summary="Sumario";
sbutton.filterIP="Editar Lista PCs";
sbutton.filterMac="Editar Lista Filtrado MAC";
sbutton.filterSer="A&#241;adir/Editar Servicio";
sbutton.reboot="Reiniciar Router";
sbutton.help="   Ayuda  ";
sbutton.wl_client_mac="Lista Clientes MAC Wi-Fi";
sbutton.update_filter="Actualizar Lista de Filtros";
sbutton.join="Unirse";
sbutton.log_in="LOG-Entrada";
sbutton.log_out="LOG-Salida";
sbutton.apply="Aplicar";
sbutton.edit_srv="A&#241;adir/Editar Servicio";
sbutton.routingtab="Ver Tabla Ruteo";
sbutton.wanmac="Obtener MAC actual del PC";
sbutton.dhcprel="Lanzar DHCP";
sbutton.dhcpren="Renovar DHCP";
sbutton.survey="Inspecci&#243;n de Sitios";
sbutton.upgrading="Actualizando";
sbutton.upgrade="Actualizar";
sbutton.preview="Previsualizar";


// ******************************************* COMMON ERROR MESSAGES  *******************************************//
var errmsg = new Object();
errmsg.err0="Debes introducir un Nombre de Usuario.";
errmsg.err1="Debes introducir un Nombre de Router.";
errmsg.err2="Fuera de rango, por favor ajusta la direcci&#243;n IP de inicio o los n&#250;meros de usuario.";
errmsg.err3="Debes elegir al menos un d&#237;a.";
errmsg.err4="La hora final debe ser posterior a la hora de inicio.";
errmsg.err5="La longitud de la direcci&#243;n MAC no es correcta.";
errmsg.err6="Debes introducir una Clave.";
errmsg.err7="Debes introducir un Nombre de Host.";
errmsg.err8="Debes introducir una direcci&#243;n IP o un Nombre de Dominio.";
errmsg.err9="Direcci&#243;n IP Ilegal en DMZ (DeMilitared Zone).";
errmsg.err10="La clave confirmada no coincide con la Clave introducida. Por favor, reintroduce la Clave.";
errmsg.err11="No est&#225;n permitidos los espacios en la Clave";
errmsg.err12="Debes introducir un comando a ejecutar.";
errmsg.err13="Las actualizaciones han fallado.";
errmsg.err45="¡No disponible en HTTPS! Por favor conecta en modo HTTP.";
errmsg.err46="No disponible en HTTPS";


//common.js error messages
errmsg.err14=" El valor est&#225; fuera de rango [";
errmsg.err15="La direcci&#243;n MAC de la WAN est&#225; fuera de rango [00 - ff].";
errmsg.err16="El segundo car&#225;cter de la MAC debe ser un n&#250;mero par : [0, 2, 4, 6, 8, A, C, E].";
errmsg.err17="La direcci&#243;n MAC no es correcta.";
errmsg.err18="La longitud de la direcci&#243;n MAC no es correcta.";
errmsg.err19="La direcci&#243;n MAC no puede ser la direcci&#243;n broadcast.";
errmsg.err20="Introducir la direcci&#243;n MAC en formato (xx:xx:xx:xx:xx:xx).";
errmsg.err21="Formato de direcci&#243;n MAC inv&#225;lido.";
errmsg.err22="La direcci&#243;n MAC de la WAN no es correcta.";
errmsg.err23="Valor hexadecimal inv&#225;lido ";
errmsg.err24=" encontrado en direcci&#243;n MAC ";
errmsg.err25="El valor de la clave no es correcto.";
errmsg.err26="La longitud de la clave no es correcta.";
errmsg.err27="M&#225;scara de subred ilegal.";
errmsg.err28=" tiene caracteres ilegales, deben ser [ 0 - 9 ].";
errmsg.err29=" tiene un c&#243;digo ASCII ilegal.";
errmsg.err30=" tiene d&#237;gitos hexadecimales ilegales.";
errmsg.err31=" valor es ilegal.";
errmsg.err32="Direcci&#243;n IP y la puerta de enlace no est&#225;n en la misma m&#225;scara de subred.";
errmsg.err33="La direcci&#243;n IP y la puerta de enlace no pueden ser iguales.";
errmsg.err34=" no est&#225; permitido que contenga espacios.";

//Wol.asp error messages
errmsg.err35="Debes introducir una direcci&#243;n MAC para funcionar.";
errmsg.err36="Debes introducir una broadcast de red para funcionar.";
errmsg.err37="Debes introducir un puerto UDP para funcionar.";

//WL_WPATable.asp error messages
//WPA.asp error messages
errmsg.err38="Por favor introduce una Clave Compartida!";
errmsg.err39="Clave incorrecta, debe estar entre los caracteres 8 y 63 del c&#243;digo ASCII o ser d&#237;gitos hexadecimales.";
errmsg.err40="Debes introducir una clave para el campo clave ";
errmsg.err41="Longitud incorrecta en la clave ";
errmsg.err43="Intervalo de cambio de clave";

//config.asp error messages
errmsg.err42="Por favor seleccione el archivo de config. a restaurar.";

//WL_ActiveTable.asp error messages
errmsg.err44="La comprobación total excede los 128";

//Site_Survey.asp error messages
errmsg.err47="SSID incorrecta.";

//Wireless_WDS.asp error messages
errmsg.err48="WDS no es compatible con la configuraci&#243;n actual del router. Por favor compruebe los siguientes puntos :\n * El modo inal&#225;mbrico debe ser AP (Punto Acceso) \n * WPA2 no est&#225; soportado bajo WDS \n * La Red Inal&#225;mbrica en modo S&#243;lo-B no est&#225; soportada bajo WDS";

//Wireless_radauth.asp error messages
errmsg.err49="Radius solo est&#225; disponible en modo AP.";

//Wireless_Basic.asp error messages
errmsg.err50="Debes introducir una SSID.";

// Management.asp error messages
errmsg.err51="La clave del Router actual es la clave por defecto. \
			Como medida de seguridad, debes cambiar la clave antes de que la Administraci&#243;n Remota est&#233; activada. \
			Click en el bot&#243;n OK para cambiar la clave. Click en el bot&#243;n Cancelar para dejar deshabilitada la Administraci&#243;n Remota.";
errmsg.err52="La confirmaci&#243;n de la clave no coincide.";

// Port_Services.asp error messages
errmsg.err53="Despues de acabar, pulsa el bot&#243;n Aplicar para guardar la configuraci&#243;n.";
errmsg.err54="Debes introducir un Nombre de Servicio.";
errmsg.err55="El Nombre de Servicio existe.";

// QoS.asp error messages
errmsg.err56="Valor del puerto fuera de rango [0 - 65535]";

// Routing.asp error messages
errmsg.err57="¿ Borrar la Entrada ?";

// Status_Lan.asp error messages
errmsg.err58="Click para borrar el lease";

//Status_Wireless.asp error messages
errmsg.err59="¡No disponible! Por favor activa la Red Inal&#225;mbrica.";

//Upgrade.asp error messages
errmsg.err60="Por favor selecciona un archivo para actualizar.";
errmsg.err61="Archivo de imagen incorrecto.";

// *******************************************  COMMON MENU ENTRIES  *******************************************//
var bmenu= new Object();
bmenu.setup="Setup";
bmenu.setupbasic="Setup B&#225;sico";
bmenu.setupddns="DDNS";
bmenu.setupmacclone="Clonar Direcci&#243;n MAC";
bmenu.setuprouting="Ruteo Avanzado";
bmenu.setupvlan="VLANs";

bmenu.wireless="Inal&#225;mbrico";
bmenu.wirelessBasic="Config B&#225;sica";
bmenu.wirelessRadius="Radius";
bmenu.wirelessSecurity="Seguridad Inal&#225;mbrica";
bmenu.wirelessMac="Filtro MAC";
bmenu.wirelessAdvanced="Config Avanzada";
bmenu.wirelessWds="WDS";

bmenu.security="Seguridad";
bmenu.firwall="Firewall";
bmenu.vpn="VPN";

bmenu.accrestriction="Restricciones de Acceso";
bmenu.webaccess="Acceso a Internet";


bmenu.applications="Aplicaciones y Juegos";
bmenu.applicationsprforwarding="Redireccionar Rango de Puertos";
bmenu.applicationspforwarding="Redireccionamiento de Puertos";
bmenu.applicationsptriggering="Mapeado de Puertos";
bmenu.applicationsUpnp="UPnP";
bmenu.applicationsDMZ="DMZ";
bmenu.applicationsQoS="QoS";

bmenu.sipath="SIPatH";
bmenu.sipathoverview="Revisi&#243;n";
bmenu.sipathphone="List&#237;n Telef&#243;nico";
bmenu.sipathstatus="Estado";

bmenu.admin="Administraci&#243;n";
bmenu.adminManagement="Administraci&#243;n";
bmenu.adminHotspot="Hotspot";
bmenu.adminServices="Servicios";
bmenu.adminAlive="Keep Alive (manter vivo)";
bmenu.adminLog="Log";
bmenu.adminDiag="Diagn&#243;sticos";
bmenu.adminWol="WOL";
bmenu.adminFactory="Valores de F&#225;brica";
bmenu.adminUpgrade="Actualizaci&#243;n de Firmware";
bmenu.adminBackup="Backup";


bmenu.statu="Estado";
bmenu.statuRouter="Router";
bmenu.statuLAN="LAN";
bmenu.statuSputnik="Agente Sputnik";
bmenu.statuWLAN="Inal&#225;mbrico";
bmenu.statuSysInfo="Info-Stma";


// ******************************************* Alive.asp *******************************************//

var alive = new Object();
alive.titl=" - Keep Alive";
alive.h2="Keep Alive";
alive.legend="Reiniciar Horario";
alive.sevr1="Reiniciar Horario";
alive.hour="A la hora establecida";
alive.legend2="WDS/Conexi&#243;n Watchdog";
alive.sevr2="Activar Watchdog";
alive.IP="Direcciones IP";
alive.legend3="Proxy/Connexi&#243;n Watchdog";
alive.sevr3="Activar el Proxy Watchdog";
alive.IP2="Direcci&#243;n IP del Proxy";
alive.port="Puerto del Proxy";

//help container
var halive = new Object();
halive.right2="Elige cuando reinicar el router. Cron debe estar activado en la pesta&#241;a de administraci&#243;n.";
halive.right4="Se permite un m&#225;ximo de tres IPs separdas por un <em>SPACE</em>.<BR/>El formato de las IPs es xxx.xxx.xxx.xxx";



// ******************************************* config.asp *******************************************//

var config = new Object();
config.titl=" - Backup y Restaurar";
config.h2="Config del Backup";
config.legend="Config del Backup";
config.mess1="Click en el bot&#243;n \"" + sbutton.backup + "\" para descargar el backup de configuraci&#243;n en tu ordenador.";
config.h22="Restaurar la Configuraci&#243;n.";
config.legend2="Restaurar Configuraci&#243;n.";
config.mess2="Por favor, selecciona el archivo a restaurar";
config.mess3="C U I D A D O";
config.mess4="Subir solamente archivos backup creados por este firmare y modelo de router.<br />No subir ning&#250;n archivo que no haya sido creado por este interfaz!";

//help container
var hconfig = new Object();
hconfig.right2="Deber&#237;as hacer un backup de tu configuraci&#243;n actual por si tuvieras que reinicar el router a sus valores originales.<br /><br />Click en el bot&#243;n <em>Backup</em> para hacer un backup de tu configuraci&#243;n.";
hconfig.right4="Click en el bot&#243;n <em>explorar...</em> button to browse for a configuration file that is currently saved on your PC.<br /><br />Click the <em>" + sbutton.restore + "</em> button to overwrite all current configurations with the ones in the configuration file.";



// ******************************************* DDNS.asp *******************************************//

var ddns = new Object();
ddns.titl=" - DNS Din&#225;mico";
ddns.h2="Sistema de Nombres de Dominio Din&#225;mico (DDNS)";
ddns.legend="DDNS";
ddns.srv="Servicio DDNS";
ddns.emailaddr="Direcci&#243;n E-mail";
ddns.typ="Tipo";
ddns.dynamic="Din&#225;mico";
// ddns.static="Static"; Please note: Mozilla doesn't like ".static", use ".sttic" , Eko 22.mar.06
ddns.custom="Personalizado";
ddns.wildcard="Comod&#237;n";
ddns.statu="Estado DDNS";

var ddnsm = new Object();
ddnsm.dyn_strange="Respuesta extra&#241;a del servidor.¿ Est&#225;s conectandote al correcto ?";
ddnsm.dyn_good="DDNS actualizado correctamente";
ddnsm.dyn_noupdate="No es necesario actualizar en este momento";
ddnsm.dyn_nohost="El Nombre del Host no existe";
ddnsm.dyn_notfqdn="El Nombre del Host Name no es correcto";
ddnsm.dyn_yours="El Host no est&#225; bajo tu control";
ddnsm.dyn_abuse="El Host ha sido bloqueado por abuso";
ddnsm.dyn_nochg="La direcci&#243;n IP no ha cambiado desde la &#250;ltima actualizaci&#243;n";
ddnsm.dyn_badauth="Fallo en la autentificaci&#243;n (Nombre de Usuario &#243; Clave)";
ddnsm.dyn_badsys="Par&#225;metro de sistema inv&#225;lido";
ddnsm.dyn_badagent="Este agente de usuario ha sido bloqueado";
ddnsm.dyn_numhost="Demasiados o muy pocos hosts encontrados";
ddnsm.dyn_dnserr="Error interno en DNS";
ddnsm.dyn_911="Error inesperado 911";
ddnsm.dyn_999="Error inesperado 999";
ddnsm.dyn_donator="La funci&#243;n requerida est&#225; solo disponible a los donantes. Por favor dona";
ddnsm.dyn_uncode="C&#243;digo de retorno desconocido";

ddnsm.tzo_good="Operaci&#243;n Completa";
ddnsm.tzo_noupdate="No se requiere actualizaci&#243;n en este momento";
ddnsm.tzo_notfqdn="Nombre de Dominio Inv&#225;lido";
ddnsm.tzo_notmail="E-mail Inv&#225;lido";
ddnsm.tzo_notact="Acci&#243;n Inv&#225;lida";
ddnsm.tzo_notkey="Clave Inv&#225;lida";
ddnsm.tzo_notip="Direcci&#243;n IP inv&#225;lida";
ddnsm.tzo_dupfqdn="Duplicar Nombre de Dominio";
ddnsm.tzo_fqdncre="El nombre de dominio ya ha sido creado para este nombre de dominio";
ddnsm.tzo_expired="La cuenta ha expirado";
ddnsm.tzo_error="Error inexperado en el servidor";

ddnsm.zone_701="La zona no est&#225; establecida en esta cuenta";
ddnsm.zone_702="Actualizaci&#243;n fallida";
ddnsm.zone_703="Uno de los par&#225;metros <em>zonas</em> &#243; <em>host</em> son requeridos";
ddnsm.zone_704="La zona debe ser un nombre de internet v&#225;lido y <em>con puntos</em>";
ddnsm.zone_705="La zona no puede estar vac&#237;a";
ddnsm.zone_707="Duplicar actualizaciones para la misma host/ip, ajustar configurci&#243;n de cliente";
ddnsm.zone_201="Ningun dato grabado necesita actualizaci&#243;n";
ddnsm.zone_badauth="Autorizaci&#243;n fallida (nombre de usuario o clave)";
ddnsm.zone_good="ZoneEdit ha sido actualizado correctamente";
ddnsm.zone_strange="Respuesta extra&#241;a del servidor.¿ Est&#225;s conectandote al correcto ?";

ddnsm.all_closed="El servidor DDNS est&#225; actualmente cerrado";
ddnsm.all_resolving="Resolviendo el nombre de dominio";
ddnsm.all_errresolv="Resoluci&#243;n de nombre de dominio fallida";
ddnsm.all_connecting="Conectando al servidor";
ddnsm.all_connectfail="Conexi&#243;n al servidor fallida";
ddnsm.all_disabled="La funci&#243;n DDNS est&#225; deshabilitada";
ddnsm.all_noip="No hay conexi&#243;n a Internet";

//help container
var hddns = new Object();
hddns.right2="El DDNS te permite acceder a tu red usando nombres de dominio en lugar de direcciones IP. \
	El servicio administra direcciones IP cambiantes y actualiza tu informaci&#243;n de dominio din&#225;micamente. \
	Debes estar logeado para usar el servicio a trav&#233;s de DynDNS.org, TZO.com &#243; ZoneEdit.com.";



// ******************************************* Diagnostics.asp *******************************************//

var diag = new Object();
diag.titl=" - Diagn&#243;sticos";
diag.h2="Diagn&#243;sticos";
diag.legend="Command Shell";
diag.cmd="Comandos";
diag.startup="Inicio";
diag.firewall="Firewall";

//help container
var hdiag = new Object();
hdiag.right2="Puedes ejecutar l&#237;neas de comando via interfaz web. Introduce el comando en el &#225;rea de texto y haz click <em>" + sbutton.runcmd + "</em> para enviarlo.";



// ******************************************* DMZ.asp *******************************************//

var dmz = new Object();
dmz.titl=" - DMZ";
dmz.h2="Zona DesMilitarizada (DMZ) - <em>fuera del firewall</em>";
dmz.legend="DMZ";
dmz.serv="Usar DMZ";
dmz.host="Direcci&#243;n IP del host DMZ";


//help container
var hdmz = new Object();
hdmz.right2="Activando esta opci&#243;n expondr&#225; el host especificado a Internet. Todos los puertos ser&#225;n accesibles desde Internet.";



// ******************************************* Factory_Defaults.asp *******************************************//

var factdef = new Object();
factdef.titl=" - Valores de F&#225;brica";
factdef.h2="Valores de F&#225;brica";
factdef.legend="Resetear la configuraci&#243;n del router";
factdef.restore="Restaurar Valores de F&#225;brica";

factdef.mess1="¡Peligro! Si haces click en OK, el dispositivo se resetear&#225; a los valores de f&#225;brica. La configuraci&#243;n anterior ser&#225; eliminada.";

//help container
var hfactdef = new Object();
hfactdef.right1="Esto resetear&#225; todos los datos de la configuraci&#243;n a sus valores de f&#225;brica. Tu configuraci&#243;n actual ser&#225; borrada.";



// ******************************************* FilterIP%AC.asp *******************************************//

var filterIP = new Object();
filterIP.titl=" - Lista de PCs";
filterIP.h2="Lista de PCs";
filterIP.h3="Introduce la direcci&#243;n MAC de los PCs en este formato: xx:xx:xx:xx:xx:xx";
filterIP.h32="Introducir la direcci&#243;n IP de los PCs";
filterIP.h33="Introducir el rango de IP de los PCs";
filterIP.ip_range="Rango de IP";



// ******************************************* Filter.asp *******************************************//

var filter = new Object();
filter.titl=" - Restriciones de Acceso";
filter.h2="Acceso a InternetInternet Access";
filter.legend="Pol&#237;tica de Acceso";
filter.restore="Restaurar Valores de F&#225;brica";
filter.pol="Pol&#237;tica";
filter.polname="Nombre de Pol&#237;tica";
filter.pcs="PCs";
filter.polallow="Acceso internet durante los d&#237;as y horas elegidos.";
filter.legend2="D&#237;as";
filter.time="Times";
filter.h24="24 Horas";
filter.legend3="Servicios Bloquedos";
filter.catchall="Capturar todos los protocolos P2P";
filter.legend4="Bloqueo Web mediante direci&#243;n URL";
filter.legend5="Bloqueo Web mediante palabra clave";

filter.mess1="¿Borrar la pol&#237;tica?";
filter.mess2="Debes elegir al menos un d&#237;a.";
filter.mess3="La hora final debe ser posterior a la hora de inicio.";

//help container
var hfilter = new Object();
hfilter.right1="Pol&#237;tica de Acceso a Intenet:";
hfilter.right2="Puedes definir hasta 10 pol&#237;ticas de acceso. Haz click <em>" + sbutton.del + "</em> para borrar una pol&#237;tica &#243; <em>" + sbutton.summary + "</em> para ver un sumario de la misma.";
hfilter.right3="Estado:";
hfilter.right4="Activar o desactivar pol&#237;tica.";
hfilter.right5="Nombre de Pol&#237;tica:";
hfilter.right6="Puedes asignar un nombre a tu pol&#237;tica.";
hfilter.right7="D&#237;as:";
hfilter.right8="Elige el d&#237;a de la semana que en que te gustar&#237;a aplicar tu pol&#237;tica.";
hfilter.right9="Horas:";
hfilter.right10="Introducir la hora del d&#237;a en que te gustar&#237;a aplicar tu pol&#237;tica.";
hfilter.right11="Servicios Bloqueados:";
hfilter.right12="Puedes elegir bloqueos de acceso a ciertos servicios. Click en <em>" + sbutton.filterSer + "</em> para modificar la configuraci&#243;n.";
hfilter.right13="Bloqueo Web mediante URL:";
hfilter.right14="Puedes bloquear ciertos sitios introduciendo su URL.";
hfilter.right15="Bloqueo Web via Palabra Clave:";
hfilter.right16="Puedes bloquear el acceso a ciertos sitios web seg&#250;n las palabras contenidas en dichas p&#225;ginas web.";



// ******************************************* FilterSummary.asp *******************************************//

var filterSum = new Object();
filterSum.titl=" - Sumario de Restricciones de Acceso";
filterSum.h2="Sumario de Pol&#237;ticas de Internet";
filterSum.polnum="No.";
filterSum.polday="Hora del D&#237;a";



// ******************************************* Firewall.asp *******************************************//

var firewall = new Object();
firewall.titl=" - Firewall";
firewall.h2="Seguridad";
firewall.legend="Protecci&#243;n Firewall";
firewall.firewall="Firewall SPI";
firewall.legend2="Filtros Adicionales";
firewall.proxy="Filtrado Proxy";
firewall.cookies="Filtrado de Cookies";
firewall.applet="Filtrado de Applets Java";
firewall.activex="Filtrado de ActiveX";
firewall.legend3="Bloqueo de Peticiones WAN";
firewall.ping="Bloqueo de Peticiones An&#243;nimas de Internet";
firewall.muticast="Filtrado Multicast";
filter.nat="Filtrado de Redirecci&#243;n NAT en Internet";
filter.port113="Filtro IDENT (Puerto 113)";

//help container
var hfirewall = new Object();
hfirewall.right2="Activar o desactivar el Firewall SPI (Inspecci&#243;n de Paquete de Estado).";



// ******************************************* Forward.asp *******************************************//

var prforward = new Object();
prforward.titl=" - Redirecci&#243;n de Rango de Puertos";
prforward.h2="Redirecci&#243;n seg&#250;n Rangos de Puerto";
prforward.legend="Redirecciones";
prforward.app="Aplicaci&#243;n";

//help container
var hprforward = new Object();
hprforward.right2="En ocasiones, ciertas aplicaciones requieren que determinados puertos est&#233;n abiertos para funcionar correctamente. \
	Ejemplos de estas aplicaciones son los servidores y ciertos juegos online. \
	Cuando se produzca la petici&#243;n de un puerto concreto desde Internet, &#233;ste dispositivo se encargar&#225; de rutear la informaci&#243;n al ordenador que t&#250; especifiques. \
	Por temas de seguridad, deber&#237;as limitar la redirecci&#243;n de puertos a tan solo los que est&#233;s usando, \
	y desmarcar la casilla de verificaci&#243;n <em>" + share.enable +"</em> despu&#233;s de haber finalizado.";



// ******************************************* ForwardSpec.asp *******************************************//

var pforward = new Object();
pforward.titl=" - Redirecci&#243;n de Puertos";
pforward.h2="Redirecci&#243;n de Puertos";
pforward.legend="Redirecciones";
pforward.app="Aplicaci&#243;n";
pforward.from="Puerto Desde";
pforward.to="Puerto Hasta";

//help container
var hpforward = new Object();
hpforward.right2="En ocasiones ciertas aplicaciones requieren que determinados puertos est&#233;n abiertos para funcionar correctamente. \
	Ejemplos de estas aplicaciones incluye servidores y ciertos juegos online. \
	Cuando se produzca la petici&#243;n de un puerto concreto desde Internet, &#233;ste dispositivo se encargar&#225; de rutear la informaci&#243;n al ordenador que t&#250; especifiques. \
	Por temas de seguridad, deber&#237;as limitar la redirecci&#243;n de puertos a tan solo los que est&#233;s usando, \
	y desmarcar la casilla de verificaci&#243;n <em>Enable</em> despu&#233;s de haber finalizado.";



// ******************************************* Hotspot.asp *******************************************//

var hotspot = new Object();
hotspot.titl=" - Hotspot";
hotspot.h2="Portal Hotspot";
hotspot.legend="Chillispot";
hotspot.hotspot="Chillispot";
hotspot.pserver="IP/DNS Principal del Radius Server";
hotspot.bserver="Backup de IP/DNS del Radius Server";
hotspot.dns="IP DNS";
hotspot.url="Redirigir URL";
hotspot.dhcp="Interfaz DHCP";
hotspot.radnas="ID del Radius NAS";
hotspot.uam="UAM Secreto";
hotspot.uamdns="UAM cualquier DNS";
hotspot.allowuam="UAM Allowed";
hotspot.macauth="Autorizaci&#243;n MAC";
hotspot.option="Opciones de Chillispot Adicionales";
hotspot.fon_chilli="Administraci&#243;n Local del Usuario de Chillispot";
hotspot.fon_user="Lista de Usuarios";
hotspot.http_legend="Redirecci&#243;n HTTP";
hotspot.http_srv="Redirecci&#243;n HTTP";
hotspot.http_ip="HTTP IP de Destino";
hotspot.http_port="HTTP Puerto de Destino";
hotspot.http_net="HTTP Red Or&#237;gen (Source NET)";
hotspot.nocat_legend="NoCatSplash";
hotspot.nocat_srv="NoCatSplash";
hotspot.nocat_gateway="Nombre de la Puerta de Enlace";
hotspot.nocat_home="P&#225;gina Inicio";
hotspot.nocat_allowweb="Hosts Web Permitidos";
hotspot.nocat_docroot="Documento Ra&#237;z";
hotspot.nocat_splash="Splash URL";
hotspot.nocat_port="Exclu&#237;r Puertos";
hotspot.nocat_timeout="Login Timeout";
hotspot.nocat_verbose="Verbosity";
hotspot.nocat_route="S&#243;lo Ruteo";
hotspot.smtp_legend="Redirecci&#243;n SMTP";
hotspot.smtp_srv="Redirecci&#243;n SMTP";
hotspot.smtp_ip="IP de Destino SMTP";
hotspot.smtp_net="Red de Or&#237;gen del SMTP";
hotspot.shat_legend="Zero IP Config";
hotspot.shat_srv="Zero IP Config";
hotspot.shat_srv2="Zero IP Config Activado";
hotspot.sputnik_legend="Sputnik";
hotspot.sputnik_srv="Sputnik Agent";
hotspot.sputnik_id="ID ServidorSputnik";
hotspot.sputnik_instant="Usar setup instant&#225;neo del Sputnik";
hotspot.sputnik_express="Usar SputnikNet Express";
hotspot.sputnik_about="A cerca de Sputnik";



// ******************************************* Info.htm *******************************************//

var info = new Object();
info.titl=" - Informaci&#243;n";
info.h2="Informaci&#243;n de Sistema";
info.wlanmac="MAC Inal&#225;mbrica";
info.srv="Servicios";
info.ap="Access Point";



// ******************************************* index_heartbeat.asp *******************************************//

var idx_h = new Object();
idx_h.srv="Servidor Heart Beat";
idx_h.con_strgy="Estrategia de Conexi&#243;n";
idx_h.max_idle="Conectar bajo demanda: Tiempo M&#225;ximo de Reposo";
idx_h.alive="Keep Alive: Per&#237;odo de Rellamada";



// ******************************************* index_l2tp.asp *******************************************//

var idx_l = new Object();
idx_l.srv="Servidor L2TP";



// ******************************************* index_pppoe.asp *******************************************//

var idx_pppoe = new Object();
idx_pppoe.use_rp="Usar RP PPPoE";



// ******************************************* index_pptp.asp *******************************************//

var idx_pptp = new Object();
idx_pptp.srv="Usar DHCP";
idx_pptp.wan_ip="Direcci&#243;n IP de Internet";
idx_pptp.gateway="Puerta de Enlace (Servidor PPTP)";
idx_pptp.encrypt="Encriptaci&#243;n PPTP";



// ******************************************* index_static.asp *******************************************//

var idx_static = new Object();
idx_static.dns="DNS Est&#225;tica";



// ******************************************* index.asp *******************************************//

var idx = new Object();
idx.titl=" - Setup";
idx.h2="Setup Internet";
idx.h22="Setup Inal&#225;mbrico";
idx.legend="Tipo de Conexion a Internet";
idx.conn_type="Tipo de Conexi&#243;n";
idx.stp="STP";
idx.stp_mess="(Deshabilitar para COMCAST ISP)";
idx.optional="Config Opcional (necesario en algunos ISPs)";
idx.mtu="MTU";
idx.h23="Setup de RED";
idx.routerip="IP del Router";
idx.lanip="IP Local (LAN)";
idx.legend2="Puerto WAN";
idx.wantoswitch="Assigar Puerto WAN Port a cambiar";
idx.legend3="Config. Hora";
idx.timeset="Zona Horaria / Horario Verano (DST)";
idx.localtime="Usar hora local";
idx.static_ip="IP Est&#225;tica";
idx.dhcp="Configuraci&#243;n Autom&#225;tica - DHCP";
idx.dhcp_legend="Config Direcci&#243;n de Servidor de Red (DHCP)";
idx.dhcp_type="Tipo de DHCP";
idx.dhcp_srv="Servidor DHCP";
idx.dhcp_fwd="Redireccionador DHCP";
idx.dhcp_start="IP Inicial";
idx.dhcp_end="IP Final";		//used in Status_Lan.asp
idx.dhcp_maxusers="Usuarios M&#225;ximos DHCP";
idx.dhcp_lease="Lease Time del Cliente";
idx.dhcp_dnsmasq="Usar DNSMasq para DHCP";
idx.dns_dnsmasq="Usar DNSMasq para DNS";
idx.auth_dnsmasq="DHCP-Autorizativo";



//help container
var hidx = new Object();
hidx.right2="Este valor de configuraci&#243;n es m&#225;s com&#250;n verlo en la mayor&#237;a de los operadores de cable.";
hidx.right4="Introducir el nombre de host designado por tu ISP (Proveedor de Internet).";
hidx.right6="Intorducir el nombre de dominio designado por tu ISP.";
hidx.right8="Esta es la direcci&#243;n del router.";
hidx.right10="Esta es la m&#225;scara de subred del router.";
hidx.right12="Permite al router administrar tus direcci&#243;nes IP.";
hidx.right14="La direcci&#243;n a partir de la cual quieres empezar.";
hidx.right16="Puedes limitar el n&#250;mero de direcci&#243;nes que quieres que maneje tu router.";
hidx.right18="Elegir la zona horaria en la que se encuentra tu horario de Verano (DST). El router puede usar la hora local o la hora GMT.";



// ******************************************* Join.asp *******************************************//

var join = new Object();

//sshd.webservices
join.titl=" - Join";
join.mess1="Unido con &#233;xito a la siguiente red como cliente: ";



// ******************************************* Log_incoming.asp *******************************************//

var log_in = new Object();
log_in.titl=" - Tabla LOG de Entrada";
log_in.h2="Tabla LOG de Entrada";
log_in.th_ip="IP Or&#237;gen";
log_in.th_port="No. Puerto Destino";



// ******************************************* Log_outgoing.asp *******************************************//

var log_out = new Object();
log_out.titl=" - Tabla LOG de Salida";
log_out.h2="Tabla LOG de Entrada";
log_out.th_lanip="IP LAN";
log_out.th_wanip="URL/IP de Destino";
log_out.th_port="Servicio/Puerto";



// ******************************************* Log.asp *******************************************//

var log = new Object();
log.titl=" - LOG";
log.h2="Administraci&#243;n de LOG";
log.legend="LOG";
log.lvl="Nivel de LOG";
log.drop="Ca&#237;do";
log.reject="Rechazado";
log.accept="Aceptado";



// ******************************************* Management.asp *******************************************//

var management = new Object();
management.titl=" - Administraci&#243;n";
management.h2="Administraci&#243;n Router";

management.psswd_legend="Clave del Router";
management.psswd_user="Usuario del Router";
management.psswd_pass="Clave del Router";
management.pass_conf="Re-introducir Clave";

management.remote_legend="Acceso Remoto";
management.remote_gui="Administraci&#243;n GUI Web";
management.remote_https="Use HTTPS";
management.remote_guiport="Puerto GUI web";
management.remote_ssh="SSH Management";
management.remote_sshport="Puerto SSH";

management.web_legend="Acceso Web";
management.web_refresh="Auto-Refresco (en segs)";
management.web_sysinfo="Activar Info Sitio";
management.web_sysinfopass="Clave para Info Sitio";
management.web_sysinfomasq="MAC Masquerading Info Sitio";

management.boot_legend="Boot Wait";
management.boot_srv="Boot Wait";

management.cron_legend="Cron";
management.cron_srvd="Cron";

management.dsn_legend="DNSMasq";
management.dsn_srv="DNSMasq";
management.dsn_loc="DNS Local";
management.dsn_opt="Opciones Adicionales DNS";

management.loop_legend="Loopback";
management.loop_srv="Loopback";

management.wifi_legend="802.1x";
management.wifi_srv="802.1x";

management.ntp_legend="Cliente NTP";
management.ntp_srv="NTP";

management.rst_legend="Bot&#243;n de Reset";
management.rst_srv="Bot&#243;n de Reset";

management.routing_legend="Ruteo";
management.routing_srv="Ruteo";

management.wol_legend="Wake-On-LAN";
management.wol_srv="WOL";
management.wol_pass="Clave SecureOn";
management.wol_mac="Direcciones MAC<br/>( Formato: xx:xx:xx:xx:xx:xx )";

management.ipv6_legend="Soporte IPv6";
management.ipv6_srv="IPv6";
management.ipv6_rad="Radvd Activado";
management.ipv6_radconf="Radvd Config";

management.jffs_legend="Soporte JFFS2";
management.jffs_srv="JFFS2";
management.jffs_clean="Limpiar JFFS2";

management.lang_legend="Selecci&#243;n Idioma";
management.lang_srv="Idioma";
management.lang_bulgarian="B&#250;lgaro";
management.lang_tradchinese="TradChino";
management.lang_croatian="Croata";
management.lang_czech="Checo";
management.lang_dutch="Holand&#233;s";
management.lang_english="Ingl&#233;s";
management.lang_french="Franc&#233;s";
management.lang_german="Alem&#225;n";
management.lang_italian="Italiano";
management.lang_brazilian="Brasile&#241;o";
management.lang_slovenian="Esloveno";
management.lang_spanish="Espa&#241;ol";
management.lang_swedish="Sueco";

management.net_legend="Config Filtro IP (Ajustar para P2P)";
management.net_port="Max Puertos";
management.net_tcptimeout="TCP Timeout (en segs)";
management.net_udptimeout="UDP Timeout (en segs)";

management.clock_legend="Overclocking";
management.clock_frq="Frecuencia";
management.clock_support="No Soportado";

management.mmc_legend="Soporte Tarjetas MMC/SD";
management.mmc_srv="Dispositivo MMC";

management.samba_legend="Automontado Samba FS";
management.samba_srv="Sist Archivos SMB";
management.samba_share="Compartir";
management.samba_stscript="Script Inicio";

management.SIPatH_srv="SIPatH";
management.SIPatH_port="Puerto SIP";
management.SIPatH_domain="Dominio SIP";

management.gui_style="Estilo GUI Router";


//revisar - review
//help container
var hmanagement = new Object();
hmanagement.right1="Auto-Refresco:";
hmanagement.right2="Ajusta el intervalo de actualizaci&#243;n autom&#225;tica del interfaz gr&#225;fico de la web (Web GUI). El valor 0 deshabilita esta funci&#243;n por completo.";



// ************ Port_Services.asp (used by Filters.asp and QoS.asp, QOSPort_Services.asp not used anymor) *****************************************//

var portserv = new Object();
portserv.titl=" - Puertos de Servicio";
portserv.h2="Puertos de Servicio";



// ******************************************* QoS.asp *******************************************//

var qos = new Object();
qos.titl=" - Calidad de Servicio";
qos.h2="Calidad de Servicio (QoS)";
qos.legend="Configuraci&#243;n QoS";
qos.srv="Iniciar QoS";
qos.type="Calendario Paquetes";
qos.uplink="Enlace Subida (Kbps)";
qos.dnlink="Enlace Bajada (Kbps)";
qos.gaming="Optimizar para Juegos";
qos.legend2="Prioridad de Servicios";
qos.prio_x="Exento";
qos.prio_p="Premium";
qos.prio_e="Expreso";
qos.prio_s="Est&#225;ndar";
qos.prio_b="Bulk";
qos.legend3="Prioridad M&#225;scara RED";
qos.ipmask="IP/Masc";
qos.maxrate_b="Kbits Max";
qos.maxrate_o="Tasa Max";
qos.legend4="Prioridad MAC";
qos.legend5="Prioridad Puerto Ethernet";
qos.legend6="Ancho de Banda por Defecto";
qos.bandwith="Ancho de Banda (Kbits)";

//help container
var hqos = new Object();
hqos.right1="Enlace Subida (UplinK):";
hqos.right2="Establecer esto entre 80%-95% (max) de tu l&#237;mite total de subida.";
hqos.right3="Enlace Bajada (Downlink):";
hqos.right4="Establecer esto entre 80%-100% (max) de tu l&#237;mite total de bajada.";
hqos.right6="Puedes controlar la tasa de transferencia de datos con respecto a la apliaci&#243;n que est&#225; consumiendo ancho de banda.";
hqos.right8="Puedes especificar prioridades para todo el tr&#225;fico procedente desde una IP concreta o desde un rango de IPs.";
hqos.right10="Puedes especificar prioridades para todo el tr&#225;fico de un dispositivo conectado a tu red d&#225;ndole un Nombre de Dispositivo, \
			especificando prioridad y metiendo su direcci&#243;n MAC.";
hqos.right12="Puedes controlar tu tasa de transferencia de datos con respecto a qu&#233; puerto LAN f&#237;sico est&#225; conectado el dispositivo. \
			Puedes asignar prioridades dependiendo del tipo de dispositivos conectados a los puertos LAN 1 a 4 del router.";



// ******************************************* RouteTable.asp *******************************************//

var routetbl = new Object();
routetbl.titl=" - Tabla de Ruteo";
routetbl.h2="Lista de Entrada Tabla de Ruta";
routetbl.th1="IP LAN de Destino";



// ******************************************* Routing.asp *******************************************//

var route = new Object();
route.titl=" - Ruteo";
route.h2="Ruteo Avanzado";
route.mod="Modo de Operaci&#243;n";
route.bgp_legend="Config BGP";
route.bgp_ip="IP Vecina";
route.bgp_as="AS# Vecina";
route.rip2_mod="Ruteo RIP2";
route.ospf_mod="Ruteo OSPF";
route.gateway_legend="Ruteo Din&#225;mico";
route.static_legend="Ruteo Est&#225;tico";
route.static_setno="Elegir N&#250;mero de SET";
route.static_name="Nombre de Ruteo";
route.static_ip="IP LAN de Destino";

//help container
var hroute = new Object();
hroute.right1="Modo de Operaci&#243;n:";
hroute.right2="Si el router est&#225; hospedando tu conexi&#243;n de Internet, elegir el modo <em>Puerta de Enlace</em> (en ingl&#233;s Gateway). Si hay otro router en tu red, entonces selecciona el modo <em>Router</em>.";
hroute.right3="Elegir N&#250;mero de SET:";
hroute.right4="Este es el n&#250;mero de ruteo &#250;nico, puedes establecer hasta 20 rutas.";
hroute.right5="Nombre de Ruteo:";
hroute.right6="Introduce el nombre que te gustar&#237;a asignar a este ruteo.";
hroute.right7="IP LAN de Destino:";
hroute.right8="Este es el host remoto al cual querr&#237;as asignar el ruteo est&#225;tico..";
hroute.right9="M&#225;scara de Subred:";
hroute.right10="Determina el host y la segmento de red.";


// ******************************************* Site_Survey.asp *******************************************//

var survey = new Object();
survey.titl=" - Inspecci&#243;n de Sitio";
survey.h2="Redes Inal&#225;mbricas Vecinas";
survey.thjoin="Unir a Sitio";



// ******************************************* Services.asp *******************************************//

var service = new Object();

service.titl=" - Servicios";
service.h2="Administraci&#243;n de Servicios";

//kaid
service.kaid_legend="XBOX Kaid";
service.kaid_srv="Iniciar Kaid";
service.kaid_mac="MACs de Consola: (deben acabar en;)";

//DHCPd
service.dhcp_legend="Cliente DHCP";
service.dhcp_vendor="Establecer Vendorclass";
service.dhcp_legend2="Servidor DHCP";
service.dhcp_srv="Demonio DHCP";
service.dhcp_jffs2="Usar JFFS2 para client lease DB";
service.dhcp_domain="Dominio Usado";
service.dhcp_landomain="Dominio LAN";
service.dhcp_option="Opciones DHCPd Adicionales";

//pptp.webservices
service.pptp_legend="PPTP";
service.pptp_srv="Servidor PPTP";
service.pptp_client="IP(s) Cliente";
service.pptp_chap="Secretos-CHAP";

//syslog.webservices
service.syslog_legend="LOG Sistema";
service.syslog_srv="Syslogd";
service.syslog_ip="Servidor Remoto";

//telnet.webservices
service.telnet_legend="Telnet";
service.telnet_srv="Telnet";

//pptpd_client.webservices
service.pptpd_legend="Cliente PPTP";
service.pptpd_option="Opciones Cliente PPTP";
service.pptpd_ipdns="IP Servidor o Nombre DNS";
service.pptpd_subnet="Subred Remota";
service.pptpd_subnetmask="Mascara Subred Remota";
service.pptpd_encry="Encriptaci&#243;n MPPE";
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
service.snmp_loc="Situaci&#243;n";
service.snmp_contact="Contacto";
service.snmp_name="Nombre";
service.snmp_read="RO Comunidad";
service.snmp_write="RW Comunidad";

//openvpn.webservices
service.vpn_legend="Cliente VPN Abierta";
service.vpn_srv="Iniciar VPN Abierta";
service.vpn_ipname="IP/Nombre Servidor";
service.vpn_mtu="TUN MTU Config";
service.vpn_mru="TUN MTU Extra";
service.vpn_mss="TCP MSS";
service.vpn_compress="Usar Compresi&#243;n LZO";
service.vpn_tunnel="Protocolo Tunelado";
service.vpn_srvcert="Cert Servidor P&#250;blico";
service.vpn_clicert="Cert Cliente P&#250;blico";
service.vpn_certtype="nsCertType";
service.vpn_clikey="Clave Cliente Privada";

//sshd.webservices
service.ssh_legend="Seguridad Shell";
service.ssh_srv="SSHd";
service.ssh_password="Clave Login";
service.ssh_key="Claves Autorizadas";



// ******************************************* Sipath.asp + cgi *******************************************//

var sipath = new Object();
sipath.titl=" - Revisi&#243;n SiPath";
sipath.phone_titl=" - Listin Telef&#243;nico";
sipath.status_titl=" - Estado";



// ******************************************* Status_Lan.asp *******************************************//

var status_lan = new Object();
status_lan.titl=" - Estado LAN";
status_lan.h2="Red Local";
status_lan.legend="Estado LAN";
status_lan.h22="Protocolo de Configuraci&#243;n de Host Din&#225;mico";
status_lan.legend2="Estado DHCP";
status_lan.legend3="Clientes DHCP";

//help container
var hstatus_lan = new Object();
hstatus_lan.right2="Esta es la direcci&#243;n MAC del Router, como es vista en tu red Ethernet Local.";
hstatus_lan.right4="Esto muestra la Direcci&#243;n IP del Router, como aparece en tu Red Ethernet Local.";
hstatus_lan.right6="Cuando el router est&#225; usando una M&#225;scara de Subred concreta, &#233;sta se muestrar&#225; en este apartado.";
hstatus_lan.right8="Si est&#225;s usando el Router como servidor DHCP, &#233;sto se mostrar&#225; en este apartado.";
hstatus_lan.right10="Haciendo click en cualquier direcci&#243;n MAC, obtendr&#225;s el Identiticador &#218;nico Organizacional (Organizationally Unique Identifier) del interfaz (la tarjeta) de red (Standards IEEE de b&#250;squeda en bases de datos OUI).";



// ******************************************* Status_Router.asp *******************************************//

var status_router = new Object();
status_router.titl=" - Estado Router";
status_router.h2="Informaci&#243;n Router";
status_router.legend="Sistema";
status_router.sys_model="Modelo de Router";
status_router.sys_firmver="versi&#243;n de Firmware";
status_router.sys_time="Hora Actual";
status_router.sys_up="Tiempo Funcionando";
status_router.sys_load="Carga Media";
status_router.legend2="CPU";
status_router.cpu="Modelo CPU";
status_router.clock="Reloj CPU";
status_router.legend3="Memoria";
status_router.mem_tot="Total Disponible";
status_router.mem_free="Libre";
status_router.mem_used="Usado";
status_router.mem_buf="Buffers";
status_router.mem_cached="Cacheado";
status_router.mem_active="Activo";
status_router.mem_inactive="Inactivo";
status_router.legend4="RED";
status_router.net_maxports="Max Puertos Filtro IP";
status_router.net_conntrack="Conexiones IP Activas";
status_router.h22="Internet";
status_router.legend5="Tipo de Configuraci&#243;n";
status_router.www_login="Tipo de Login";
status_router.www_loginstatus="Estado de Login";

//help container
var hstatus_router = new Object();
hstatus_router.right2="Este es el nombre espec&#237;fico para el router, el cual estableces en la pesta&#241;a <i>Setup</i>.";
hstatus_router.right4="Esta es la direcci&#243;n MAC del router, tal y como es vista por tu ISP.";
hstatus_router.right6="Este es el firmware actual de tu router.";
hstatus_router.right8="Esta es la hora, que puedes establecer en la pesta&#241;a <em>Setup</em>.";
hstatus_router.right10="Aqu&#237; podemos ver el tiempo que el router lleva encendido \"up\" y funcionando.";
hstatus_router.right12="Esto viene a ser 3 n&#250;meros que representan la carga de sistema durante los &#250;ltimos per&#237;odos de 1, 5 &#243; 15 minutos.";
hstatus_router.right14="Esto muestra la informaci&#243;n requerida por tu ISP para la conexi&#243;n a internet. \
			Esta informaci&#243;n fue introducida en la Pesta&#241;a Setup. Puedes <em>Conectar</em> &#243; <em>desconectar</em> tu conexi&#243;n aqu&#237; haciendo click en ese bot&#243;n.";



// ******************************************* Status_SputnikAPD.asp *******************************************//

var status_sputnik = new Object();
status_sputnik.titl=" - Estado del Agente Sputnik";
status_sputnik.h2="Sputnik&reg; Agent&trade;";
status_sputnik.manage="Administrado Por";
status_sputnik.license="Licencia SCC No.";

//help container
var hstatus_sputnik = new Object();
hstatus_sputnik.right1="Estado del Agente Sputnik";
hstatus_sputnik.right2="Esta pantalla muestra el estado de proceso del Agente Sputnik.";
hstatus_sputnik.right4="El Centro de Control del Sputnikal cual este punto de acceso est&#225; conectado.";
hstatus_sputnik.right6="El estado actual del Agente.";
hstatus_sputnik.right8="El n&#250;mero de licencia del Centro de Control del Sputnik.";



// ******************************************* Status_Wireless.asp *******************************************//

var status_wireless = new Object();
status_wireless.titl=" - Estado Wi-Fi";
status_wireless.h2="Wi-Fi";
status_wireless.legend="Estado Wi-Fi";
status_wireless.net="RED";
status_wireless.pptp="Estado PPTP";
status_wireless.legend2="Informaci&#243;n de Paquetes";
status_wireless.rx="Recibidos (RX)";
status_wireless.tx="Transmitidos (TX)";
status_wireless.h22="Nodos Inal&#225;mbricos";
status_wireless.legend3="Clientes Wi-Fi";
status_wireless.signal_qual="Calidad de Se&#241;al";
status_wireless.wds="NodosWDS";

//help container
var hstatus_wireless = new Object();
hstatus_wireless.right2="Esta es la direcci&#243;n MAC del Router, tal y como puede ser vista en tu red local inal&#225;mbrica.";
hstatus_wireless.right4="Como hemos seleccionado en la pesta&#241;a Wi-Fi, esto mostrar&#225; el modo de operaci&#243;n del Wi-Fi (Mezcla, S&#243;lo-G, S&#243;lo-B &#243; Desactivado) usado por la red.";



// ******************************************* Triggering.asp *******************************************//

var trforward = new Object();
trforward.titl=" - Mapeado de Puertos";
trforward.h2="Mapeado de Puertos";
trforward.legend="Redireccionado";
trforward.trrange="Rango de Puertos Mapeados";
trforward.fwdrange="Rango de Puertos Redirec.";
trforward.app="Aplicaci&#243;n";

//help container
var htrforward = new Object();
htrforward.right2="Introducir el nombre de la aplicaci&#243;n a mapear.";
htrforward.right4="Muestra un listado de rangos de puertos mapeados para cada aplicaci&#243;n. Debes revisar la documentaci&#243;n de las aplicaciones con el fin de saber qu&#233; rango de puertos deberemos introducir.";
htrforward.right6="Muestra un listado de rangos de puertos redireccionados para cada aplicaci&#243;n. Debes revisar la documentaci&#243;n de las aplicaciones con el fin de saber qu&#233; rango de puertos deberemos introducir.";
htrforward.right8="Introducir el n&#250;mero de puerto inicial del rango mapedo y redireccionado.";
htrforward.right10="Introducir el n&#250;mero de puerto final del rango mapeado y redireccionado.";



// ******************************************* Upgrade.asp *******************************************//

var upgrad = new Object();
upgrad.titl=" - Actualizar Firmware";
upgrad.h2="Administraci&#243;n de Firmware";
upgrad.legend="Actualizaci&#243;n del Firmware";
upgrad.info1="Despues de Flashear, resetear a";
upgrad.resetOff="No Resetear";
upgrad.resetOn="Valores por Defecto";
upgrad.file="Por Favor elige el archivo a actualizar";
upgrad.warning="P E L I G R O !";
upgrad.mess1="La actualizaci&#243;n del firmware podr&#237;a tardar varios miutos.<br />No quites la corriente ni pulses el bot&#243;n reset!";

//help container
var hupgrad = new Object();
hupgrad.right2="Haz click en el bot&#243;n <em>Explorar...</em> para elegir el archivo de firmware que deseas subir al router.<br /><br /> \
			Haz click en el bot&#243;n <em>Actualizar</em> para comenzar el proceso de actualizaci&#243;n. La actualizaci&#243;n NO DEBE SER INTERRUMPIDA.";



// ******************************************* UPnP.asp *******************************************//

var upnp = new Object();
upnp.titl=" - UPnP";
upnp.h2="Universal Plug and Play (UPnP)";
upnp.legend="Avances";
upnp.legend2="Configuraci&#243;n UPnP";
upnp.serv="Servicio UPnP";
upnp.clear="Limpiar puertos de avance al arrancar";
upnp.url="Enviar URL de Presentaci&#243;n";
upnp.msg1="Click para borrar Lease";
upnp.msg2="¿Borrar Entradas?";


//help container
var hupnp = new Object();
hupnp.right2="Click en la basura para borrar una entrada de forma individual.";
hupnp.right4="Permite a las aplicaciones configurar autom&#225;ticamente los puertos de redirecci&#243;n.";



// ******************************************* VPN.asp *******************************************//

var vpn = new Object();
vpn.titl=" - VPN";
vpn.h2="Red Privada Virtual (VPN)";
vpn.legend="Passthrough VPN (paso a trav&#233;s)";
vpn.ipsec="IPSec Passthrough";
vpn.pptp="PPTP Passthrough";
vpn.l2tp="L2TP Passthrough";

//help container
var hvpn = new Object();
hvpn.right1="Puedes activar el passthrough (paso a trav&#233;s) IPSec, PPTP y/o L2TP para permitir la comunicaci&#243;n de tus dispositivos de red v&#237;a VPN.";


// ******************************************* Vlan.asp *******************************************//

var vlan = new Object();
vlan.titl=" - LAN Virtual";
vlan.h2="Red de &#193;rea Local Virtual (VLAN)";
vlan.legend="VLAN";
vlan.bridge="Asignados al<br />Puente";
vlan.tagged="Etiquetado";
vlan.negociate="Auto-Negociar";
vlan.aggregation="Agregaci&#243;n Link<br>en puertos 3 y 4";
vlan.trunk="Tronco";


// ******************************************* WEP.asp *******************************************//

var wep = new Object();
wep.defkey="Clave Transmisi&#243;n Defecto";
wep.passphrase="Passphrase";



// ******************************************* WOL.asp *******************************************//

var wol = new Object();
wol.titl=" - WOL";
wol.h2="Wake-On-LAN";
wol.legend="Hosts Disponibles";
wol.legend2="Direcci&#243;nes WOL";
wol.legend3="Salida";
wol.legend4="WOL Manual";
wol.enable="Activar WOL?";
wol.add_wol="A&#241;adir Host WOL";
wol.restore="Restaurar Valores de F&#225;brica";
wol.mac="Direcci&#243;n(es) MAC Address";
wol.broadcast="Broadcast de Red";
wol.udp="Puerto UDP";
wol.msg1="Click para eliminar host WOL";

//help container
var hwol = new Object();
hwol.right2="Esta p&#225;gina te permite <em>Wake Up - Despertar</em> hosts de tu red local (Ej: localmente conectado a tu router).";
hwol.right4="Las direcci&#243;nes MAC han de ser introducidas en el formato xx:xx:xx:xx:xx:xx (Ej: 01:23:45:67:89:AB)";
hwol.right6="La direcci&#243;n IP es generalmente la direcci&#243;n de broadcast de la red local, pero puede ser una direcci&#243;n remota si el host a despertar no est&#225; conectado al router de la red local."



// ******************************************* WanMAC.asp *******************************************//

var wanmac = new Object();
wanmac.titl=" - Clonar Direcci&#243;n MAC";
wanmac.h2="Clonar Direcci&#243;n MAC";
wanmac.legend="Clonar MAC";
wanmac.wan="Clonar MAC de la WAN";
wanmac.wlan="Clonar la MAC Inal&#225;mbrica";

//help container
var hwanmac = new Object();
hwanmac.right2="Algunos ISP necesitar&#225;n que registres tu direcci&#243;n MAC. \
		Si no deseas re-registrar tu direcc&#243;n MAC, puedes clonar la MAC que ya est&#225; registrada en tu ISP.";



// ******************************************* WL_WPATable.asp / WPA.asp / Radius.asp *******************************************//

var wpa = new Object();
wpa.titl=" - Seguridad Wi-Fi";
wpa.h2="Seguridad Wi-Fi";
wpa.legend="Encriptaci&#243;n Inal&#225;mbrica";
wpa.auth_mode="Autenticaci&#243;n de red";
wpa.psk="Clave WPA Pre-Compartida";
wpa.wpa="WPA";
wpa.radius="Radius";
wpa.gtk_rekey="Intervalo Rotaci&#243;n Clave WPA";
wpa.rekey="Intervalo Renovaci&#243;n Clave (en Segs)";
wpa.radius_ipaddr="Direcci&#243;n Servidor RADIUS";
wpa.radius_port="Puerto Servidor RADIUS";
wpa.radius_key="Clave RADIUS";
wpa.algorithms="Algoritmos WPA";
wpa.shared_key="Clave Compartida WPA";
wpa.rekeyInt="Intervalo Rotacion Clave";

//help container
var hwpa = new Object();
hwpa.right1="Modo de Seguridad:";
hwpa.right2="Deber&#237;as escoger entre Desactivar, WEP, Clave WPA Pre-Compartida, WPA RADIUS, o RADIUS. Todos los dispositivos de tu red deben usar el mismo modo de seguridad.";



// ******************************************* WL_FilterTable.asp *******************************************//

var wl_filter = new Object();
wl_filter.titl=" - Listado de Filtros de Direcciones MAC";
wl_filter.h2="Listado de Filtros de Direcciones MAC";
wl_filter.h3="Introducir direcci&#243;n MAC en este formato&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx";



// ******************************************* WL_ActiveTable.asp *******************************************//

var wl_active = new Object();
wl_active.titl=" - Listado MACs de Clientes Inal&#225;mbricos Activos";
wl_active.h2="Listado MACs de Clientes Inal&#225;mbricos Activos";
wl_active.h3="Enable MAC Filter";
wl_active.active="PC Activo";
wl_active.inactive="PC Inactivo";



// ******************************************* Wireless_WDS.asp *******************************************//

var wds = new Object();
wds.titl=" - WDS";
wds.h2="Sistema de Distribuci&#243;n Wi-Fi";
wds.legend="Configuraci&#243;n WDS";
wds.label="Lazy WDS";
wds.label2="Subred WDS";
wds.wl_mac="MAC Inal&#225;mbrica";
wds.lazy_default="Por Defecto: Desactivado";
wds.nat1="wLAN->WDS";
wds.nat2="WDS->wLAN";
wds.subnet="Subred";
wds.legend2="Opciones Extra";



// ******************************************* Wireless_radauth.asp *******************************************//

var radius = new Object();
radius.titl=" - Radius";
radius.h2="Servicio de Usuario Dial-In de Autenticaci&#243;n Remota";
radius.legend="Radius";
radius.label="MAC de Cliente Radius";
radius.label2="Formato MAC";
radius.label3="IP Servidor Radius";
radius.label4="Puerto Servidor Radius";
radius.label5="M&#225;ximo Usuarios No-Identificados";
radius.label6="Formato Clave";
radius.label7="Secreto Compartido RADIUS";
radius.label8="Invalidar Servidor no Disponible";
radius.mac="MAC";



// ******************************************* Wireless_MAC.asp *******************************************//

var wl_mac = new Object();
wl_mac.titl=" - Filtrado MAC";
wl_mac.h2="Filtrado MAC Wi-Fi";
wl_mac.legend="Filtrado MAC";
wl_mac.label="Usar Filtrado";
wl_mac.label2="Modo de Filtrado";
wl_mac.deny="Evitar que entren en la red inal&#225;mbrica los PCs listados";
wl_mac.allow="Permitir que entren en la red inal&#225;mbrica los PCs listados";



// ******************************************* Wireless_Basic.asp *******************************************//

var wl_basic = new Object();
wl_basic.titl=" - Wi-Fi";
wl_basic.h2="Wi-Fi";
wl_basic.legend="Configuraci&#243;n B&#225;sica";
wl_basic.label="Modo Inal&#225;mbrico";
wl_basic.label2="Modo de Red Wi-Fi";
wl_basic.label3="Nombre de Red Wi-Fi (SSID)";
wl_basic.label4="Canal Inal&#225;mbrico";
wl_basic.label5="Broadcast SSID Inal&#225;mbrico";
wl_basic.label6="Sensitivity Range (ACK Timing)";
wl_basic.ap="AP";
wl_basic.client="Cliente";
wl_basic.clientBridge="Puente Cliente";
wl_basic.adhoc="Adhoc";
wl_basic.mixed="Mezcla";
wl_basic.b="S&#243;lo-B";
wl_basic.g="S&#243;lo-G";
wl_basic.sensitivity="Por defecto: 20000 metros";

//help container
var hwl_basic = new Object();
hwl_basic.right2="Si deseas que exclu&#237;r a los clientes Wireless-G, debes elegir el modo <em>S&#243;lo-B</em>. Si lo que quieres es desabilitar el acceso inal&#225;mbrico (cualquiera) debes escoger <em>Desactivar</em>.";
hwl_basic.right3="Rango Sensividad: ";
hwl_basic.right4="Ajusta el Tiempo de ACK (acknowledgement - reconocimiento). 0 deshabilita el Tiempo de ACK timing completamente.";



// ******************************************* Wireless_Advanced.asp *******************************************//

var wl_adv = new Object();
wl_adv.titl=" - Configuraci&#243;n Wi-Fi Avanzada";
wl_adv.h2="Configuraci&#243;n Wi-Fi Avanzada";
wl_adv.legend="Configuraci&#243;n Avanzada";
wl_adv.legend2="Configuraci&#243;n Inal&#225;mbrica de Soporte Multimedia";
wl_adv.label="Tipo de Autenticaci&#243;n";
wl_adv.label2="Tasa B&#225;sica";
wl_adv.label3="Tasa Transmisi&#243;n";
wl_adv.label4="Modo Protecci&#243;n CTS";
wl_adv.label5="Frame Burst";
wl_adv.label6="Internvalo RadioFaro";
wl_adv.label7="Intervalo DTIM";
wl_adv.label8="Umbral de Fragmentaci&#243;n";
wl_adv.label9="Umbral RTS";
wl_adv.label10="Clientes Max Asociados";
wl_adv.label11="Aislamiento AP";
wl_adv.label12="Antena TX (transf)";
wl_adv.label13="Antena RX (recep)";
wl_adv.label14="Pre&#225;mbulo";
wl_adv.reference="Referencia Ru&#237;do";
wl_adv.label15="Xmit Power";
wl_adv.label16="Afterburner";
wl_adv.label17="Acceso GUI via Wi-Fi";
wl_adv.label18="Soporte WMM";
wl_adv.label19="Sin-Acknowledgement";
wl_adv.table1="EDCA AP Parameters (AP a Cliente)";
wl_adv.col1="CWmin";
wl_adv.col2="CWmax";
wl_adv.col3="AIFSN";
wl_adv.col4="TXOP(b)";
wl_adv.col5="TXOP(a/g)";
wl_adv.col6="Admin Forzado";
wl_adv.row1="Fondo";
wl_adv.row2="Mejor Esfuerzo";
wl_adv.row3="Video";
wl_adv.row4="Voz";
wl_adv.table2="EDCA STA Parameters (Cliente a AP)";
wl_adv.lng="Largo"; 					//************* don't use .long ! *************
wl_adv.shrt="Corto"; 				//************* don't use .short ! **************

//help container
var hwl_adv = new Object();
hwl_adv.right1="Tipo de Autenticaci&#243;n:";
hwl_adv.right2="Deber&#237;as escoger entre <em>Auto</em> &#243; <em>Clave Compartida</em>. La autenticaci&#243;n por clave compartida es m&#225;s segura, pero todos los dispositivos en tu red deber&#225;n soportar <em>Autenticaci&#243;n por Clave Compartida</em>.";



// ******************************************* Fail_s.asp / Fail_u_s.asp / Fail.asp *******************************************//

var fail = new Object();
fail.mess1="Los valores introducidos no son v&#225;lidos. Por favor, intenta de nuevo.";
fail.mess2="Actualizaci&#243;n fallida.";



// ******************************************* Success*.asp / Reboot.asp  *******************************************//

var success = new Object();
success.saved="Configuraci&#243;n Grabada.";
success.restore="Configuraci&#243;n Restaurada.<br/>La unidad est&#225; se est&#225; reiniciando. Por favor, espera un momento...";
success.upgrade="Actualizaci&#243;n exitosa.<br/>La unidad se est&#225; reiniciando. Por favor, espera un momento...";
success.success_noreboot="Configurado con &#233;xito.";
success.success_reboot=success.success_noreboot + "<br />La unidad se est&#225; reiniciando. Por favor, espera un momento...";

success.alert_reset="Toda la configuraci&#243;n han sido restaurada a sus valores por defecto.<br /><br />";
success.alert1="Por favor comprueba lo siguiente antes de conectar de nuevo:";
success.alert2="Si has cambiado la IP de tu router, por favor aseg&#250;rate de que pides/renuevas tu(s) direcci&#243;n(es) en la red.";
success.alert3="Si est&#225;s conectado via WLAN, por favor, &#250;nete a la red y haz click en <em>Continuar</em>.";

// *****************************************************		OLD PAGES 		************************************************************************//
// **************************************************************** DHCPTable.asp **********************************************************************//

var dhcp = new Object();
dhcp.titl=" - Tabla IP Activa en DHCP";
dhcp.h2="Tabla IP Activa en DHCP";
dhcp.server="Direcci&#243;n IP del Servidor DHCP :";
dhcp.tclient="Nombre del Cliente Host";

var donate = new Object();
donate.mb="Puedes donar a trav&#233;s de la cuenta Moneybookers - mb@dd-wrt.com";