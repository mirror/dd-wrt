<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Port Range Forwarding</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = '<% nvram_get("router_name"); %>'+prforward.titl;

function forward_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Forward";
	F.submit_type.value = "add_forward";
 	F.action.value = "Apply";
	F.submit();
}

function forward_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Forward";
	F.submit_type.value = "remove_forward";
 	F.action.value = "Apply";
	F.submit();
}

function to_submit(F) {
	F.submit_button.value = "Forward";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
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
											<li><span><script type="text/javascript">Capture(bmenu.applicationsprforwarding)</script></span></li>
											<li><a href="ForwardSpec.asp"><script type="text/javascript">Capture(bmenu.applicationspforwarding)</script></a></li>
											<li><a href="Triggering.asp"><script type="text/javascript">Capture(bmenu.applicationsptriggering)</script></a></li>
											<% nvram_match("dist_type", "micro", "<!--"); %>
											<li><a href="UPnP.asp"><script type="text/javascript">Capture(bmenu.applicationsUpnp)</script></a></li>
											<% nvram_match("dist_type", "micro", "-->"); %>
											<li><a href="DMZ.asp"><script type="text/javascript">Capture(bmenu.applicationsDMZ)</script></a></li>
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
						<form name="portRange" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<input type="hidden" name="forward_port" value="13" />
							<h2><script type="text/javascript">Capture(prforward.h2)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(prforward.legend)</script></legend>
								<table class="table center" cellspacing="5">
									<tr>
										<th><script type="text/javascript">Capture(prforward.app)</script></th>
										<th><script type="text/javascript">Capture(share.start)</script></th>
										<th><script type="text/javascript">Capture(share.end)</script></th>
										<th><script type="text/javascript">Capture(share.proto)</script></th>
										<th><script type="text/javascript">Capture(share.ip)</script></th>
										<th><script type="text/javascript">Capture(share.enable)</script></th>
									</tr>
									<% show_forward(); %>
								</table><br />
								<div class="center">
									<script>document.write("<input class=\"btn\" type=\"button\" value=\"" + sbutton.add + "\" onclick=\"forward_add_submit(this.form)\">");</script>
									<script>document.write("<input class=\"btn\" type=\"button\" value=\"" + sbutton.remove + "\" onclick=\"forward_remove_submit(this.form)\">");</script>
								</div>
						 	</fieldset><br />
							<div class="submitFooter">
								<script>document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\">");</script>
								<script>document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\">");</script>
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
							<dt class="term"><script type="text/javascript">Capture(hprforward.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hprforward.right2)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HForwardRange.asp')"><script type="text/javascript">Capture(share.more)</script></a>
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