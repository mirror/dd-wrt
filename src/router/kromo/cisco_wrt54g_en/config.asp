<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Backup &amp; Restore</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
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
												<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
												<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											    <li><span>Backup</span></li>
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
						<form name="nvramrestore" action="nvram.cgi" method="POST" encType="multipart/form-data">
							<h2>Backup Configuration</h2>
							<fieldset>
								<legend>Backup Settings</legend>
								<div class="setting">
									Click the "Backup" button to download the configuration backup file to your computer.
								</div>
							</fieldset><br />
							<h2>Restore Configuration</h2>
							<fieldset>
								<legend>Restore Settings</legend>
								<div class="setting">
									<div class="label">Please select a file to restore</div>
									<input type="file" name="file" size="40" />
								</div>
							</fieldset><br />
							<div class="warning">
								<p><b>W A R N I N G</b></p>
								<p>Only upload files backed up using this firmware and from the same model of router.<br />
								Do not upload any files that were not created by this interface!</p>
							</div><br />
							<div class="submitFooter">
								<input type="button" value="Backup" name="B1" onclick="window.location.href='/nvrambak.bin'" />
								<input type="submit" value=" Restore "/>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Backup: </dt>
							<dd class="definition">You may backup your current configuration in case you need to reset the router back to its factory default settings.<br /><br />
							Click the <em>Backup</em> button to backup your current configuration.</dd>
							<dt class="term">Restore: </dt>
							<dd class="definition">Click the <em>Browse...</em> button to browse for a configuration file that is currently saved on your PC.<br /><br />
							Click <em>Restore</em> to overwrite all current configurations with the ones in the configuration file.</dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow('HBackup.asp')">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <a href="javascript:openAboutWindow()"><% get_firmware_version(); %></a></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>