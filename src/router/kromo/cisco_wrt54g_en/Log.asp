<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Log</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "Log";
	F.action.value = "Apply";
	apply(F);
}

function setLog(val) {
	setElementsActive("log_level", "log_outgoing", val == "1");
	setElementActive("log_all", val == "1");
}

addEvent(window, "load", function() {
	setLog("<% nvram_get("log_enable"); %>");
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
								<li class="current"><span>Administration</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp">Management</a></li>
											<li><a href="Hotspot.asp">Hotspot</a></li>
											<li><a href="Services.asp">Services</a></li>
											<li><a href="Alive.asp">Keep Alive</a></li>
											<li><span>Log</span></li>
											<li><a href="Diagnostics.asp">Diagnostics</a></li>
											<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
											<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											<li><a href="config.asp">Backup</a></li>
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="log" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="action"/>
							<h2>Log Management</h2>
							<fieldset>
								<legend>Log</legend>
								<div class="setting">
									<div class="label">Log</div>
									<input type="radio" value="1" name="log_enable" <% nvram_checked("log_enable", "1"); %> onclick="setLog(this.value)" />Enable
									<input type="radio" value="0" name="log_enable" <% nvram_checked("log_enable", "0"); %> onclick="setLog(this.value)" />Disable
								</div>
								<div class="setting">
									<div class="label">Log Level</div>
									<select name="log_level">
										<option value="0" <% nvram_selected("log_level", "0"); %>>Low</option>
										<option value="1" <% nvram_selected("log_level", "1"); %>>Medium</option>
										<option value="2" <% nvram_selected("log_level", "2"); %>>High</option>
									</select>
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Options</legend>
								<div class="setting">
									<div class="label">Dropped</div>
									<select name="log_dropped">
										<option value="0" <% nvram_invmatch("log_dropped", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_dropped", "1", "selected"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label">Rejected</div>
									<select name="log_rejected">
										<option value="0" <% nvram_invmatch("log_rejected", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_rejected", "1", "selected"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label">Accepted</div>
									<select name="log_accepted">
										<option value="0" <% nvram_invmatch("log_accepted", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_accepted", "1", "selected"); %>>On</option>
									</select>
								</div>
								<% support_invmatch("SYSLOG_SUPPORT", "1", "<!--"); %>
								<div class="setting">
									<div class="label">IP Address</div>
									<% prefix_ip_get("lan_ipaddr",1); %>
									<input class="num" size="3" maxlength="3" name="log_ipaddr" onblur="valid_range(this,0,254,'IP')" value="<% nvram_get("log_ipaddr"); %>" />
								</div>
								<% support_invmatch("SYSLOG_SUPPORT", "1", "-->"); %>
							</fieldset><br />
							<div class="center">
								<input type="button" value="Incoming Log" name="log_incoming" onclick="openWindow('Log_incoming.asp', 580, 600)" />
								<input type="button" value="Outgoing Log" name="log_outgoing" onclick="openWindow('Log_outgoing.asp', 760, 600)" /><% support_invmatch("SYSLOG_SUPPORT", "1", "<!--"); %>
								<input type="button" value="System Log" name="log_all" onclick="openWindow('Log_all.asp', 580, 600)" /><% support_invmatch("SYSLOG_SUPPORT", "1", "-->"); %>
							</div><br />
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="button" value="Cancel Changes" onclick="window.location.replace('Log.asp')" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<br />
						<a href="javascript:openHelpWindow('HLog.asp');">More...</a>
					</div>
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