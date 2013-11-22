<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="30" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="30" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" value="<% nvram_get("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.advanced"); %></div>
	<input type="checkbox" name="pppoeadv" value="1" <% selchecked("pppoeadv","1"); %> onclick="toggle_layer(this, 'pppoe_advanced')" />
</div>
<div id="pppoe_advanced">
	<div class="setting">
	<div class="label"><% tran("share.srv"); %></div>
	<input name="ppp_service" size="30" maxlength="63" onblur="valid_name(this,share.srv)" value="<% nvram_get("ppp_service"); %>" />
	</div>
	<div class="setting">
	<div class="label"><% tran("idx_pptp.srv"); %></div>
	<input class="spaceradio" type="radio" value="1" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","1"); %> onclick="show_layer_ext(this, 'idpptpdhcp', false)" /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" value="0" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","0"); %> onclick="show_layer_ext(this, 'idpptpdhcp', true)" /><% tran("share.no"); %>
</div>
<div id="idpptpdhcp">
	<div class="setting">
		<div class="label"><% tran("idx_pptp.wan_ip"); %></div>
		<input type="hidden" name="wan_ipaddr_static" value="4"/>
		<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_static_3" onblur="valid_range(this,1,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr_static","3"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("share.subnet"); %></div>
		<input type="hidden" name="wan_netmask_static" value="4"/>
		<input class="num" maxlength="3" size="3" name="wan_netmask_static_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_nm("wan_netmask_static","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_static_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_nm("wan_netmask_static","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_static_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_nm("wan_netmask_static","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_static_3" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_nm("wan_netmask_static","3"); %>" />
	</div>
</div>

	<div class="setting">
		<div class="label"><% tran("share.compression"); %></div>
		<input class="spaceradio" type="radio" name="ppp_compression" value="1" <% nvram_checked("ppp_compression","1"); %> /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" name="ppp_compression" value="0" <% nvram_checked("ppp_compression","0"); %> /><% tran("share.disable"); %> 
	</div>
	<div class="setting">
		<div class="label"><% tran("service.pptpd_encry"); %></div>
		<input size="30" maxlength="63" onblur="valid_name(this,service.pptpd_encry)" name="ppp_mppe" value="<% nvram_get("ppp_mppe"); %>" />
	</div>
	<div class="setting">
		<div class="label"><% tran("idx_pptp.iptv"); %></div>
		<input class="spaceradio" type="radio" name="pptp_iptv" value="1" <% nvram_checked("pptp_iptv", "1"); %> /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" name="pptp_iptv" value="0" <% nvram_checked("pptp_iptv", "0"); %> /><% tran("share.no"); %>
	</div>
	
<script>
//<![CDATA[
        show_layer_ext(this, 'pppoe_advanced', <% else_selmatch("pppoeadv", "1", "1", "0"); %> == 1);
//]]>
</script>
