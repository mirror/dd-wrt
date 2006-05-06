<div class="setting">
	<div class="label"><script type="text/javascript">Capture(share.usrname)</script></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,'User Name')" value="<% nvram_get("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(share.passwd)</script></div>
	<input name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,'Password')" type="password" value="<% nvram_invmatch("ppp_passwd","","d6nw5v1x2pc7st9m"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<script type="text/javascript">Capture(share.unmask)</script></input>
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pppoe.srv)</script></div>
	<input name="ppp_service" size="40" maxlength="63" onblur="valid_name(this,'Service Name')" value="<% nvram_get("ppp_service"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_h.con_strgy)</script><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_checked("ppp_demand","1"); %> /><script type="text/javascript">Capture(idx_h.max_idle)</script>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,0,9999,'Idle time')" value="<% nvram_get("ppp_idletime"); %>" />&nbsp;<script type="text/javascript">Capture(share.mins)</script><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_checked("ppp_demand","0"); %> /><script type="text/javascript">Capture(idx_h.alive)</script>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,'Redial period')" value="<% nvram_get("ppp_redialperiod"); %>" />&nbsp;<script type="text/javascript">Capture(share.secs)</script>
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pppoe.use_rp)</script></div>
	<input class="spaceradio" type="radio" value="1" name="pppoe_ver" <% nvram_checked("pppoe_ver","1",""); %> /><script type="text/javascript">Capture(share.enable)</script>&nbsp;&nbsp;
	<input class="spaceradio" type="radio" value="0" name="pppoe_ver" <% nvram_checked("pppoe_ver","0",""); %> /><script type="text/javascript">Capture(share.disable)</script>
</div>