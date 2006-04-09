<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - LAN Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
function deleteLease(val) {
	document.forms[0].d_0.value = val;
	document.forms[0].submit();
}

function setDHCPTable() {
	var val = arguments;
	var table = document.getElementById("dhcp_leases_table");
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 5;
		cell.align = "center";
		cell.innerHTML = "- None - ";
		return;
	}
	for(var i = 0; i < val.length; i = i + 5) {
		var row = table.insertRow(-1);
		row.style.height = "15px";
		row.insertCell(-1).innerHTML = val[i];
		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		var cell = row.insertCell(-1);
		cell.className = "bin";
		cell.title = "Click to delete lease";
		eval("addEvent(cell, 'click', function() { deleteLease(" + val[i + 4] + ") })");
	}
}

var update;

addEvent(window, "load", function() {
	setElementContent("dhcp_end_ip", "<% prefix_ip_get("lan_ipaddr",1); %>" + (parseInt("<% nvram_get("dhcp_start"); %>") + parseInt("<% nvram_get("dhcp_num"); %>") - 1));
	setDHCPTable(<% dumpleases(0); %>);
	setElementVisible("dhcp_1", "<% nvram_get("lan_proto"); %>" == "dhcp");
	setElementVisible("dhcp_2", "<% nvram_get("lan_proto"); %>" == "dhcp");

	update = new StatusUpdate("Status_Lan.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("lan_proto", function(u) {
		setElementVisible("dhcp_1", u.lan_proto == "dhcp");
		setElementVisible("dhcp_2", u.lan_proto == "dhcp");
	});
	update.onUpdate("dhcp_start", function(u) {
		setElementContent("dhcp_start_ip", u.lan_ip_prefix + u.dhcp_start);
		setElementContent("dhcp_end_ip", u.lan_ip_prefix + (parseInt(u.dhcp_start) + parseInt(u.dhcp_num) - 1));
	});
	update.onUpdate("dhcp_leases", function(u) {
		eval('setDHCPTable(' + u.dhcp_leases + ')');
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
		</script>
	 </head>

	 <body class="gui"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li><a href="Wireless_Basic.asp">Wireless</a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li><a href="Management.asp">Administration</a></li>
								<li class="current"><span>Status</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Status_Router.asp">Router</a></li>
											<li><span>LAN</span></li>
											<li><a href="Status_Wireless.asp">Wireless</a></li>
											<% show_sputnik(); %>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm">Sys-Info</a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
										</ul>
									</div>
								</li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="DHCPTable" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" value="delete" />
							<input type="hidden" name="d_0" />
							<input type="hidden" name="next_page" value="Status_Lan.asp" />
							<h2>Local Network</h2>
							<fieldset>
								<legend>LAN Status</legend>
								<div class="setting">
									<div class="label">MAC Address</div>
									<span id="lan_mac"><% nvram_get("lan_hwaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">IP Address</div>
									<span id="lan_ip"><% nvram_get("lan_ipaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Subnet Mask</div>
									<span id="lan_netmask"><% nvram_get("lan_netmask"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Gateway</div>
									<span id="lan_gateway"><% nvram_get("lan_gateway"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Local DNS</div>
									<span id="lan_dns"><% nvram_get("sv_localdns"); %></span>&nbsp;
								</div>
							</fieldset><br />
							<h2>Dynamic Host Configuration Protocol</h2>
							<fieldset>
								<legend>DHCP Status</legend>
								<div class="setting">
									<div class="label">DHCP Server</div>
									<span id="dhcp_enabled"><% nvram_match("lan_proto", "dhcp", "Enabled"); %><% nvram_match("lan_proto", "static", "Disabled"); %></span>&nbsp;
								</div>
								<span id="dhcp_1" style="display:none">
									<div class="setting">
										<div class="label">DHCP Daemon</div>
										<span id="dhcp_daemon"><% nvram_else_match("dhcp_dnsmasq", "1", "DNSMasq", "uDHCPd"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">Start IP Address</div>
										<span id="dhcp_start_ip"><% prefix_ip_get("lan_ipaddr", "1"); %><% nvram_get("dhcp_start"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">End IP Address</div>
										<span id="dhcp_end_ip"></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">Client Lease Time</div>
										<span id="dhcp_lease_time"><% nvram_get("dhcp_lease"); %> minutes</span>&nbsp;
									</div>
								</span>
							</fieldset><br />
							<span id="dhcp_2" style="display:none">
								<fieldset>
									<legend>DHCP Clients</legend>
									<table class="table center" cellspacing="6" id="dhcp_leases_table">
										<tr>
											<th width="25%">Host&nbsp;Name</th>
											<th width="25%">IP&nbsp;Address</th>
											<th width="25%">MAC&nbsp;Address</th>
											<th width="25%">Expires</th>
											<th>Delete</th>
										</tr>
									</table>
								</fieldset><br />
							</span>
							<div class="submitFooter">
								<input type="button" name="refresh_button" value="<% nvram_else_match("refresh_time","0","Refresh","Auto-Refresh ON"); %>" onclick="window.location.reload()" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">MAC Address: </dt>
							<dd class="definition">This is the Router's MAC Address, as seen on your local, Ethernet network.</dd>
							<dt class="term">IP Address: </dt>
							<dd class="definition">This shows the Router's IP Address, as it appears on your local, Ethernet network.</dd>
							<dt class="term">Subnet Mask: </dt>
							<dd class="definition">When the Router is using a Subnet Mask, it is shown here.</dd>
							<dt class="term">DHCP Server: </dt>
							<dd class="definition">If you are using the Router as a DHCP server, that will be displayed here.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HStatusLan.asp')">More...</a></div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>