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
	<div class="label"><% tran("share.srv"); %></div>
	<input name="ppp_service" size="40" maxlength="63" onblur="valid_name(this,share.srv)" value="<% nvram_get("ppp_service"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.compression"); %></div>
	<input class="spaceradio" type="radio" name="ppp_compression" value="1" <% nvram_checked("ppp_compression","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="ppp_compression" value="0" <% nvram_checked("ppp_compression","0"); %> /><% tran("share.disable"); %> 
</div>
<div class="setting">
	<div class="label"><% tran("share.vdsl"); %></div>
	<input class="spaceradio" type="radio" name="wan_vdsl" value="1" <% nvram_checked("wan_vdsl","1"); %> onclick="show_layer_ext(this, 'idvlan8', true)" /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_vdsl" value="0" <% nvram_checked("wan_vdsl","0"); %> onclick="show_layer_ext(this, 'idvlan8', false)" /><% tran("share.disable"); %> 
	<div id="idvlan8">
		<div class="setting">
		    <div class="label"><% tran("share.vdslvlan8"); %></div>
		    <input class="spaceradio" type="radio" name="dtag_vlan8" value="1" <% nvram_checked("dtag_vlan8","1"); %> /><% tran("share.enable"); %>&nbsp;
		    <input class="spaceradio" type="radio" name="dtag_vlan8" value="0" <% nvram_checked("dtag_vlan8","0"); %> /><% tran("share.disable"); %> 
		</div>
	</div>
</div>
<script>
//<![CDATA
	show_layer_ext(document.setup.wan_vdsl, 'idvlan8', <% nvram_else_match("wan_vdsl", "1", "1", "0"); %> == 1);
//]]>
</script>
<div class="setting">
	<div class="label"><% tran("service.pptpd_encry"); %></div>
	<input size="27" maxlength="63" onblur="valid_name(this,service.pptpd_encry)" name="ppp_mppe" value="<% nvram_get("ppp_mppe"); %>" />
</div>
<!--
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_checked("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,0,9999,'Idle time')" value="<% nvram_get("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_checked("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,'Redial period')" value="<% nvram_get("ppp_redialperiod"); %>" />&nbsp;<% tran("share.secs"); %>
</div>
-->

	<div class="setting">
		<div class="label"><% tran("idx_h.reconnect"); %></div>
		<input class="spaceradio" type="radio" value="1" name="reconnect_enable" <% nvram_checked("reconnect_enable","1"); %> onclick="show_layer_ext(this, 'idreconnect', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" value="0" name="reconnect_enable" <% nvram_checked("reconnect_enable","0"); %> onclick="show_layer_ext(this, 'idreconnect', false)" /><% tran("share.disable"); %>
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
