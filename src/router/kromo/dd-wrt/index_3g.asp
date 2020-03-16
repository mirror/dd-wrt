<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvg("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvg("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.dial"); %></div>
	<select name="wan_dial" >
		<option value="0" <% nvsm("wan_dial", "0", "selected"); %> >*99***1# (UMTS/3G/3.5G)</option>
		<option value="1" <% nvsm("wan_dial", "1", "selected"); %> >*99# (UMTS/3G/3.5G)</option>
		<option value="3" <% nvsm("wan_dial", "3", "selected"); %> >#99***3# (LTE/3.75/4G)</option>
		<option value="2" <% nvsm("wan_dial", "2", "selected"); %> >#777 (CDMA/EVDO)</option>
		<option value="4" <% nvsm("wan_dial", "4", "selected"); %> >*99***2#(UMTS/3G/3.5G)</option>
		<option value="5" <% nvsm("wan_dial", "5", "selected"); %> >*99***4#(UMTS/3G/3.5G/LTE)</option>
<% ifndef("MULTISIM", "<!--"); %>
		<option value="97" <% nvsm("wan_dial", "97", "selected"); %> >Force MBIM</option>
		<option value="98" <% nvsm("wan_dial", "98", "selected"); %> >Force QMI</option>
		<option value="99" <% nvsm("wan_dial", "99", "selected"); %> >Force DirectIP</option>
<% ifndef("MULTISIM", "-->"); %>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.apn"); %></div>
	<input name="wan_apn" size="40" maxlength="63" onblur="valid_name(this,share.apn)" value="<% nvg("wan_apn"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.pin"); %></div>
	<input id="wan_pin" name="wan_pin" size="4" maxlength="4" onblur="valid_name(this,share.pin)" type="password" autocomplete="new-password" value="<% nvg("wan_pin"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wan_pin_unmask" value="0" onclick="setElementMask('wan_pin', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.mode_3g"); %></div>
	<select name="wan_conmode" >
	<script type="text/javascript">
	//<![CDATA[
	document.write("<option value=\"0\" <% nvsm("wan_conmode", "0", "selected"); %> >" + share.mode_3g_auto + "</option>");
	document.write("<option value=\"6\" <% nvsm("wan_conmode", "6", "selected"); %> >" + share.mode_3g_4g + "</option>");
	document.write("<option value=\"1\" <% nvsm("wan_conmode", "1", "selected"); %> >" + share.mode_3g_3g + "</option>");
	document.write("<option value=\"2\" <% nvsm("wan_conmode", "2", "selected"); %> >" + share.mode_3g_2g + "</option>");
	document.write("<option value=\"3\" <% nvsm("wan_conmode", "3", "selected"); %> >" + share.mode_3g_prefer_3g + "</option>");
	document.write("<option value=\"4\" <% nvsm("wan_conmode", "4", "selected"); %> >" + share.mode_3g_prefer_2g + "</option>");	
	document.write("<option value=\"5\" <% nvsm("wan_conmode", "5", "selected"); %> >" + share.mode_3g_3g2g + "</option>");
	//]]>
	</script>
	</select>
</div>
<div class="setting">
	<div class="label">Allow Roaming</div>
	<input class="spaceradio" type="radio" name="wan_roaming" value="1" <% nvc("wan_roaming","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_roaming" value="0" <% nvc("wan_roaming","0"); %> /><% tran("share.disable"); %> 
</div>
<% ifndef("MULTISIM", "<!--"); %>
<div class="setting">
	<div class="label">Multi Simcard</div>
	<input class="spaceradio" type="radio" name="wan_select_enable" value="1" <% nvc("wan_select_enable", "1"); %> onclick="show_layer_ext(this, 'wanselect', true)" /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_select_enable" value="0" <% nvc("wan_select_enable", "0"); %> onclick="show_layer_ext(this, 'wanselect', false)" /><% tran("share.disable"); %>
</div>
<div id="wanselect">
<div><hr></div>
<div class="setting">
	<div class="label"><% tran("share.usrname"); %> B</div>
	<input name="ppp_username_2" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvg("ppp_username_2"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %> B</div>
	<input id="ppp_passwd_2" name="ppp_passwd_2" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvg("ppp_passwd_2"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_2_unmask" value="0" onclick="setElementMask('ppp_passwd_2', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.dial"); %> B</div>
	<select name="wan_dial_2" >
		<option value="0" <% nvsm("wan_dial_2", "0", "selected"); %> >*99***1# (UMTS/3G/3.5G)</option>
		<option value="1" <% nvsm("wan_dial_2", "1", "selected"); %> >*99# (UMTS/3G/3.5G)</option>
		<option value="3" <% nvsm("wan_dial_2", "3", "selected"); %> >#99***3# (LTE/3.75/4G)</option>
		<option value="2" <% nvsm("wan_dial_2", "2", "selected"); %> >#777 (CDMA/EVDO)</option>
		<option value="4" <% nvsm("wan_dial_2", "4", "selected"); %> >*99***2#(UMTS/3G/3.5G)</option>
		<option value="5" <% nvsm("wan_dial_2", "5", "selected"); %> >*99***4#(UMTS/3G/3.5G/LTE)</option>
		<option value="97" <% nvsm("wan_dial_2", "97", "selected"); %> >Force MBIM</option>
		<option value="98" <% nvsm("wan_dial_2", "98", "selected"); %> >Force QMI</option>
		<option value="99" <% nvsm("wan_dial_2", "99", "selected"); %> >Force DirectIP</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.apn"); %> B</div>
	<input name="wan_apn_2" size="40" maxlength="63" onblur="valid_name(this,share.apn)" value="<% nvg("wan_apn_2"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.pin"); %> B</div>
	<input id="wan_pin_2" name="wan_pin_2" size="4" maxlength="4" onblur="valid_name(this,share.pin)" type="password" autocomplete="new-password" value="<% nvg("wan_pin_2"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wan_pin_2_unmask" value="0" onclick="setElementMask('wan_pin_2', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.mode_3g"); %> B</div>
	<select name="wan_conmode_2" >
	<script type="text/javascript">
	//<![CDATA[
	document.write("<option value=\"0\" <% nvsm("wan_conmode_2", "0", "selected"); %> >" + share.mode_3g_auto + "</option>");
	document.write("<option value=\"6\" <% nvsm("wan_conmode_2", "6", "selected"); %> >" + share.mode_3g_4g + "</option>");
	document.write("<option value=\"1\" <% nvsm("wan_conmode_2", "1", "selected"); %> >" + share.mode_3g_3g + "</option>");
	document.write("<option value=\"2\" <% nvsm("wan_conmode_2", "2", "selected"); %> >" + share.mode_3g_2g + "</option>");
	document.write("<option value=\"3\" <% nvsm("wan_conmode_2", "3", "selected"); %> >" + share.mode_3g_prefer_3g + "</option>");
	document.write("<option value=\"4\" <% nvsm("wan_conmode_2", "4", "selected"); %> >" + share.mode_3g_prefer_2g + "</option>");	
	document.write("<option value=\"5\" <% nvsm("wan_conmode_2", "5", "selected"); %> >" + share.mode_3g_3g2g + "</option>");
	//]]>
	</script>
	</select>
</div>
<div class="setting">
	<div class="label">Allow Roaming</div>
	<input class="spaceradio" type="radio" name="wan_roaming_2" value="1" <% nvc("wan_roaming_2","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_roaming_2" value="0" <% nvc("wan_roaming_2","0"); %> /><% tran("share.disable"); %> 
</div>
<div><hr></div>
<div class="setting">
	<div class="label"><% tran("share.usrname"); %> C</div>
	<input name="ppp_username_3" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvg("ppp_username_3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %> C</div>
	<input id="ppp_passwd_3" name="ppp_passwd_3" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" autocomplete="new-password" value="<% nvg("ppp_passwd_3"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_3_unmask" value="0" onclick="setElementMask('ppp_passwd_3', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.dial"); %> C</div>
	<select name="wan_dial_3" >
		<option value="0" <% nvsm("wan_dial_3", "0", "selected"); %> >*99***1# (UMTS/3G/3.5G)</option>
		<option value="1" <% nvsm("wan_dial_3", "1", "selected"); %> >*99# (UMTS/3G/3.5G)</option>
		<option value="3" <% nvsm("wan_dial_3", "3", "selected"); %> >#99***3# (LTE/3.75/4G)</option>
		<option value="2" <% nvsm("wan_dial_3", "2", "selected"); %> >#777 (CDMA/EVDO)</option>
		<option value="4" <% nvsm("wan_dial_3", "4", "selected"); %> >*99***2#(UMTS/3G/3.5G)</option>
		<option value="5" <% nvsm("wan_dial_3", "5", "selected"); %> >*99***4#(UMTS/3G/3.5G/LTE)</option>
		<option value="97" <% nvsm("wan_dial_3", "97", "selected"); %> >Force MBIM</option>
		<option value="98" <% nvsm("wan_dial_3", "98", "selected"); %> >Force QMI</option>
		<option value="99" <% nvsm("wan_dial_3", "99", "selected"); %> >Force DirectIP</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.apn"); %> C</div>
	<input name="wan_apn_3" size="40" maxlength="63" onblur="valid_name(this,share.apn)" value="<% nvg("wan_apn_3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.pin"); %> C</div>
	<input id="wan_pin_3" name="wan_pin_3" size="4" maxlength="4" onblur="valid_name(this,share.pin)" type="password" autocomplete="new-password" value="<% nvg("wan_pin_3"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wan_pin_3_unmask" value="0" onclick="setElementMask('wan_pin_3', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.mode_3g"); %> C</div>
	<select name="wan_conmode_3" >
	<script type="text/javascript">
	//<![CDATA[
	document.write("<option value=\"0\" <% nvsm("wan_conmode_3", "0", "selected"); %> >" + share.mode_3g_auto + "</option>");
	document.write("<option value=\"6\" <% nvsm("wan_conmode_3", "6", "selected"); %> >" + share.mode_3g_4g + "</option>");
	document.write("<option value=\"1\" <% nvsm("wan_conmode_3", "1", "selected"); %> >" + share.mode_3g_3g + "</option>");
	document.write("<option value=\"2\" <% nvsm("wan_conmode_3", "2", "selected"); %> >" + share.mode_3g_2g + "</option>");
	document.write("<option value=\"3\" <% nvsm("wan_conmode_3", "3", "selected"); %> >" + share.mode_3g_prefer_3g + "</option>");
	document.write("<option value=\"4\" <% nvsm("wan_conmode_3", "4", "selected"); %> >" + share.mode_3g_prefer_2g + "</option>");	
	document.write("<option value=\"5\" <% nvsm("wan_conmode_3", "5", "selected"); %> >" + share.mode_3g_3g2g + "</option>");
	//]]>
	</script>
	</select>
</div>
<div class="setting">
	<div class="label">Allow Roaming</div>
	<input class="spaceradio" type="radio" name="wan_roaming_3" value="1" <% nvc("wan_roaming_3","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="wan_roaming_3" value="0" <% nvc("wan_roaming_3","0"); %> /><% tran("share.disable"); %> 
</div>
<div><hr></div> 
<div class="setting">
	<div class="label">SIMcard</div>
	<select name="wan_select" >
		<option value="1" <% nvsm("wan_select", "1", "selected"); %> >SIM A</option>
		<option value="2" <% nvsm("wan_select", "2", "selected"); %> >SIM B</option>
		<option value="3" <% nvsm("wan_select", "3", "selected"); %> >SIM C</option>
	</select>
</div>
</div>
<% ifndef("MULTISIM", "-->"); %>

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
