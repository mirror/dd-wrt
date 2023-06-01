<div class="setting">
	<div class="label"><% tran("idx_pptp.wan_ip"); %></div>
	<input type="hidden" name="wan_ipaddr" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","3"); %>" /> / <input class="num" maxlength="3" size="3" name="wan_netmask" onblur="valid_range(this,0,32,share.subnet)" value="<% get_cidr_mask("wan_netmask"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.gateway"); %></div>
	<input type="hidden" name="wan_gateway" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_gateway_0" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_gateway_1" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","1"); %>" />.<input class="num" maxlength="3" name="wan_gateway_2" size="3" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","2"); %>" />.<input class="num" maxlength="3" name="wan_gateway_3" size="3" onblur="valid_range(this,0,255,share.gateway)" value="<% get_single_ip("wan_gateway","3"); %>" />
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
<% atmsettings("pppoe"); %>
