<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="30" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_selget("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="30" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvram_selget("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.dial"); %></div>
	<select name="wan_dial" >
		<option value="0" <% nvsm("wan_dial", "0", "selected"); %> >*99***1# (3G/UMTS)</option>
		<option value="1" <% nvsm("wan_dial", "1", "selected"); %> >*99# (3G/UMTS)</option>
		<option value="2" <% nvsm("wan_dial", "2", "selected"); %> >#777 (CDMA/EVDO)</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.apn"); %></div>
	<input name="wan_apn" size="30" maxlength="63" onblur="valid_name(this,share.apn)" value="<% nvram_selget("wan_apn"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.pin"); %></div>
	<input id="wan_pin" name="wan_pin" size="4" maxlength="4" onblur="valid_name(this,share.pin)" type="password" autocomplete="new-password" value="<% nvram_selget("wan_pin"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wan_pin_unmask" value="0" onclick="setElementMask('wan_pin', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
		<div class="label"><% tran("idx_h.reconnect"); %></div>
		<input class="spaceradio" type="radio" value="1" name="reconnect_enable" <% sas_nvc("reconnect_enable","1"); %> onclick="show_layer_ext(this, 'idreconnect', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" value="0" name="reconnect_enable" <% sas_nvc("reconnect_enable","0"); %> onclick="show_layer_ext(this, 'idreconnect', false)" /><% tran("share.disable"); %>
</div>
<div id="idreconnect">
	<div class="setting">
		<div class="label"><% tran("share.time"); %></div>
		<select name="reconnect_hours">
			<% sas_make_time_list("reconnect_hours","0","23"); %>
		</select>:<select name="reconnect_minutes">
			<% sas_make_time_list("reconnect_minutes","0","59"); %>
		</select>
	</div>
</div>
