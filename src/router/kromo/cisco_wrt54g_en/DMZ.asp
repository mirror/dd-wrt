<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - DMZ</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "DMZ";

	F.action.value = "Apply";
	apply(F);
}

function setDMZ(val) {
	setElementActive("dmz_ipaddr", val == "1");
}

addEvent(window, "load", function() {
	setDMZ("<% nvram_get("dmz_enable"); %>");
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
								<li class="current"><span><script type="text/javascript">Capture(bmenu.applications)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applicationsprforwarding)</script></a></li>
											<li><a href="ForwardSpec.asp"><script type="text/javascript">Capture(bmenu.applicationspforwarding)</script></a></li>
											<li><a href="Triggering.asp"><script type="text/javascript">Capture(bmenu.applicationsptriggering)</script></a></li>
											<% nvram_match("dist_type", "micro", "<!--"); %>
											<li><a href="UPnP.asp"><script type="text/javascript">Capture(bmenu.applicationsUpnp)</script></a></li>
											<% nvram_match("dist_type", "micro", "-->"); %>
	  										<li><span><script type="text/javascript">Capture(bmenu.applicationsDMZ)</script></span></li>
	  										<li><a href="QoS.asp"><script type="text/javascript">Capture(bmenu.applicationsQoS)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="dmz" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="DMZ" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" value="Apply" />
							<h2>Demilitarized Zone (DMZ)</h2>
							<fieldset>
								<legend>DMZ</legend>
	                			<div class="setting">
	                				<div class="label">Use DMZ</div>
	                				<input type="radio" value="1" name="dmz_enable" onclick="setDMZ(this.value)" <% nvram_checked("dmz_enable", "1"); %> />Enable
	                				<input type="radio" value="0" name="dmz_enable" onclick="setDMZ(this.value)" <% nvram_checked("dmz_enable", "0"); %> />Disable
	                			</div>
	                			<div class="setting">
	                				<div class="label">DMZ Host IP Address</div>
	                				<% prefix_ip_get("lan_ipaddr",1); %>
	                				<input class="num" maxLength="3" onblur="valid_range(this,1,254,'IP')" size="3" name="dmz_ipaddr" value='<% nvram_get("dmz_ipaddr"); %>' />
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
							<dt class="term">DMZ: </dt>
							<dd class="definition">Enabling this option will expose your router to the Internet. All ports will be accessible from the Internet.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HDMZ.asp')">More...</a>
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