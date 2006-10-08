<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Router Status</title>
		<script type="text/javascript">
//<![CDATA[

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
		
//]]>
</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Status_Router.asp","Status_Router.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="status" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" value="Apply" />
							<h2><% tran("status_router.h2"); %></h2>
							
							<fieldset>
							<legend><% tran("status_router.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.routername"); %></div>
									<% nvram_get("router_name"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_model"); %></div>
									<% nvram_get("DD_BOARD"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_firmver"); %></div>
									<% get_firmware_version(); %> - build <% get_firmware_svnrev(); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"wan_mac\" style=\"cursor:pointer\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("wan_hwaddr"); %>')\" >");
									<% nvram_get("wan_hwaddr"); %>
									document.write("</span>");
									//]]>
									</script>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<% nvram_get("wan_hostname"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.wandomainname"); %></div>
									<% show_wan_domain(); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.landomainname"); %></div>
									<% nvram_get("lan_domain"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_time"); %></div>
									<span id="router_time"><% localtime(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_up"); %></div>
									<span id="uptime_up"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_load"); %></div>
									<span id="uptime_load"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_router.legend2"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_router.cpu"); %></div>
									<% show_cpuinfo(); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.clock"); %></div>
									<% get_clkfreq(); %>&nbsp;MHz
								</div>
								<% show_cpu_temperature(); %>
								<% show_voltage(); %>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_router.legend3"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_tot"); %></div>
									<span id="mem_total"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_free"); %></div>
									<span id="mem_free"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_used"); %></div>
									<span id="mem_used"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_buf"); %></div>
									<span id="mem_buffer"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_cached"); %></div>
									<span id="mem_cached"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_active"); %></div>
									<span id="mem_active"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_inactive"); %></div>
									<span id="mem_inactive"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_router.legend4"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_router.net_maxports"); %></div>
									<% nvram_get("ip_conntrack_max"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.net_conntrack"); %></div>
									<span id="ip_count"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<h2><% tran("status_router.h22"); %></h2>
							<fieldset>
								<legend><% tran("status_router.legend5"); %></legend>
								<div class="setting">
									<div class="label"><% tran("idx.conn_type"); %></div>
									<% nvram_match("wan_proto", "dhcp", "<script type="text/javascript">Capture(idx.dhcp)</script>"); %><% nvram_match("wan_proto", "static", "<script type="text/javascript">Capture(share.sttic)</script>"); %><% nvram_match("wan_proto", "pppoe", "PPPoE"); %><% nvram_match("wan_proto", "pptp", "PPTP"); %><% nvram_match("wan_proto", "l2tp", "L2TP"); %><% nvram_match("wan_proto", "heartbeat", "HeartBeatSignal"); %><% nvram_match("wan_proto", "disabled", "<script type="text/javascript">Capture(share.disabled)</script>"); %>&nbsp;
								</div>
								<div id="wan_info" style="display:none">
									<div class="setting" id="wan_connection">
										<div class="label"><% tran("status_router.www_loginstatus"); %></div>
										<span id="wan_status"><% nvram_status_get("status2"); %>&nbsp;<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>')" /></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("share.ip"); %></div>
										<span id="wan_ipaddr"><% nvram_status_get("wan_ipaddr"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("share.subnet"); %></div>
										<span id="wan_netmask"><% nvram_status_get("wan_netmask"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("share.gateway"); %></div>
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
									<div id="wan_dhcp">
										<div class="setting">
											<div class="label"><% tran("status_router.leasetime"); %></div>
											<span id="dhcp_remaining"><% dhcp_remaining_time(); %></span>&nbsp;
										</div>
										<div class="center">
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input type=\"button\" value=\"" + sbutton.dhcprel + "\" onclick=\"DHCPAction(this.form,'release')\">");
											//]]>
											</script>&nbsp;
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input type=\"button\" value=\"" + sbutton.dhcpren + "\" onclick=\"DHCPAction(this.form,'renew')\">");
											//]]>
											</script>
										</div>
									</div>
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("share.routername"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right2"); %></dd>
							<dt class="term"><% tran("share.mac"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right4"); %></dd>
							<dt class="term"><% tran("status_router.sys_firmver"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right6"); %></dd>
							<dt class="term"><% tran("status_router.sys_time"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right8"); %></dd>
							<dt class="term"><% tran("status_router.sys_up"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right10"); %></dd>
							<dt class="term"><% tran("status_router.sys_load"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right12"); %></dd>
							<dt class="term"><% tran("status_router.legend5"); %>: </dt>
							<dd class="definition"><% tran("hstatus_router.right14"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HStatus.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>