<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% charset(); %>
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<title><% nvram_get("router_name"); %> - Log</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + log.titl;

function to_submit(F) {
	F.submit_button.value = "Log";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

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
								<li class="current"><span><script type="text/javascript">Capture(bmenu.admin)</script></span></li>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.adminManagement)</script></a></li>
											<li><a href="Hotspot.asp"><script type="text/javascript">Capture(bmenu.adminHotspot)</script></a></li>
											<li><a href="Services.asp"><script type="text/javascript">Capture(bmenu.adminServices)</script></a></li>
											<li><a href="Alive.asp"><script type="text/javascript">Capture(bmenu.adminAlive)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminLog)</script></span></li>
											<li><a href="Diagnostics.asp"><script type="text/javascript">Capture(bmenu.adminDiag)</script></a></li>
											<li><a href="Wol.asp"><script type="text/javascript">Capture(bmenu.adminWol)</script></a></li>
											<li><a href="Factory_Defaults.asp"><script type="text/javascript">Capture(bmenu.adminFactory)</script></a></li>
								<script type="text/javascript">
										https_visit = "<% support_elsematch("HTTPS","1","1","0"); %>";
										if (https_visit =="1") {
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminUpgrade + "</em></a></li>");
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminBackup + "</em></a></li>");
										} else {
											document.write("<li><a href=\"Upgrade.asp\">" + bmenu.adminUpgrade + "</a></li>");
											document.write("<li><a href=\"config.asp\">" + bmenu.adminBackup + "</a></li>");
										}											
								</script>
<!--										<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											<li><a href="config.asp">Backup</a></li>
 -->
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
						<form name="log" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><script type="text/javascript">Capture(log.h2)</script></h2>
							
							<fieldset>
								<legend><script type="text/javascript">Capture(log.legend)</script></legend>
								<div class="setting">
									<div class="label">Log</div>
									<input class="spaceradio" type="radio" value="1" name="log_enable" <% nvram_checked("log_enable", "1"); %> onclick="setLog(this.value)" /><script type="text/javascript">Capture(share.enable)</script>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="log_enable" <% nvram_checked("log_enable", "0"); %> onclick="setLog(this.value)" /><script type="text/javascript">Capture(share.disable)</script>
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(log.lvl)</script></div>
									<select name="log_level">
										<script type="text/javascript">document.write("<option value=\"0\" <% nvram_selected("log_level", "0", "js"); %> >" + share.low + "</option>");</script>
										<script type="text/javascript">document.write("<option value=\"1\" <% nvram_selected("log_level", "1", "js"); %> >" + share.medium + "</option>");</script>
										<script type="text/javascript">document.write("<option value=\"2\" <% nvram_selected("log_level", "2", "js"); %> >" + share.high + "</option>");</script>
									</select>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(share.option)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(log.drop)</script></div>
									<select name="log_dropped">
										<option value="0" <% nvram_selected("log_dropped", "0"); %>>Off</option>
										<option value="1" <% nvram_selected("log_dropped", "1"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(log.reject)</script></div>
									<select name="log_rejected">
										<option value="0" <% nvram_selected("log_rejected", "0"); %>>Off</option>
										<option value="1" <% nvram_selected("log_rejected", "1"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(log.accept)</script></div>
									<select name="log_accepted">
										<option value="0" <% nvram_selected("log_accepted", "0"); %>>Off</option>
										<option value="1" <% nvram_selected("log_accepted", "1"); %>>On</option>
									</select>
								</div>
							</fieldset><br />
							
							<div class="center">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"log_incoming\" value=\"" + sbutton.log_in + "\" onclick=\"openWindow('Log_incoming.asp', 580, 600)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"log_outgoing\" value=\"" + sbutton.log_out + "\" onclick=\"openWindow('Log_outgoing.asp', 760, 600)\" />")</script>
							</div><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"setLog('<% nvram_get("log_enable"); %>')\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div>
						<a href="javascript:openHelpWindow('HLog.asp');"><script type="text/javascript">Capture(share.more)</script></a>
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