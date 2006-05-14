<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pptp.srv)</script></div>
	<input class="spaceradio" type="radio" value="1" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","1"); %> onclick="pptpUseDHCP(this.form,'1')" /><script type="text/javascript">Capture(share.yes)</script>&nbsp;
	<input class="spaceradio" type="radio" value="0" name="pptp_use_dhcp" <% nvram_checked("pptp_use_dhcp","0"); %> onclick="pptpUseDHCP(this.form,'0')" /><script type="text/javascript">Capture(share.no)</script>
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pptp.wan_ip)</script></div>
	<input type="hidden" name="wan_ipaddr" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,254,idx_pptp.wan_ip)" value="<% get_single_ip("wan_ipaddr","3"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(share.subnet)</script></div>
	<input type="hidden" name="wan_netmask" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","0"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","1"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","2"); %>" />.<input class="num" maxlength="3" size="3" name="wan_netmask_3" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("wan_netmask","3"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pptp.gateway)</script></div>
	<input type="hidden" name="pptp_server_ip" value="4"/>
	<input class="num" maxlength="3" size="3" name="pptp_server_ip_0" onblur="valid_range(this,0,255,idx_pptp.gateway)" value="<% get_single_ip("pptp_server_ip","0"); %>" />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_1" onblur="valid_range(this,0,255,idx_pptp.gateway)" value="<% get_single_ip("pptp_server_ip","1"); %>" />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_2" onblur="valid_range(this,0,255,idx_pptp.gateway)" value="<% get_single_ip("pptp_server_ip","2"); %>" />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_3" onblur="valid_range(this,1,254,idx_pptp.gateway)" value="<% get_single_ip("pptp_server_ip","3"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(share.usrname)</script></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(share.passwd)</script></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" value="<% nvram_get("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<script type="text/javascript">Capture(share.unmask)</script></input>
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_h.con_strgy)</script><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_checked("ppp_demand","1"); %> /><script type="text/javascript">Capture(idx_h.max_idle)</script>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,idx_h.max_idle)" value="<% nvram_get("ppp_idletime"); %>" />&nbsp;<script type="text/javascript">Capture(share.mins)</script><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_checked("ppp_demand","0"); %> /><script type="text/javascript">Capture(idx_h.alive)</script>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,idx_h.alive)" value="<% nvram_get("ppp_redialperiod"); %>" />&nbsp;<script type="text/javascript">Capture(share.secs)</script>
</div>
<div class="setting">
	<div class="label"><script type="text/javascript">Capture(idx_pptp.encrypt)</script></div>
	<input class="spaceradio" type="radio" name="pptp_encrypt" value="1" <% nvram_checked("pptp_encrypt","1"); %> /><script type="text/javascript">Capture(share.enable)</script>&nbsp;
	<input class="spaceradio" type="radio" name="pptp_encrypt" value="0" <% nvram_checked("pptp_encrypt","0"); %> /><script type="text/javascript">Capture(share.disable)</script>
</div>