<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Diagnostics</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]> <link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /> <![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F, I) {
	if(!valid(F, I)) return;
	F.submit_type.value = I;
	F.submit_button.value = "Ping";
	F.change_action.value = "gozila_cgi";
	apply(F);
}

function valid(F,I) {
	if(I == "start" && F.ping_ip.value == ""){
		alert("You must input a command to run!");
		F.ping_ip.focus();
		return false;
	}
	return true;
}
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
								<li class="current"><span>Administration</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp">Management</a></li>
											<li><a href="Hotspot.asp">Hotspot</a></li>
											<li><a href="Services.asp">Services</a></li>
											<li><a href="Alive.asp">Keep Alive</a></li>
											<li><a href="Log.asp">Log</a></li>
											<li><span>Diagnostics</span></li>
											<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
											<li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Firmware Upgrade</a>","<a href="Upgrade.asp">Firmware Upgrade</a>"); %></li>
											<li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Backup</a>","<a href="config.asp">Backup</a>"); %></li>
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
						<form name="ping" action="apply.cgi" method="post">
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="submit_type" value="start" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="ping_times" value="1" />
							<input type="hidden" name="next_page" value="Diagnostics.asp" />
							<h2>Diagnostics</h2>
							<fieldset>
								<legend>Command Shell</legend>
								<div class="setting">
									<div class="label">Commands</div>
									<textarea id="ping_ip" name="ping_ip" rows="6" cols="40" style="font-family:Courier, Courier New"><% nvram_get("ping_ip"); %></textarea>
								</div><br />
								<script type="text/javascript">
var table = new Array(<% dump_ping_log(""); %>);
if(table.length > 0 && location.href.indexOf("Diagnostics.asp") == -1) {
	document.write("<pre style=\"padding:.906em; background:#000; color:#fff; overflow:scroll;\">" + table.join("\n") + "</pre>");
}
								</script>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" value="Run Commands" name="ping" onclick="to_submit(this.form, 'start');"/>
								<input type="button" value="Save Startup" name="startup" onclick="to_submit(this.form, 'startup');" />
								<input type="button" value="Save Firewall" name="firewall" onclick="to_submit(this.form, 'firewall');" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Commands: </dt>
							<dd class="definition">You can run command lines via the webinterface. Fill the text area with your command and click <i>Run Commands</i> to submit.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HDiagnostics.asp');">More...</a>
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