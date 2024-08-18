<% do_pagehead("service.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function verify_unique_static_ip(F){
	static_leasenum = <% nvg("static_leasenum"); %>;
	
	//Check all static leases
	var static_leases=' ';
	for(i=0;i < static_leasenum;i++){
		var elem = F.elements["lease"+i+"_ip"];
		if (static_leases.indexOf(" " + elem.value + " ") == -1){
			static_leases += elem.value + " ";
		} else {
			alert(elem.value + errmsg.err62);
			elem.focus();
			return false;
		}
	}
	return true;
}

function checked(F) {
	if (F._dhcpd_usejffs) {
		(F._dhcpd_usejffs.checked == true) ? F.dhcpd_usejffs.value = 1 : F.dhcpd_usejffs.value = 0;
	}

	if (F._dhcpd_usenvram) {
		(F._dhcpd_usenvram.checked == true) ? F.dhcpd_usenvram.value = 1 : F.dhcpd_usenvram.value = 0;
	}

	if (F._nstx_log) {
		(F._nstx_log.checked == true) ? F.nstx_log.value = 1 : F.nstx_log.value = 0;
	}
}

function lease_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_lease";
	checked(F);
	F.submit();
}

function lease_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_lease";
	F.lease_del_value.value=I;
	checked(F);
	F.submit();
}

function to_reboot(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.action.value = "Reboot";
	apply(F);
}

function to_submit(F) {
	if(!verify_unique_static_ip(F)) {
		return false;
	}
	
	if(F.rstats_enable) {
		F.rstats_path.value = (F.rstats_select.value == '*user') ? F.u_path.value : F.rstats_select.value;
	}
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	checked(F);
	apply(F);
	return true;
}

function to_apply(F) {
	if(!verify_unique_static_ip(F)) {
		return false;
	}
	
	if(F.rstats_enable) {
		F.rstats_path.value = (F.rstats_select.value == '*user') ? F.u_path.value : F.rstats_select.value;
	}
	F.change_action.value = "";
	F.submit_type.value = "";
	F.apply_button.value = sbutton.applied;
	checked(F);
	applytake(F);
	return true;
}

function download_sshkey(F) {
	//alert(" F:" + F.name );
	window.location.href='/id_ed25519.ssh';
	alert(service.ssh_keydownload);
	window.location.reload();
}

function generate_sshkey(F) {
	if (confirm(service.ssh_keyalert) == true) {
		F.change_action.value="gozila_cgi";
		F.submit_type.value = "ssh_downloadkey";
		applytake(F);
	}
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	set_disable_state(document.setup.sshd_enable, 'idssh_keyready', <% nvem("sshd_keyready", "1", "0", "1"); %> == 1);
	show_layer_ext(document.setup.tor_enable, 'idtor', <% nvem("tor_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.zabbix_enable, 'idzabbix', <% nvem("zabbix_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mdns_enable, 'idmdns', <% nvem("mdns_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.sshd_enable, 'idssh', <% nvem("sshd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.dnsmasq_enable, 'iddnsmasq', <% nvem("dnsmasq_enable", "1", "1", "0"); %> == 1);
	/*show_layer_ext(document.setup.kaid_enable, 'idkaid', <% nvem("kaid_enable", "1", "1", "0"); %> == 1);*/
	show_layer_ext(document.setup.snmpd_enable, 'idsnmp', <% nvem("snmpd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.rflow_enable, 'idrflow', <% nvem("rflow_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.macupd_enable, 'idMACupd', <% nvem("macupd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_enable, 'idpptp', <% nvem("pptpd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_client_enable, 'idpptpcli', <% nvem("pptpd_client_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.syslogd_enable, 'idsyslog', <% nvem("syslogd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.nstx_ipenable, 'idnstxip', <% nvem("nstx_ipenable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.nstxd_enable, 'idnstx', <% nvem("nstxd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.oet1_en, 'idoet', <% nvem("oet1_en", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.radiooff_button, 'idradiooff', <% nvem("radiooff_button", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.usb_enable, 'idusb', <% nvem("usb_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.usb_storage, 'idusbstor', <% nvem("usb_storage", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.gps, 'idgps', <% nvem("gps", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mactelnetd_enable, 'idmactelnetd', <% nvem("mactelnetd_enable", "1", "1", "0"); %> == 1);
	if (document.setup.dns_crypt) {
		show_layer_ext(document.setup.dnsmasq_enable, 'iddns_crypt', <% nvem("dns_crypt", "1", "1", "0"); %> == 1);
	}
	if (document.setup.smartdns) {
		show_layer_ext(document.setup.dnsmasq_enable, 'idsmartdns', <% nvem("smartdns", "1", "1", "0"); %> == 1);
	}
	if (document.setup.dnsmasq_enable) {
		show_layer_ext(document.setup.dnsmasq_enable, 'idipv6dns', <% nvem("ipv6_enable", "1", "1", "0"); %> == 1);
	}
	if (document.setup.dnsmasq_enable) {
		show_layer_ext(document.setup.dnsmasq_enable, 'iddnsmasq_ipv6', <% nvem("dnsipv6_enable", "1", "1", "0"); %> == 1);
	}
	if (document.setup.lighttpd_enable) {
		show_layer_ext(document.setup.lighttpd_enable, 'lighttpdconfig', <% nvem("lighttpd_enable", "1", "1", "0"); %> == 1);
	}
	if(document.setup.rstats_enable) {
		rstats_select = '*user';
		path_input = '<% nvg("rstats_path"); %>';
		switch (path_input) {
			case '':
			case '*nvram':
			case '/jffs/':
			case '/tmp/mnt/smbshare/':
			rstats_select = path_input;
			break;
		}
		document.setup.rstats_select.value=rstats_select;
		(rstats_select == '*user') ? document.setup.u_path.value=path_input : document.setup.u_path.value='';
			
		setRstatsVal(document.setup);
		show_layer_ext(document.setup.rstats_enable, 'idrstats', <% nvem("rstats_enable", "1", "1", "0"); %> == 1);
	}
		
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
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Services.asp","Services.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="post" autocomplete="new-password" spellcheck="false">
							<input type="hidden" name="submit_button" value="Services" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />

							<input type="hidden" name="static_leases" value="13" />
							<input type="hidden" name="openvpn_certtype" />
							<input type="hidden" name="dhcpd_usejffs" />
							<input type="hidden" name="lease_del_value" />
							<input type="hidden" name="dhcpd_usenvram" />
							<input type="hidden" name="nstx_log" />

							<h2><% tran("service.h2"); %></h2>
							<% show_modules(".webservices"); %>
							//experimental by egc
							//<% ifndef("HAVE_IPV6", "<!--"); %>
							//	<% show_dnsipv6if(); %>
							//<% ifndef("HAVE_IPV6", "-->"); %>
							<% show_mdnsif(); %>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1,1);
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
							<dt class="term"><% tran("management.jffs_legend"); %>&nbsp;<% tran("share.requiremts"); %>:</dt>
							<dd class="definition"><% tran("hservice.right1"); %></dd>
							<dt class="term"><% tran("service.dns_smartdns"); %>:</dt>
							<dt class="term"><% tran("service.dns_smartdns_use_dns"); %></dt>
							<dd class="definition"><% tran("hservice.right2"); %></dd>
							<dt class="term"><% tran("service.ssh_keylegend"); %></dt>
							<dd class="definition"><% tran("hservice.sshshelp"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HServices.asp');"><% tran("share.more"); %></a>
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
