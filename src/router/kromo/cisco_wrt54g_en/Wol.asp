<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - WOL</title>
		<script type="text/javascript">
		//<![CDATA[

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

function get_available_hosts() {
	var available_hosts = get_static_leases();

	var dhcp_hosts = get_dhcp_hosts().split(" ");
	while (dhcp_hosts.length > 0) {
		var host = dhcp_hosts.shift();
		if (available_hosts.indexOf(host) == -1) {
			available_hosts = available_hosts + " " + host;
		}
	}

	while (available_hosts.indexOf(" ") == 0) {
		available_hosts = available_hosts.substr(1);
	}

	return available_hosts;
}

function get_wol_hosts() {
	return "<% nvram_get("wol_hosts"); %>";
}

function get_static_leases() {
	return "<% nvram_get("static_leases"); %>";
}

function get_dhcp_hosts() {
	return parse_dhcp_hosts(<% dumpleases(0); %>);
}

function parse_dhcp_hosts() {
	var val = arguments;
	var dhcp_hosts = '';
	
	if (!val.length)
		return dhcp_hosts;
	
	for(var i = 0; i < val.length; i = i + 5) {
		dhcp_hosts = dhcp_hosts + val[i+2] + "=" + val[i] + "=" + val[i+1] + " ";
	}
	
	if (dhcp_hosts.indexOf(" ") == 0) {
		dhcp_hosts = dhcp_hosts.substr(1);
	}

	return dhcp_hosts;
}

function add_wol_host(F) {
	edit_wol_hosts(F.wol_hosts_mac.value, F.wol_hosts_hostname.value, F.wol_hosts_ip.value, "true");
}

function del_wol_host(mac) {
	edit_wol_hosts(mac, "", "", "false");
}

function edit_wol_hosts(mac, host, ip, add) {
	F = document.forms["ping"];
	var wol_hosts = get_wol_hosts();

	var validate = new Object;
	validate.value = mac;

	if(!valid_macs_17(validate) || mac == ""){
		if (mac == "") alert(errmsg.err35);
		F.wol_hosts_mac.focus();
		return false;
	}

	if(add == "true") {
		validate.value = ip;
		if(!valid_ip_str(validate) || ip == ""){
		if (ip == "") alert(errmsg.err36);
			F.wol_hosts_ip.focus();
			return false;
		}
		wol_hosts = wol_hosts + " " + mac + "=" + host + "=" + ip;
	} else {
		var current_hosts = wol_hosts.split(" ");
		wol_hosts = ' ';
		while (current_hosts.length > 0) {
			var host = current_hosts.shift();
			if (host.indexOf(mac) == -1) {
				wol_hosts = wol_hosts + host + " ";
			}
		}
	}
	
	while (wol_hosts.indexOf(" ") == 0) {
		wol_hosts = wol_hosts.substr(1);
	}

	if (wol_hosts.length == 0)
		wol_hosts = " ";

	F.wol_type.value = "update";
	F.wol_hosts.value = wol_hosts;
	apply(F);
}

function submit_wol(mac, ip) {
	F = document.forms["ping"];
	F.manual_wol_mac.value = mac;
	F.manual_wol_network.value = ip;
	if(F.manual_wol_port.value == "")
		F.manual_wol_port.value = 7;
		
	F.wol_type.value = "wol";
	apply(F);
}

function submit_manual_wol(F) {
	if(!valid(F))
		return;
	
	F.manual_wol_mac.value = F.manual_wol_mac.value.replace("\n", " ");
	F.wol_type.value = "manual";
	apply(F);
}

function valid(F) {
	if(F.manual_wol_mac.value == ""){
		alert(errmsg.err35);
		F.manual_wol_mac.focus();
		return false;
	}
	if(F.manual_wol_network.value == ""){
		alert(errmsg.err36);
		F.manual_wol_network.focus();
		return false;
	}
	if(F.manual_wol_port.value == ""){
		alert(errmsg.err37);
		F.manual_wol_port.focus();
		return false;
	}
	return true;
}

function valid_port(I) {
	if(I.value == "") return true;
	return valid_range(I, 1, 65535, wol.udp);
}

function setAvailableHostsTable() {
	var available_hosts = get_available_hosts().split(" ");
	
	var table = document.getElementById("available_hosts_table");
	cleanTable(table);

	if(!available_hosts || available_hosts == "," || available_hosts == "") {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 4;
		cell.align = "center";
		cell.innerHTML = "- " + share.none +" -";
		return;
	}

	while(available_hosts.length > 0) {
		var host = available_hosts.shift().split("=");
		var mac = host[0];
		var hostname = host[1];
		var ip = host[2];
		if (mac!=undefined && hostname!=undefined && ip!=undefined) {
			var row = table.insertRow(-1);
			row.style.height = "15px";
			row.insertCell(-1).innerHTML = mac;
			row.insertCell(-1).innerHTML = hostname;
			var cell = row.insertCell(-1);
			cell.align = "right";
			cell.innerHTML = ip;
			ip = guess_broadcast(ip);
			cell = row.insertCell(-1);
			cell.align = "center";	
			if(get_wol_hosts().indexOf(mac) == -1) {
				cell.innerHTML = "\t\t<input type=\"checkbox\" value=\"0\" onclick=\"edit_wol_hosts('" + mac + "','" + hostname + "','" + ip + "','true');\" />";
			} else {
				cell.innerHTML = "\t\t<input type=\"checkbox\" value=\"1\" onclick=\"edit_wol_hosts('" + mac + "','" + hostname + "','" + ip + "','false');\" checked=\"checked\" />";
			}
		}
	}
}

function setWolHostsTable() {
	var wol_hosts = get_wol_hosts().split(" ");
	
	var table = document.getElementById("wol_hosts_table");

	table.insertRow(1).style.height = "8px";

	if(!wol_hosts || wol_hosts == "," || wol_hosts == "") {
		var cell = table.insertRow(1).insertCell(-1);
		cell.colSpan = 4;
		cell.align = "center";
		cell.innerHTML = "- " + share.none +" -";
	}

	while(wol_hosts.length > 0) {
		var host = wol_hosts.shift().split("=");
		var mac = host[0];
		var hostname = host[1];
		var ip = host[2];
		if (mac!=undefined && hostname!=undefined && ip!=undefined) {
			var row = table.insertRow(1);
			row.style.height = "15px";
			row.insertCell(-1).innerHTML = mac;
			row.insertCell(-1).innerHTML = hostname;
			var cell = row.insertCell(-1);
			cell.align = "right";
			cell.innerHTML = ip;
			cell = row.insertCell(-1);
			cell.className = "bin";
			cell.title = wol.msg1;
			eval("addEvent(cell, 'click', function() { del_wol_host('" + mac + "') })");
			row.insertCell(-1).innerHTML = "\t\t<input class=\"button\" type=\"button\" value=\"" + sbutton.wol + "\" onclick=\"submit_wol('" + mac + "','" + ip + "');\" />";
		}
	}

}

function callDump() {
	var table = new Array(<% dump_ping_log(""); %>);
		if(table.length > 0 && location.href.indexOf("Wol.asp") == -1) {
		document.write("<fieldset>");
		document.write("<legend>" + wol.legend3 + "</legend>");
		document.write("<br /><pre>" + table.join("\n") + "</pre>");
		document.write("<\/fieldset><br />");
	}
}

addEvent(window, "load", function() {
	setAvailableHostsTable();
	setWolHostsTable();
	show_layer_ext(document.ping.wol_enable, 'idwol', <% nvram_else_match("wol_enable", "1", "1", "0"); %> == 1);
});
	
		//]]>
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper"> 
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Management.asp","Wol.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ping" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" value="wol" />
							<input type="hidden" name="next_page" value="Wol.asp" />
							
							<input type="hidden" name="wol_hosts" value="" />
							<input type="hidden" name="wol_type" value="wol" />
							
							<h2><% tran("wol.h2"); %></h2>
							<fieldset>
								<legend><% tran("wol.legend"); %></legend>
								<table class="table center" cellspacing="5" id="available_hosts_table" summary="available hosts table">
									<tr>
										<th width="25%"><% tran("share.mac"); %></th>
										<th width="35%"><% tran("share.hostname"); %></th>
										<th width="20%"><% tran("share.ip"); %></th>
										<th width="30%"><% tran("wol.enable"); %></th>
									</tr>
								</table>
							</fieldset><br />

							<fieldset>
								<legend><% tran("wol.legend2"); %></legend>
								<table class="table center" cellspacing="5" id="wol_hosts_table" summary="wol hosts table">
									<tr>
										<th width="25%"><% tran("share.mac"); %></th>
										<th width="35%"><% tran("share.hostname"); %></th>
										<th width="20%"><% tran("wol.broadcast"); %></th>
										<th><% tran("share.remove"); %></th>
									</tr>
									<tr>
										<td><input maxlength="17" size="17" id="wol_hosts_mac" name="wol_hosts_mac" onblur="valid_macs_17(this)" value="" /></td>
										<td><input maxlength="24" size="24" id="wol_hosts_hostname" name="wol_hosts_hostname" value="" /></td>
										<td><input class="num" maxlength="15" size="15" id="wol_hosts_ip" name="wol_hosts_ip" onblur="valid_ip_str(this, wol.broadcast)" value="" /></td>
										<td></td>
										<td><script type="text/javascript">
										//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" name=\"add\" value=\"" + sbutton.add_wol + "\" onclick=\"add_wol_host(this.form);\" />");
										//]]>
										</script></td>
									</tr>
								</table>
							</fieldset><br />

							<% nvram_selmatch("wol_cmd","","<!--"); %><script type="text/javascript">callDump();</script><% nvram_selmatch("wol_cmd","","-->"); %>

							<fieldset> 
								<legend><% tran("wol.legend4"); %></legend>
									<div class="setting">
										<div class="label"><% tran("wol.mac"); %></div>
										<textarea id="manual_wol_mac" name="manual_wol_mac" onblur="valid_macs_list(this)" rows="3" cols="60"><% nvram_get("manual_wol_mac"); %></textarea>
									</div>
									<div class="setting">
										<div class="label"><% tran("share.ip"); %></div>
										<input class="num" maxlength="15" size="15" id="manual_wol_network" onblur="valid_ip_str(this, share.ip)" name="manual_wol_network" value="<% nvram_get("manual_wol_network"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("wol.udp"); %></div>
										<input class="num" maxlength="5" size="5" id="manual_wol_port" name="manual_wol_port" onblur="valid_port(this)"  value="<% nvram_get("manual_wol_port"); %>" />
									</div>

								<div class="submitFooter">
									<script type="text/javascript">
									//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" name=\"ping\" value=\"" + sbutton.manual_wol + "\" onclick=\"submit_manual_wol(this.form);\" />");
									//]]>
									</script>
								</div>
							</fieldset><br />
							
							<h2><% tran("wold.h22"); %></h2>
							<fieldset>
								<legend><% tran("wol.legend5"); %></legend>
								<div class="setting">
									<div class="label"><% tran("wol.srv"); %></div>
									<input class="spaceradio" type="radio" name="wol_enable" value="1" <% nvram_checked("wol_enable", "1"); %> onclick="show_layer_ext(this, 'idwol', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="wol_enable" value="0" <% nvram_checked("wol_enable", "0"); %> onclick="show_layer_ext(this, 'idwol', false)" /><% tran("share.disable"); %>
								</div>
								<div id="idwol">
									<div class="setting">
										<div class="label"><% tran("share.inter"); %></div>
										<input class="num" maxlength="5" size="5" name="wol_interval" onblur="valid_range(this,1,86400,'WOL Interval')" value="<% nvram_get("wol_interval"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
											document.write("(" + share.deflt + ": 86400, " + share.range + ": 1 - 86400)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("share.hostname"); %></div>
										<input maxlength="100" size="25" name="wol_hostname" value="<% nvram_get("wol_hostname"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("wol.pass"); %></div>
										<input maxlength="63" size="25" name="wol_passwd" value="<% nvram_get("wol_passwd"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("wol.mac"); %></div>
										<textarea id="wol_macs" name="wol_macs" cols="60" rows="3"></textarea>
										<script type="text/javascript">
										//<![CDATA[
											var wol_macs = fix_cr( '<% nvram_get("wol_macs"); %>' );
											document.getElementById("wol_macs").value = wol_macs;
										//]]>
										</script>
									</div>
								</div>
							</fieldset><br/>

						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl> 
							<dt class="term"><% tran("wol.h2"); %>:</dt>
							<dd class="definition"><% tran("hwol.right2"); %></dd>
							<dt class="term"><% tran("wol.mac"); %>:</dt>
							<dd class="definition"><% tran("hwol.right4"); %></dd>
							<dt class="term"><% tran("share.ip"); %>:</dt>
							<dd class="definition"><% tran("hwol.right6"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWol.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>
