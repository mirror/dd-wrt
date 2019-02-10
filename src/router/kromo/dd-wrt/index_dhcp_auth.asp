<% ifdef("WANVLAN", "<!--"); %>
<div class="setting">
<div class="label">
<script type="text/javascript">Capture(idx.wan_priority)</script></div>
<input type="checkbox" name="_wan_priority" value="1" <% selchecked("wan_priority","1"); %> />
</div>
<% ifdef("WANVLAN", "-->"); %>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_userclass"); %></div>
	<input name="dhcp_userclass" size="63" maxlength="1023" value="<% nvg("dhcp_userclass"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("service.dhcp_vendor"); %></div>
	<input maxlength="79" size="63" name="dhcpc_vendorclass" value="<% nvg("dhcpc_vendorclass"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_clientid"); %></div>
	<input name="dhcp_clientid" size="63" maxlength="1023" value="<% nvg("dhcp_clientid"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp_authentication"); %></div>
	<input name="dhcp_authentication" size="63" maxlength="1023" value="<% nvg("dhcp_authentication"); %>" />
</div>
