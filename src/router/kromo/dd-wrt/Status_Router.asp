<% do_pagehead_nopwc("status_router.titl"); %>
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
	setMeterBar("mem_total", memTotal / memSystem * 100, memTotal + " KiB / " + memSystem + " KiB");
	setMeterBar("mem_free", memFree / memTotal * 100, memFree + " KiB / " + memTotal + " KiB");
	setMeterBar("mem_used", memUsed / memTotal * 100, memUsed + " KiB / " + memTotal + " KiB");
	setMeterBar("mem_buffer", memBuffer / memUsed * 100, memBuffer + " KiB / " + memUsed + " KiB");
	setMeterBar("mem_cached", memCached / memUsed * 100, memCached + " KiB / " + memUsed + " KiB");
	setMeterBar("mem_active", memActive / memUsed * 100, memActive + " KiB / " + memUsed + " KiB");
	setMeterBar("mem_inactive", memInactive / memUsed * 100, memInactive + " KiB / " + memUsed + " KiB");
}

function setUptimeValues(val) {
	setElementContent("uptime_up", val.substring(val.indexOf("up") + 3, val.indexOf("load") - 3));
	var loadAverage = val.substring(val.indexOf("average") + 9).split(",");
	setMeterBar("uptime_load", parseFloat(loadAverage[0]) / <% show_cpucores(); %> * 100.0, loadAverage.join(","));
}

function setIpconntrackValues(val) {
	setMeterBar("ip_count", val / <% nvg("ip_conntrack_max"); %> * 100, val);
}

addEvent(window, "load", function() {
	setMemoryValues("<% dumpmeminfo(); %>");
	setUptimeValues("<% get_uptime(); %>");
	setIpconntrackValues("<% dumpip_conntrack(); %>");

	update = new StatusUpdate("Status_Router.live.asp", <% nvg("refresh_time"); %>);
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
									<% nvg("router_name"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_model"); %></div>
									<% get_sysmodel(); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_firmver"); %></div>
									<% get_firmware_version_noreg(); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_kernel"); %></div>
									<% get_syskernel(); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"wan_mac\" class=\"link\" style=\"cursor:pointer;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvg("wan_hwaddr"); %>');\" >");
									document.write("<% nvg("wan_hwaddr"); %>");
									document.write("</span>");
									//]]>
									</script>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<% nvg("wan_hostname"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.wandomainname"); %></div>
									<% show_wan_domain(); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.landomainname"); %></div>
									<% nvg("lan_domain"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_time"); %></div>
									<span id="router_time"><% localtime("1"); %></span>&nbsp;
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
									<div class="label"><% tran("status_router.cores"); %></div>
									<% show_cpucores(); %>&nbsp;
								</div>
								<% show_cpufeatures(); %>
								<div class="setting">
									<div class="label"><% tran("status_router.clock"); %></div>
									<span id="clkfreq"><% get_clkfreq(); %></span>&nbsp;MHz
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.sys_load"); %></div>
									<span id="uptime_load"></span>&nbsp;
								</div>
							</fieldset><br />
							<% show_cpu_temperature(); %>
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
								<legend><% tran("status_router.legend6"); %></legend>
									<div class="setting">
										<div class="label">NVRAM</div>
										<span id="nvram"><% statnv(); %></span>&nbsp;
									</div>
									<% ifndef("SAMBA", "<!--"); %>
									<div class="setting">
										<div class="label">CIFS</div>
										<script type="text/javascript">
										//<![CDATA[
										<% statfs("/tmp/mnt/smbshare", "samba"); %>
										document.write( ((<% nvg("samba_mount"); %>) && (samba.size)) ? (scaleSize(samba.used) + ' / ' + scaleSize(samba.size)) : '<span style="opacity: .8;"><em>(' + share.nmounted + ')</em></span>' );
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
										document.write( ((<% nvg("enable_jffs2"); %>) && (my_jffs.size)) ? (scaleSize(my_jffs.used) + ' / ' + scaleSize(my_jffs.size)) : '<span style="opacity: .8;"><em>(' + share.nmounted + ')</em></span>' );
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
										document.write( ((<% nvg("mmc_enable0"); %>) && (mmc.size)) ? (scaleSize(mmc.used) + ' / ' + scaleSize(mmc.size)) : '<span style="opacity: .8;"><em>(' + share.nmounted + ')</em></span>' );
										//]]>
										</script>
									</div>
									<% ifndef("MMC", "-->"); %>
							</fieldset><br />
							<fieldset>
								<legend><% tran("status_router.legend4"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_router.net_ipcontrkmax"); %></div>
									<% nvg("ip_conntrack_max"); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_router.net_conntrack"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span class=\"link\" id=\"ip_count\" style=\"cursor: pointer;\" title=\"" + share.detail + "\" onclick=\"openWindow('Status_Conntrack.asp', 800, 600)\" >");
									document.write("</span>&nbsp;");
									//]]>
									</script>&nbsp;
								</div>
							</fieldset><br />
							<div class="submitFooter nostick">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
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
					<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
