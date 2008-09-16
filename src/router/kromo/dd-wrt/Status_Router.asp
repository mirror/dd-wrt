<% do_pagehead("status_router.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

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
<% nvram_invmatch("show_hidden","1","/"); %><% nvram_invmatch("show_hidden","1","/"); %>	setMeterBar("mem_hidden", 100 , "32768 kB / 32768 kB");
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
						<form name="status" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Status_Router" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
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
									document.write("<span id=\"wan_mac\" style=\"cursor:pointer; text-decoration:underline;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("wan_hwaddr"); %>');\" >");
									document.write("<% nvram_get("wan_hwaddr"); %>");
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
								<% show_voltage(); %>
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
								<div class="setting">
									<div class="label"><% tran("status_router.sys_load"); %></div>
									<span id="uptime_load"></span>&nbsp;
								</div>
								<% show_cpu_temperature(); %>
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
<% nvram_invmatch("show_hidden","1","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("status_router.mem_hidden"); %></div>
									<span id="mem_hidden"></span>&nbsp;
								</div>
<% nvram_invmatch("show_hidden","1","-->"); %>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_router.legend6"); %></legend>
				<% ifndef("SAMBA", "<!--"); %>
									<div class="setting">
										<div class="label">CIFS</div>
										<script type="text/javascript">
										//<![CDATA[
										<% statfs("/tmp/smbshare", "samba"); %>
										document.write( ((<% nvram_get("samba_mount"); %>) && (samba.size)) ? (scaleSize(samba.size) + ' / ' + scaleSize(samba.free)) : '<span style="color:#999999;"><em>(' + share.nmounted + ')</em></span>' );
										//]]>
										</script>
									</div>
				<% ifndef("SAMBA", "-->"); %>
				<% ifndef("JFFS2", "<!--"); %>
								<div class="setting">
									<div class="label">JFFS2</div>
									<script type="text/javascript">
									//<![CDATA[
									<% statfs("/jffs", "my_jffs"); %>
									document.write( ((<% nvram_get("enable_jffs2"); %>) && (my_jffs.size)) ? (scaleSize(my_jffs.size) + ' / ' + scaleSize(my_jffs.free)) : '<span style="color:#999999;"><em>(' + share.nmounted + ')</em></span>' );
									//]]>
									</script>
								</div>
				<% ifndef("JFFS2", "-->"); %>
				<% ifndef("MMC", "<!--"); %>
								<div class="setting">
									<div class="label">MMC</div>
									<script type="text/javascript">
									//<![CDATA[
									<% statfs("/mmc", "mmc"); %>
									document.write( ((<% nvram_get("mmc_enable0"); %>) && (mmc.size)) ? (scaleSize(mmc.size) + ' / ' + scaleSize(mmc.free)) : '<span style="color:#999999;"><em>(' + share.nmounted + ')</em></span>' );
									//]]>
									</script>
								</div>
				<% ifndef("MMC", "-->"); %>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_router.legend4"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_router.net_maxports"); %></div>
									<% nvram_get("ip_conntrack_max"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.net_conntrack"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"ip_count\" style=\"cursor:pointer; text-decoration:underline;\" title=\"" + share.detail + "\" onclick=\"openWindow('Status_Conntrack.asp', 800, 600)\" >");
									document.write("</span>&nbsp;");
									//]]>
									</script>&nbsp;
								</div>
							</fieldset><br />
							
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
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
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HStatus.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
					//]]>
					</script>
				</div>
				<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
				<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>
