<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Factory Defaults</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	if( F.FactoryDefaults[0].checked == 1 ) {
		if(!confirm("Warning! If you click OK, the device will reset to factory default and all previous settings will be erased.")) {
			return false;
		}
		F.action.value='Restore';
		F.submit_button.value = "Factory_Defaults";
		F.submit();

       	return true;
	}
	F.action.value="Apply";
	F.submit_button.value = "Factory_Defaults";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();

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
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li class="current"><span>Administration</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp">Management</a></li>
											<li><a href="Hotspot.asp">Hotspot</a></li>
											<li><a href="Services.asp">Services</a></li>
											<li><a href="Alive.asp">Keep Alive</a></li>
											<li><a href="Log.asp">Log</a></li>
											<li><a href="Diagnostics.asp">Diagnostics</a></li>
											<li><span>Factory Defaults</span></li>
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
						<form name="default" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="change_action"/>
							<h2>Factory Defaults</h2>
							<fieldset>
							<legend>Reset router settings</legend>
							<div class="setting">
								<div class="label">Restore Factory Defaults</div>
								<input type="radio" name="FactoryDefaults" value="1"/> Yes<input type="radio" name="FactoryDefaults" value="0" checked="checked"/> No
							</div>
							</fieldset>
							<br/>
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)"/>
								<input type="reset" value="Cancel Changes"/>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dd class="definition">This will reset all settings back to factory defaults. All of your settings will be erased.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HDefault.asp')">More...</a>
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