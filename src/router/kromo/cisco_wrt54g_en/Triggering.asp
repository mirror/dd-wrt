<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Port Triggering</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript">
function trigger_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Triggering";
	F.submit_type.value = "add_trigger";
	F.action.value = "Apply";
	F.submit();
}

function trigger_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Triggering";
	F.submit_type.value = "remove_trigger";
	F.action.value = "Apply";
	F.submit();
}

function to_submit(F)
{
	F.submit_button.value = "Triggering";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
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
								<li><a href="index.asp"><script>Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script>Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script>Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script>Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script>Capture(bmenu.accrestriction)</script></a></li>
								<li class="current"><span><script>Capture(bmenu.applications)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Forward.asp"><script>Capture(bmenu.applicationsprforwarding)</script></a></li>
											<li><a href="ForwardSpec.asp"><script>Capture(bmenu.applicationspforwarding)</script></a></li>
											<li><span><script>Capture(bmenu.applicationsptriggering)</script></span></li>
											<li><a href="UPnP.asp"><script>Capture(bmenu.applicationsUpnp)</script></a></li>
											<li><a href="DMZ.asp"><script>Capture(bmenu.applicationsDMZ)</script></a></li>
											<li><a href="QoS.asp"><script>Capture(bmenu.applicationsQoS)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp"><script>Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script>Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="trigger" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="port_trigger" value="10" />
							<h2>Port Triggering</h2>
							<fieldset>
								<legend>Forwards</legend>
								<table class="table center" cellspacing="5">
									<tr>
										<td></td>
										<th colspan="2">Triggered Range</th>
										<th colspan="2">Forwarded Range</th>
										<td></td>
									</tr>
									<tr>
										<th>Application</th>
										<th>Start Port</th>
										<th>End Port</th>
										<th>Start Port</th>
										<th>End Port</th>
										<th>Enable</th>
									</tr>
									<% show_triggering(); %>
								</table><br />
								<div class="center">
									<input type="button" value=" Add " onclick="trigger_add_submit(this.form)" />&nbsp;
									<input type="button" value="Remove" onclick="trigger_remove_submit(this.form)" />
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="reset" value="Cancel Changes" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Application: </dt>
							<dd class="definition">Enter the application name of the trigger.</dd>
							<dt class="term">Triggered Range: </dt>
							<dd class="definition">For each application, list the triggered port number range. Check with the Internet application documentation for the port number(s) needed.</dd>
							<dt class="term">Start Port: </dt>
							<dd class="definition">Enter the starting port number of the Triggered Range.</dd>
							<dt class="term">End Port: </dt>
							<dd class="definition">Enter the ending port number of the Triggered Range.</dd>
							<dt class="term">Forwarded Range: </dt>
							<dd class="definition">For each application, list the forwarded port number range. Check with the Internet application documentation for the port number(s) needed.</dd>
							<dt class="term">Start Port: </dt>
							<dd class="definition">Enter the starting port number of the Forwarded Range.</dd>
							<dt class="term">End Port: </dt>
							<dd class="definition">Enter the ending port number of the Forwarded Range.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HTrigger.asp')">More...</a>
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