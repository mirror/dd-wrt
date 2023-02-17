<% do_pagehead("firewall.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function submitcheck(F) {
	if(F._block_proxy){
		F.block_proxy.value = F._block_proxy.checked ? 1 : 0;
	}
	if(F._block_cookie){
		F.block_cookie.value = F._block_cookie.checked ? 1 : 0;
	}
	if(F._block_java){
		F.block_java.value = F._block_java.checked ? 1 : 0;
	}
	if(F._block_activex){
		F.block_activex.value = F._block_activex.checked ? 1 : 0;
	}
	if (F._block_wan){
		F.block_wan.value = F._block_wan.checked ? 1 : 0;
	}
	if(F._block_multicast) {
		F.block_multicast.value = F._block_multicast.checked ? 1 : 0;
	}
	if(F._block_loopback){
		F.block_loopback.value = F._block_loopback.checked ? 1 : 0;
	}
	if(F._block_ident){
		F.block_ident.value = F._block_ident.checked ? 1 : 0;
	}
	if(F._block_snmp){
		F.block_snmp.value = F._block_snmp.checked ? 1 : 0;
	}
	if(F._arp_spoofing){
		F.arp_spoofing.value = F._arp_spoofing.checked ? 1 : 0;
	}
	if(F._filter_tos){
		F.filter_tos.value = F._filter_tos.checked ? 1 : 0;
	}
	if(F._limit_ssh){
		F.limit_ssh.value = F._limit_ssh.checked ? 1 : 0;
	}
	if(F._limit_telnet){
		F.limit_telnet.value = F._limit_telnet.checked ? 1 : 0;
	}
	if(F._limit_pptp){
		F.limit_pptp.value = F._limit_pptp.checked ? 1 : 0;
	}
	if(F._limit_ftp){
		F.limit_ftp.value = F._limit_ftp.checked ? 1 : 0;
	}
	if (F.filter.value == "off"){
		if (F.log_enable) {
			F.log_enable.value = 0;
		}
	}
}

function to_submit(F) {
	submitcheck(F);
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	submitcheck(F);
	F.apply_button.value = sbutton.applied;
	applytake(F);
}

function setFirewall(F, val) {
<% ifdef("MICRO", "/"); %><% ifdef("MICRO", "*"); %>if (val != "on") { document.firewall.log_enable[1].click(); }<% ifdef("MICRO", "*"); %><% ifdef("MICRO", "/"); %>
	if (val != "on") {
		if(F._block_proxy){
			F._block_proxy.checked = false;
		}
		if(F._block_cookie){
			F._block_cookie.checked = false;
		}
		if(F._block_java){
			F._block_java.checked = false;
		}
		if(F._block_activex){
			F._block_activex.checked = false;
		}
		if (F._block_wan){
			F._block_wan.checked = false;
		}
		if(F._block_multicast) {
			F._block_multicast.checked = false;
		}
		if(F._block_loopback){
			F._block_loopback.checked = false;
		}
		if(F._block_ident){
			F._block_ident.checked = false;
		}
		if(F._block_snmp){
			F._block_snmp.checked = false;
		}
		if(F._arp_spoofing){
			F._arp_spoofing.checked = false;
		}
		if(F._filter_tos){
			F._filter_tos.checked = false;
		}
		if(F._limit_ssh){
			F._limit_ssh.checked = false;
		}
		if(F._limit_telnet){
			F._limit_telnet.checked = false;
		}
		if(F._limit_pptp){
			F._limit_pptp.checked = false;
		}
		if(F._limit_ftp){
			F._limit_ftp.checked = false;
		}
	}
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	setFirewall(this.document.firewall, "<% nvg("filter"); %>");
	show_layer_ext(document.firewall.log_enable, 'idlog1', <% nvem("log_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.firewall.log_enable, 'idlog2', <% nvem("log_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.firewall.warn_enabled, 'idwarn', <% nvem("warn_enabled", "1", "1", "0"); %> == 1);

	update = new StatusbarUpdate();
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
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<% do_menu("Firewall.asp","Firewall.asp"); %>
				</div>
				<div id="main">
				<div id="contents">
					<form name="firewall" action="apply.cgi" method="post" >
						<input type="hidden" name="submit_button" value="Firewall" />
						<input type="hidden" name="action" value="Apply" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="submit_type" />

						<input type="hidden" name="block_wan" />
						<input type="hidden" name="block_loopback" />
						<input type="hidden" name="block_multicast" />
						<input type="hidden" name="block_ident" />
						<input type="hidden" name="block_cookie" />
						<input type="hidden" name="block_java" />
						<input type="hidden" name="block_proxy" />
						<input type="hidden" name="block_activex" />
						<input type="hidden" name="block_snmp" />
						<input type="hidden" name="arp_spoofing" />
						<input type="hidden" name="filter_tos" />
						<input type="hidden" name="limit_ssh" />
						<input type="hidden" name="limit_telnet" />
						<input type="hidden" name="limit_pptp" />
						<input type="hidden" name="limit_ftp" />
						<h2><% tran("firewall.h2"); %></h2>
						<fieldset>
							<legend><% tran("firewall.legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("firewall.firewall"); %></div>
								<input class="spaceradio" type="radio" value="on" name="filter" <% nvc("filter", "on"); %> onclick="setFirewall(this.form, this.value);" /><% tran("share.enable"); %>&nbsp;
								<input class="spaceradio" type="radio" value="off" name="filter" <% nvc("filter", "off"); %> onclick="setFirewall(this.form, this.value);" /><% tran("share.disable"); %>
							</div>
						</fieldset><br />
						<div id="idfilter">
							<fieldset>
								<legend><% tran("firewall.legend2"); %></legend>
									<div class="setting">
									<div class="label"><% tran("firewall.proxy"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_block_proxy" <% nvc("block_proxy", "1"); %> />
									</div>
									<div class="setting">
									<div class="label"><% tran("firewall.cookies"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_block_cookie" <% nvc("block_cookie", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("firewall.applet"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_block_java" <% nvc("block_java", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("firewall.activex"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_block_activex" <% nvc("block_activex", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("firewall.filter_tos"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_filter_tos" <% nvc("filter_tos", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("firewall.arp_spoofing"); %></div>
										<input class="spaceradio" type="checkbox" value="1" name="_arp_spoofing" <% nvc("arp_spoofing", "1"); %> />
									</div>
								</fieldset><br />
								<fieldset>
									<legend><% tran("firewall.legend3"); %></legend>
										<div class="setting">
											<div class="label"><% tran("firewall.ping"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_block_wan" <% nvc("block_wan", "1"); %> />
										</div>
										<% ifndef("MULTICAST", "<!--"); %>
										<div class="setting">
											<div class="label"><% tran("firewall.muticast"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_block_multicast" <% nvc("block_multicast", "1"); %> />
										</div>
										<% ifndef("MULTICAST", "-->"); %>
										<div class="setting">
											<div class="label"><% tran("filter.nat"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_block_loopback" <% nvc("block_loopback", "1"); %> />
										</div>
										<div class="setting">
											<div class="label"><% tran("filter.port113"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_block_ident" <% nvc("block_ident", "1"); %> />
										</div>
										<% ifndef("SNMP", "<!--"); %>
										<div class="setting">
											<div class="label"><% tran("filter.snmp"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_block_snmp" <% nvc("block_snmp", "1"); %> />
										</div>
										<% ifndef("SNMP", "-->"); %>
									</fieldset>
								<% ifdef("MICRO", "<!--"); %>
								<br />
								<fieldset>
									<legend><% tran("firewall.legend4"); %></legend>
										<div class="setting">
											<div class="label"><% tran("firewall.ssh"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_limit_ssh" <% nvc("limit_ssh", "1"); %> />
										</div>
										<div class="setting">
											<div class="label"><% tran("firewall.telnet"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_limit_telnet" <% nvc("limit_telnet", "1"); %> />
										</div>
										<div class="setting">
											<div class="label"><% tran("firewall.pptp"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_limit_pptp" <% nvc("limit_pptp", "1"); %> />
										</div>
										<div class="setting">
											<div class="label"><% tran("firewall.ftp"); %></div>
											<input class="spaceradio" type="checkbox" value="1" name="_limit_ftp" <% nvc("limit_ftp", "1"); %> />
										</div>
								</fieldset>
								<% ifdef("MICRO", "-->"); %>
								<br /></div>
								<% show_modules(".websecurity"); %>
								<% ifdef("MICRO", "<!--"); %>
								<h2><% tran("log.h2"); %></h2>
							<fieldset>
								<legend><% tran("log.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("log.label"); %></div>
									<input class="spaceradio" type="radio" value="1" name="log_enable" <% nvc("log_enable", "1"); %> onclick="show_layer_ext(this, 'idlog1', true);show_layer_ext(this,'idlog2', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="log_enable" <% nvc("log_enable", "0"); %> onclick="show_layer_ext(this, 'idlog1', false);show_layer_ext(this,'idlog2', false)" /><% tran("share.disable"); %>
								</div>
							<div id="idlog1">
								<div class="setting">
									<div class="label"><% tran("log.lvl"); %></div>
									<select name="log_level">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsjs("log_level", "0"); %> >" + share.disabled + "</option>");
										document.write("<option value=\"1\" <% nvsjs("log_level", "1"); %> >" + share.medium + "</option>");
										document.write("<option value=\"2\" <% nvsjs("log_level", "2"); %> >" + share.high + "</option>");
										//]]>
										</script>
									</select>
								</div>
							</div>
							<div id="idlog2"><br />
									<div class="setting">
										<div class="label"><% tran("log.drop"); %></div>
										<select name="log_dropped">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsjs("log_dropped", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvsjs("log_dropped", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.reject"); %></div>
										<select name="log_rejected">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsjs("log_rejected", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvsjs("log_rejected", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.accept"); %></div>
										<select name="log_accepted">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsjs("log_accepted", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvsjs("log_accepted", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"log_incoming\" value=\"" + sbutton.log_in + "\" onclick=\"openWindow('Log_incoming.asp', 580, 600);\" />&nbsp;");
									document.write("<input class=\"button\" type=\"button\" name=\"log_outgoing\" value=\"" + sbutton.log_out + "\" onclick=\"openWindow('Log_outgoing.asp', 760, 600);\" />");
									//]]>
									</script>
								</div>
							</div>
								</fieldset><br />
								<% ifdef("MICRO", "-->"); %>
								<div id="footer" class="submitFooter">
									<script type="text/javascript">
									//<![CDATA[
									submitFooterButton(1,1);
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
							<dt class="term"><% tran("firewall.legend"); %>:</dt>
							<dd class="definition"><% tran("hfirewall.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HFirewall.asp');"><% tran("share.more"); %></a>
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
