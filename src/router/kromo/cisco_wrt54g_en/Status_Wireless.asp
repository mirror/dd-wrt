<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Wireless Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
var update;

addEvent(window, "load", function() {
	update = new StatusUpdate("Status_Wireless.live.asp", 3);
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
								<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
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
									<div class="label">DHCP Server</div>
									<span id="lan_dhcp"><% nvram_else_match("lan_proto", "dhcp", "Enabled", "Disabled"); %></span>&nbsp;
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
							<span id="active_wireless"><% active_wireless(0); %></span>
							<span id="active_wds"><% active_wds(0); %></span>
							<div class="center">
								<input type="button" value="Site Survey" onclick="openWindow('Site_Survey.asp', 760, 700)" />
							</div><br />
							<div class="submitFooter">
								<input type="button" value="Refresh" onclick="window.location.replace('Status_Wireless.asp')" />
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
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
