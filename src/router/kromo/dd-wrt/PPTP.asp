<% do_pagehead("service.pptp_legend"); %>
	<script type="text/javascript">
	//<![CDATA[

function import_vpntunnel(F,myid) {
	//this is triggered by on change event of filepicker
	//alert("getvpnfile: F:" + F.name + "; key-tun: " +  "; myid: " + myid);
	var vpnfileid = document.getElementById(myid).files[0];
	//console.log("vpnfileid.name: " + vpnfileid.name + "; F: " + F + "; key-tun: " + "; myid: " + myid);
	if (vpnfileid.size > 128000) {
		//alert("File: " + vpnfileid.name +" with size: " + vpnfileid.size +"B exceeds limit, is this the right file?");
		if(!confirm("File:" + vpnfileid.name +" with size:" + vpnfileid.size +"B exceeds limit, is this the right file?")) {
			return false;
		}
	}
 	var vpnreader = new FileReader();
 	vpnreader.readAsText(vpnfileid, "UTF-8");
	vpnreader.onload = function () {
		var vpnfile = vpnreader.result;
		//console.log("vpnconffile: " + vpnfile);
		//F.keyindex.value = keyindex;
		F.vpn_conf_file.value = vpnfile;
		//console.log("F.vpn_conf_file.value: " + F.vpn_conf_file.value);
		F.change_action.value="gozila_cgi";
		F.submit_type.value = "import_vpntunnel";  //either execute script or .c module to parse data
		apply(F);
	}
}

function chap_user_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_chap_user";
	checked(F);
	F.submit();
}

function chap_user_remove_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "remove_chap_user";
	checked(F);
	F.submit();
}

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	if (F._openvpncl_certtype) {
		F.openvpncl_certtype.value = F._openvpncl_certtype.checked ? 1 : 0;
	}
	if (F._openvpncl_randomsrv) {
		F.openvpncl_randomsrv.value = F._openvpncl_randomsrv.checked ? 1 : 0;
	}
	if (F._openvpncl_fw) {
		F.openvpncl_fw.value = F._openvpncl_fw.checked ? 1 : 0;
	}
	if (F._openvpncl_killswitch) {
		F.openvpncl_killswitch.value = F._openvpncl_killswitch.checked ? 1 : 0;
	}
	if (F._openvpncl_splitdns) {
		F.openvpncl_splitdns.value = F._openvpncl_splitdns.checked ? 1 : 0;
	}
	if (F._openvpn_fw) {
		F.openvpn_fw.value = F._openvpn_fw.checked ? 1 : 0;
	}
	apply(F);
}

function to_apply(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.apply_button.value = sbutton.applied;
	if (F._openvpncl_certtype) {
		F.openvpncl_certtype.value = F._openvpncl_certtype.checked ? 1 : 0;
	}
	if (F._openvpncl_randomsrv) {
		F.openvpncl_randomsrv.value = F._openvpncl_randomsrv.checked ? 1 : 0;
	}
	if (F._openvpncl_fw) {
		F.openvpncl_fw.value = F._openvpncl_fw.checked ? 1 : 0;
	}
	if (F._openvpncl_killswitch) {
		F.openvpncl_killswitch.value = F._openvpncl_killswitch.checked ? 1 : 0;
	}
	if (F._openvpncl_splitdns) {
		F.openvpncl_splitdns.value = F._openvpncl_splitdns.checked ? 1 : 0;
	}
	if (F._openvpn_fw) {
		F.openvpn_fw.value = F._openvpn_fw.checked ? 1 : 0;
	}
	applytake(F);
}

function changevpnclprot(F, value) {
	if (value == "tap" ) {
		show_layer_ext(F, "idnat", true);
		show_layer_ext(F, "idsec", false);
	} else {
		show_layer_ext(F, "idnat", false);
		show_layer_ext(F, "idsec", true);
	}
}

function changesrvtuntap(F, value) {
	if (value == "tap" ) {
		show_layer_ext(F, "idrouter", false);
		show_layer_ext(F, "idrouter2", false);
		show_layer_ext(F, "idbridge", true);
	} else {
		show_layer_ext(F, "idrouter", true);
		show_layer_ext(F, "idrouter2", true);
		show_layer_ext(F, "idbridge", false);
	}
}

function showidpki(F, value) {
	if (value == 3 ) {
		show_layer_ext(F, "idpki", true);
		show_layer_ext(F, "idtls", false);
		show_layer_ext(F, "idstatic", false);
	}
	else if (value == 2 ) {
		show_layer_ext(F, "idpki", false);
		show_layer_ext(F, "idtls", false);
		show_layer_ext(F, "idstatic", true);
	} else {
		show_layer_ext(F, "idpki", true);
		show_layer_ext(F, "idtls", true);
		show_layer_ext(F, "idstatic", false);
	}
}

function showidspki(F, value) {
	if (value == 3 ) {
		show_layer_ext(F, "idspki", true);
		show_layer_ext(F, "idstls", false);
		show_layer_ext(F, "idsstatic", false);
	}
	else if (value == 2 ) {
		show_layer_ext(F, "idspki", false);
		show_layer_ext(F, "idstls", false);
		show_layer_ext(F, "idsstatic", true);
	} else {
		show_layer_ext(F, "idspki", true);
		show_layer_ext(F, "idstls", true);
		show_layer_ext(F, "idsstatic", false);
	}
}

function changevpnpbr(F, value) {
	//alert(" F:" + F.name + "; value: " + value);
	if (value == 1 || value == 2) {
		show_layer_ext(F, "idpbr", true);
	} else {
		show_layer_ext(F, "idpbr", false);
	}
}

function userpass_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_userpass";
	//console.log("openvpn userpass: add_userpass");
	F.submit();
}

function userpass_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_userpass";
	F.userpass_del_value.value=I;
	F.submit();
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);

	show_layer_ext(document.setup.pptpd_radius, 'idradius', <% nvem("pptpd_radius", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_radius, 'idlocal', <% nvem("pptpd_radius", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_enable, 'idpptp', <% nvem("pptpd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_enable, 'idpptpcred', <% nvem("pptpd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pptpd_client_enable, 'idpptpcli', <% nvem("pptpd_client_enable", "1", "1", "0"); %> == 1);
<% ifndef("ANTAIRA_AGENT", "/*"); %>
	show_layer_ext(document.setup.antaira_agent_enable, 'idantairavpn', <% nvem("antaira_agent_enable", "1", "1", "0"); %> == 1);
<% ifndef("ANTAIRA_AGENT", "*/"); %>
	show_layer_ext(document.setup.openvpn_enable, 'idvpn', <% nvem("openvpn_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_enable, 'idvpn_ipv6', <% nvem("ipv6_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_tuntap, 'idrouter', <% nvem("openvpn_tuntap", "tun", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_tuntap, 'idrouter2', <% nvem("openvpn_tuntap", "tun", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_tuntap, 'idbridge', <% nvem("openvpn_tuntap", "tap", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_proxy, 'idproxy', <% nvem("openvpn_proxy", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_switch, 'idnew', <% nvem("openvpn_switch", "1", "1", "0"); %> == 1);
	//show_layer_ext(document.setup.openvpn_switch, 'idold', <% nvem("openvpn_switch", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_enable, 'idstls', <% nvem("openvpn_tls_btn", "3", "0", "1"); %> == 1);
	show_layer_ext(document.setup.openvpn_enable, 'idsstatic', <% nvem("openvpn_tls_btn", "2", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_enable, 'idspki', <% nvem("openvpn_tls_btn", "2", "0", "1"); %> == 1);
	show_layer_ext(document.setup.openvpn_pkcs, 'idspkcs12y', <% nvem("openvpn_pkcs", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_pkcs, 'idspkcs12n', <% nvem("openvpn_pkcs", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_adv, 'idmtu', <% nvem("openvpn_adv", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_enable, 'idvpncl', <% nvem("openvpncl_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_enuserpass, 'iduserpass', <% nvem("openvpn_enuserpass", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_adv, 'idmtucl', <% nvem("openvpncl_adv", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_upauth, 'idupauth', <% nvem("openvpncl_upauth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_nat, 'idnat', <% nvem("openvpncl_tuntap", "tap", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_tuntap, 'idsec', <% nvem("openvpncl_tuntap", "tun", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_wdog, 'idwdog', <% nvem("openvpncl_wdog", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpn_dh_btn, 'iddhpem', <% nvem("openvpn_dh_btn", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_multirem, 'idmultirem', <% nvem("openvpncl_multirem", "1", "1", "0"); %> == 1);
	// if nvram match with arg 1 then arg 2 otherwise arg 3
	show_layer_ext(document.setup.openvpncl_spbr, 'idpbr', <% nvem("openvpncl_spbr", "0", "0", "1"); %> == 1);
	show_layer_ext(document.setup.openvpncl_enable, 'idtls', <% nvem("openvpncl_tls_btn", "3", "0", "1"); %> == 1);
	show_layer_ext(document.setup.openvpncl_enable, 'idpki', <% nvem("openvpncl_tls_btn", "2", "0", "1"); %> == 1);
	show_layer_ext(document.setup.openvpncl_pkcs, 'idpkcs12y', <% nvem("openvpncl_pkcs", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_pkcs, 'idpkcs12n', <% nvem("openvpncl_pkcs", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.openvpncl_enable, 'idstatic', <% nvem("openvpncl_tls_btn", "2", "1", "0"); %> == 1);
	//show_layer_ext(document.setup.openvpncl_enable, 'idstatkey', <% nvem("openvpncl_keytype", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.setherserver_enable, 'idsether', <% nvem("setherserver_enable", "1", "1", "0"); %> == 1);

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
					<% do_menu("Services.asp","PPTP.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="post">
							<input type="hidden" name="submit_button" value="PPTP" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="openvpncl_certtype" />
							<input type="hidden" name="openvpncl_randomsrv" />
							<input type="hidden" name="openvpncl_fw" />
							<input type="hidden" name="openvpncl_killswitch" />
							<input type="hidden" name="openvpncl_splitdns" />
							<input type="hidden" name="openvpn_fw" />
							<input type="hidden" name="vpn_conf_file" />
							<input type="hidden" name="userpass_del_value" />
							<input type="hidden" name="openvpn_userpass" value="13" />

							<h2><% tran("service.pptp_h2"); %></h2>
							<fieldset>
								<legend><% tran("service.pptp_srv"); %></legend>
								<div class="setting">
									<div class="label"><% tran("service.pptp_option_srv"); %></div>
									<input class="spaceradio" type="radio" name="pptpd_enable" value="1" <% nvc("pptpd_enable", "1"); %> onclick="show_layer_ext(this, 'idpptp', true);show_layer_ext(this, 'idpptpcred', true);" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="pptpd_enable" value="0" <% nvc("pptpd_enable", "0"); %> onclick="show_layer_ext(this, 'idpptp', false);show_layer_ext(this, 'idpptpcred', false);" /><% tran("share.disable"); %>
								</div>
								<div id="idpptp">
									<div class="setting">
										<div class="label"><% tran("share.broadcast"); %></div>
										<input class="spaceradio" type="radio" name="pptpd_bcrelay" value="1" <% nvc("pptpd_bcrelay", "1"); %> /><% tran("share.enable"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pptpd_bcrelay" value="0" <% nvc("pptpd_bcrelay", "0"); %> /><% tran("share.disable"); %>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_encry"); %></div>
										<input class="spaceradio" type="radio" name="pptpd_forcemppe" value="1" <% nvc("pptpd_forcemppe", "1"); %> /><% tran("share.enable"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pptpd_forcemppe" value="0" <% nvc("pptpd_forcemppe", "0"); %> /><% tran("share.disable"); %>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.dns1"); %></div>
										<input size="25" name="pptpd_dns1" value="<% nvg("pptpd_dns1"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.dns2"); %></div>
										<input size="25" name="pptpd_dns2" value="<% nvg("pptpd_dns2"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.wins1"); %></div>
										<input size="25" name="pptpd_wins1" value="<% nvg("pptpd_wins1"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.wins2"); %></div>
										<input size="25" name="pptpd_wins2" value="<% nvg("pptpd_wins2"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mtu"); %></div>
										<input size="5" maxlength="5" class="num" name="pptpd_mtu" onblur="valid_range(this,1,65535,share.port)" value="<% nvg("pptpd_mtu"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1436)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mru"); %></div>
										<input size="5" maxlength="5" class="num" name="pptpd_mru" onblur="valid_range(this,1,65535,share.port)" value="<% nvg("pptpd_mru"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1436)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("share.srvip"); %></div>
										<input size="25" name="pptpd_lip" value="<% nvg("pptpd_lip"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptp_client"); %></div>
										<input size="25" name="pptpd_rip" value="<% nvg("pptpd_rip"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("wl_adv.label10"); %></div>
										<input size="5" maxlength="5" class="num" name="pptpd_conn" onblur="valid_range(this,1,65535,share.port)" value="<% nvg("pptpd_conn"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 64)");
										//]]>
										</script></span>
									</div>
									<% ifndef("RADIUSPLUGIN", "<!--"); %>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_auth"); %></div>
										<input class="spaceradio" type="radio" name="pptpd_radius" value="1" <% nvc("pptpd_radius", "1"); %> onclick="toggle_layer_ext(this, 'idradius', 'idlocal', true)" /><% tran("radius.legend"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pptpd_radius" value="0" <% nvc("pptpd_radius", "0"); %> onclick="toggle_layer_ext(this, 'idradius', 'idlocal', false)" /><% tran("service.pppoesrv_chaps"); %>
									</div>
									<% ifndef("RADIUSPLUGIN", "-->"); %>	
								</div>
							</fieldset>
							<div id="idpptpcred">
							<br />
							<% ifndef("RADIUSPLUGIN", "<!--"); %>
							<div id="idlocal">
							<% ifndef("RADIUSPLUGIN", "-->"); %>
								<fieldset>
									<legend><% tran("service.pptp_chap"); %></legend>
									<div class="setting">
									<textarea id="pptpd_auth" name="pptpd_auth" cols="60" rows="4"></textarea>
										<script type="text/javascript">
										//<![CDATA[
										var var_pptpd_auth = fix_cr( '<% nvg("pptpd_auth"); %>' );
										document.getElementById("pptpd_auth").value = var_pptpd_auth;
										//]]>
										</script>
									</div>
								</fieldset>
							<% ifndef("RADIUSPLUGIN", "<!--"); %>
							</div>
							<% ifndef("RADIUSPLUGIN", "-->"); %>
							<div id="idradius">
								<fieldset>
									<% ifndef("RADIUSPLUGIN", "<!--"); %>
									<legend><% tran("service.pppoesrv_radauth"); %></legend>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_radip"); %></div>
										<input maxlength="15" size="24" name="pptpd_radserver" onblur="valid_ip_str(this, share.ip)" value="<% nvg("pptpd_radserver"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_radauthport"); %></div>
										<input size="5" maxlength="5" class="num" name="pptpd_radport" value="<% nvg("pptpd_radport"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1812)");
										//]]>
										</script></span>
									</div>			
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_radaccport"); %></div>
										<input size="5" maxlength="5" class="num" name="pptpd_acctport" value="<% nvg("pptpd_acctport"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1813)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_radkey"); %></div>
										<input size="20" maxlength="63" type="password" autocomplete="new-password" name="pptpd_radpass" value="d6nw5v1x2pc7st9m" />
									</div>
									<% ifndef("RADIUSPLUGIN", "-->"); %>
								</fieldset>
							</div>
							</div><br/>
							<fieldset>
								<legend><% tran("service.pptpd_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("service.pptpd_lblcli"); %></div>
									<input class="spaceradio" type="radio" name="pptpd_client_enable" value="1" <% nvc("pptpd_client_enable", "1"); %> onclick="show_layer_ext(this, 'idpptpcli', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="pptpd_client_enable" value="0" <% nvc("pptpd_client_enable", "0"); %> onclick="show_layer_ext(this, 'idpptpcli', false)" /><% tran("share.disable"); %>
								</div>
								<div id="idpptpcli">
									<div class="setting">
										<div class="label"><% tran("service.pptpd_ipdns"); %></div>
										<input size="27" name="pptpd_client_srvip" value="<% nvg("pptpd_client_srvip"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_subnet"); %></div>
										<input type="hidden" name="pptpd_client_srvsub" value="0.0.0.0" />
										<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_0" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","0"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_1" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","1"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_2" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","2"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_3" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","3"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_subnetmask"); %></div>
										<input type="hidden" name="pptpd_client_srvsubmsk" value="0.0.0.0" />
										<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_0" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","0"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_1" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","1"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_2" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","2"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_3" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","3"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_encry"); %></div>
										<input size="27" name="pptpd_client_srvsec" value="<% nvg("pptpd_client_srvsec"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mtu"); %></div>
										<input class="num" maxlength="4" size="5" name="pptpd_client_srvmtu" onblur="valid_range(this,0,1500,service.pptpd_mtu)" value="<% nvg("pptpd_client_srvmtu"); %>" />&nbsp;
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1436)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mru"); %></div>
										<input class="num" maxlength="4" size="5" name="pptpd_client_srvmru" onblur="valid_range(this,0,1500,service.pptpd_mru)" value="<% nvg("pptpd_client_srvmru"); %>" />&nbsp;
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1436)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_nat"); %></div>
										<input class="spaceradio" type="radio" name="pptpd_client_nat" value="1" <% nvc("pptpd_client_nat", "1"); %> /><% tran("share.enable"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pptpd_client_nat" value="0" <% nvc("pptpd_client_nat", "0"); %> /><% tran("share.disable"); %>
									</div>
									<div class="setting">
										<div class="label"><% tran("share.usrname"); %></div>
										<input size="27" name="pptpd_client_srvuser" value="<% nvg("pptpd_client_srvuser"); %>" />
									</div>		
									<div class="setting">
										<div class="label"><% tran("share.passwd"); %></div>
										<input size="27" id="pptpd_client_srvpass" name="pptpd_client_srvpass" type="password" autocomplete="new-password" value="<% nvg("pptpd_client_srvpass"); %>" />&nbsp;
										<input type="checkbox" name="_wl_unmask" value="0" onclick="setElementMask('pptpd_client_srvpass', this.checked)" />&nbsp;<% tran("share.unmask"); %>
									</div>
									<div class="setting">		
										<div class="label"><% tran("idx_pptp.addopt"); %></div>
										<textarea cols="60" rows="2" id="pptpd_client_options" name="pptpd_client_options"></textarea>
										<script type="text/javascript">
										//<![CDATA[
											var pptpd_client_options = fix_cr( '<% nvg("pptpd_client_options"); %>' );
											document.getElementById("pptpd_client_options").value = pptpd_client_options;
										//]]>
										</script>
									</div>
								</div>
							</fieldset><br />
							<% show_modules(".webvpn"); %>
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
							<dt class="term"><% tran("bmenu.statuVPN"); %></dt>
							<dd class="definition"><% tran("hstatus_vpn.right3"); %></dd>
							<dt class="term"><% tran("service.vpnd_hlegend1"); %></dt>
							<dd class="definition"><% tran("hstatus_vpn.right2"); %></dd>
							<dt class="term"><% tran("service.vpn_legend"); %></dt>
							<dd class="definition"><% tran("hstatus_vpn.right1"); %></dd>
							<dd class="definition"><% tran("hstatus_vpn.cfg_xp"); %></dd>
							<dt class="term"><% tran("eoip.wireguard_lanac"); %></dt>
							<dd class="definition"><% tran("hstatus_vpn.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HPPTP.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"<% get_firmware_version_href(); %>\"><% get_firmware_version(); %></a>");
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
