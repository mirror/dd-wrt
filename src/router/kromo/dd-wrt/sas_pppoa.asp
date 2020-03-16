<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_selget("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvram_selget("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.compression"); %></div>
	<input class="spaceradio" type="radio" name="ppp_compression" value="1" <% sas_nvc("ppp_compression","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="ppp_compression" value="0" <% sas_nvc("ppp_compression","0"); %> /><% tran("share.disable"); %> 
</div>
<!--div class="setting">
	<div class="label"><% tran("service.pptpd_encry"); %></div>
	<input size="27" maxlength="63" onblur="valid_name(this,service.pptpd_encry)" name="ppp_mppe" value="<% nvg("ppp_mppe"); %>" />
</div-->
<!--div class="setting">
	<div class="label"><% tran("share.mlppp"); %></div>
	<input type="checkbox" value="1" name="_ppp_mlppp" <% nvc("ppp_mlppp", "1"); %> />
</div-->
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
