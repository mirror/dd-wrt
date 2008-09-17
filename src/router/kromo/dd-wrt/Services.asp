<% do_pagehead("service.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

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
		(F._openvpn_certtype.checked == true) ?	F.openvpn_certtype.value = 1 : F.openvpn_certtype.value = 0;
	}
	
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

function lease_remove_submit(F) {
	F.change_action.value="gozila_cgi";
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
	F.save_button.value = sbutton.saving;
	checked(F);
	applytake(F);
	return true;
}

var update;

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
		show_layer_ext(document.setup.openvpncl_enable, 'idvpncl', <% nvram_else_match("openvpncl_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.syslogd_enable, 'idsyslog', <% nvram_else_match("syslogd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.nstx_ipenable, 'idnstxip', <% nvram_else_match("nstx_ipenable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.nstxd_enable, 'idnstx', <% nvram_else_match("nstxd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.oet1_en, 'idoet', <% nvram_else_match("oet1_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.radiooff_button, 'idradiooff', <% nvram_else_match("radiooff_button", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.usb_enable, 'idusb', <% nvram_else_match("usb_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.usb_storage, 'idusbstor', <% nvram_else_match("usb_storage", "1", "1", "0"); %> == 1);
		
		if(document.setup.rstats_enable) {
			rstats_select = '*user';
			path_input = '<% nvram_get("rstats_path"); %>';
			switch (path_input) {
				case '':
				case '*nvram':
				case '/jffs/':
				case '/tmp/smbshare/':
				rstats_select = path_input;
				break;
			}
			document.setup.rstats_select.value=rstats_select;
			(rstats_select == '*user') ? document.setup.u_path.value=path_input : document.setup.u_path.value='';
			
			setRstatsVal(document.setup);
			show_layer_ext(document.setup.rstats_enable, 'idrstats', <% nvram_else_match("rstats_enable", "1", "1", "0"); %> == 1);
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
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Services.asp","Services.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="post">
							<input type="hidden" name="submit_button" value="Services" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<input type="hidden" name="static_leases" value="13" />
							<input type="hidden" name="openvpn_certtype" />
							<input type="hidden" name="dhcpd_usejffs" />
							<input type="hidden" name="dhcpd_usenvram" />
							<input type="hidden" name="nstx_log" />
							
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
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HServices.asp');"><% tran("share.more"); %></a>
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