<div class="setting">
	<div class="label">Use DHCP</div>
	<input type="radio" value="0" name="pptp_use_dhcp" <% nvram_match("pptp_use_dhcp","0","checked"); %> onclick="pptpUseDHCP(this.form,'0')" /> No
	<input type="radio" value="1" name="pptp_use_dhcp" <% nvram_match("pptp_use_dhcp","1","checked"); %> onclick="pptpUseDHCP(this.form,'1')" /> Yes
</div>
<div class="setting">
	<div class="label">Internet IP Address</div>
	<input type="hidden" name="wan_ipaddr" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","0"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","1"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","2"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,254,'IP')" value='<% get_single_ip("wan_ipaddr","3"); %>' />
</div>
<div class="setting">
	<div class="label">Subnet Mask</div>
	<input type="hidden" name="wan_netmask" value="4"/>
	<input class="num" maxlength="3" size="3" name="wan_netmask_0" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","0"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_1" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","1"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_2" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","2"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_3" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","3"); %>' />
</div>
<div class="setting">
	<div class="label">Gateway (PPTP Server)</div>
	<input type="hidden" name="pptp_server_ip" value="4"/>
	<input class="num" maxlength="3" size="3" name="pptp_server_ip_0" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("pptp_server_ip","0"); %>' />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_1" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("pptp_server_ip","1"); %>' />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_2" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("pptp_server_ip","2"); %>' />.<input class="num" maxlength="3" size="3" name="pptp_server_ip_3" onblur="valid_range(this,1,254,'IP')" value='<% get_single_ip("pptp_server_ip","3"); %>' />
</div>
<div class="setting">
	<div class="label">User Name</div>
	<input name="ppp_username" size="50" maxlength="63" onblur="valid_name(this,'User Name')" value='<% nvram_get("ppp_username"); %>' />
</div>
<div class="setting">
	<div class="label">Password</div>
	<input name="ppp_passwd" size="50" maxlength="63" onblur="valid_name(this,'Password')" type="password" value='<% nvram_invmatch("ppp_passwd","","d6nw5v1x2pc7st9m"); %>' />
</div>
<div class="setting">
	<div class="label">Connection Strategy<br />&nbsp;</div>
	<input type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_match("ppp_demand","1","checked"); %> />Connect on Demand: Max Idle Time <input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,1,9999,'Idle time')" value='<% nvram_get("ppp_idletime"); %>' /> Min.<br />
	<input type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_match("ppp_demand","0","checked"); %> />Keep Alive: Redial Period <input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,'Redial period')" value='<% nvram_get("ppp_redialperiod"); %>' /> Sec.
</div>
<div class="setting">
	<div class="label">PPTP Encyption</div>
	<input type="radio" name="pptp_encrypt" value="1" <% nvram_match("pptp_encrypt","1","checked"); %> />Encrypted&nbsp;
	<input type="radio" name="pptp_encrypt" value="0" <% nvram_match("pptp_encrypt","0","checked"); %> />Unencrypted
</div>