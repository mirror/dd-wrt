<div class="setting">
	<div class="label"><% tran("idx.ignore_wan_dns"); %></div>
	<input type="checkbox" value="1" name="_ignore_wan_dns" <% nvc("ignore_wan_dns", "1"); %> />
</div>
<% ifndef("WANVLAN", "<!--"); %>
<div class="setting">
<div class="label">
<script type="text/javascript">Capture(idx.wan_priority)</script></div>
<input type="checkbox" name="_wan_priority" value="1" <% nvc("wan_priority","1"); %> />
</div>
<% ifndef("WANVLAN", "-->"); %>
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
	<div class="label"><% tran("idx.dhcp_auth"); %></div>
	<input name="dhcp_authentication" size="63" maxlength="1023" value="<% nvg("dhcp_authentication"); %>" />
</div>
<% ifndef("HAVE_IPV6", "<!--"); %>
<div class="setting">
	<div class="label"><% tran("idx.dhcp6c_userclass"); %></div>
	<input name="dhcp6c_userclass" size="63" maxlength="1023" value="<% nvg("dhcp6c_userclass"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("service.dhcp6c_vendor"); %></div>
	<input maxlength="79" size="63" name="dhcp6c_vendorclass" value="<% nvg("dhcp6c_vendorclass"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp6c_clientid"); %></div>
	<input name="dhcp6c_clientid" size="63" maxlength="1023" value="<% nvg("dhcp6c_clientid"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx.dhcp6c_auth"); %></div>
	<input name="dhcp6c_authentication" size="63" maxlength="1023" value="<% nvg("dhcp6c_authentication"); %>" />
</div>
<% ifndef("HAVE_IPV6", "-->"); %>
