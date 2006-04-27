<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html>
	<head> 
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - WOL</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript"> 

document.title = "<% nvram_get("router_name"); %>" + wol.titl;

function guess_broadcast(ip) {
	var netmask = "<% nvram_get("lan_netmask"); %>".split(".");
	var ipaddr = ip.split(".");	
	var network = new Array();
	var broadcast = new Array();
	for (var x=0; x<4; x++) {
		network[x] = eval(netmask[x] & ipaddr[x]);
		broadcast[x] = ((network[x]) ^ (~ netmask[x]) & 255);
	}
	return broadcast.join(".");
}

function get_wol_hosts() {
	return "<% nvram_get("wol_hosts"); %>";
}

function get_static_leases() {
	return "<% nvram_get("static_leases"); %>";
}

function add_wol_host(F) {
	edit_wol_hosts(F.wol_hosts_mac.value, F.wol_hosts_hostname.value, F.wol_hosts_ip.value, "true");
}

function edit_wol_hosts(mac, host, ip, add) {
	F = document.forms["ping"];
	var wol_hosts = get_wol_hosts();
	
	if(add == "true") {
		wol_hosts = wol_hosts + " " + mac + "=" + host + "=" + ip;
	} else {
		var current_hosts = wol_hosts.split(" ");
		var wol_hosts = '';
		while (current_hosts.length > 0) {
			var host = current_hosts.shift();
			if (host.indexOf(mac) == -1) {
				wol_hosts = wol_hosts + host + " ";
			}
		}
	}
	
	if (wol_hosts.indexOf(" ") == 0) {
		wol_hosts = wol_hosts.substr(1);
	}

//    F.ping_ip.value = "";
	F.wol_type.value = "update";
	F.wol_hosts.value = wol_hosts;
	apply(F);
}

function submit_wol(mac, ip) {
	F = document.forms["ping"];
/*
	cmd = F.ping_ip.value;
	cmd = cmd + ip + " -p ";
	cmd = cmd + F.manual_wol_port.value + " ";
	cmd = cmd + mac;
	F.ping_ip.value = cmd;
*/
    F.manual_wol_mac.value = mac;
    F.manual_wol_network.value = ip;
	F.wol_type.value = "wol";
	apply(F);
}

function submit_manual_wol(F) {
	if(!valid(F)) return;
	
	F.manual_wol_mac.value = F.manual_wol_mac.value.replace("\n", " ");
/*
	cmd = F.ping_ip.value;
	cmd = cmd + F.manual_wol_network.value + " -p ";
	cmd = cmd + F.manual_wol_port.value + " ";
	cmd = cmd + F.manual_wol_mac.value;
	F.ping_ip.value = cmd;
*/
	F.wol_type.value = "manual";
	apply(F);
}

function valid(F) {
	if(F.manual_wol_mac.value == ""){
//		alert("You must input a MAC address to run.");
		alert(errmsg.err35);
		F.manual_wol_mac.focus();
		return false;
	}
	if(F.manual_wol_network.value == ""){
//		alert("You must input a network broadcast address to run.");
		alert(errmsg.err36);
		F.manual_wol_network.focus();
		return false;
	}
	if(F.manual_wol_port.value == ""){
//		alert("You must input a UDP port to run.");
		alert(errmsg.err37);
		F.manual_wol_port.focus();
		return false;
	}
	return true;
}

function display_static_leases() {
	var static_leases = get_static_leases().split(" ");
	while (static_leases.length > 0) {
		var lease = static_leases.shift().split("=")
		var mac = lease[0];
		var host = lease[1];
		var ip = lease[2];
		if(mac!=undefined && host!=undefined && ip!=undefined) {
			document.write("<tr>");
			document.write("\t<td>" + mac + "</td>");
			document.write("\t<td >" + host + "</td>");
			document.write("\t<td>" + ip + "</td>");
			ip = guess_broadcast(ip);
			document.write("\t<td align=\"center\">");
			if(get_wol_hosts().indexOf(mac) == -1) {
				document.write("\t\t<input type=checkbox value=\"0\" onclick=\"edit_wol_hosts('" + mac + "','" + host + "','" + ip + "','true');\" />");
			} else {
				document.write("\t\t<input type=checkbox value=\"1\" onclick=\"edit_wol_hosts('" + mac + "','" + host + "','" + ip + "','false');\" checked/>");
			}
			document.write("\t</td>");
			document.write("</tr>");
		} 
	}
}

function display_manual_wol_hosts() {
	var wol_hosts = get_wol_hosts().split(" ");
	while (wol_hosts.length > 0) {
		var host = wol_hosts.shift().split("=")
		var mac = host[0];
		var hostname = host[1];
		var ip = host[2];
		if(get_static_leases().indexOf(mac) == -1 && mac!=undefined && host!=undefined && ip!=undefined) {
			document.write("<tr>");
			document.write("\t<td>" + mac + "</td>");
			document.write("\t<td >" + hostname + "</td>");
			document.write("\t<td>" + ip + "</td>");
			ip = ip.substring(0,ip.lastIndexOf(".")) + ".255";
			document.write("\t<td align=\"center\">");
			if(get_wol_hosts().indexOf(mac) == -1) {
				document.write("\t\t<input type=checkbox value=\"0\" onclick=\"edit_wol_hosts('" + mac + "','" + host + "','" + ip + "','true');\" />");
			} else {
				document.write("\t\t<input type=checkbox value=\"1\" onclick=\"edit_wol_hosts('" + mac + "','" + host + "','" + ip + "','false');\" checked=\"checked\"/>");
			}
			document.write("\t</td>");
			document.write("</tr>");
		} 
	}
}

function display_wol_hosts() {
	var wol_hosts = get_wol_hosts().split(" ");
	while (wol_hosts.length > 0) {
		var host = wol_hosts.shift().split("=")
		var mac = host[0];
		var hostname = host[1];
		var ip = host[2];
		if(mac!=undefined && hostname!=undefined && ip!=undefined) {
			document.write("<tr>");
			document.write("\t<td>" + mac + "</td>");
			document.write("\t<td >" + hostname + "</td>");
			document.write("\t<td>" + ip + "</td>");
			document.write("\t<td>");
			document.write("\t\t<input type=button value=\"" + sbutton.wol + "\" onclick=\"submit_wol('" + mac + "','" + ip + "');\" />");
			document.write("\t</td>");
			document.write("</tr>");
		} 
	}
}

	</script>

</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper"> 
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
								<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.admin)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.adminManagement)</script></a></li>
											<li><a href="Hotspot.asp"><script type="text/javascript">Capture(bmenu.adminHotspot)</script></a></li>
											<li><a href="Services.asp"><script type="text/javascript">Capture(bmenu.adminServices)</script></a></li>
											<li><a href="Alive.asp"><script type="text/javascript">Capture(bmenu.adminAlive)</script></a></li>
											<li><a href="Log.asp"><script type="text/javascript">Capture(bmenu.adminLog)</script></a></li>
											<li><a href="Diagnostics.asp"><script type="text/javascript">Capture(bmenu.adminDiag)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminWol)</script></span></li>
											<li><a href="Factory_Defaults.asp"><script type="text/javascript">Capture(bmenu.adminFactory)</script></a></li>
											<li><a href="Upgrade.asp"><script type="text/javascript">Capture(bmenu.adminUpgrade)</script></a></li>
											<li><a href="config.asp"><script type="text/javascript">Capture(bmenu.adminBackup)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ping" action="apply.cgi" method="post">
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="submit_type" value="wol" />
							<input type="hidden" name="wol_hosts" value="" />
							<input type="hidden" name="wol_type" value="wol" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="ping_times" value="1" />
							<input type="hidden" name="next_page" value="Wol.asp" />
							<input type="hidden" name="ping_ip" value="/usr/sbin/wol -v -i " />

							<h2><script type="text/javascript">Capture(wol.h2)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(wol.legend)</script></legend>
								<table class="table center" cellspacing="5" id="static_lease_table">
									<tr>
										<th width="25%"><script type="text/javascript">Capture(share.mac)</script></th>
										<th width="35%"><script type="text/javascript">Capture(share.hostname)</script></th>
										<th width="20%"><script type="text/javascript">Capture(share.ip)</script></th>
										<th width="30%"><script type="text/javascript">Capture(wol.enable)</script></th>
									</tr>
									<% nvram_selmatch("static_leases","","<!--"); %>
									<script type="text/javascript">	display_static_leases(); </script>
									<% nvram_selmatch("static_leases","","-->"); %>
									<script type="text/javascript">	display_manual_wol_hosts(); </script>
								</table>
							</fieldset><br />

							<fieldset>
								<legend><script type="text/javascript">Capture(wol.legend2)</script></legend>
								<table class="table center" cellspacing="5" id="wol_hosts_table">
									<tr>
										<th width="25%"><script type="text/javascript">Capture(share.mac)</script></th>
										<th width="35%"><script type="text/javascript">Capture(share.hostname)</script></th>
										<th width="20%"><script type="text/javascript">Capture(wol.broadcast)</script></th>
									</tr>
									<script type="text/javascript">	display_wol_hosts(); </script>
									<tr></tr>
									<tr>
										<td><input maxlength="17" size="17" id="wol_hosts_mac" name="wol_hosts_mac" value="00:00:00:00:00:00"/></td>
										<td><input maxlength="17" size="17" id="wol_hosts_hostname" name="wol_hosts_hostname" value=""/></td>
										<td><input maxlength="15" size="15" id="wol_hosts_ip" name="wol_hosts_ip" value="192.168.1.255"/></td>
										<td><script type="text/javascript">document.write("<input type=\"button\" name=\"add\" value=\"" + sbutton.add_wol + "\" onclick=\"add_wol_host(this.form)\" />")</script></td>
									</tr>
								</table>
							</fieldset><br />

							<% nvram_selmatch("wol_cmd","","<!--"); %>
							<script type="text/javascript">
								var table = new Array(<% dump_ping_log(""); %>);
								if(table.length > 0 && location.href.indexOf("Wol.asp") == -1) {
									document.write("<fieldset>");
									document.write("<legend>" + wol.legend3 + "</legend>");
									document.write("<br /><pre style=\"margin: 0\">" + table.join("\n") + "</pre>");
									document.write("</fieldset><br />");
								}
							</script>
							<% nvram_selmatch("wol_cmd","","-->"); %>

							<fieldset> 
								<legend><script type="text/javascript">Capture(wol.legend4)</script></legend>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(wol.mac)</script></div>
										<textarea id="manual_wol_mac" name="manual_wol_mac" rows="3"  cols="20"><% nvram_get("manual_wol_mac"); nvram_selmatch("manual_wol_mac","","00:00:00:00:00:00"); %></textarea>
									</div>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(wol.broadcast)</script></div>
										<input maxlength="15" size="15" id="manual_wol_network" name="manual_wol_network" size="20" value='<% nvram_get("manual_wol_network"); nvram_selmatch("manual_wol_network","","192.168.1.255"); %>'/>
									</div>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(wol.udp)</script></div>
										<input class="num" maxlength="5" size="5" id="manual_wol_port" name="manual_wol_port" onblur="valid_range(this,1,65535,'Port number')"  value='<% nvram_get("manual_wol_port"); nvram_selmatch("manual_wol_port","","7"); %>'/>
									</div>

								<div class="submitFooter">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"ping\" value=\"" + sbutton.manual_wol + "\" onclick=\"submit_manual_wol(this.form)\" />")</script>
								</div>
							</fieldset><br />

						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div>
						<dl> 
							<dt class="term"><script type="text/javascript">Capture(hwol.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hwol.right2)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HWol.asp');"><script type="text/javascript">Capture(share.more)</script></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>