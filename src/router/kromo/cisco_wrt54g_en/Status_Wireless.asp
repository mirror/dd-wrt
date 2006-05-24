<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% charset(); %>
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<title><% nvram_get("router_name"); %> - Wireless Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + status_wireless.titl;

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
		
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;

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
		
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;
		
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
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.statu)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statuRouter)</script></a></li>
											<li><a href="Status_Lan.asp"><script type="text/javascript">Capture(bmenu.statuLAN)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.statuWLAN)</script></span></li>
											<% show_sputnik(); %>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm"><script type="text/javascript">Capture(bmenu.statuSysInfo)</script></a></li>
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
						<form>
							<h2><script type="text/javascript">Capture(status_wireless.h2)</script></h2>
							
							<fieldset>
								<legend><script type="text/javascript">Capture(status_wireless.legend)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.mac)</script></div>
									<script>document.write("<span id=\"wl_mac\" style=\"cursor:pointer\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("wl0_hwaddr"); %>')\" >");</script><% nvram_get("wl0_hwaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.mode)</script></div>
									<span id="wl_mode"><% nvram_match("wl_mode", "wet", "Client Bridge"); %><% nvram_match("wl_mode", "ap", "AP"); %><% nvram_match("wl_mode", "sta", "Client"); %><% nvram_match("wl_mode", "infra", "Adhoc"); %><% nvram_match("wl_mode", "apsta", "Repeater"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_wireless.net)</script></div>
									<span id="wl_net_mode"><% nvram_match("wl_net_mode", "disabled", "Disabled"); %><% nvram_match("wl_net_mode", "mixed", "Mixed"); %><% nvram_match("wl_net_mode", "g-only", "G-Only"); %><% nvram_match("wl_net_mode", "b-only", "B-Only"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.ssid)</script></div>
									<span id="wl_ssid"><% nvram_get("wl_ssid"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.channel)</script></div>
									<span id="wl_channel"><% get_curchannel(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Xmit</div>
									<span id="wl_xmit"><% nvram_get("txpwr"); %> mW</span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.rates)</script></div>
									<span id="wl_rate"><% get_currate(); %> Mbps</span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.encrypt)</script></div>
									<span id="wl_security"><% nvram_match("security_mode", "disabled", "Disabled"); %><% nvram_invmatch("security_mode", "disabled", "Enabled,&nbsp;"); %><% nvram_match("security_mode", "psk", "WPA Pre-shared Key"); %><% nvram_match("security_mode", "wpa", "WPA RADIUS"); %><% nvram_match("security_mode", "psk2", "WPA2 Pre-Shared Key Only"); %><% nvram_match("security_mode", "wpa2", "WPA2 RADIUS Only"); %><% nvram_match("security_mode", "psk psk2", "WPA2 Pre-Shared Key Mixed"); %><% nvram_match("security_mode", "wpa wpa2", "WPA2 RADIUS Mixed"); %><% nvram_match("security_mode", "radius", "RADIUS"); %><% nvram_match("security_mode", "wep", "WEP"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_wireless.pptp)</script></div>
									<span id="pptp"><% nvram_else_match("pptpd_connected", "1", "Connected", "Disconnected"); %></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(status_wireless.legend2)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_wireless.rx)</script></div>
									<span id="packet_rx"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_wireless.tx)</script></div>
									<span id="packet_tx"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<h2><script type="text/javascript">Capture(status_wireless.h22)</script></h2>
							<fieldset>
								<legend id="wireless_table_legend"><script type="text/javascript">Capture(status_wireless.legend3)</script></legend>
								<table class="table center" cellspacing="6" id="wireless_table">
									<tr>
										<th width="54%"><script type="text/javascript">Capture(share.mac)</script></th>
										<th width="8%"><script type="text/javascript">Capture(share.signal)</script></th>
										<th width="8%"><script type="text/javascript">Capture(share.noise)</script></th>
										<th width="8%">SNR</th>
										<th width="22%"><script type="text/javascript">Capture(status_wireless.signal_qual)</script></th>
									</tr>
								</table>
							</fieldset><br />
							
							<span id="wds" style="display:none">
								<fieldset>
									<legend><script type="text/javascript">Capture(status_wireless.wds)</script></legend>
									<table class="table center" cellspacing="6" id="wds_table">
										<tr>
											<th width="20%"><script type="text/javascript">Capture(share.mac)</script></th>
											<th width="31%"><script type="text/javascript">Capture(share.descr)</script></th>
											<th width="8%"><script type="text/javascript">Capture(share.signal)</script></th>
											<th width="8%"><script type="text/javascript">Capture(share.noise)</script></th>
											<th width="8%">SNR</th>
											<th width="22%"><script type="text/javascript">Capture(status_wireless.signal_qual)</script></th>
										</tr>
									</table>
								</fieldset><br />
								
							</span>
							<div class="center">
								<script>document.write("<input type=\"button\" name=\"site_survey\" value=\"" + sbutton.survey + "\" onclick=\"<% nvram_else_match("wl_net_mode", "disabled", "alert(errmsg.err59)", "openWindow('Site_Survey.asp', 760, 700)"); %>\" />");</script>
							</div><br />
							<div class="submitFooter">
								<script>document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\" />");</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div>
						<dl>
							<dt class="term"><script type="text/javascript">Capture(share.mac)</script>:</dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_wireless.right2)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_wireless.net)</script>:</dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_wireless.right4)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(share.oui)</script>:</dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_lan.right10)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HStatusWireless.asp');"><script type="text/javascript">Capture(share.more)</script></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><script type="text/javascript">Capture(share.firmware)</script>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
