<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - VPN</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "VPN";
	F.save_button.value = "Saved";

	F.action.value = "Apply";
	apply(F);
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
								<li class="current"><span>Security</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Firewall.asp">Firewall</a></li>
											<li><span>VPN</span></li>
										</ul>
									</div>
								</li>
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="action"/>
							<h2>Virtual Private Network (VPN)</h2>
							<fieldset>
								<legend>VPN Passthrough</legend>
								<div class="setting">
									<div class="label">IPSec Passthrough</div>
									<input type="radio" value="1" name="ipsec_pass" <% nvram_match("ipsec_pass","1","checked"); %>>Enable</input>
									<input type="radio" value="0" name="ipsec_pass" <% nvram_match("ipsec_pass","0","checked"); %>>Disable</input>
								</div>
								<div class="setting">
									<div class="label">PPTP Passthrough</div>
									<input type="radio" value="1" name="pptp_pass" <% nvram_match("pptp_pass","1","checked"); %>>Enable</input>
									<input type="radio" value="0" name="pptp_pass" <% nvram_match("pptp_pass","0","checked"); %>>Disable</input>
								</div>
								<div class="setting">
									<div class="label">L2TP Passthrough</div>
									<input type="radio" value="1" name="l2tp_pass" <% nvram_match("l2tp_pass","1","checked"); %>>Enable</input>
									<input type="radio" value="0" name="l2tp_pass" <% nvram_match("l2tp_pass","0","checked"); %>>Disable</input>
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
							<dd class="definition">You may choose to enable PPTP, L2TP of IPSec passthrough to allow your network devices to communicate via VPN.</dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow('HVPN.asp')">More...</a>
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