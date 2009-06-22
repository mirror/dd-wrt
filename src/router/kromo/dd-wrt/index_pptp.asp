<div class="setting">
	<div class="label"><% tran("idx_pptp.srv"); %></div>
	<input class="spaceradio" type="radio" value="1" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","1"); %> onclick="pptpUseDHCP(this.form, this.value)" /><% tran("share.yes"); %>&nbsp;
	<input class="spaceradio" type="radio" value="0" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","0"); %> onclick="pptpUseDHCP(this.form, this.value)" /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.wan_ip"); %></div>
	<input type="hidden" name="wan_ipaddr" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,254,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.subnet"); %></div>
	<input type="hidden" name="wan_netmask" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_3" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.gateway"); %></div>
	<input type="hidden" name="wan_gateway" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_gateway_0" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_gateway_1" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","1"); %>" />.<input class="num" maxlength="3" name="wan_gateway_2" size="3" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","2"); %>" />.<input class="num" maxlength="3" name="wan_gateway_3" size="3" onblur="valid_range(this,0,254,share.gateway)" value="<% get_single_ip("wan_gateway","3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.gateway"); %></div>
	<input name="pptp_server_name" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_get("pptp_server_name"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" value="<% nvram_get("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_checked("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvram_get("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_checked("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvram_get("ppp_redialperiod"); %>" />&nbsp;<% tran("share.secs"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.encrypt"); %></div>
	<input class="spaceradio" type="radio" name="pptp_encrypt" value="1" <% nvram_checked("pptp_encrypt","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="pptp_encrypt" value="0" <% nvram_checked("pptp_encrypt","0"); %> /><% tran("share.disable"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.reorder"); %></div>
	<input class="spaceradio" type="radio" name="pptp_reorder" value="1" <% nvram_checked("pptp_reorder","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="pptp_reorder" value="0" <% nvram_checked("pptp_reorder","0"); %> /><% tran("share.disable"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_pptp.addopt"); %></div>
		<textarea cols="60" rows="3" id="pptp_extraoptions" name="pptp_extraoptions"></textarea>
		<script type="text/javascript">
		//<![CDATA[
		var pptp_extraoptions = fix_cr( '<% nvram_get("pptp_extraoptions"); %>' );
		document.getElementById("pptp_extraoptions").value = pptp_extraoptions;
		//]]>
		</script>
</div>