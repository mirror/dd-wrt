<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Backup &amp; Restore</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = '<% nvram_get("router_name"); %>'+config.titl;
		
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
												<li><a href="Diagnostics.asp"><script type="text/javascript">Capture(bmenu.adminDiag)</script></a></li>
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
						<form name="nvramrestore" action="nvram.cgi" method="POST" encType="multipart/form-data">
							<h2><script type="text/javascript">Capture(config.h2)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(config.legend)</script></legend>
								<div class="setting">
									<script type="text/javascript">Capture(config.mess1)</script>
								</div>
							</fieldset><br />
							<h2><script type="text/javascript">Capture(config.h22)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(config.legend2)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(config.mess2)</script></div>
									<input type="file" name="file" size="40" />
								</div>
							</fieldset><br />
							<div class="warning">
								<p><b><script type="text/javascript">Capture(config.mess3)</script></b></p>
								<p><script type="text/javascript">Capture(config.mess4)</script></p>
							</div><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"B1\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/nvrambak.bin'\" />")</script>
								<script type="text/javascript">document.write("<input type=\"submit\" value=\"" + sbutton.restore + "\" />")</script>
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
							<dt class="term"><script type="text/javascript">Capture(hconfig.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hconfig.right2)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(hconfig.right3)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hconfig.right4)</script></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow('HBackup.asp');"><script type="text/javascript">Capture(share.more)</script></a>
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