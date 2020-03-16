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
	<div class="label"><% tran("idx_h.srv"); %></div>
	<input type="hidden" name="hb_server_ip" value="4" />
	<input class="num" maxlength="3" size="3" name="hb_server_ip_0" onblur="valid_range(this,0,255,'IP')" value="<% get_single_ip("hb_server_ip","0"); %>" />.<input class="num" maxlength="3" size="3" name="hb_server_ip_1" onblur="valid_range(this,0,255,'IP')" value="<% get_single_ip("hb_server_ip","1"); %>" />.<input class="num" maxlength="3" size="3" name="hb_server_ip_2" onblur="valid_range(this,0,255,'IP')" value="<% get_single_ip("hb_server_ip","2"); %>" />.<input class="num" maxlength="3" size="3" name="hb_server_ip_3" onblur="valid_range(this,1,254,'IP')" value="<% get_single_ip("hb_server_ip","3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvc("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvg("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvc("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvg("ppp_redialperiod"); %>" />&nbsp;<% tran("share.secs"); %>
</div>