<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Router Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + status_router.titl;

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

var update;

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
	var loadAverage = val.substring(val.indexOf("average") + 9).split(",");
	setMeterBar("uptime_load", (parseFloat(loadAverage[0]) + parseFloat(loadAverage[1]) + parseFloat(loadAverage[2])) * 33.3, loadAverage.join(","));
}

function setIpconntrackValues(val) {
	setMeterBar("ip_count", val / <% nvram_get("ip_conntrack_max"); %> * 100, val);
}

addEvent(window, "load", function() {
	setMemoryValues("<% dumpmeminfo(); %>");
	setUptimeValues("<% get_uptime(); %>");
	setIpconntrackValues("<% dumpip_conntrack(); %>");
	setElementVisible("wan_info", "<% nvram_get("wan_proto"); %>" != "disabled");
	setElementVisible("wan_dhcp", "<% nvram_get("wan_proto"); %>" == "dhcp");
	setElementVisible("wan_connection", "<% nvram_get("wan_proto"); %>" != "dhcp" && "<% nvram_get("wan_proto"); %>" != "static");

	update = new StatusUpdate("Status_Router.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("mem_info", function(u) {
		setMemoryValues(u.mem_info);
	});
	update.onUpdate("uptime", function(u) {
		setUptimeValues(u.uptime);
	});
	update.onUpdate("ip_conntrack", function(u) {
		setIpconntrackValues(u.ip_conntrack);
	});
	update.onUpdate("wan_shortproto", function(u) {
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
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.statu)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span><script type="text/javascript">Capture(bmenu.statuRouter)</script></span></li>
											<li><a href="Status_Lan.asp"><script type="text/javascript">Capture(bmenu.statuLAN)</script></a></li>
											<li><a href="Status_Wireless.asp"><script type="text/javascript">Capture(bmenu.statuWLAN)</script></a></li>
											<% show_sputnik(); %>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm"><script type="text/javascript">Capture(bmenu.statuSysInfo)</script></a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
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
							<h2><script type="text/javascript">Capture(status_router.h2)</script></h2>
							
							<fieldset>
							<legend><script type="text/javascript">Capture(status_router.legend)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.routername)</script></div>
									<span id="router_name"><% nvram_get("router_name"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.sys_model)</script></div>
									<span id="router_model"><% nvram_get("DD_BOARD"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.sys_firmver)</script></div>
									<span id="router_firmware"><% get_firmware_version(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.mac)</script></div>
									<script>document.write("<span id=\"wan_mac\" style=\"cursor:pointer\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("wan_hwaddr"); %>')\" >");</script><% nvram_get("wan_hwaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.hostname)</script></div>
									<span id="wan_host"><% nvram_get("wan_hostname"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.domainname)</script></div>
									<span id="wan_name"><% nvram_get("wan_domain"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.sys_time)</script></div>
									<span id="router_time"><% localtime(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.sys_up)</script></div>
									<span id="uptime_up"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.sys_load)</script></div>
									<span id="uptime_load"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(status_router.legend2)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.cpu)</script></div>
									<span id="cpu_info"><% show_cpuinfo(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.clock)</script></div>
									<span id="cpu_clock"><% get_clkfreq(); %></span>&nbsp;MHz
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(status_router.legend3)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_tot)</script></div>
									<span id="mem_total"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_free)</script></div>
									<span id="mem_free"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_used)</script></div>
									<span id="mem_used"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_buf)</script></div>
									<span id="mem_buffer"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_cached)</script></div>
									<span id="mem_cached"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_active)</script></div>
									<span id="mem_active"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.mem_inactive)</script></div>
									<span id="mem_inactive"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(status_router.legend4)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.net_maxports)</script></div>
									<span id="ip_conntrack_max"><% nvram_get("ip_conntrack_max"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.net_conntrack)</script></div>
									<span id="ip_count"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<h2><script type="text/javascript">Capture(status_router.h22)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(status_router.legend5)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(status_router.www_login)</script></div>
									<span id="wan_proto"><% nvram_match("wan_proto", "dhcp", "<script type="text/javascript">Capture(idx.dhcp)</script>"); %><% nvram_match("wan_proto", "static", "<script type="text/javascript">Capture(share.sttic)</script>"); %><% nvram_match("wan_proto", "pppoe", "PPPoE"); %><% nvram_match("wan_proto", "pptp", "PPTP"); %><% nvram_match("wan_proto", "l2tp", "L2TP"); %><% nvram_match("wan_proto", "heartbeat", "HeartBeatSignal"); %><% nvram_match("wan_proto", "disabled", "<script type="text/javascript">Capture(share.disable)</script>"); %></span>&nbsp;
								</div>
								<span id="wan_info" style="display:none">
									<div class="setting" id="wan_connection">
										<div class="label"><script type="text/javascript">Capture(status_router.www_loginstatus)</script></div>
										<span id="wan_status"><% nvram_status_get("status2"); %>&nbsp;
										<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>')" /></span>
									</div>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(share.ip)</script></div>
										<span id="wan_ipaddr"><% nvram_status_get("wan_ipaddr"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(share.subnet)</script></div>
										<span id="wan_netmask"><% nvram_status_get("wan_netmask"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(share.gateway)</script></div>
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
										<script>document.write("<input type=\"button\" value=\"" + sbutton.dhcprel + "\" onclick=\"DHCPAction(this.form,'release')\">");</script>&nbsp;
										<script>document.write("<input type=\"button\" value=\"" + sbutton.dhcpren + "\" onclick=\"DHCPAction(this.form,'renew')\">");</script>
									</div>
								</span>
							</fieldset><br />
							<div class="submitFooter">
								<script>document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");</script>
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
							<dt class="term"><script type="text/javascript">Capture(share.routername)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right2)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(share.mac)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right4)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_router.sys_firmver)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right6)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_router.sys_time)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right8)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_router.sys_up)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right10)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_router.sys_load)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right12)</script></dd>
							<dt class="term"><script type="text/javascript">Capture(status_router.legend5)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hstatus_router.right14)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HStatus.asp');"><script type="text/javascript">Capture(share.more)</script></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>