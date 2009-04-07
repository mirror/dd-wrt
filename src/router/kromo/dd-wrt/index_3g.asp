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
	<div class="label"><% tran("share.dial"); %></div>
	<select name="wan_dial" >
		<option value="0" <% nvram_selmatch("wan_dial", "0", "selected"); %> >*99***1#</option>
		<option value="1" <% nvram_selmatch("wan_dial", "1", "selected"); %> >*99#</option>
		<option value="2" <% nvram_selmatch("wan_dial", "2", "selected"); %> >#777</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.apn"); %></div>
	<input name="wan_apn" size="40" maxlength="63" onblur="valid_name(this,share.apn)" value="<% nvram_get("wan_apn"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.pin"); %></div>
	<input id="wan_pin" name="wan_pin" size="4" maxlength="4" onblur="valid_name(this,share.pin)" type="password" value="<% nvram_get("wan_pin"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wan_pin_unmask" value="0" onclick="setElementMask('wan_pin', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
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
