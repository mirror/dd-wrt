<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Router Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function DHCPAction(F,I) {
	F.submit_type.value = I;
	F.submit_button.value = "Status_Router";
	F.change_action.value = "gozila_cgi";
	F.submit();
}

function connect(F,I) {
	F.submit_type.value = I;
	F.submit_button.value = "Status_Router";
	F.change_action.value = "gozila_cgi";
	F.submit();
}

/* function init() {
	<% show_status("onload"); %>
} */

/* function ShowAlert(M) {
	var str = "";
	var mode = "";
	var wan_ip = "<% nvram_status_get("wan_ipaddr"); %>";
	var wan_proto = "<% nvram_safe_get("wan_proto"); %>";

	if(document.status.wan_proto.value == "pppoe")
		mode = "PPPoE";
	else if(document.status.wan_proto.value == "heartbeat")
		mode = "HBS";
	else
		mode = "PPTP";

	if(M == "AUTH_FAIL" || M == "PAP_AUTH_FAIL" || M == "CHAP_AUTH_FAIL")
		str = mode + " authentication fail";
	else if(M == "IP_FAIL" || (M == "TIMEOUT" && wan_ip == "0.0.0.0"))
		str = "Can not get a IP address from " + mode + " server";
	else if(M == "NEG_FAIL")
		str = mode + " negotication fail";
	else if(M == "LCP_FAIL")
		str = mode + " LCP negotication fail";
	else if(M == "TCP_FAIL" || (M == "TIMEOUT" && wan_ip != "0.0.0.0" && wan_proto == "heartbeat"))
		str = "Can not build a TCP connection to " + mode + " server";
	else
		str = "Can not connect to " + mode + " server";

	alert(str);

	Refresh();
} */

function setMemoryValues(val) {
	var mem = val.replace(/'/g, "").split(",");
	var memTotal = parseInt(mem[19]);
	var memSystem = Math.pow(2, Math.ceil(Math.log(memTotal) / Math.LN2));
	var memFree = parseInt(mem[22]);
	var memUsed = memTotal - memFree;
	var memBuffer = parseInt(mem[28]);
	var memCached = parseInt(mem[31]);
	var memActive = parseInt(mem[37]);
	var memInactive = parseInt(mem[40]);
	setMeterBar("mem_total", memTotal / memSystem * 100, memTotal + " kB / " + memSystem + " kB");
	setMeterBar("mem_free", memFree / memTotal * 100, memFree + " kB / " + memTotal + " kB");
	setMeterBar("mem_used", memUsed / memTotal * 100, memUsed + " kB / " + memTotal + " kB");
	setMeterBar("mem_buffer", memBuffer / memUsed * 100, memBuffer + " kB / " + memUsed + " kB");
	setMeterBar("mem_cached", memCached / memUsed * 100, memCached + " kB / " + memUsed + " kB");
	setMeterBar("mem_active", memActive / memUsed * 100, memActive + " kB / " + memUsed + " kB");
	setMeterBar("mem_inactive", memInactive / memUsed * 100, memInactive + " kB / " + memUsed + " kB");
}

function setUptimeValues(val) {
	setElementContent("uptime_up", val.substring(val.indexOf("up") + 3, val.indexOf("load") - 2));
	setElementContent("uptime_load", val.substring(val.indexOf("average") + 9));
}

var update;

addEvent(window, "load", function() {
	setMemoryValues("<% dumpmeminfo(); %>");
	setUptimeValues("<% get_uptime(); %>");
	setElementVisible("wan_info", "<% nvram_get("wan_proto"); %>" != "disabled");
	setElementVisible("wan_dhcp", "<% nvram_get("wan_proto"); %>" == "dhcp");
	setElementVisible("wan_connection", "<% nvram_get("wan_proto"); %>" != "dhcp" && "<% nvram_get("wan_proto"); %>" != "static");

	update = new StatusUpdate("Status_Router.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate(function(u) {
		setMemoryValues(u.mem_info);
		setUptimeValues(u.uptime);
		setElementVisible("wan_info", u.wan_shortproto != "disabled");
		setElementVisible("wan_dhcp", u.wan_shortproto == "dhcp");
		setElementVisible("wan_connection", u.wan_shortproto != "dhcp" && u.wan_shortproto != "static");
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
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
								<li><a href="Management.asp">Administration</a></li>
								<li class="current"><span>Status</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span>Router</span></li>
											<li><a href="Status_Lan.asp">LAN</a></li>
											<li><a href="Status_Wireless.asp">Wireless</a></li>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm">Sys-Info</a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
											<% show_sputnik(); %>
										</ul>
									</div>
								</li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="status" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" value="Apply" />
							<!-- <input type="hidden" name="wan_proto" value='<% nvram_get("wan_proto"); %>' /> -->
							<h2>Router Information</h2>
							<fieldset>
							<legend>System</legend>
								<div class="setting">
									<div class="label">Router Name</div>
									<span id="router_name"><% nvram_get("router_name"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Router Model</div>
									<span id="router_model"><% nvram_get("DD_BOARD"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Firmware Version</div>
									<span id="router_firmware"><% get_firmware_version(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">MAC Address</div>
									<span id="wan_mac"><% nvram_get("wan_hwaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Host Name</div>
									<span id="wan_host"><% nvram_get("wan_hostname"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Domain Name</div>
									<span id="wan_name"><% nvram_get("wan_domain"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Current Time</div>
									<span id="router_time"><% localtime(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Uptime</div>
									<span id="uptime_up"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Load Average</div>
									<span id="uptime_load"></span>&nbsp;
								</div>
							</fieldset><br />
							<fieldset>
								<legend>CPU</legend>
								<div class="setting">
									<div class="label">CPU Model</div>
									<span id="cpu_info"><% show_cpuinfo(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">CPU Clock</div>
									<span id="cpu_clock"><% get_clkfreq(); %> MHz</span>&nbsp;
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Memory</legend>
								<div class="setting">
									<div class="label">Total Available</div>
									<span id="mem_total"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Free</div>
									<span id="mem_free"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Used</div>
									<span id="mem_used"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Buffers</div>
									<span id="mem_buffer"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Cached</div>
									<span id="mem_cached"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Active</div>
									<span id="mem_active"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label">Inactive</div>
									<span id="mem_inactive"></span>&nbsp;
								</div>
							</fieldset><br />
							<h2>Internet</h2>
							<fieldset>
								<legend>Configuration Type</legend>
								<div class="setting">
									<div class="label">Login Type</div>
									<span id="wan_proto"><% nvram_match("wan_proto", "dhcp", "Automatic Configuration - DHCP"); %><% nvram_match("wan_proto", "static", "Static"); %><% nvram_match("wan_proto", "pppoe", "PPPoE"); %><% nvram_match("wan_proto", "pptp", "PPTP"); %><% nvram_match("wan_proto", "l2tp", "L2TP"); %><% nvram_match("wan_proto", "heartbeat", "HeartBeatSignal"); %><% nvram_match("wan_proto", "disabled", "Disabled"); %></span>&nbsp;
								</div>
								<span id="wan_info" style="display:none">
									<div class="setting" id="wan_connection">
										<div class="label">Login Status</div>
										<span id="wan_status"><% nvram_status_get("status2"); %>&nbsp;
										<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>')" /></span>
									</div>
									<div class="setting">
										<div class="label">IP Address</div>
										<span id="wan_ipaddr"><% nvram_status_get("wan_ipaddr"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">Subnet Mask</div>
										<span id="wan_netmask"><% nvram_status_get("wan_netmask"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">Default Gateway</div>
										<span id="wan_gateway"><% nvram_status_get("wan_gateway"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">DNS 1</div>
										<span id="wan_dns0"><% nvram_status_get("wan_dns0"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">DNS 2</div>
										<span id="wan_dns1"><% nvram_status_get("wan_dns1"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label">DNS 3</div>
										<span id="wan_dns2"><% nvram_status_get("wan_dns2"); %></span>&nbsp;
									</div>
									<div class="center" id="wan_dhcp">
										<input onclick="DHCPAction(this.form,'release')" type="button" value="DHCP Release" />&nbsp;
										<input onclick="DHCPAction(this.form,'renew')" type="button" value="DHCP Renew" />
									</div>
								</span>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="refresh_button" value="Refresh" onclick="window.location.reload()" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Router Name: </dt>
							<dd class="definition">This is the specific name for the router, which you set on the <i>Setup</i> tab.</dd>
							<dt class="term">MAC Address: </dt>
							<dd class="definition">This is the router's MAC Address, as seen by your ISP.</dd>
							<dt class="term">Firmware Version: </dt>
							<dd class="definition">This is the router's current firmware.</dd>
							<dt class="term">Current Time: </dt>
							<dd class="definition">This is the time, as you set on the Setup Tab.</dd>
							<dt class="term">Uptime: </dt>
							<dd class="definition">This is a measure of the time the router has been "up" and running.</dd>
							<dt class="term">Load Average: </dt>
							<dd class="definition">This is given as three numbers that represent the system load during the last one, five, and fifteen minute periods.</dd>
							<dt class="term">Configuration Type: </dt>
							<dd class="definition">This shows the information required by your ISP for connection to the Internet. This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HStatus.asp')">More...</a>
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