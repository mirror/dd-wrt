<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Diagnostics</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]> <link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /> <![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = '<% nvram_get("router_name"); %>' + diag.titl;

function to_submit(F, I) {
	if(!valid(F, I)) return;
	F.submit_type.value = I;
	F.submit_button.value = "Ping";
	F.change_action.value = "gozila_cgi";
	apply(F);
}

function valid(F,I) {
	if(I == "start" && F.ping_ip.value == ""){
//		alert("You must input a command to run!");
		alert(errmsg.err12);
		F.ping_ip.focus();
		return false;
	}
	return true;
}
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
								<li class="current"><span><script type="text/javascript">Capture(bmenu.admin)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.adminManagement)</script></a></li>
											<li><a href="Hotspot.asp"><script type="text/javascript">Capture(bmenu.adminHotspot)</script></a></li>
											<li><a href="Services.asp"><script type="text/javascript">Capture(bmenu.adminServices)</script></a></li>
											<li><a href="Alive.asp"><script type="text/javascript">Capture(bmenu.adminAlive)</script></a></li>
											<li><a href="Log.asp"><script type="text/javascript">Capture(bmenu.adminLog)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminDiag)</script></span></li>
											<li><a href="Factory_Defaults.asp"><script type="text/javascript">Capture(bmenu.adminFactory)</script></a></li>
											<li><a href="Upgrade.asp"><script type="text/javascript">Capture(bmenu.adminUpgrade)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminBackup)</script></span></li>
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
						<form name="ping" action="apply.cgi" method="post" >
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="submit_type" value="start" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="ping_times" value="1" />
							<input type="hidden" name="next_page" value="Diagnostics.asp" />
							<h2><script type="text/javascript">Capture(diag.h2)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(diag.legend)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(diag.cmd)</script></div>
									<textarea id="ping_ip" name="ping_ip" rows="6" cols="40" style="font-family:Courier, Courier New"><% nvram_get("ping_ip"); %></textarea>
								</div>
								<script type="text/javascript">
var table = new Array(<% dump_ping_log(""); %>);
if(table.length > 0 && location.href.indexOf("Diagnostics.asp") == -1) {
	document.write("<br /><pre style=\"margin:0\">" + table.join("\n") + "</pre>");
}
								</script>
							</fieldset><br />
							<% nvram_match("rc_startup", "", "<!--"); %><fieldset>
								<legend><script type="text/javascript">Capture(diag.startup)</script></legend>
								<pre id="startup" style="margin:0"><% nvram_get("rc_startup"); %></pre><br />
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"button_start\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('startup').firstChild.data\" />")</script>
								</div>
							</fieldset><br /><% nvram_match("rc_startup", "", "-->"); %>
							<% nvram_match("rc_firewall", "", "<!--"); %><fieldset>
								<legend><script type="text/javascript">Capture(diag.firewall)</script></legend>
								<pre id="firewall" style="margin:0"><% nvram_get("rc_firewall"); %></pre><br />
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"button_start\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('firewall').firstChild.data\" />")</script>
								</div>
							</fieldset><br /><% nvram_match("rc_firewall", "", "-->"); %>
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"ping\" value=\"" + sbutton.runcmd + "\" onclick=\"to_submit(this.form, 'start')\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"startup\" value=\"" + sbutton.startup + "\" onclick=\"to_submit(this.form, 'startup')\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"firewall\" value=\"" + sbutton.firewall + "\" onclick=\"to_submit(this.form, 'firewall')\" />")</script>
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
							<dt class="term"><script type="text/javascript">Capture(hdiag.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hdiag.right2)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HDiagnostics.asp');"><script type="text/javascript">Capture(share.more)</script></a>
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