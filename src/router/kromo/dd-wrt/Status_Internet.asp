<% do_pagehead("status_inet.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

var update;

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

function ttraff_remove_submit(F) {
	if(!confirm(share.del)) {
	return false;
	}
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "delete_ttraffdata";
	apply(F);
}

function ttraff_restore_submit(F) {
	if (F.file.value == "")	{
	alert(errmsg.err42);
	return false;
	}
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "restore_ttraffdata";
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
	setElementVisible("wan_show", "<% nvram_get("wl0_mode"); %>" != "wet" && "<% nvram_get("wl0_mode"); %>" != "apstawet");
	setElementVisible("wan_showtraff", "<% nvram_get("wl0_mode"); %>" != "wet" && "<% nvram_get("wl0_mode"); %>" != "apstawet" && "<% nvram_get("wan_proto"); %>" != "disabled" && "<% nvram_get("ttraff_enable"); %>" == "1");	
	setElementVisible("wan_showdisabled", "<% nvram_get("wl0_mode"); %>" == "wet" || "<% nvram_get("wl0_mode"); %>" == "apstawet");
	setElementVisible("wan_info", "<% nvram_get("wan_proto"); %>" != "disabled");
	setElementVisible("wan_dhcp", "<% nvram_get("wan_proto"); %>" == "dhcp");
	setElementVisible("wan_connection", "<% nvram_get("wan_proto"); %>" != "dhcp" && "<% nvram_get("wan_proto"); %>" != "static");

	update = new StatusUpdate("Status_Internet.live.asp", <% nvram_get("refresh_time"); %>);
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

	<body class="gui" onload="reloadIt()">
		<% showad(); %>
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
							<fieldset>
								<legend><% tran("status_inet.conft"); %></legend>
								<div class="setting" id="wan_showdisabled">
									<div class="label"><% tran("idx.conn_type"); %></div>
									<% tran("share.disabled"); %>
								</div>
								<div id="wan_show" style="display:none">
									<div class="setting">
										<div class="label"><% tran("idx.conn_type"); %></div>
										<% nvram_match("wan_proto", "dhcp", "<script type="text/javascript">Capture(idx.dhcp)</script>"); %><% nvram_match("wan_proto", "static", "<script type="text/javascript">Capture(share.sttic)</script>"); %><% nvram_match("wan_proto", "pppoe", "PPPoE"); %><% nvram_match("wan_proto", "pptp", "PPTP"); %><% nvram_match("wan_proto", "l2tp", "L2TP"); %><% nvram_match("wan_proto", "heartbeat", "HeartBeatSignal"); %><% nvram_match("wan_proto", "disabled", "<script type="text/javascript">Capture(share.disabled)</script>"); %>&nbsp;
									</div>
									<div id="wan_info" style="display:none">
										<div class="setting" id="wan_connection">
											<div class="label"><% tran("status_inet.www_loginstatus"); %></div>
											<span id="wan_status"><% nvram_status_get("status2"); %>&nbsp;<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>');" /></span>
										</div>
										 <div class="setting">
											<div class="label"><% tran("status_inet.wanuptime"); %></div>
											<span id="wan_uptime"><% get_wan_uptime(); %></span>&nbsp;
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
							
							<div id="wan_showtraff" style="display:none">
							<h2><% tran("status_inet.traff"); %></h2>
								<fieldset>
									<legend><% tran("status_inet.traff_tot"); %></legend>
										 <div class="setting">
											<div class="label"><% tran("status_inet.traffin"); %>&nbsp;(MBytes)</div>
											<span id="ttraff_in"><% get_totaltraff("in"); %></span>&nbsp;
										</div>
										<div class="setting">
											<div class="label"><% tran("status_inet.traffout"); %>&nbsp;(MBytes)</div>
											<span id="ttraff_out"><% get_totaltraff("out"); %></span>&nbsp;
										</div>
								</fieldset><br />
								<fieldset>
								<legend><% tran("status_inet.traff_mon"); %></legend>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<iframe id=\"graph\" src=\"" + load_file(count) + "\" width=\"555\" height=\"350\" frameborder=\"0\" type=\"text/html\">");
									//]]>
									</script>

									</iframe>
										<div class="center">
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" value=\"" + status_inet.previous + "\" onclick=\"do_show_prev();\">");
											document.write("<input class=\"button\" type=\"button\" value=\"" + status_inet.next + "\" onclick=\"do_show_next();\">");
											//]]>
											</script>
										</div><br /><hr>
										<div class="setting"><% tran("bmenu.admin"); %></div>
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" name=\"backup_button\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/traffdata.bak';\" />");
											document.write("<input class=\"button\" type=\"button\" name=\"restore_button\" value=\"" + sbutton.restore + "\" onclick=\"ttraff_restore_submit(this.form);\" />");
											document.write("<input class=\"button\" type=\"button\" name=\"delete_button\" value=\"" + sbutton.del + "\" onclick=\"ttraff_remove_submit(this.form);\" />");
											//]]>
											</script>
										<div class="setting">
											<div class="label"><% tran("config.mess2"); %></div>
											<input type="file" name="file" size="40" />
										</div>
								</fieldset><br />
							</div>

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
							<dt class="term"><% tran("status_inet.conft"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right2"); %></dd>
							<dt class="term"><% tran("status_inet.traff_tot"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right4"); %></dd>
							<dt class="term"><% tran("status_inet.traff_mon"); %>: </dt>
							<dd class="definition"><% tran("hstatus_inet.right6"); %></dd>
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
