<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - DMZ</title>
		<link type="text/css" rel="stylesheet" href="style.css" />
		<script type="text/JavaScript" src="common.js">{}</script>
		<script type="text/JavaScript">

function to_submit(F) {
	F.submit_button.value = "DMZ";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function dmz_enable_disable(F,I) {
	var state = ( I == 0 );
	F.dmz_ipaddr.disabled = state;
}

function init() {
	dmz_enable_disable(document.dmz, <% nvram_get("dmz_enable"); %>);
}
		</script>
	</head>
	
	<body class="gui" onload="init()">
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
								<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
								<li class="current"><span>Applications&nbsp;&amp;&nbsp;Gaming</span>
									<div id="menuSub">
										<ul id="menuSubList">
	  										<li><a href="Forward.asp">Port Range Forward</a></li>
	  										<li><a href="ForwardSpec.asp">Port Forwarding</a></li> 
	  										<li><a href="Triggering.asp">Port Triggering</a></li> 
	  										<li><a href="UPnP.asp">UPnP Forward</a></li> 
	  										<li><span>DMZ</span></li>
	  										<li><a href="QoS.asp">QoS</a></li>
	  									</ul>
									</div>
								</li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
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
	                				<input type="radio" value="1" name="dmz_enable" onclick="dmz_enable_disable(this.form, 1)" <% nvram_checked("dmz_enable", "1"); %> />Enable
	                				<input type="radio" value="0" name="dmz_enable" onclick="dmz_enable_disable(this.form, 0)" <% nvram_checked("dmz_enable", "0"); %> />Disable
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
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">DMZ: </dt>
							<dd class="definition">Enabling this option will expose your router to the Internet. All ports will be accessible from the Internet.</dd>
						</dl>
						<br />
						<a href="javascript:help('help/HDMZ.asp');">More...</a>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>