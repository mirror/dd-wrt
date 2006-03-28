<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - MAC Filter</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "Wireless_MAC";
	F.change_action.value = "apply_cgi";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function setMAC(val) {
	setElementActive("wl_macmode", val == "other");
}

addEvent(window, "load", function() {
	setMAC("<% nvram_get("wl_macmode1"); %>");
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
								<li class="current"><span>Wireless</span>
								  <div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Wireless_Basic.asp">Basic Settings</a></li>
											<li><a href="Wireless_radauth.asp">Radius</a></li>
											<li><a href="WL_WPATable.asp">Wireless Security</a></li>
											<li><span>MAC Filter</span></li>
											<li><a href="Wireless_Advanced.asp">Advanced Settings</a></li>
											<li><a href="Wireless_WDS.asp">WDS</a></li>
										</ul>
									</div>
								</li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2>Wireless MAC Filter</h2>
							<fieldset>
								<legend>MAC Filter</legend>
								<div class="setting">
									<div class="label">Use Filter</div>
									<input type="radio" value="other" name="wl_macmode1" onclick="setMAC(this.value)" <% nvram_match("wl_macmode1","other","checked"); %> />Enable&nbsp;
									<input type="radio" value="disabled" name="wl_macmode1" onclick="setMAC(this.value)" <% nvram_match("wl_macmode1","disabled","checked"); %> />Disable
								</div>
								<div class="setting">
									<div class="label">Filter Mode<br />&nbsp;</div>
									<input type="radio" value="deny" name="wl_macmode" <% nvram_invmatch("wl_macmode","allow","checked"); %> />Prevent PCs listed from accessing the wireless network<br />
									<input type="radio" value="allow" name="wl_macmode" <% nvram_match("wl_macmode","allow","checked"); %> />Permit only PCs listed to access the wireless network
								</div><br />
								<div class="center">
									<input type="button" name="mac_filter_button" value="Edit MAC Filter List" onclick="openWindow('WL_FilterTable.asp', 600, 530)" />
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="button" value="Cancel Changes" name="cancel" onclick="window.location.reload()" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2>Help</h2>
						</div><br />
						<a href="javascript:openHelpWindow('HWirelessMAC.asp')">More...</a>
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