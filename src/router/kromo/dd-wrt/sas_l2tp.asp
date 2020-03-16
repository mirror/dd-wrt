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
	<div class="label"><% tran("idx_l.srv"); %></div>
	<input name="l2tp_server_name" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_selget("l2tp_server_name"); %>" />
</div>
<div class="setting">
		<div class="label"><% tran("idx_l.req_chap"); %></div>
		<input class="spaceradio" type="radio" name="l2tp_req_chap" value="yes" <% sas_nvc("l2tp_req_chap", "yes"); %> /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" name="l2tp_req_chap" value="no" <% sas_nvc("l2tp_req_chap", "no"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
		<div class="label"><% tran("idx_l.ref_pap"); %></div>
		<input class="spaceradio" type="radio" name="l2tp_ref_pap" value="yes" <% sas_nvc("l2tp_ref_pap", "yes"); %> /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" name="l2tp_ref_pap" value="no" <% sas_nvc("l2tp_ref_pap", "no"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
		<div class="label"><% tran("idx_l.req_auth"); %></div>
		<input class="spaceradio" type="radio" name="l2tp_req_auth" value="yes" <% sas_nvc("l2tp_req_auth", "yes"); %> /><% tran("share.yes"); %>&nbsp;
		<input class="spaceradio" type="radio" name="l2tp_req_auth" value="no" <% sas_nvc("l2tp_req_auth", "no"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
        <div class="label"><% tran("idx_l.iptv"); %></div>
        <input class="spaceradio" type="radio" name="pptp_iptv" value="1" <% nvc("pptp_iptv", "1"); %> /><% tran("share.yes"); %>&nbsp;
        <input class="spaceradio" type="radio" name="pptp_iptv" value="0" <% nvc("pptp_iptv", "0"); %> /><% tran("share.no"); %>
</div>
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% sas_nvc("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvram_selget("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% sas_nvc("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvram_selget("ppp_redialperiod"); %>" />&nbsp;<% tran("share.secs"); %>
</div>
