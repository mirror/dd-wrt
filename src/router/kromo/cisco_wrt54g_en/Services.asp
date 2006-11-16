<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Services</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + service.titl;

function verify_unique_static_ip(F){                                              
	static_leasenum = <% nvram_get("static_leasenum"); %>;
	
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
}

function lease_add_submit(F) {
	F.submit_type.value = "add_lease";
	checked(F);
	F.submit();
}

function lease_remove_submit(F) {
	F.submit_type.value = "remove_lease";
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
		show_layer_ext(document.setup.oet1_en, 'idoet', <% nvram_else_match("oet1_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.oet1_bridged, 'idbridged', <% nvram_else_match("oet1_bridged", "1", "0", "1"); %> == 1);
		
		if(document.setup.rstats_enable) {
			rstats_select = '*user';
			path_input = '';
			a = '<% nvram_get("rstats_path"); %>';
			switch (a) {
				case '':
				case '*nvram':
				case '/jffs/':
				case '/tmp/smbshare/':
				rstats_select = a;
				break;
			}
			document.setup.rstats_select.value=rstats_select;
			(rstats_select == '*user') ? document.setup.u_path.value=path_input : document.setup.u_path.value='';
			
			setRstatsVal(document.setup);
			show_layer_ext(document.setup.rstats_enable, 'idrstats', <% nvram_else_match("rstats_enable", "1", "1", "0"); %> == 1);
		}
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
					<% do_menu("Management.asp","Services.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Services" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<input type="hidden" name="static_leases" value="13" />
							<input type="hidden" name="openvpn_certtype" />
							<input type="hidden" name="dhcpd_usejffs" />
							<input type="hidden" name="dhcpd_usenvram" />
							<h2><% tran("service.h2"); %></h2>
							<% show_modules(".webservices"); %>
							<div class="submitFooter">
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
						<div><h2><% tran("share.help"); %></h2></div>
						<br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HServices.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>