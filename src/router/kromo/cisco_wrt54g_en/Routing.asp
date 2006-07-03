<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Routing</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + route.titl;

function DeleteEntry(F) {
	if(!confirm(errmsg.err57)) return;
	F.submit_button.value = "Routing";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "del";
	F.submit();
}

function to_submit(F) {
	if (F.routing_bgp_neighbor_ip != null) {
		F.routing_bgp_neighbor_ip.value = F.routing_bgp_neighbor_ip_0.value+'.'+F.routing_bgp_neighbor_ip_1.value+'.'+F.routing_bgp_neighbor_ip_2.value+'.'+F.routing_bgp_neighbor_ip_3.value;
	}
	
	if(!valid_value(F)) return;
	F.submit_button.value = "Routing";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

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

function SelRoute(num,F) {
	F.submit_button.value = "Routing";
	F.change_action.value = "gozila_cgi";
	F.route_page.value = F.route_page.options[num].value;
	F.submit();
}

function SelMode(num,F) {
	F.submit_button.value = "Routing";
	F.change_action.value = "gozila_cgi";
	F.wk_mode.value = F.wk_mode.options[num].value;
	F.submit();
}
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
								<li class="current"><span><% tran("bmenu.setup"); %></span>
								  <div id="menuSub">
										<ul id="menuSubList">
											<li><a href="index.asp"><% tran("bmenu.setupbasic"); %></a></li>
											<li><a href="DDNS.asp"><% tran("bmenu.setupddns"); %></a></li>
											<li><a href="WanMAC.asp"><% tran("bmenu.setupmacclone"); %></a></li>
											<li><span><% tran("bmenu.setuprouting"); %></span></li>
											<li><a href="Vlan.asp"><% tran("bmenu.setupvlan"); %></a></li>
  										</ul>
  									</div>
  								</li>
  								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="static" action="apply.cgi" method="<% get_http_method(); %>" >
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<input type="hidden" name="static_route" />
							<h2><% tran("route.h2"); %></h2>
							<div class="setting">
								<div class="label"><% tran("route.mod"); %></div>
								<select name="wk_mode" onchange="SelMode(this.form.wk_mode.selectedIndex,this.form)">
									<% show_routing(); %>
								</select>
							</div>
							<% nvram_else_selmatch("wk_mode","bgp","","<!--"); %>
							
							<fieldset>
								<legend><% tran("route.bgp_legend"); %></legend>
								<div class="setting">
									<div class="label">BGP</div>
									<input size="10" name="routing_bgp_as" value="<% nvram_get("routing_bgp_as"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("route.bgp_ip"); %></div>
									<input type="hidden" name="routing_bgp_neighbor_ip" value="0.0.0.0" /><input size="3" maxlength="3" name="routing_bgp_neighbor_ip_0" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% static_route_setting("routing_bgp_neighbor_ip","0"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_1" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% static_route_setting("routing_bgp_neighbor_ip","1"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_2" onblur="valid_range(this,0,255,route.bgp_ip)" class="num" value="<% static_route_setting("routing_bgp_neighbor_ip","2"); %>" />.<input size="3" maxlength="3" name="routing_bgp_neighbor_ip_3" onblur="valid_range(this,0,254,route.bgp_ip)" class="num" value="<% static_route_setting("routing_bgp_neighbor_ip","3"); %>" />
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
										<script type="text/javascript">document.write("<option value=\"0\" <% nvram_selected("dr_setting", "0", "js"); %> >" + share.disable + "</option>");</script>
										<option value="1" <% nvram_selected("dr_setting", "1"); %> >WAN</option>
										<option value="2" <% nvram_selected("dr_setting", "2"); %> >LAN &amp; WLAN</option>
										<script type="text/javascript">document.write("<option value=\"3\" <% nvram_selected("dr_setting", "3", "js"); %> >" + share.both + "</option>");</script>
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
									<script type="text/javascript">document.write("<input type=\"button\" value=\"" + sbutton.del + "\" onclick=\"DeleteEntry(this.form)\" />")</script>
								</div>
								<div class="setting">
									<div class="label"><% tran("route.static_name"); %></div>
									<input name="route_name" size="25" maxlength="25" onblur="valid_name(this,route.static_name)" value="<% static_route_setting("name",""); %>" />
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
										<option value="lan" <% static_route_setting("lan","0"); %> >LAN &amp; WLAN</option>
										<option value="wan" <% static_route_setting("wan","0"); %> >WAN</option>
									</select>
								</div>
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"button2\" value=\"" + sbutton.routingtab + "\" onclick=\"openWindow('RouteTable.asp', 720, 600)\" />")</script>
									<input type="hidden" value="0" name="Route_reload" />
								</div>
							</fieldset><br />
							
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div>
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HRouting.asp');"><% tran("share.more"); %></a>
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
