<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="30" maxlength="63" autocomplete="new-password" onblur="valid_name(this,share.usrname)" value="<% nvg("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="30" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvg("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.advanced"); %></div>
	<input type="checkbox" name="pppoeadv" value="1" <% selchecked("pppoeadv","1"); %> onclick="toggle_layer(this, 'pppoe_advanced')" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.ignore_wan_dns"); %></div>
	<input type="checkbox" value="1" name="_ignore_wan_dns" <% nvc("ignore_wan_dns", "1"); %> />
</div>
<div id="pppoe_advanced">
	<div class="setting">
		<div class="label"><% tran("share.srv"); %></div>
		<input name="ppp_service" size="30" maxlength="63" onblur="valid_name(this,share.srv)" value="<% nvg("ppp_service"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("share.host_uniq"); %></div>
		<input name="pppoe_host_uniq" size="30" maxlength="63" onblur="valid_name(this,share.host_uniq)" value="<% nvg("pppoe_host_uniq"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_pptp.srv"); %></div>
		<input class="spaceradio" type="radio" value="1" name="pptp_use_dhcp" <% nvc("pptp_use_dhcp","1"); %> onchange="show_layer_ext(this, 'idpptpdhcp', false); dhcp_show_static_dns(1);" /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" value="0" name="pptp_use_dhcp" <% nvc("pptp_use_dhcp","0"); %> onchange="show_layer_ext(this, 'idpptpdhcp', true); dhcp_show_static_dns();" /><% tran("share.no"); %>
	</div>
<div id="idpptpdhcp">
	<div class="setting">
		<div class="label"><% tran("idx_pptp.wan_ip"); %></div>
		<input type="hidden" name="wan_ipaddr_static" value="4"/>
		<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_3" onblur="valid_range(this,1,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","3"); %>" /> / <input class="num" maxlength="3" size="3" name="wan_netmask_static" onblur="valid_range(this,0,32,share.subnet)" value="<% get_cidr_mask("wan_netmask_static"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("share.gateway"); %></div>
		<input type="hidden" name="wan_gateway" value="4" />
		<input class="num" maxlength="3" size="3" name="l2tp_wan_gateway_0" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("l2tp_wan_gateway","0"); %>" />.<input class="num" maxlength="3" size="3" name="l2tp_wan_gateway_1" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("l2tp_wan_gateway","1"); %>" />.<input class="num" maxlength="3" name="l2tp_wan_gateway_2" size="3" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("l2tp_wan_gateway","2"); %>" />.<input class="num" maxlength="3" name="l2tp_wan_gateway_3" size="3" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("l2tp_wan_gateway","3"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_static.dns"); %>&nbsp;1</div>
		<input type="hidden" name="wan_dns" value="4" />
		<input class="num" name="wan_dns0_0" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","0","0"); %>" />.<input class="num" name="wan_dns0_1" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","0","1"); %>" />.<input class="num" name="wan_dns0_2" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","0","2"); %>" />.<input class="num" name="wan_dns0_3" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","0","3"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_static.dns"); %>&nbsp;2</div>
		<input class="num" name="wan_dns1_0" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","1","0"); %>" />.<input class="num" name="wan_dns1_1" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","1","1"); %>" />.<input class="num" name="wan_dns1_2" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","1","2"); %>" />.<input class="num" name="wan_dns1_3" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","1","3"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_static.dns"); %>&nbsp;3</div>
		<input class="num" name="wan_dns2_0" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","2","0"); %>" />.<input class="num" name="wan_dns2_1" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","2","1"); %>" />.<input class="num" name="wan_dns2_2" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","2","2"); %>" />.<input class="num" name="wan_dns2_3" size="3" maxlength="3" onblur="valid_range(this,0,255,idx_static.dns)" value="<% get_dns_ip("wan_dns","2","3"); %>" />
	</div>
</div>
	<div class="setting">
		<div class="label"><% tran("share.compression"); %></div>
		<input class="spaceradio" type="radio" name="ppp_compression" value="1" <% nvc("ppp_compression","1"); %> /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" name="ppp_compression" value="0" <% nvc("ppp_compression","0"); %> /><% tran("share.disable"); %> 
	</div>
	<div class="setting">
		<div class="label"><% tran("service.pptpd_encry"); %></div>
		<input size="30" maxlength="63" onblur="valid_name(this,service.pptpd_encry)" name="ppp_mppe" value="<% nvg("ppp_mppe"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_pptp.iptv"); %></div>
		<input class="spaceradio" type="radio" name="pptp_iptv" value="1" <% nvc("pptp_iptv", "1"); %> /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" name="pptp_iptv" value="0" <% nvc("pptp_iptv", "0"); %> /><% tran("share.no"); %>
	</div>
	<script type="text/javascript">
	//<![CDATA[
	show_layer_ext(this, 'pppoe_advanced', <% else_selmatch("pppoeadv", "1", "1", "0"); %> == 1);
	//]]>
	</script>
