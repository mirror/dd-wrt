<div class="setting">
	<div class="label">User Name</div>
	<input name="ppp_username" size="50" maxlength="63" onblur="valid_name(this,'User Name')" value='<% nvram_get("ppp_username"); %>' />
</div>
<div class="setting">
	<div class="label">Password</div>
	<input name="ppp_passwd" size="50" maxlength="63" onblur="valid_name(this,'Password')" type="password" value='<% nvram_invmatch("ppp_passwd","","d6nw5v1x2pc7st9m"); %>' />
</div>
<div class="setting">
	<div class="label">Service Name</div>
	<input name="ppp_service" size="50" maxlength="63" onblur="valid_name(this,'Service Name')" value='<% nvram_get("ppp_service"); %>' />
</div>
<div class="setting">
	<div class="label">Connection Strategy<br />&nbsp;</div>
	<input type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_match("ppp_demand","1","checked"); %> />Connect on Demand: Max Idle Time <input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,0,9999,'Idle time')" value='<% nvram_get("ppp_idletime"); %>' /> Min.<br />
	<input type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_match("ppp_demand","0","checked"); %> />Keep Alive: Redial Period <input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,'Redial period')" value='<% nvram_get("ppp_redialperiod"); %>' /> Sec.
</div>
<div class="setting">
	<div class="label">Use RP PPPoE</div>
	<input type="radio" value="1" name="pppoe_ver" <% nvram_match("pppoe_ver","1","checked"); %> />Enable&nbsp;
	<input type="radio" value="0" name="pppoe_ver" <% nvram_match("pppoe_ver","0","checked"); %> />Disable
</div>