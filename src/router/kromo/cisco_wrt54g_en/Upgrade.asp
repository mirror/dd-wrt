<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Firmware Upgrade</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function process_aborted(F) {
	bar1.togglePause();
	alert("Upgrade failed !");
	return false;
}

function upgrade(F,id) {
	var len = F.file.value.length;
	var ext = new Array('.','b','i','n');
	if (F.file.value == '')	{
		alert("Please select a file to upgrade !");
		return false;
	}
	var IMAGE = F.file.value.toLowerCase();
	for (i=0; i < 4; i++)	{
		if (ext[i] != IMAGE.charAt(len-4+i)){
			alert("Incorrect image file !");
			return false;
		}
	}

	choose_disable(F.Upgrade_b);
	F.Upgrade_b.value = " Upgrading ";
	F.submit_button.value = "Upgrade";
	bar1.togglePause();
	change_style(id,'textblink');
	apply(F);
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
											<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
											<li><span>Firmware Upgrade</span></li>
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
						<form name="firmware" method="post" action="upgrade.cgi" enctype="multipart/form-data">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<h2>Firmware Management</h2>
							<fieldset>
								<legend>Firmware Upgrade</legend>
								<div class="setting">
									<div class="label">Please select a file to upgrade</div>
									<input type="file" name="file" size="40"/>
								</div>
								<div class="setting">
									<div class="label">After flashing, reset to</div>
									<input type="radio" value="0" name="erase" checked="checked" />No reset&nbsp;
									<input type="radio" value="1" name="erase" />Default settings
								</div>
							</fieldset><br />
							<div class="warning">
								<p><div id="warning_text"><b>W A R N I N G</b></div></p>
								<p>Upgrading firmware may take a few minutes.<br />
								Do not turn off the power or press the reset button!</p>
								<div align="center"><script type="text/javascript">
									var bar1 = createBar(500,15,100,15,50,"process_aborted(this.form)");
									bar1.togglePause();
								</script></div><br />
							</div><br />
							<div class="submitFooter">
								 <input type="button" name="Upgrade_b" value=" Upgrade " onclick="upgrade(this.form,'warning_text')"/>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Upgrade: </dt>
							<dd class="definition">Click on the <em>Browse...</em> button to select the firmware file to be uploaded to the router.<br /><br />
							Click the <em>Upgrade</em> button to begin the upgrade process. Upgrade must not be interrupted.</dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow('HUpgrade.asp')">More...</a>
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
