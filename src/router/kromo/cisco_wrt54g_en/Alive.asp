<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Keep Alive</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">

function to_reboot(F) {
	F.action.value='Reboot';
	F.submit();
	return true;
}

function to_submit(F) {
	F.submit_button.value = "Alive";
	F.save_button.value = "Saved";

	F.action.value="Apply";
	apply(F);
	return true;
}

function setWDS(val) {
	setElementsActive("wds_watchdog_interval_sec", "wds_watchdog_ips", val == 1);
}

function alive_enable_disable(val) {
  if (val == 1) {
	  setElementsActive("schedule_hours", "schedule_weekdays", val == 2);
	  setElementActive("schedule_hour_time", val == 1);
	  setElementActive("schedule_time", val == 1);
  }
  else if (val == 2) {
  	setElementsActive("schedule_hours", "schedule_weekdays", val == 2);
  	setElementActive("schedule_hour_time", val == 2);
  	setElementActive("schedule_time", val == 1);
  }
  else {
    setElementsActive("schedule_hour_time", "schedule_weekdays", val == 9);
  }
}

function init() {
	setWDS(<% nvram_get("wds_watchdog_enable"); %>);
	if (<% nvram_get("schedule_enable"); %> == 1) {
	  alive_enable_disable(<% nvram_get("schedule_hour_time"); %>);
	} else {
	  alive_enable_disable(0)
	}
}
		</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
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
											<li><span>Keep Alive</span></li>
											<li><a href="Log.asp">Log</a></li>
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
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="reboot_button"/>
							<input type="hidden" name="commit" value="1"/>
							<h2>Keep Alive</h2>
							<% show_modules(".webalive"); %>
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="reset" value="Cancel Changes" onclick="init()" />
								<input type="button" value="Reboot Router" onclick="to_reboot(this.form)" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">At a set Time:</dt>
							<dd class="definition">Choose when reboot the router. Cron must be enabled in the managment tab.</dd>
							<dt class="term">IP Addresses:</dt>
							<dd class="definition">A maximum of three IPs separated by a <em>SPACE</em> is allowed.<BR/>IPs Format is xxx.xxx.xxx.xxx.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HAlive.asp');">More...</a>
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