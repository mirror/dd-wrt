<div class="setting">
	<div class="label"><% tran("idx.ignore_wan_dns"); %></div>
	<input type="checkbox" value="1" name="_ignore_wan_dns" <% nvc("ignore_wan_dns", "1"); %> />
</div>
<% atmsettings("pppoe"); %>
<% ifdef("WANVLAN", "<!--"); %>
<div class="setting">
	<div class="label">
		<script type="text/javascript">Capture(idx.wan_priority)</script></div>
		<input type="checkbox" name="_wan_priority" value="1" <% selchecked("wan_priority","1"); %> />
</div>
<% ifdef("WANVLAN", "-->"); %>
