<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Keep Alive</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = '<% nvram_get("router_name"); %>'+alive.titl;

function to_reboot(F) {
	F.action.value='Reboot';
	F.submit();
	return true;
}

function to_submit(F) {
	F.submit_button.value = "Alive";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value="Apply";
	apply(F);
}

function setWDS(val) {
	setElementsActive("wds_watchdog_interval_sec", "wds_watchdog_ips", val == 1);
}

function setAlive() {
	alive = document.getElementsByName('schedule_enable');
	if (alive[0].checked) {	// enable
		time = document.getElementsByName('schedule_hour_time');
		if (time[0].checked) { // Time
			setElementsActive("schedule_hour_time", "schedule_time", true);
			setElementActive("schedule_hour_time", true);
			setElementsActive("schedule_hours", "schedule_weekdays", false);
		} else { //At a set Time
			setElementsActive("schedule_hour_time", "schedule_weekdays", true);
			setElementActive("schedule_time", false);
		}
	} else { // disable
		setElementsActive("schedule_hour_time", "schedule_weekdays", false);
  }


function init() {
	setWDS(<% nvram_get("wds_watchdog_enable"); %>);
	setAlive();
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
											<li><span><script type="text/javascript">Capture(bmenu.adminAlive)</script></span></li>
											<li><a href="Log.asp"><script type="text/javascript">Capture(bmenu.adminLog)</script></a></li>
											<li><a href="Diagnostics.asp"><script type="text/javascript">Capture(bmenu.adminDiag)</script></a></li>
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
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="reboot_button" />
							<input type="hidden" name="commit" value="1" />
							<h2><script type="text/javascript">Capture(alive.h2)</script></h2>
							<% show_modules(".webalive"); %>
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"init()\" />")</script>
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
							<dt class="term"><script type="text/javascript">Capture(halive.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(halive.right2)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(halive.right3)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(halive.right4)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HAlive.asp');"><script type="text/javascript">Capture(share.more)</script></a>
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