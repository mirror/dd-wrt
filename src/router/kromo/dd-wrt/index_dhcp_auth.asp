<% atmsettings("pppoe"); %>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_userclass"); %></div>
	<input name="dhcp_userclass" size="127" maxlength="127" value="<% nvg("dhcp_userclass"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_clientid"); %></div>
	<input name="dhcp_clientid" size="127" maxlength="127" value="<% nvg("dhcp_clientid"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_classid"); %></div>
	<input name="dhcp_classid" size="127" maxlength="127" value="<% nvg("dhcp_classid"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_authentication"); %></div>
	<input name="dhcp_authentication" size="127" maxlength="127" value="<% nvg("dhcp_authentication"); %>" />
</div>
