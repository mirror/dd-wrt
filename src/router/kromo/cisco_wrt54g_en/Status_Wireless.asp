<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Wireless Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function setWirelessTable() {
	var table = document.getElementById("wireless_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 5;
		cell.align = "center";
		cell.innerHTML = "- None - ";
		return;
	}
	for(var i = 0; i < val.length; i = i + 4) {
		var row = table.insertRow(-1);
		row.insertCell(-1).innerHTML = val[i];
		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		setMeterBar(row.insertCell(-1), (val[i + 1] == "0" ? 0 : parseInt(val[i + 1]) * 1.24 + 116), "");
	}
}

function setWDSTable() {
	var table = document.getElementById("wds_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		setElementVisible("wds", false);
		return;
	}
	for(var i = 0; i < val.length; i = i + 5) {
		var row = table.insertRow(-1);
		row.insertCell(-1).innerHTML = val[i];
		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		row.insertCell(-1).innerHTML = val[i + 4];
		setMeterBar(row.insertCell(-1), (val[i + 2] == "0" ? 0 : parseInt(val[i + 2]) * 1.24 + 116), "");
	}
	setElementVisible("wds", true);
}

function setPacketInfo(val) {
	var packet = val.replace(/[A-Za-z=]/g, "").split(";");
	setMeterBar("packet_rx",
		(parseInt(packet[1]) == 0 ? 100 : parseInt(packet[0]) / (parseInt(packet[0]) + parseInt(packet[1])) * 100),
		packet[0] + " OK, " + (packet[1] > 0 ? packet[1] : "no") + " errors"
	);
	setMeterBar("packet_tx",
		(parseInt(packet[3]) == 0 ? 100 : parseInt(packet[2]) / (parseInt(packet[2]) + parseInt(packet[3])) * 100),
		packet[2] + " OK, " + (packet[3] > 0 ? packet[3] : "no") + " errors"
	);
}

var update;

addEvent(window, "load", function() {
	setWirelessTable(<% active_wireless(0); %>);
	setWDSTable(<% active_wds(0); %>);
	setPacketInfo("<% wl_packet_get(); %>");

	update = new StatusUpdate("Status_Wireless.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("active_wireless", function(u) {
		eval('setWirelessTable(' + u.active_wireless + ')');
	});
	update.onUpdate("active_wds", function(u) {
		eval('setWDSTable(' + u.active_wds + ')');
	});
	update.onUpdate("packet_info", function(u) {
		setPacketInfo(u.packet_info);
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
											<li><a href="Status_Lan.asp">LAN</a></li>
											<li><span>Wireless</span></li>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm">Sys-Info</a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
											<% show_sputnik(); %>
										</ul>
									</div>
								</li>
							</ul>
						</div>
					</div>
				</div>

				<div id="main">
					<div id="contents">
						<form>
							<h2>Wireless</h2>
							<fieldset>
								<legend>Wireless Status</legend>
								<div class="setting">
									<div class="label">MAC Address</div>
									<span id="wl_mac"><% nvram_get("wl0_hwaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Mode</div>
									<span id="wl_mode"><% nvram_match("wl_mode", "wet", "Client Bridge"); %><% nvram_match("wl_mode", "ap", "AP"); %><% nvram_match("wl_mode", "sta", "Client"); %><% nvram_match("wl_mode", "infra", "Adhoc"); %><% nvram_match("wl_mode", "apsta", "Repeater"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Network</div>
									<span id="wl_net_mode"><% nvram_match("wl_net_mode", "disabled", "Disabled"); %><% nvram_match("wl_net_mode", "mixed", "Mixed"); %><% nvram_match("wl_net_mode", "g-only", "G-Only"); %><% nvram_match("wl_net_mode", "b-only", "B-Only"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">SSID</div>
									<span id="wl_ssid"><% nvram_get("wl_ssid"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Channel</div>
									<span id="wl_channel"><% get_curchannel(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Xmit</div>
									<span id="wl_xmit"><% nvram_get("txpwr"); %> mW</span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Rate</div>
									<span id="wl_rate"><% get_currate(); %> Mbps</span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Encryption</div>
									<span id="wl_security"><% nvram_match("security_mode", "disabled", "Disabled"); %><% nvram_invmatch("security_mode", "disabled", "Enabled,&nbsp;"); %><% nvram_match("security_mode", "psk", "WPA Pre-shared Key"); %><% nvram_match("security_mode", "wpa", "WPA RADIUS"); %><% nvram_match("security_mode", "psk2", "WPA2 Pre-Shared Key Only"); %><% nvram_match("security_mode", "wpa2", "WPA2 RADIUS Only"); %><% nvram_match("security_mode", "psk psk2", "WPA2 Pre-Shared Key Mixed"); %><% nvram_match("security_mode", "wpa wpa2", "WPA2 RADIUS Mixed"); %><% nvram_match("security_mode", "radius", "RADIUS"); %><% nvram_match("security_mode", "wep", "WEP"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">PPTP Status</div>
									<span id="pptp"><% nvram_else_match("pptpd_connected", "1", "Connected", "Disconnected"); %></span>&nbsp;
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Packet Info</legend>
								<div class="setting">
									<div class="label">Received (RX)</div>
									<span id="packet_rx"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Transmitted (TX)</div>
									<span id="packet_tx"></span>&nbsp;
								</div>
							</fieldset><br />
							<h2>Wireless Nodes</h2>
							<fieldset>
								<legend id="wireless_table_legend">Clients</legend>
								<table class="table center" cellspacing="6" id="wireless_table">
									<tr>
										<th width="48%">MAC Address</th>
										<th width="10%">Signal</th>
										<th width="10%">Noise</th>
										<th width="10%">SNR</th>
										<th width="22%">Signal Quality</th>
									</tr>
								</table>
							</fieldset><br />
							<span id="wds">
								<fieldset>
									<legend>WDS</legend>
									<table class="table center" cellspacing="6" id="wds_table">
										<tr>
											<th width="22%">MAC Address</th>
											<th width="22%">Description</th>
											<th width="10%">Signal</th>
											<th width="10%">Noise</th>
											<th width="10%">SNR</th>
											<th width="22%">Signal Quality</th>
										</tr>
									</table>
								</fieldset><br />
							</span>
							<div class="center">
								<input type="button" name="site_survey" value="Site Survey" onClick="<% nvram_else_match("wl_net_mode", "disabled", "alert('Not&nbsp;available!&nbsp;Please&nbsp;enable&nbsp;wireless&nbsp;network.')", "openWindow('Site_Survey.asp', 760, 700)"); %>" />
							</div><br />
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
							<dd class="definition">This is the Router's MAC Address, as seen on your local, wireless network.</dd>
							<dt class="term">Network: </dt>
							<dd class="definition">As selected from the Wireless tab, this will display the wireless mode (Mixed, G-Only, or Disabled) used by the network.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HStatusWireless.asp')">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <a href="javascript:openAboutWindow()"><% get_firmware_version(); %></a></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>