<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Services</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + service.titl;

function verify_unique_static_ip(F) {                                              
	static_leasenum = <% nvram_get("static_leasenum"); %>;
                                                                                
	//Check all static leases
	var static_leases=' ';                                                   
    for(i=0;i < static_leasenum;i++) {                                               
		var elem = F.elements["lease"+i+"_ip"];                                       
		if (static_leases.indexOf(" " + elem.value + " ") == -1) {
			static_leases += elem.value + " ";
		} else {
//			alert(elem.value + " is already defined as a static lease.");
			alert(elem.value + errmsg.err62);
			elem.focus();
			return false;
		}
    }
    return true;                                                            
} 

function lease_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Services";
	F.submit_type.value = "add_lease";
 	F.action.value = "Apply";
	F.submit();
}

function lease_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Services";
	F.submit_type.value = "remove_lease";
 	F.action.value = "Apply";
	F.submit();
}

function to_reboot(F) {
	F.action.value = "Reboot";
	F.submit();
}

function to_submit(F) {
	
	if(!verify_unique_static_ip(F)) {
		return false;
	}
	
	if (F._openvpn_certtype) {
		if (F._openvpn_certtype.checked == true) {
			F.openvpn_certtype.value = 1;
		} else {
			F.openvpn_certtype.value = 0;
		}
	}
	
	if (F._dhcpd_usejffs) {
		if (F._dhcpd_usejffs.checked == true) {
			F.dhcpd_usejffs.value = 1;
		} else {
			F.dhcpd_usejffs.value = 0;
		}
	}

	if (F._dhcpd_usenvram) {
		if (F._dhcpd_usenvram.checked == true) {
			F.dhcpd_usenvram.value = 1;
		} else {
			F.dhcpd_usenvram.value = 0;
		}
	}
	
	F.submit_button.value = "Services";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {

		show_layer_ext(document.setup.sshd_enable, 'idssh', <% nvram_else_match("sshd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.dnsmasq_enable, 'iddnsmasq', <% nvram_else_match("dnsmasq_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.kaid_enable, 'idkaid', <% nvram_else_match("kaid_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.snmpd_enable, 'idsnmp', <% nvram_else_match("snmpd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.rflow_enable, 'idrflow', <% nvram_else_match("rflow_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.macupd_enable, 'idMACupd', <% nvram_else_match("macupd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.pptpd_enable, 'idpptp', <% nvram_else_match("pptpd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.pptpd_client_enable, 'idpptpcli', <% nvram_else_match("pptpd_client_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.openvpn_enable, 'idvpn', <% nvram_else_match("openvpn_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.syslogd_enable, 'idsyslog', <% nvram_else_match("syslogd_enable", "1", "1", "0"); %> == 1);
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
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li class="current"><span><% tran("bmenu.admin"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><% tran("bmenu.adminManagement"); %></a></li>
											<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
											<li><span><% tran("bmenu.adminServices"); %></span></li>
											<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
											<li><a href="Log.asp"><% tran("bmenu.adminLog"); %></a></li>
											<li><a href="Diagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
											<li><a href="Wol.asp"><% tran("bmenu.adminWol"); %></a></li>
											<li><a href="Factory_Defaults.asp"><% tran("bmenu.adminFactory"); %></a></li>
								<script type="text/javascript">
										https_visit = <% support_elsematch("HTTPS","1","1","0"); %>;
										if (https_visit =="1") {
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminUpgrade + "</em></a></li>");
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminBackup + "</em></a></li>");
										} else {
											document.write("<li><a href=\"Upgrade.asp\">" + bmenu.adminUpgrade + "</a></li>");
											document.write("<li><a href=\"config.asp\">" + bmenu.adminBackup + "</a></li>");
										}											
								</script>
<!--										<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											<li><a href="config.asp">Backup</a></li>
 -->
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="reboot_button" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="static_leases" value="13" />
							<input type="hidden" name="openvpn_certtype" />
							<input type="hidden" name="dhcpd_usejffs" />
							<h2><% tran("service.h2"); %></h2>
							<% show_modules(".webservices"); %>
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"reboot_button\" value=\"" + sbutton.reboot + "\" onclick=\"to_reboot(this.form)\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div>
						<br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HServices.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>