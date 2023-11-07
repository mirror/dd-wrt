<% do_pagehead("route.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

<% ifdef("QUAGGA", "/*"); %>
route.zebra_copt = route.bird_copt;
route.zebra_legend = route.bird_legend;
route.zebra_log = route.bird_log;
<% ifdef("QUAGGA", "*/"); %>

function valid_value(F) {
	if(F.wk_mode.value != "ospf") {
		if(!valid_ip(F,"F.route_ipaddr","IP",0))
			return false;
		if(!valid_ip(F,"F.route_gateway","Gateway",MASK_NO))
			return false;
	}
	return true;
}

function DeleteEntry(F) {
	if(!confirm(errmsg.err57)) return;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del";
	apply(F);
}

function DeleteRuleEntry(F) {
	if(!confirm(errmsg.err57)) return;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_rule";
	apply(F);
}

function SelRoute(num,F) {
	F.change_action.value="gozila_cgi";
	F.route_page.value = F.route_page.options[num].value;
	F.submit();
}

function SelRule(num,F) {
	F.change_action.value="gozila_cgi";
	F.rule_page.value = F.rule_page.options[num].value;
	F.submit();
}

function SelMode(num,F) {
	F.change_action.value="gozila_cgi";
	F.wk_mode.value = F.wk_mode.options[num].value;
	F.submit();
}

function olsrd_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_olsrd";
	F.submit();
}

function olsrd_del_submit(F,num) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_olsrd";
	F.olsrd_delcount.value = num;
	F.submit();
}

function olsrd_checkDijkstra(F) {
	if (F.olsrd_lqdijkstramin && F.olsrd_lqdijkstramax) {
		if (F.olsrd_lqdijkstramin.value>F.olsrd_lqdijkstramax.value) {
			alert(route.olsrd_lqdmin + errmsg.err103 + route.olsrd_lqdmax +".");
		}
	}
}

function submitcheck(F) {
	if (F.routing_bgp_neighbor_ip != null) {
		F.routing_bgp_neighbor_ip.value = F.routing_bgp_neighbor_ip_0.value+'.'+F.routing_bgp_neighbor_ip_1.value+'.'+F.routing_bgp_neighbor_ip_2.value+'.'+F.routing_bgp_neighbor_ip_3.value;
	}

	if(!valid_value(F)) return;
	if(F._route_nat){
		F.route_nat.value = F._route_nat.checked ? 1 : 0;
	}
	if(F._src_en){
		F.src_en.value = F._src_en.checked ? 1 : 0;
	}
	if(F._scope_en){
		F.scope_en.value = F._scope_en.checked ? 1 : 0;
	}
	if(F._table_en){
		F.table_en.value = F._table_en.checked ? 1 : 0;
	}
	if(F._mtu_en){
		F.mtu_en.value = F._mtu_en.checked ? 1 : 0;
	}
	if(F._advmss_en){
		F.advmss_en.value = F._advmss_en.checked ? 1 : 0;
	}
	if(F._not){
		F.not.value = F._not.checked ? 1 : 0;
	}
	if(F._from_en){
		F.from_en.value = F._from_en.checked ? 1 : 0;
	}
	if(F._to_en){
		F.to_en.value = F._to_en.checked ? 1 : 0;
	}
	if(F._priority_en){
		F.priority_en.value = F._priority_en.checked ? 1 : 0;
	}
	if(F._tos_en){
		F.tos_en.value = F._tos_en.checked ? 1 : 0;
	}
	if(F._fwmark_en){
		F.fwmark_en.value = F._fwmark_en.checked ? 1 : 0;
	}
	if(F._realms_en){
		F.realms_en.value = F._realms_en.checked ? 1 : 0;
	}
	if(F._pbr_table_en){
		F.pbr_table_en.value = F._pbr_table_en.checked ? 1 : 0;
	}
	if(F._suppress_prefixlength_en){
		F.suppress_prefixlength_en.value = F._suppress_prefixlength_en.checked ? 1 : 0;
	}
	if(F._iif_en){
		F.iif_en.value = F._iif_en.checked ? 1 : 0;
	}
	if(F._nat_en){
		F.nat_en.value = F._nat_en.checked ? 1 : 0;
	}
	if(F._type_en){
		F.type_en.value = F._type_en.checked ? 1 : 0;
	}
	if(F._sport_en){
		F.sport_en.value = F._sport_en.checked ? 1 : 0;
	}
	if(F._dport_en){
		F.dport_en.value = F._dport_en.checked ? 1 : 0;
	}
	if(F._ipproto_en){
		F.ipproto_en.value = F._ipproto_en.checked ? 1 : 0;
	}
	F.change_action.value = "";
	F.submit_type.value = "";
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

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);

	show_layer_ext(document.static.ospfd_copt, 'idospfd', <% nvem("ospfd_copt", "1", "1", "0"); %> == 1);
	show_layer_ext(document.static.ripd_copt, 'idripd', <% nvem("ripd_copt", "1", "1", "0"); %> == 1);
	show_layer_ext(document.static.zebra_copt, 'idzebra', <% nvem("zebra_copt", "1", "1", "0"); %> == 1);

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
					<% do_menu("index.asp","Routing.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="static" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="Routing" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="route_nat" />
							<input type="hidden" name="src_en" />
							<input type="hidden" name="scope_en" />
							<input type="hidden" name="table_en" />
							<input type="hidden" name="mtu_en" />
							<input type="hidden" name="advmss_en" />
							<input type="hidden" name="not" />
							<input type="hidden" name="from_en" />
							<input type="hidden" name="to_en" />
							<input type="hidden" name="priority_en" />
							<input type="hidden" name="tos_en" />
							<input type="hidden" name="fwmark_en" />
							<input type="hidden" name="realms_en" />
							<input type="hidden" name="pbr_table_en" />
							<input type="hidden" name="suppress_prefixlength_en" />
							<input type="hidden" name="iif_en" />
							<input type="hidden" name="nat_en" />
							<input type="hidden" name="type_en" />
							<input type="hidden" name="ipproto_en" />
							<input type="hidden" name="sport_en" />
							<input type="hidden" name="dport_en" />
							<input type="hidden" name="olsrd_delcount" />
							<input type="hidden" name="static_route" />
							<% ifndef("HAVE_PBR", "-->"); %>
							<input type="hidden" name="pbr_rule" />
							<% ifndef("HAVE_PBR", "<--"); %>
							<h2><% tran("route.h2"); %></h2>
							<fieldset>
								<legend><% tran("route.mod"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.mod"); %></div>
									<select name="wk_mode" onchange="SelMode(this.form.wk_mode.selectedIndex,this.form)">
										<% show_routing(); %>
									</select>
								</div>
							</fieldset><br />
							<% show_olsrd(); %>
							<% has_routing("ospf","<!--"); %>
							<fieldset>
							<legend><% tran("route.ospf_legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("route.ospf_copt"); %></div>
								<input class="spaceradio" type="radio" value="1" name="ospfd_copt" <% nvc("ospfd_copt","1"); %> onclick="show_layer_ext(this, 'idospfd', true)" /><% tran("route.copt_gui"); %>&nbsp;
								<input class="spaceradio" type="radio" value="0" name="ospfd_copt" <% nvc("ospfd_copt","0"); %> onclick="show_layer_ext(this, 'idospfd', false)" /><% tran("route.copt_vtysh"); %>
							</div>
							<div id="idospfd">
								<div class="setting">
								    <div class="label"><% tran("route.ospf_conf"); %></div>
								    <textarea cols="60" rows="12" id="ospfd_conf" name="ospfd_conf"></textarea>
								    <script type="text/javascript">
								    //<![CDATA[
								    var ospfd_conf = fix_cr( '<% nvg("ospfd_conf"); %>' );
								    document.getElementById("ospfd_conf").value = ospfd_conf;
								    //]]>
								    </script>
								</div>
							</div>
							</fieldset><br />
							<% has_routing("ospf","-->"); %>
							<% has_routing("bgp","<!--"); %>
							<fieldset>
								<legend><% tran("route.bgp_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.bgp_own_as"); %></div>
									<input size="10" name="routing_bgp_as" value="<% nvg("routing_bgp_as"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.bgp_ip"); %></div>
									<input type="hidden" name="routing_bgp_neighbor_ip" value="0.0.0.0" /><input size="3" maxlength="3" name="routing_bgp_neighbor_ip_0" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","0"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_1" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","1"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_2" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","2"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_3" onblur="valid_range(this,0,254,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","3"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.bgp_as"); %></div>
									<input size="10" name="routing_bgp_neighbor_as" value="<% nvg("routing_bgp_neighbor_as"); %>" />
								</div>
							</fieldset><br/>
							<% has_routing("bgp","-->"); %>
							<% has_routing("router","<!--"); %>
							<fieldset>
							<legend><% tran("route.rip2_legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("route.rip2_copt"); %></div>
								<input class="spaceradio" type="radio" value="1" name="ripd_copt" <% nvc("ripd_copt","1"); %> onclick="show_layer_ext(this, 'idripd', true)" /><% tran("route.copt_gui"); %>&nbsp;
								<input class="spaceradio" type="radio" value="0" name="ripd_copt" <% nvc("ripd_copt","0"); %> onclick="show_layer_ext(this, 'idripd', false)" /><% tran("route.copt_vtysh"); %>
							</div>
							<div id="idripd">
								<div class="setting">
								    <div class="label"><% tran("route.rip2_conf"); %></div>
								    <textarea cols="60" rows="12" id="ripd_conf" name="ripd_conf"></textarea>
								    <script type="text/javascript">
								    //<![CDATA[
								    var ripd_conf = fix_cr( '<% nvg("ripd_conf"); %>' );
								    document.getElementById("ripd_conf").value = ripd_conf;
								    //]]>
								    </script>
								</div>
							</div>
							</fieldset><br />
							<% has_routing("router","-->"); %>
							<% has_routing("zebra","<!--"); %>
							<fieldset>
								<legend><% tran("route.zebra_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.zebra_copt"); %></div>
									<input class="spaceradio" type="radio" value="1" name="zebra_copt" <% nvc("zebra_copt","1"); %> onclick="show_layer_ext(this, 'idzebra', true)" /><% tran("route.copt_gui"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="zebra_copt" <% nvc("zebra_copt","0"); %> onclick="show_layer_ext(this, 'idzebra', false)" /><% tran("route.copt_vtysh"); %>
								</div>
								<div id="idzebra">
								<div class="setting">
								    <div class="label"><% tran("route.zebra_legend"); %></div>
								    <textarea cols="60" rows="12" id="zebra_conf" name="zebra_conf"></textarea>
								    <script type="text/javascript">
								    //<![CDATA[
								    var zebra_conf = fix_cr( '<% nvg("zebra_conf"); %>' );
								    document.getElementById("zebra_conf").value = zebra_conf;
								    //]]>
								    </script>
								</div>
								<div class="setting">
									<div class="label"><% tran("route.zebra_log"); %></div>
									<input class="spaceradio" type="radio" name="zebra_log" value="1" <% nvc("zebra_log", "1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="zebra_log" value="0" <% nvc("zebra_log", "0"); %> /><% tran("share.disable"); %>
								</div>
								</div>
							</fieldset><br />
							<% has_routing("zebra","-->"); %>
							<% has_routing("gateway", "<!--"); %>
							<fieldset>
								<legend><% tran("route.gateway_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.intrface"); %></div>
									<select size="1" name="dr_setting">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsjs("dr_setting", "0"); %> >" + share.disable + "</option>");
										//]]>
										</script>
										<option value="1" <% nvs("dr_setting", "1"); %> >WAN</option>
										<option value="2" <% nvs("dr_setting", "2"); %> >LAN &amp; WLAN</option>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"3\" <% nvsjs("dr_setting", "3"); %> >" + share.both + "</option>");
										//]]>
										</script>
									</select>
								</div>
							 </fieldset><br/>
							 <% has_routing("gateway", "-->"); %>

							 <fieldset>
								<legend><% tran("route.static_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.static_setno"); %></div>
									<select size="1" name="route_page" onchange="SelRoute(this.form.route_page.selectedIndex,this.form)">
										<% static_route_table("select"); %>
									</select>&nbsp;&nbsp;
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.del + "\" onclick=\"DeleteEntry(this.form);\" />");
									//]]>
									</script>
								</div>
								<div class="setting">
									<div class="label"><% tran("route.static_name"); %></div>
									<input name="route_name" size="25" maxlength="25" onblur="valid_name(this,route.static_name)" value="<% static_route_setting("name"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("routetbl.th1"); %></div>
									<input type="hidden" name="route_ipaddr" value="4" />
									<input name="route_ipaddr_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","0"); %>" />.<input name="route_ipaddr_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","1"); %>" />.<input name="route_ipaddr_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","2"); %>" />.<input name="route_ipaddr_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","3"); %>" /> / <input name="route_netmask" size="3" maxlength="3" onblur="valid_range(this,0,32,share.subnet)" class="num" value="<% static_route_setting("netmask"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.gateway"); %></div>
									<input type="hidden" name="route_gateway" value="4" />
									<input size="3" maxlength="3" name="route_gateway_0" onblur="valid_range(this,0,255,share.gateway)" class="num" value="<% static_route_setting("gateway","0"); %>" />.<input size="3" maxlength="3" name="route_gateway_1" onblur="valid_range(this,0,255,share.gateway)" class="num" value="<% static_route_setting("gateway","1"); %>" />.<input size="3" maxlength="3" name="route_gateway_2" onblur="valid_range(this,0,255,share.gateway)" class="num" value="<% static_route_setting("gateway","2"); %>" />.<input size="3" maxlength="3" name="route_gateway_3" onblur="valid_range(this,0,254,share.gateway)" class="num" value="<% static_route_setting("gateway","3"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.intrface"); %></div>
									<select name="route_ifname">
										<% show_routeif(); %>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("route.metric"); %></div>
									<input name="route_metric" size="4" maxlength="4" onblur="valid_range(this,0,9999,route.metric)" class="num" value="<% static_route_setting("metric"); %>" />
								</div>
								<% has_routing("gateway", "<!--"); %>
								<div class="setting">
									<div class="label"><% tran("routetbl.masquerade"); %></div>
									<input class="spaceradio" type="checkbox" value="1" name="_route_nat" <% static_route_setting("nat"); %> />
								</div>
								<% has_routing("gateway", "-->"); %>
								<% ifndef("HAVE_PBR", "<!--"); %>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_src_en" <% static_route_setting("src_en"); %> />
									<div class="label"><% tran("routetbl.src"); %></div>
									<input type="hidden" name="route_src" value="4" />
									<input name="route_src_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.src)" class="num" value="<% static_route_setting("src","0"); %>" />.<input name="route_src_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.src)" class="num" value="<% static_route_setting("src","1"); %>" />.<input name="route_src_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.src)" class="num" value="<% static_route_setting("src","2"); %>" />.<input name="route_src_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.src)" class="num" value="<% static_route_setting("src","3"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_scope_en" <% static_route_setting("scope_en"); %> />
									<div class="label"><% tran("routetbl.scope"); %></div>
									<select name="route_scope">
										<option value="0" <% static_route_setting("scope", "0"); %> ><% tran("route.global"); %></option>
										<option value="255" <% static_route_setting("scope", "255"); %> ><% tran("route.nowhere"); %></option>
										<option value="254" <% static_route_setting("scope", "254"); %> ><% tran("route.host"); %></option>
										<option value="253" <% static_route_setting("scope", "253"); %> ><% tran("route.link"); %></option>
										<option value="200" <% static_route_setting("scope", "200"); %> ><% tran("route.site"); %></option>
									</select>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_table_en" <% static_route_setting("table_en"); %> />
									<div class="label"><% tran("routetbl.table"); %></div>
									<input name="route_table" size="5" maxlength="5" onblur="valid_range(this,0,2147483647,routetbl.table)" class="num" value="<% static_route_setting("table"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_mtu_en" <% static_route_setting("mtu_en"); %> />
									<div class="label"><% tran("idx.mtu"); %></div>
									<input name="route_mtu" size="5" maxlength="5" onblur="valid_range(this,68,9000,idx.mtu)" class="num" value="<% static_route_setting("mtu"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_advmss_en" <% static_route_setting("advmss_en"); %> />
									<div class="label"><% tran("routetbl.advmss"); %></div>
									<input name="route_advmss" size="5" maxlength="5" onblur="valid_range(this,28,8960,routetbl.advmss)" class="num" value="<% static_route_setting("advmss"); %>" />
								</div>
								<% ifndef("HAVE_PBR", "-->"); %>
								<div class="center"><br />
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button2\" value=\"" + sbutton.routingtab + "\" onclick=\"openWindow('RouteTable.asp', 790, 600);\" />");
									//]]>
									</script>
									<input type="hidden" value="0" name="Route_reload" />
								</div>
							</fieldset><br />
							<% ifndef("HAVE_PBR", "<!--"); %>
							<fieldset>
								<legend><% tran("route.rules"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.policy_setno"); %></div>
									<select size="1" name="rule_page" onchange="SelRule(this.form.rule_page.selectedIndex,this.form)">
										<% pbr_rule_table("select"); %>
									</select>&nbsp;&nbsp;
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.del + "\" onclick=\"DeleteRuleEntry(this.form);\" />");
									//]]>
									</script>
								</div>
								<div class="setting">
									<div class="label"><% tran("route.rule_name"); %></div>
									<input name="rule_name" size="25" maxlength="25" onblur="valid_name(this,route.rule_name)" value="<% pbr_rule_setting("name"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("routetbl.not"); %></div>
									<input class="spaceradio" type="checkbox" value="1" name="_not" <% pbr_rule_setting("not"); %> />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_from_en" <% pbr_rule_setting("from_en"); %> />
									<div class="label"><% tran("routetbl.from"); %></div>
									<input type="hidden" name="rule_from" value="4" />
									<input name="rule_from_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.from)" class="num" value="<% pbr_rule_setting("from","0"); %>" />.<input name="rule_from_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.from)" class="num" value="<% pbr_rule_setting("from","1"); %>" />.<input name="rule_from_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.from)" class="num" value="<% pbr_rule_setting("from","2"); %>" />.<input name="rule_from_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.from)" class="num" value="<% pbr_rule_setting("from","3"); %>" /> / <input name="rule_from_4" size="3" maxlength="3" onblur="valid_range(this,0,32,routetbl.from)" class="num" value="<% pbr_rule_setting("from","4"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_to_en" <% pbr_rule_setting("to_en"); %> />
									<div class="label"><% tran("routetbl.to"); %></div>
									<input type="hidden" name="rule_to" value="4" />
									<input name="rule_to_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.to)" class="num" value="<% pbr_rule_setting("to","0"); %>" />.<input name="rule_to_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.to)" class="num" value="<% pbr_rule_setting("to","1"); %>" />.<input name="rule_to_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.to)" class="num" value="<% pbr_rule_setting("to","2"); %>" />.<input name="rule_to_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.to)" class="num" value="<% pbr_rule_setting("to","3"); %>" /> / <input name="rule_to_4" size="3" maxlength="3" onblur="valid_range(this,0,32,routetbl.to)" class="num" value="<% pbr_rule_setting("to","4"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_priority_en" <% pbr_rule_setting("priority_en"); %> />
									<div class="label"><% tran("routetbl.priority"); %></div>
									<input name="rule_priority" size="5" maxlength="5" onblur="valid_range(this,0,2147483647,routetbl.priority)" class="num" value="<% pbr_rule_setting("priority"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_tos_en" <% pbr_rule_setting("tos_en"); %> />
									<div class="label"><% tran("routetbl.tos"); %></div>
									<input name="rule_tos" size="5" maxlength="5" onblur="valid_range(this,0,255,routetbl.tos)" class="num" value="<% pbr_rule_setting("tos"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_fwmark_en" <% pbr_rule_setting("fwmark_en"); %> />
									<div class="label"><% tran("routetbl.fwmark"); %></div>
							<% ifndef("HAVE_EXT_IPROUTE", "<!--"); %>
									<input name="rule_fwmark" size="10" maxlength="10" class="num" value="<% pbr_rule_setting("fwmark","0"); %>" /> / <input name="rule_fwmask" size="10" maxlength="10" class="num" value="<% pbr_rule_setting("fwmark","1"); %>" />
							<% ifndef("HAVE_EXT_IPROUTE", "-->"); %>
							<% ifdef("HAVE_EXT_IPROUTE", "<!--"); %>
									<input name="rule_fwmark" size="10" maxlength="10" class="num" value="<% pbr_rule_setting("fwmark","0"); %>" />
							<% ifdef("HAVE_EXT_IPROUTE", "-->"); %>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_realms_en" <% pbr_rule_setting("realms_en"); %> />
									<div class="label"><% tran("routetbl.realms"); %></div>
									<input name="rule_realms" size="5" maxlength="5" onblur="valid_range(this,0,2147483647,routetbl.realms)" class="num" value="<% pbr_rule_setting("realms"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_pbr_table_en" <% pbr_rule_setting("table_en"); %> />
									<div class="label"><% tran("routetbl.table"); %></div>
									<input name="rule_table" size="5" maxlength="5" onblur="valid_range(this,0,2147483647,routetbl.table)" class="num" value="<% pbr_rule_setting("table"); %>" />
								</div>
							<% ifndef("HAVE_EXT_IPROUTE", "<!--"); %>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_suppress_prefixlength_en" <% pbr_rule_setting("suppress_prefixlength_en"); %> />
									<div class="label"><% tran("routetbl.suppress_prefixlength"); %></div>
									<input name="rule_suppress_prefixlength" size="5" maxlength="5" onblur="valid_range(this,0,2147483647,routetbl.suppress_prefixlength)" class="num" value="<% pbr_rule_setting("suppress_prefixlength"); %>" />
								</div>
							<% ifndef("HAVE_EXT_IPROUTE", "-->"); %>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_iif_en" <% pbr_rule_setting("iif_en"); %> />
									<div class="label"><% tran("routetbl.iif"); %></div>
									<select name="rule_iif">
										<% show_ruleiif(); %>
									</select>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_oif_en" <% pbr_rule_setting("oif_en"); %> />
									<div class="label"><% tran("routetbl.oif"); %></div>
									<select name="rule_oif">
										<% show_ruleoif(); %>
									</select>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_nat_en" <% pbr_rule_setting("nat_en"); %> />
									<div class="label"><% tran("routetbl.nat"); %></div>
									<input type="hidden" name="rule_nat" value="4" />
									<input name="rule_nat_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.nat)" class="num" value="<% pbr_rule_setting("nat","0"); %>" />.<input name="rule_nat_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.nat)" class="num" value="<% pbr_rule_setting("nat","1"); %>" />.<input name="rule_nat_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.nat)" class="num" value="<% pbr_rule_setting("nat","2"); %>" />.<input name="rule_nat_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.nat)" class="num" value="<% pbr_rule_setting("nat","3"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_type_en" <% pbr_rule_setting("type_en"); %> />
									<div class="label"><% tran("routetbl.type"); %></div>
									<select name="rule_type">
										<option value="unicast" <% pbr_rule_setting("type", "unicast"); %> ><% tran("route.unicast"); %></option>
										<option value="blackhole" <% pbr_rule_setting("type", "blackhole"); %> ><% tran("route.blackhole"); %></option>
										<option value="unreachable" <% pbr_rule_setting("type", "unreachable"); %> ><% tran("route.unreachable"); %></option>
										<option value="prohibit" <% pbr_rule_setting("type", "prohibit"); %> ><% tran("route.prohibit"); %></option>
										<option value="nat" <% pbr_rule_setting("type", "nat"); %> ><% tran("route.nat"); %></option>
									</select>
								</div>
							<% ifndef("HAVE_EXT_IPROUTE", "<!--"); %>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_ipproto_en" <% pbr_rule_setting("ipproto_en"); %> />
									<div class="label"><% tran("routetbl.ipproto"); %></div>
									<select name="rule_ipproto">
										<option value="0" <% pbr_rule_setting("ipproto", "0"); %> >IP</option>
										<option value="1" <% pbr_rule_setting("ipproto", "1"); %> >ICMP</option>
										<option value="2" <% pbr_rule_setting("ipproto", "2"); %> >IGMP</option>
										<option value="4" <% pbr_rule_setting("ipproto", "4"); %> >IPIP</option>
										<option value="6" <% pbr_rule_setting("ipproto", "6"); %> >TCP</option>
										<option value="8" <% pbr_rule_setting("ipproto", "8"); %> >EGP</option>
										<option value="12" <% pbr_rule_setting("ipproto", "12"); %> >PUP</option>
										<option value="17" <% pbr_rule_setting("ipproto", "17"); %> >UDP</option>
										<option value="22" <% pbr_rule_setting("ipproto", "22"); %> >IDP</option>
										<option value="29" <% pbr_rule_setting("ipproto", "29"); %> >TP</option>
										<option value="33" <% pbr_rule_setting("ipproto", "33"); %> >DCCP</option>
										<option value="41" <% pbr_rule_setting("ipproto", "41"); %> >IPV6</option>
										<option value="43" <% pbr_rule_setting("ipproto", "43"); %> >ROUTING</option>
										<option value="44" <% pbr_rule_setting("ipproto", "44"); %> >FRAGMENT</option>
										<option value="46" <% pbr_rule_setting("ipproto", "46"); %> >RSVP</option>
										<option value="47" <% pbr_rule_setting("ipproto", "47"); %> >GRE</option>
										<option value="50" <% pbr_rule_setting("ipproto", "50"); %> >ESP</option>
										<option value="51" <% pbr_rule_setting("ipproto", "51"); %> >AH</option>
										<option value="58" <% pbr_rule_setting("ipproto", "58"); %> >ICMPV6</option>
										<option value="60" <% pbr_rule_setting("ipproto", "60"); %> >DSTOPTS</option>
										<option value="92" <% pbr_rule_setting("ipproto", "92"); %> >MTP</option>
										<option value="94" <% pbr_rule_setting("ipproto", "94"); %> >BEETPH</option>
										<option value="97" <% pbr_rule_setting("ipproto", "97"); %> >ETHERIP</option>
										<option value="98" <% pbr_rule_setting("ipproto", "98"); %> >ENCAP</option>
										<option value="103" <% pbr_rule_setting("ipproto", "103"); %> >PIM</option>
										<option value="108" <% pbr_rule_setting("ipproto", "108"); %> >COMP</option>
										<option value="132" <% pbr_rule_setting("ipproto", "132"); %> >SCTP</option>
										<option value="135" <% pbr_rule_setting("ipproto", "135"); %> >MH</option>
										<option value="136" <% pbr_rule_setting("ipproto", "136"); %> >UDPLITE</option>
										<option value="137" <% pbr_rule_setting("ipproto", "137"); %> >MPLS</option>
									</select>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_sport_en" <% pbr_rule_setting("sport_en"); %> />
									<div class="label"><% tran("routetbl.sport"); %></div>
									<input name="rule_sport_from" size="5" maxlength="5" onblur="valid_range(this,1,65535,routetbl.sport)" class="num" value="<% pbr_rule_setting("sport","0"); %>" /> ~ <input name="rule_sport_to" size="5" maxlength="5" onblur="valid_range(this,1,65535,routetbl.sport)" class="num" value="<% pbr_rule_setting("sport","1"); %>" />
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_dport_en" <% pbr_rule_setting("dport_en"); %> />
									<div class="label"><% tran("routetbl.dport"); %></div>
									<input name="rule_dport_from" size="5" maxlength="5" onblur="valid_range(this,1,65535,routetbl.dport)" class="num" value="<% pbr_rule_setting("dport","0"); %>" /> ~ <input name="rule_dport_to" size="5" maxlength="5" onblur="valid_range(this,1,65535,routetbl.dport)" class="num" value="<% pbr_rule_setting("dport","1"); %>" />
								</div>
							<% ifndef("HAVE_EXT_IPROUTE", "-->"); %>
								<div class="center"><br />
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button2\" value=\"" + sbutton.policytab + "\" onclick=\"openWindow('RuleTable.asp', 1090, 600);\" />");
									//]]>
									</script>
									<input type="hidden" value="0" name="Route_reload" />
								</div>
							</fieldset><br />
							<% ifndef("HAVE_PBR", "-->"); %>

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
							<dt class="term"><% tran("route.mod"); %>:</dt>
							<dd class="definition"><% tran("hroute.right2"); %></dd>
							<dt class="term"><% tran("route.static_setno"); %>:</dt>
							<dd class="definition"><% tran("hroute.right4"); %></dd>
							<dt class="term"><% tran("route.static_name"); %>:</dt>
							<dd class="definition"><% tran("hroute.right6"); %></dd>
							<dt class="term"><% tran("route.static_ip"); %>:</dt>
							<dd class="definition"><% tran("hroute.right8"); %></dd>
							<dt class="term"><% tran("share.subnet"); %>:</dt>
							<dd class="definition"><% tran("hroute.right10"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HRouting.asp');"><% tran("share.more"); %></a>
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
