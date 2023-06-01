<div class="setting">
	<div class="label"><% tran("idx_l.srv"); %></div>
	<input name="l2tp_server_name" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvg("l2tp_server_name"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="40" maxlength="63" autocomplete="new-password" onblur="valid_name(this,share.usrname)" value="<% nvg("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvg("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("idx.ignore_wan_dns"); %></div>
	<input type="checkbox" value="1" name="_ignore_wan_dns" <% nvc("ignore_wan_dns", "1"); %> />
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.srv"); %></div>
	<input class="spaceradio" type="radio" value="1" name="l2tp_use_dhcp" <% nvc("l2tp_use_dhcp","1"); %> onchange="show_layer_ext(this, 'idl2tpdhcp', false); dhcp_show_static_dns(1);" /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" value="0" name="l2tp_use_dhcp" <% nvc("l2tp_use_dhcp","0"); %> onchange="show_layer_ext(this, 'idl2tpdhcp', true); dhcp_show_static_dns();" /><% tran("share.no"); %>
</div>
<div id="idl2tpdhcp">
	<div class="setting">
		<div class="label"><% tran("idx_pptp.wan_ip"); %></div>
		<input type="hidden" name="wan_ipaddr" value="4"/>
		<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","3"); %>" /> / <input class="num" maxlength="3" size="3" name="wan_netmask" onblur="valid_range(this,0,32,share.subnet)" value="<% get_cidr_mask("wan_netmask"); %>" />
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
	<div class="label"><% tran("service.pptpd_encry"); %></div>
	<input class="spaceradio" type="radio" name="l2tp_encrypt" value="1" <% nvc("l2tp_encrypt","1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="l2tp_encrypt" value="0" <% nvc("l2tp_encrypt","0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_l.req_chap"); %></div>
	<input class="spaceradio" type="radio" name="l2tp_req_chap" value="1" <% nvc("l2tp_req_chap", "1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="l2tp_req_chap" value="0" <% nvc("l2tp_req_chap", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_l.ref_pap"); %></div>
	<input class="spaceradio" type="radio" name="l2tp_ref_pap" value="1" <% nvc("l2tp_ref_pap", "1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="l2tp_ref_pap" value="0" <% nvc("l2tp_ref_pap", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_l.req_auth"); %></div>
	<input class="spaceradio" type="radio" name="l2tp_req_auth" value="1" <% nvc("l2tp_req_auth", "1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="l2tp_req_auth" value="0" <% nvc("l2tp_req_auth", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("share.wan_dualaccess"); %></div>
	<input class="spaceradio" type="radio" name="wan_dualaccess" value="1" <% nvc("wan_dualaccess", "1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_dualaccess" value="0" <% nvc("wan_dualaccess", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_l.iptv"); %></div>
	<input class="spaceradio" type="radio" name="pptp_iptv" value="1" <% nvc("pptp_iptv", "1"); %> /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" name="pptp_iptv" value="0" <% nvc("pptp_iptv", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvc("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvg("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvc("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvg("ppp_redialperiod"); %>" />&nbsp;<% tran("share.seconds"); %>
</div>
<% atmsettings("pppoe"); %>
