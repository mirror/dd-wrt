<% do_pagehead("idx.titl"); %>
	<script type="text/javascript">
	//<![CDATA[
// WAN related JS
var wan_proto = "<% nvram_selget("wan_proto"); %>";
var dhcp_win = null;

function pptpUseDHCP(F, val) {
	setElementsActive("wan_ipaddr_0", "wan_gateway_3", val==0)
}

function valid_mtu(I) {
	var start = null;
	var end = null;
	if(wan_proto == "pppoe") {
		start = 576;
		end = 1492;
	} else if (wan_proto == "pptp" || wan_proto == "l2tp") {
		start = 1200;
		end = 1492;
	} else {
		start = 576;
		end = 16320;
	}

	valid_range(I,start,end,"MTU");
}

function SelMTU(num,F) {
	mtu_enable_disable(F,num);
}

function mtu_enable_disable(F,I) {
	if ( I == "0" )
		choose_disable(F.wan_mtu);
	else
		choose_enable(F.wan_mtu);
}

function valid_value(F) {
	if (!('<% nvram_selget("wl0_mode"); %>' == 'wet') && !('<% nvram_selget("wl0_mode"); %>' == 'apstawet')) {
		if (F.now_proto.value == "pptp" || F.now_proto.value == "static") {
			pptp_dhcp = "";
	
			// Sveasoft: allow 0.0.0.0 for pptp. We'll let DHCP take care of it.
			if (F.now_proto.value == "pptp" &&
					F.wan_ipaddr_0.value == "0" &&
					F.wan_ipaddr_1.value == "0" &&
					F.wan_ipaddr_2.value == "0" &&
					F.wan_ipaddr_3.value == "0")
						pptp_dhcp = "skip";

			if (!F.pptp_use_dhcp || F.pptp_use_dhcp.value == "0") {
				if(pptp_dhcp != "skip" && !valid_ip(F,"F.wan_ipaddr","IP",ZERO_NO|MASK_NO))
					return false;
			}

			if(pptp_dhcp != "skip" && F.now_proto.value == "pptp") {
				if (F.pptp_use_dhcp.value == "0") {
					if(!valid_ip(F,"F.pptp_server_ip","Gateway",ZERO_NO|MASK_NO))
						return false;

					if(!valid_ip_gw(F,"F.wan_ipaddr","F.wan_netmask","F.pptp_server_ip"))
						return false;
				}
			}
		}
	}
	
	if(F.now_proto.value == "pptp" || F.now_proto.value == "l2tp") {
		if(F.ppp_username.value == "") {
			alert(errmsg.err0);
			F.ppp_username.focus();
			return false;
		}
	}
	
	if(!<% nvram_else_selmatch("dhcpfwd_enable", "1", "1", "0"); %>) {
		if(!valid_dhcp_server(F))
			return false;
	}

	if(F.router_name)
		if(F.router_name.value == "") {
			alert(errmsg.err1);
			F.router_name.focus();
			return false;
		}
	if(document.setupassitant)
		if(document.setupassistant.now_proto)
			if(document.setupassistant.now_proto.value == "pptp")
				pptpUseDHCP(document.setupassistant, '<% nvram_selget("pptp_use_dhcp"); %>');

	return true;
}

function valid_dhcp_server(F) {
	if(F.lan_proto == null)
		return true;
	if (F.lan_proto.selectedIndex == 0)
		return true;

	a1 = parseInt(F.dhcp_start.value,10);
	a2 = parseInt(F.dhcp_num.value,10);
	if (a1 + a2 > 999) {
		alert(errmsg.err2);
		return false;
	}

	if (F.wan_dns0 != null)
		if (!valid_ip(F,"F.wan_dns0","DNS",MASK_NO))
			return false;

	if (F.wan_dns1 != null)
		if (!valid_ip(F,"F.wan_dns1","DNS",MASK_NO))
			return false;

	if (F.wan_dns2 != null)
		if (!valid_ip(F,"F.wan_dns2","DNS",MASK_NO))
			return false;

	if (F.wan_wins != null)
		if (!valid_ip(F,"F.wan_wins","WINS",MASK_NO))
			return false;

	return true;
}

function SelDHCP(T,F) {
	dhcp_enable_disable(F,T);
}

function dhcp_enable_disable(F,T) {
	var start = '';
	var end = '';
	var total = F.elements.length;

	for(var i=0 ; i < total ; i++) {
		if(F.elements[i].name == "dhcp_start")
			start = i;
		if(F.elements[i].name == "dhcp_num")
			end = i;
	}
	
	if(start == '' || end == '')
		return true;

	if( T == "static" ) {
		for(i = start; i<=end ;i++) {
			choose_disable(F.elements[i]);
		}
	} else {
		for(i = start; i<=end ;i++) {
			choose_enable(F.elements[i]);
		}
	}
	return true;
}

function ppp_enable_disable(F,I) {
	if( I == "0") {
		choose_disable(F.ppp_idletime);
		choose_enable(F.ppp_redialperiod);
	} else {
		choose_enable(F.ppp_idletime);
		choose_disable(F.ppp_redialperiod);
	}
}

function SelWAN(num,F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "wan_proto";
	F.wan_proto.value=F.wan_proto.options[num].value;
	F.submit();
}

function SelDHCPFWD(num,F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "dhcpfwd";
	F.dhcpfwd_enable.value = F.dhcpfwd_enable.options[num].value;
	F.submit();
}

/** Wireless **/
var wl0_channel = '<% nvram_selget("wl0_channel"); %>';
var wl0_nctrlsb = '<% nvram_selget("wl0_nctrlsb"); %>';
var wl0_nbw = '<% nvram_selget("wl0_nbw"); %>';
var wl0_phytype = '<% nvram_selget("wl0_phytype"); %>';
var wl0_40m_disable = '<% nvram_selget("wl0_40m_disable"); %>';
var wl1_channel = '<% nvram_selget("wl1_channel"); %>';
var wl1_nctrlsb = '<% nvram_selget("wl1_nctrlsb"); %>';
var wl1_nbw = '<% nvram_selget("wl1_nbw"); %>';
var wl1_phytype = '<% nvram_selget("wl1_phytype"); %>';
var wl1_40m_disable = '<% nvram_selget("wl1_40m_disable"); %>';

function create_wchannel0_auto(F) {
	F.wl0_wchannel.length = 1;

	F.wl0_wchannel[0] = new Option(share.auto);
	F.wl0_wchannel[0].value = "0";
}

function create_wchannel1_auto(F) {
	F.wl1_wchannel.length = 1;

	F.wl1_wchannel[0] = new Option(share.auto);
	F.wl1_wchannel[0].value = "0";
}

function create_wchannel0(F) {
	var max_channel = '14';
	var wch;

	if(wl0_nctrlsb == "lower") {
		wch = parseInt(F.wl0_channel.value)+2;
	}
	else {
		wch = parseInt(F.wl0_channel.value)-2;
	}

	F.wl0_wchannel.length = parseInt(max_channel)-4;

	for(ch=3 ; ch<=(parseInt(max_channel)-2) ; ch++) {
		F.wl0_wchannel[ch-3] = new Option(ch);
		F.wl0_wchannel[ch-3].value = ch;
	}
	if(wch < 3 || wch > max_channel-2 || wch == "0")
		F.wl0_wchannel[0].selected = true;
	else
		F.wl0_wchannel[wch-3].selected = true;
}

function create_wchannel1(F) {
	var max_channel = '14';
	var wch;

	if(wl1_nctrlsb == "lower") {
		wch = parseInt(F.wl1_channel.value)+2;
	}
	else {
		wch = parseInt(F.wl1_channel.value)-2;
	}

	F.wl1_wchannel.length = parseInt(max_channel)-4;

	for(ch=3 ; ch<=(parseInt(max_channel)-2) ; ch++){
		F.wl1_wchannel[ch-3] = new Option(ch);
		F.wl1_wchannel[ch-3].value = ch;
	}
	if(wch < 3 || wch > max_channel-2 || wch == "0")
		F.wl1_wchannel[0].selected = true;
	else
		F.wl1_wchannel[wch-3].selected = true;
}

function InitBW0(num,F) {
	if(wl0_channel == "0") {
		if(F.wl0_wchannel) choose_enable(F.wl0_wchannel);
			choose_enable(F.wl0_schannel);

		if(F.wl0_wchannel) create_wchannel0_auto(F)
	}
	else
		SelBW0(num,F);
}

function InitBW1(num,F) {
	if(wl1_channel == "0") {
		if(F.wl1_wchannel) choose_enable(F.wl1_wchannel);
			choose_enable(F.wl1_schannel);

		if(F.wl1_wchannel) create_wchannel1_auto(F)
	}
	else
		SelBW1(num,F);
}

function SelBW0(num,F) {
	if (num == 0) { // Auto
		if(F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);

		choose_enable(F.wl0_channel);
		if(F.wl0_wchannel)
			create_wchannel0_auto(F);
	}
	else if (num == 10 || num == 20) {
		if(F.wl0_wchannel)
			choose_disable(F.wl0_wchannel);

		choose_enable(F.wl0_schannel);
		if(F.wl0_wchannel)
			create_wchannel0(F);
	}
	else {
		if(F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);

		choose_enable(F.wl0_schannel);
		if(F.wl0_wchannel)
			create_wchannel0(F);
	}
}

function SelBW1(num,F) {
	if (num == 0) { // Auto
		if(F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);

		choose_enable(F.wl1_channel);
		if(F.wl1_wchannel)
			create_wchannel1_auto(F);
	}
	else if (num == 10 || num == 20) {
		if(F.wl1_wchannel)
			choose_disable(F.wl1_wchannel);

		choose_enable(F.wl1_schannel);
		if(F.wl1_wchannel)
			create_wchannel1(F);
	}
	else {
		if(F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);

		choose_enable(F.wl1_schannel);
		if(F.wl1_wchannel)
			create_wchannel1(F);
	}
}

/**
 * Wireless Security
 */
function SelMode(varname,num,F) {
	F.change_action.value="gozila_cgi";
	//F.submit_type.value = "security";
	F.security_varname.value = varname;
	F.submit();
}

function keyMode(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "keysize";
	F.submit();
}

function generateKey(F,PREFIX) {
	F.change_action.value="gozila_cgi";
	F.security_varname.value = PREFIX;
	F.submit_type.value = "wep_key_generate";
	F.submit();
}

function enable_idttls(ifname) {
	show_layer_ext(this, 'idttls' + ifname, true)
	show_layer_ext(this, 'idtls'  + ifname, false)
	show_layer_ext(this, 'idpeap' + ifname, false)
	show_layer_ext(this, 'idleap' + ifname, false)
}

function enable_idpeap(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false)
	show_layer_ext(this, 'idtls' + ifname, false)
	show_layer_ext(this, 'idpeap' + ifname, true)
	show_layer_ext(this, 'idleap' + ifname, false)
}

function enable_idleap(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false)
	show_layer_ext(this, 'idtls' + ifname, false)
	show_layer_ext(this, 'idpeap' + ifname, false)
	show_layer_ext(this, 'idleap' + ifname, true)
}

function enable_idtls(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false)
	show_layer_ext(this, 'idtls' + ifname, true)
	show_layer_ext(this, 'idpeap' + ifname, false)
	show_layer_ext(this, 'idleap' + ifname, false)
}

/**
 * check values before submitting
 */
function submitcheck(F) {
	if( !checkformelements( F )) {
		return false;
	}
	switch(F.sas_stage.value) {
		case '1':
			// WAN connection
			if(valid_value(F)) {
				if(F._dns_dnsmasq) {
					F.dns_dnsmasq.value = F._dns_dnsmasq.checked ? 1 : 0;
				}
				if(F._auth_dnsmasq) {
					F.auth_dnsmasq.value = F._auth_dnsmasq.checked ? 1 : 0;
				}
				if(F._fullswitch) {
					F.fullswitch.value = F._fullswitch.checked ? 1 : 0;
				}
				if(F._ppp_mlppp) {
					F.ppp_mlppp.value = F._ppp_mlppp.checked ? 1 : 0;
				}
				if(F._ignore_wan_dns) {
					F.ignore_wan_dns.value = F._ignore_wan_dns.checked ? 1 : 0;
				}
			} else {
				return false;
			}
			break;
		case '3':
			// Wireless
			if(F.wlan0_ssid)
				if(F.wlan0_ssid.value == "") {
					alert(errmsg.err50);
					F.wlan0_ssid.focus();
					return false;
				}
				if(F.wl1_ssid)
					if(F.wl1_ssid.value == "") {
						alert(errmsg.err50);
						F.wl1_ssid.focus();
						return false;
					}
					if(F.wl0_nbw) {
						if(F.wl0_nbw.value == 0) { // Auto
							F.wl0_channel.value = 0;
						}
						else if(F.wl0_nbw.value == 10) { // 10MHz
							F.wl0_nctrlsb.value = "none";
							F.wl0_nbw.value = 10;
						}
						else if(F.wl0_nbw.value == 20) { // 20MHz
							F.wl0_nctrlsb.value = "none";
							F.wl0_nbw.value = 20;
						}
						else { // 40MHz
							if(F.wl0_channel.selectedIndex == 0) {
								F.wl0_nctrlsb.value = "lower";
							}
							else {
								F.wl0_nctrlsb.value = "upper";
							}
							F.wl0_nbw.value = 40;
						}
					}
					if(F.wl1_nbw) {
						if(F.wl1_nbw.value == 0) { // Auto
							F.wl1_channel.value = 0;
						}
						else if(F.wl1_nbw.value == 10) { // 10MHz
							F.wl1_nctrlsb.value = "none";
							F.wl1_nbw.value = 10;
						}
						else if(F.wl1_nbw.value == 20) { // 20MHz
							F.wl1_nctrlsb.value = "none";
							F.wl1_nbw.value = 20;
						}
						else { // 40MHz
							if(F.wl1_channel.selectedIndex == 0) {
								F.wl1_nctrlsb.value = "lower";
							}
							else {
								F.wl1_nctrlsb.value = "upper";
							}
							F.wl1_nbw.value = 40;
						}
					}
			break;
		case "4":
			break;
		default:
			break;
	}
	return true;
	//F.submit_type.value = "save";
	//F.change_action.value = "";
	//F.save_button.value = sbutton.saving;
//	}
}

function toggleAOSS(button, show) {
	show_layer_ext(button, 'aoss_options', show);
}

/**
 * refreshes stage view
 */
function refresh(F) {
	F.change_action.value="gozila_cgi";
	//F.submit_type.value = "save";
	F.submit();
}

/**
 * Moves to next setup assistant step
 */
function to_next(F) {
	if(submitcheck(F)) {
		F.sas_stage.value = parseInt(F.sas_stage.value) + 1;
		F.change_action.value="gozila_cgi";
		//F.submit_type.value = "save";
		F.submit();
	}
}

/**
 * Moves to previous setup assistant step
 */
function to_prev(F) {
	if(submitcheck(F)) {
		F.sas_stage.value = parseInt(F.sas_stage.value) - 1;
		F.change_action.value="gozila_cgi";
		//F.submit_type.value = "save";
		F.submit();
	}
}

/**
 * Applies the changes
 */
function to_apply(F) {
	if(submitcheck(F)) {
		F.submit_type.value = "save";
		F.apply_button.value = sbutton.applied;
		F.next_page.disabled = false;
		F.next_page.value = "index.asp";
		applytake(F);
	}
}

var update;

/**
 * on load initialization
 */
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	// WAN
	//mtu_enable_disable(document.setupassistant,'<% nvram_selget("mtu_enable"); %>');
	if (document.setupassistant.now_proto.value == "pptp")
		pptpUseDHCP(document.setupassistant, '<% nvram_selget("pptp_use_dhcp"); %>');
			
	if (document.setupassistant.now_proto.value == "pppoe" ||
		document.setupassistant.now_proto.value == "pptp" ||
		document.setupassistant.now_proto.value == "l2tp" ||
		document.setupassistant.now_proto.value == "heartbeat") 
			ppp_enable_disable(document.setupassistant,'<% nvram_selget("ppp_demand"); %>');
	
	dhcp_enable_disable(document.setupassistant,'<% nvram_selget("lan_proto"); %>');

	show_layer_ext(document.setupassistant.ntp_enable, 'idntp', <% nvem("ntp_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setupassistant.reconnect_enable, 'idreconnect', <% nvem("reconnect_enable", "1", "1", "0"); %> == 1);

	// Wireless
	var wl0_mode = "<% nvram_selget("wl0_mode"); %>";
	if (wl0_mode=="ap" || wl0_mode=="infra") {
		if (wl0_phytype == 'n')
			InitBW0('<% nvram_selget("wl0_nbw"); %>' ,document.wireless);
	}
	var wl1_mode = "<% nvram_selget("wl1_mode"); %>";
	if (wl1_mode=="ap" || wl1_mode=="infra") {
		if (wl1_phytype == 'n')
			InitBW1('<% nvram_selget("wl1_nbw"); %>' ,document.wireless);
	}
	// Wireless Security
	<% sas_init_80211x_layers(); %>

	var F = document.forms[0];
	if(F.security_mode && F.wl_wep_bit)
		if(F.security_mode.value == "wep" || F.security_mode.value == "radius") {
			keyMode(F.wl_wep_bit.value, F);
		}

	update = new StatusbarUpdate();
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

function submitSaveNextButtons() {
	if( <% print_sas_stage(); %> < 4 ) {
		document.write("<input title=\"" + sbutton.nexttitle + "\" class=\"button\" style=\"float: right;\" type=\"button\" name=\"next_button\" value=\"" + sbutton.next + "\" onclick=\"to_next(this.form);\" />");
	} else {
		document.write("<input title=\"" + sbutton.savetitle + "\" class=\"button\" style=\"float: right;\" type=\"button\" name=\"save_button\" value=\"" + sbutton.apply + "\" onclick=\"to_apply(this.form);\" />");
	}
}

function submitSavePrevButtons() {
	if( <% print_sas_stage(); %> > 1 ) {
		document.write("<input title=\"" + sbutton.prevtitle + "\" class=\"button\" style=\"float: left;\" type=\"button\" name=\"prev_button\" value=\"" + sbutton.prev + "\" onclick=\"to_prev(this.form);\" />");
	} else {
		
	}
}
		//]]>
		</script>
	</head>

	<body class="gui">
	
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp",""); %>
				</div>
				<div id="main">
					<div id="contents">
						<!--p>Assistant stage: <% print_sas_stage(); %></p-->

						<% do_sas_stage_menu(); %>
						<form name="setupassistant" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="SetupAssistant" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="next_page" disabled />

							<input type="hidden" name="sas_stage" value="<% print_sas_stage(); %>">

							<!-- WAN Setup related -->
							<input type="hidden" name="now_proto" value="<% nvram_gozila_get("wan_proto"); %>" />
							<input type="hidden" name="lan_ipaddr" value="<% nvram_selget("lan_ipaddr"); %>" />

							<!-- Wireless related -->
							<input type="hidden" name="wl0_nctrlsb" />
							<input type="hidden" name="wl1_nctrlsb" />
							<input type="hidden" name="iface" />

							<input type="hidden" name="security_varname" />
							<input type="hidden" name="security_mode_last" />
							<input type="hidden" name="wl_wep_last" />
							<input type="hidden" name="filter_mac_value" />

							<h2 style="<% sas_stage_visible_css("1"); %>"><script type="text/javascript">Capture(sas.hwan);</script></h2>

							<!-- Internet Setup -->
							<fieldset style="<% sas_stage_visible_css("1"); %>">
								<legend><% tran("idx.legend"); %></legend>
								<% ifndef("WET", "<!--"); %>
								<div class="setting">
									<div class="label"><% tran("idx.conn_type"); %></div>
									<% tran("share.disabled"); %>
								</div>
								<% ifndef("WET", "-->"); %>

								<% ifdef("WET", "<!--"); %>
								<div class="setting">
										<div class="label"><% tran("idx.conn_type"); %></div>
										<select name="wan_proto" onchange="SelWAN(this.form.wan_proto.selectedIndex,this.form)">
									<% show_connectiontype(); %>
									</select>
								</div>
								<% show_sas_wan_setting(); %>
								<% ifdef("WET", "-->"); %>
								<!--div class="setting">
									<div class="label"><% tran("idx.stp"); %></div>
									<input class="spaceradio" type="radio" value="1" name="lan_stp" <% nvc("lan_stp", "1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="lan_stp" <% nvc("lan_stp", "0"); %> /><% tran("share.disable"); %>
								</div-->
							</fieldset><br style="<% sas_stage_visible_css("1"); %>" />

							<!-- Network Setup -->
							<h2 style="<% sas_stage_visible_css("2"); %>"><% tran("idx.h23"); %></h2>
							<fieldset style="<% sas_stage_visible_css("2"); %>">
								<legend><% tran("idx.routerip"); %></legend>
								<div class="setting">
									<div class="label"><% tran("idx.lanip"); %></div>
									<input class="num" maxlength="3" size="3" onblur="valid_range(this,1,223,'IP')" name="lan_ipaddr_0" value="<% sas_get_single_ip("lan_ipaddr","0"); %>"/>.<input class="num" maxlength="3" size="3" onblur="valid_range(this,0,255,'IP')" name="lan_ipaddr_1" value="<% sas_get_single_ip("lan_ipaddr","1"); %>"/>.<input class="num" maxlength="3" size="3" onblur="valid_range(this,0,255,'IP')" name="lan_ipaddr_2" value="<% sas_get_single_ip("lan_ipaddr","2"); %>"/>.<input class="num" maxlength="3" size="3" onblur="valid_range(this,1,254,'IP')" name="lan_ipaddr_3" value="<% sas_get_single_ip("lan_ipaddr","3"); %>"/>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.subnet"); %></div>
									<input type="hidden" name="lan_netmask" value="4" />
									<input class="num" maxlength="3" size="3" name="lan_netmask_0" onblur="valid_range(this,0,255,'Netmask')" value="<% sas_get_single_nm("lan_netmask","0"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_netmask_1" onblur="valid_range(this,0,255,'Netmask')" value="<% sas_get_single_nm("lan_netmask","1"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_netmask_2" onblur="valid_range(this,0,255,'Netmask')" value="<% sas_get_single_nm("lan_netmask","2"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_netmask_3" onblur="valid_range(this,0,255,'Netmask')" value="<% sas_get_single_nm("lan_netmask","3"); %>"/>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.gateway"); %></div>
									<input type="hidden" name="lan_gateway" value="4" />
									<input class="num" maxlength="3" size="3" name="lan_gateway_0" onblur="valid_range(this,0,255,share.gateway)" value="<% sas_get_single_ip("lan_gateway","0"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_gateway_1" onblur="valid_range(this,0,255,share.gateway)" value="<% sas_get_single_ip("lan_gateway","1"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_gateway_2" onblur="valid_range(this,0,255,share.gateway)" value="<% sas_get_single_ip("lan_gateway","2"); %>"/>.<input class="num" maxlength="3" size="3" name="lan_gateway_3" onblur="valid_range(this,0,255,share.gateway)" value="<% sas_get_single_ip("lan_gateway","3"); %>"/>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.localdns"); %></div>
									<input type="hidden" name="sv_localdns" value="4" />
									<input class="num" maxlength="3" size="3" name="sv_localdns_0" onblur="valid_range(this,0,255,share.localdns)" value="<% sas_get_single_ip("sv_localdns","0"); %>"/>.<input class="num" maxlength="3" size="3" name="sv_localdns_1" onblur="valid_range(this,0,255,share.localdns)" value="<% sas_get_single_ip("sv_localdns","1"); %>"/>.<input class="num" maxlength="3" size="3" name="sv_localdns_2" onblur="valid_range(this,0,255,share.localdns)" value="<% sas_get_single_ip("sv_localdns","2"); %>"/>.<input class="num" maxlength="3" size="3" name="sv_localdns_3" onblur="valid_range(this,0,254,share.localdns)" value="<% sas_get_single_ip("sv_localdns","3"); %>"/>
								</div>
							</fieldset><br style="<% sas_stage_visible_css("2"); %>" />

							<% sas_show_dhcpd_settings("2"); %>
							<!-- Wireless Setup -->
							<% sas_show_wireless("3"); %>
							<!--% sas_show_security("3"); %-->
								
							<!-- Other Settings -->
							<!-- AOSS start -->
							<h2 style="<% sas_stage_visible_css("4"); %>"><% tran("aoss.aoss"); %></h2>

							<% ifaoss_possible("yes", "<!--"); %>
							<fieldset style="<% sas_stage_visible_css("4"); %>">
								<legend><% tran("aoss.service"); %></legend>
								<div class="setting">
									<div class="label"><% tran("aoss.enable"); %></div>
									<input class="spaceradio" type="radio" value="1" name="aoss_enable" disabled /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="aoss_enable" checked disabled /><% tran("share.disable"); %>
								</div>
							</fieldset>
							<br style="<% sas_stage_visible_css("4"); %>"/>
							<div class="warning" style="<% sas_stage_visible_css("4"); %>">
								<p><% tran("aoss.ap_mode_notice"); %></p>
							</div>
							<br style="<% sas_stage_visible_css("4"); %>" />
							<% ifaoss_possible("yes", "-->"); %>
							<% ifaoss_possible("no", "<!--"); %>
							<fieldset style="<% sas_stage_visible_css("4"); %>">
								<legend><% tran("aoss.service"); %></legend>
								<div class="setting">
									<div class="label"><% tran("aoss.enable"); %></div>
									<input class="spaceradio" type="radio" value="1" name="aoss_enable" <% sas_nvc("aoss_enable", "1"); %> onClick="toggleAOSS(this, true);" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="aoss_enable" <% sas_nvc("aoss_enable", "0"); %> onClick="toggleAOSS(this, false);" /><% tran("share.disable"); %>
								</div>
							</fieldset>
							<br style="<% sas_stage_visible_css("4"); %>"/>
							<div id="aoss_options" style="<% visible_css("aoss_enable", "1"); %>" style="<% sas_stage_visible_css("4"); %>">
							<fieldset style="<% sas_stage_visible_css("4"); %>">
								<legend><% tran("aoss.securitymodes"); %></legend>
								<div class="setting">
									<div class="label"><% tran("aoss.wpaaes"); %></div>
									<input type="checkbox" name="aoss_aes" value="1"<% isChecked("aoss_aes", "1"); %>></input>
								</div>
								<div class="setting">
									<div class="label"><% tran("aoss.wpatkip"); %></div>
									<input type="checkbox" name="aoss_tkip" value="1"<% isChecked("aoss_tkip", "1"); %>></input>
								</div>
								<div class="setting">
									<div class="label"><% tran("aoss.wep"); %></div>
									<input type="checkbox" name="aoss_wep" value="1"<% isChecked("aoss_wep", "1"); %> style="float: left;"></input>
									<div style="float: left;"><% tran("aoss.wep_info"); %></div>
								</div>
							</fieldset>
							<br  style="<% sas_stage_visible_css("4"); %>"/>
							<div class="warning" style="<% sas_stage_visible_css("4"); %>">
								<p><% tran("aoss.notice"); %></p>
								<p><% tran("aoss.wep_notice"); %></p>
							</div>
							<br  style="<% sas_stage_visible_css("4"); %>"/>
							</div>
							<% ifaoss_possible("no", "-->"); %>
							<h2 style="<% sas_stage_visible_css("4"); %>"><% tran("idx.optional"); %></h2>
							<fieldset style="<% sas_stage_visible_css("4"); %>">
								<legend><% tran("idx.optional"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.routername"); %></div>
									<input maxlength="39" name="router_name" size="20" onblur="valid_name(this,&#34;Router%20Name&#34;)" value="<% nvram_selget("router_name"); %>"/>
								</div>
								<% ifdef("WET", "<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input maxlength="39" name="wan_hostname" size="20" onblur="valid_name(this,&#34;Host%20Name&#34;)" value="<% nvram_selget("wan_hostname"); %>"/>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.domainname"); %></div>
									<input maxlength="79" name="wan_domain" size="20" onblur="valid_name(this,&#34;Domain%20name&#34;,SPACE_NO)" value="<% nvram_selget("wan_domain"); %>" />
								</div>
								<!--table class="table" cellspacing="5" summary="ports forwarding table">
									<tr>
										<th><% tran("pforward.app"); %></th>
										<th><% tran("pforward.from"); %></th>
										<th><% tran("share.proto"); %></th>
										<th><% tran("share.ip"); %></th>
										<th><% tran("pforward.to"); %></th>
										<th><% tran("share.enable"); %></th>
									</tr>
									<% show_forward_spec(); %>
								</table>
								<br />
								<div class="center">
									<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"forward_add_submit(this.form);\" />");
										document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"forward_remove_submit(this.form);\" />");
										//]]>
									</script>
								</div-->
							</fieldset><br style="<% sas_stage_visible_css("4"); %>" />
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitSavePrevButtons();
								submitSaveNextButtons();
								//]]>
								</script>
								<div style="clear: both;"></div>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term" style="<% sas_stage_visible_css("1"); %>"><% tran("sas.hwan"); %>:</dt>
							<dd class="definition"  style="<% sas_stage_visible_css("1"); %>"><% tran("hsas.wan"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("2"); %>"><% tran("hsas.h_routerip"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("2"); %>"><% tran("hsas.routerip"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("2"); %>"><% tran("hsas.h_dhcp"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("2"); %>"><% tran("hsas.dhcp"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("3"); %>"><% tran("hsas.h_wireless_physical"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("3"); %>"><% tran("hsas.wireless_physical"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("3"); %>"><% tran("hsas.h_wireless_security"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("3"); %>"><% tran("hsas.wireless_security"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("4"); %>"><% tran("aoss.service"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("4"); %>"><% tran("haoss.basic"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("4"); %>"><% tran("aoss.securitymodes"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("4"); %>"><% tran("haoss.securitymodes"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("4"); %>"><% tran("hsas.h_routername"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("4"); %>"><% tran("hsas.routername"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("4"); %>"><% tran("share.hostname"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("4"); %>"><% tran("hidx.right4"); %></dd>
							<dt class="term" style="<% sas_stage_visible_css("4"); %>"><% tran("share.domainname"); %>:</dt>
							<dd class="definition" style="<% sas_stage_visible_css("4"); %>"><% tran("hidx.right6"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSetup.asp');"><% tran("share.more"); %></a>
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
