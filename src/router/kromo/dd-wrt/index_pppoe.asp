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
	<div class="label"><% tran("idx_h.reconnect"); %></div>
	<input class="spaceradio" type="radio" value="1" name="reconnect_enable" <% nvc("reconnect_enable","1"); %> onclick="show_layer_ext(this, 'idreconnect', true)" /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" value="0" name="reconnect_enable" <% nvc("reconnect_enable","0"); %> onclick="show_layer_ext(this, 'idreconnect', false)" /><% tran("share.disable"); %>
</div>
<div id="idreconnect">
	<div class="setting">
		<div class="label"><% tran("share.time"); %></div>
		<select name="reconnect_hours">
			<% make_time_list("reconnect_hours","0","23"); %>
		</select>:<select name="reconnect_minutes">
			<% make_time_list("reconnect_minutes","0","59"); %>
		</select>
	</div>
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
		<div class="label"><% tran("share.vdslvlan7"); %></div>
		<input class="spaceradio" type="radio" name="wan_vdsl" value="1" <% nvc("wan_vdsl","1"); %> onclick="show_layer_ext(this, 'idvlan8', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" name="wan_vdsl" value="0" <% nvc("wan_vdsl","0"); %> onclick="show_layer_ext(this, 'idvlan8', false)" /><% tran("share.disable"); %>
	</div>
	<div id="idvlan8">
		<div class="setting">
			<div class="label"><% tran("share.vdslvlan8"); %></div>
			<input class="spaceradio" type="radio" name="dtag_vlan8" value="1" <% nvc("dtag_vlan8","1"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" name="dtag_vlan8" value="0" <% nvc("dtag_vlan8","0"); %> /><% tran("share.disable"); %> 
		</div>
		<div class="setting">
			<div class="label"><% tran("share.vdslbng"); %></div>
			<input class="spaceradio" type="radio" name="dtag_bng" value="1" <% nvc("dtag_bng","1"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" name="dtag_bng" value="0" <% nvc("dtag_bng","0"); %> /><% tran("share.disable"); %> 
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
		<div class="label"><% tran("share.mlppp"); %></div>
		<input type="checkbox" value="1" name="_ppp_mlppp" <% nvc("ppp_mlppp", "1"); %> />
	</div>
	<div class="setting"> 
  	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div> 
		<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvc("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvg("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br /> 
		<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvc("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvg("ppp_redialperiod"); %>" />&nbsp;<% tran("share.seconds"); %> 
	</div> 
	<% atmsettings("pppoe"); %>
</div>
<script type="text/javascript">
//<![CDATA[
	show_layer_ext(document.setup.pppoeadv, 'pppoe_advanced', <% else_selmatch("pppoeadv", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wan_vdsl, 'idvlan8', <% nvram_else_selmatch("wan_vdsl", "1", "1", "0"); %> == 1);
//]]>
</script>
