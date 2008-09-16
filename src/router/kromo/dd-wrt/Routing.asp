<% do_pagehead("route.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function valid_value(F) {
	if(F.wk_mode.value != "ospf") {
		if(!valid_ip(F,"F.route_ipaddr","IP",0))
			return false;
		if(!valid_mask(F,"F.route_netmask",ZERO_OK))
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

function SelRoute(num,F) {
	F.change_action.value="gozila_cgi";
	F.route_page.value = F.route_page.options[num].value;
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
	
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
}

function to_submit(F) {
    submitcheck(F);
    apply(F);
}
function to_apply(F) {
    submitcheck(F);
    applytake(F);
}

var update;

addEvent(window, "load", function() {
	
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
					<% do_menu("index.asp","Routing.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="static" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="Routing" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="olsrd_delcount" />
							<input type="hidden" name="static_route" />
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
							<% nvram_else_selmatch("wk_mode","ospf","","<!--"); %>
							<fieldset>
								<legend><% tran("route.ospf_legend"); %></legend>
								<div class="setting">
								    <div class="label"><% tran("route.ospf_conf"); %></div>
								    <textarea cols="60" rows="4" id="ospfd_conf" name="ospfd_conf"></textarea>
								    <script type="text/javascript">
								    //<![CDATA[
								    var ospfd_conf = fix_cr( '<% nvram_get("ospfd_conf"); %>' );
								    document.getElementById("ospfd_conf").value = ospfd_conf;
								    //]]>
								    </script>
								</div>
							</fieldset>
							<% nvram_else_selmatch("wk_mode","ospf","","-->"); %>
							<% nvram_else_selmatch("wk_mode","bgp","","<!--"); %>							
							<fieldset>
								<legend><% tran("route.bgp_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("route.bgp_own_as"); %></div>
									<input size="10" name="routing_bgp_as" value="<% nvram_get("routing_bgp_as"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.bgp_ip"); %></div>
									<input type="hidden" name="routing_bgp_neighbor_ip" value="0.0.0.0" /><input size="3" maxlength="3" name="routing_bgp_neighbor_ip_0" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","0"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_1" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","1"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_2" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","2"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_3" onblur="valid_range(this,0,254,route.bgp_ip)" class="num" value="<% get_single_ip("routing_bgp_neighbor_ip","3"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.bgp_as"); %></div>
									<input size="10" name="routing_bgp_neighbor_as" value="<% nvram_get("routing_bgp_neighbor_as"); %>" />
								</div>
							</fieldset><br/>							
							<% nvram_else_selmatch("wk_mode","bgp","","-->"); %>
							
							<% nvram_selmatch("wk_mode", "gateway", "<!--"); %>
							<fieldset>
								<legend><% tran("route.gateway_legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.intrface"); %></div>
									<select size="1" name="dr_setting">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected_js("dr_setting", "0"); %> >" + share.disable + "</option>");
										//]]>
										</script>
										<option value="1" <% nvram_selected("dr_setting", "1"); %> >WAN</option>
										<option value="2" <% nvram_selected("dr_setting", "2"); %> >LAN &amp; WLAN</option>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"3\" <% nvram_selected_js("dr_setting", "3"); %> >" + share.both + "</option>");
										//]]>
										</script>
									</select>
								</div>
							 </fieldset><br/>
							 <% nvram_selmatch("wk_mode","gateway", "-->"); %>
							 
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
									<input name="route_name" size="25" maxlength="25" onblur="valid_name(this,route.static_name)" value="<% static_route_setting("name",""); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.metric"); %></div>
									<input name="route_metric" size="4" maxlength="4" onblur="valid_range(this,0,9999,route.metric)" class="num" value="<% static_route_setting("metric",""); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("routetbl.th1"); %></div>
									<input type="hidden" name="route_ipaddr" value="4" />
									<input name="route_ipaddr_0" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","0"); %>" />.<input name="route_ipaddr_1" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","1"); %>" />.<input name="route_ipaddr_2" size="3" maxlength="3" onblur="valid_range(this,0,255,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","2"); %>" />.<input name="route_ipaddr_3" size="3" maxlength="3" onblur="valid_range(this,0,254,routetbl.th1)" class="num" value="<% static_route_setting("ipaddr","3"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.subnet"); %></div>
									<input type="hidden" name="route_netmask" value="4" />
									<input name="route_netmask_0" size="3" maxlength="3" onblur="valid_range(this,0,255,share.subnet)" class="num" value="<% static_route_setting("netmask","0"); %>" />.<input name="route_netmask_1" size="3" maxlength="3" onblur="valid_range(this,0,255,share.subnet)" class="num" value="<% static_route_setting("netmask","1"); %>" />.<input name="route_netmask_2" size="3" maxlength="3" onblur="valid_range(this,0,255,share.subnet)" class="num" value="<% static_route_setting("netmask","2"); %>" />.<input name="route_netmask_3" size="3" maxlength="3" onblur="valid_range(this,0,255,share.subnet)" class="num" value="<% static_route_setting("netmask","3"); %>" />
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
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button2\" value=\"" + sbutton.routingtab + "\" onclick=\"openWindow('RouteTable.asp', 720, 600);\" />");
									//]]>
									</script>
									<input type="hidden" value="0" name="Route_reload" />
								</div>
							</fieldset><br />
							
							<div class="submitFooter">
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
						<div><h2><% tran("share.help"); %></h2></div>
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
