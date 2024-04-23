<% do_pagehead_nopwc("status_inet.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

var update;
var dummy="";

function DHCPAction(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = I;
	apply(F);
}

function connect(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = I;
	apply(F);
}

function speedtest(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "speedtest";
	apply(F);
}

function ttraff_remove_submit(F) {
	if(!confirm(status_inet.delete_confirm)) {
		return false;
	}
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "delete_ttraffdata";
	apply(F);
}

var d = new Date();
var count = d.getFullYear() * 12 + d.getMonth();

function get_month(count) {
	return count % 12 + 1;
}

function get_year(count) {
	return parseInt (count / 12);
}

function load_file(count) {
	return "/ttgraph.cgi?" + get_month(count) + "-" + get_year(count);
}

function do_show_prev() {
	count--;
	do_refresh()
}

function do_refresh() {
	var f = document.getElementById('graph');
	f.src = load_file(count);
}

function do_show_next() {
	count++;
	do_refresh()
}

function reloadIt() {
	do_refresh();
	setTimeout("reloadIt()", 30000);
}

addEvent(window, "load", function() {
	setElementVisible("wan_show", "<% getWET(); %>" == "0");
	setElementVisible("wan_showtraff", "<% nvg("ttraff_enable"); %>" == "1" && ("<% getWET(); %>" == "0" && "<% nvg("wan_proto"); %>" != "disabled" || "<% nvg("ttraff_iface"); %>" != "") );
	setElementVisible("wan_showdisabled", "<% getWET(); %>" == "1");
	setElementVisible("wan_info", "<% nvg("wan_proto"); %>" != "disabled");
	setElementVisible("wan_dhcp", "<% nvg("wan_proto"); %>" == "dhcp" || "<% nvg("wan_proto"); %>" == "dhcp_auth");
	setElementVisible("wan_connection", "<% nvg("wan_proto"); %>" != "dhcp" && "<% nvg("wan_proto"); %>" != "dhcp_auth" && "<% nvg("wan_proto"); %>" != "static");
	setElementVisible("wan_signal", "<% nvg("wan_proto"); %>" == "3g");
	setElementVisible("wan_acname", "<% nvg("wan_proto"); %>" == "pppoe");
	setElementVisible("ipv6", "<% nvg("ipv6_enable"); %>" == "1");

	update = new StatusUpdate("Status_Internet.live.asp", <% nvg("refresh_time"); %>);
	update.onUpdate("wan_shortproto", function(u) {
		setElementVisible("wan_info", u.wan_shortproto != "disabled");
		setElementVisible("wan_dhcp", u.wan_shortproto == "dhcp" || u.wan_shortproto == "dhcp_auth");
		setElementVisible("wan_connection", u.wan_shortproto != "dhcp" && u.wan_shortproto != "dhcp_auth" && u.wan_shortproto != "static");
	});

	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

	//]]>
	</script>
	</head>

	<body class="gui" onload="reloadIt()">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Status_Router.asp","Status_Internet.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="status" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Status_Internet" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("status_inet.h11"); %></h2>
							<% wan_if_status(); %>
							<fieldset>
								<legend><% tran("status_inet.conft"); %></legend>
								<div class="setting" id="wan_showdisabled">
									<div class="label"><% tran("idx.conn_type"); %></div>
									<% tran("share.disabled"); %>
								</div>
								<div id="wan_show" style="display:none">
									<div class="setting">
										<div class="label"><% tran("idx.conn_type"); %></div>
										<% nvm("wan_proto", "dhcp", "<script type="text/javascript">Capture(idx.dhcp)</script>"); %>
										<% nvm("wan_proto", "dhcp_auth", "<script type="text/javascript">Capture(idx.dhcp_auth)</script>"); %>
										<% nvm("wan_proto", "static", "<script type="text/javascript">Capture(share.sttic)</script>"); %>
										<% nvm("wan_proto", "pppoe", "PPPoE"); %>
										<% nvm("wan_proto", "pptp", "PPTP"); %>
										<% nvm("wan_proto", "l2tp", "L2TP"); %>
										<% nvm("wan_proto", "3g", "LTE-4G/3G/2G"); %>
										<% nvm("wan_proto", "heartbeat", "HeartBeatSignal"); %>
										<% nvm("wan_proto", "disabled", "<script type="text/javascript">Capture(share.disabled)</script>"); %>&nbsp;
									</div>
									<div id="wan_info" style="display:none">
										<div class="setting" id="wan_connection">
											<div class="label"><% tran("status_inet.www_loginstatus"); %></div>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<span id=\"wan_status\">" + <% nvram_status_get("status2","3"); %> + "&nbsp;<input class=\"button\" type=\"button\" value=\"" + <% nvram_status_get("button1","3"); %> + "\" onclick=\"connect(this.form, '<% nvram_status_get("button1","0"); %>_<% nvg("wan_proto"); %>');\"></span>");
												//]]>
												</script>
										</div>
										<div class="setting" id="wan_signal">
											<div class="label"><% tran("status_inet.sig_status"); %></div>
											<span id="wan_3g_signal"><% nvram_status_get("wan_3g_signal"); %></span>
										</div>
										<div class="setting" id="wan_acname">
											<div class="label"><% tran("status_inet.acname"); %></div>
											<span id="pppoe_ac_name"><% nvg("pppoe_ac_name"); %></span>&nbsp;
										</div>
										 <div class="setting">
											<div class="label"><% tran("status_inet.wanuptime"); %></div>
											<span id="wan_uptime"><% get_wan_uptime("1"); %></span>&nbsp;
										</div>
										<div class="setting" id="ipv6">
											<div class="label"><% tran("share.ipv6"); %></div>
											<span id="wan_ipv6addr"><% nvram_status_get("wan_ipv6addr","2"); %></span>&nbsp;
										</div>
										<div class="setting">
											<div class="label"><% tran("share.ipv4"); %></div>
											<span id="wan_ipaddr"><% nvram_status_get("wan_ipaddr","2"); %></span>&nbsp;
										</div>
										<div class="setting">
											<div class="label"><% tran("share.gateway"); %></div>
											<span id="wan_gateway"><% nvram_status_get("wan_gateway","0"); %></span>&nbsp;
										</div>
										<% show_dnslist(); %>
										<div id="wan_dhcp">
											<div class="setting">
												<div class="label"><% tran("status_inet.leasetime"); %></div>
												<span id="dhcp_remaining"><% dhcp_remaining_time(); %></span>&nbsp;
											</div>
											<div class="center">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.dhcprel + "\" onclick=\"DHCPAction(this.form,'release');\">");
												document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.dhcpren + "\" onclick=\"DHCPAction(this.form,'renew');\">");
												//]]>
												</script>
											</div>
										</div>
									</div>
								</div>
							</fieldset><br />
							<% ifndef("SPEEDTEST_CLI", "<!--"); %>
							<h2><% tran("status_inet.speed"); %></h2>
								<fieldset>
									<legend><% tran("status_inet.speedtest"); %></legend>
										 <div class="setting">
											<div class="label"><% tran("status_inet.test"); %></div>
											<script>
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" value=\"" + status_inet.test +"\" onclick=\"speedtest(this.form);\">");
											//]]>
											</script>
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.sponsor"); %></div>
											<span id="speed_sponsor"><% speed_sponsor(); %></span>
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.town"); %></div>
											<span id="speed_town"><% speed_name(); %></span>
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.country"); %></div>
											<span id="speed_country"><% speed_country(); %></span>
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.latency"); %></div>
											<span id="speed_latency"><% speed_latency(); %></span>&nbsp;ms
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.down"); %></div>
											<span id="speed_down"><% speed_down(); %></span>&nbsp;Mbit/s
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.up"); %></div>
											<span id="speed_up"><% speed_up(); %></span>&nbsp;Mbit/s
										</div>
								</fieldset>
							<% ifndef("SPEEDTEST_CLI", "-->"); %>
							<div id="wan_showtraff" style="display:none">
							<h2><% tran("status_inet.traff"); %> <% nvg("ttraff_iface"); %></h2>
								<fieldset>
									<legend><% tran("status_inet.traff_tot"); %></legend>
										<div class="setting">
											<div class="label"><% tran("status_inet.traffin"); %></div>
											<span id="ttraff_in"><% get_totaltraff("in"); %></span>&nbsp;MiB
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.traffout"); %></div>
											<span id="ttraff_out"><% get_totaltraff("out"); %></span>&nbsp;MiB
										</div>
								</fieldset><br />
								<fieldset class="center">
								<legend><% tran("status_inet.traff_mon"); %></legend>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<iframe id=\"graph\" src=\"" + load_file(count) + "\" title=\"" + service.ttraff_legend + "\" width=\"560\" height=\"370\" frameborder=\"0\" type=\"text/html\"></iframe>");
									//]]>
									</script>
									<div class="center">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" value=\"" + status_inet.previous + "\" onclick=\"do_show_prev();\">");
										document.write("<input class=\"button\" type=\"button\" value=\"" + status_inet.next + "\" onclick=\"do_show_next();\">");
										//]]>
										</script>
									</div>
								</fieldset><br />
								<fieldset>
								<legend><% tran("status_inet.dataadmin"); %></legend>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"backup_button\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/traffdata.bak';\" />");
									document.write("<input class=\"button\" type=\"button\" name=\"restore_button\" value=\"" + sbutton.restore + "\" onclick=\"openWindow('Traff_admin.asp', 600, 180);\" />");
									document.write("<input class=\"button\" type=\"button\" name=\"delete_button\" value=\"" + sbutton.del + "\" onclick=\"ttraff_remove_submit(this.form);\" />");
									//]]>
									</script>
								</fieldset><br />
							</div>
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
							<dt class="term"><% tran("status_inet.conft"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right2"); %></dd>
							<dt class="term"><% tran("status_inet.traff_tot"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right4"); %></dd>
							<dt class="term"><% tran("status_inet.traff_mon"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right6"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HStatusWan.asp');"><% tran("share.more"); %></a>
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
