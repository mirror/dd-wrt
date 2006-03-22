<div class="setting">
<div class="label">RADIUS Server Address</div>
<input type="hidden" name="wl_radius_ipaddr" value="4" />
<input size="3" maxlength="3" name="wl_radius_ipaddr_0" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% get_single_ip("wl_radius_ipaddr","0"); %>' />.
<input size="3" maxlength="3" name="wl_radius_ipaddr_1" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% get_single_ip("wl_radius_ipaddr","1"); %>' />.
<input size="3" maxlength="3" name="wl_radius_ipaddr_2" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% get_single_ip("wl_radius_ipaddr","2"); %>' />.
<input size="3" maxlength="3" name="wl_radius_ipaddr_3" onBlur="valid_range(this,1,254,'IP')" class="num" value='<% get_single_ip("wl_radius_ipaddr","3"); %>' />
</div>
<div class="setting">
<div class="label">RADIUS Port</div>
<input name="wl_radius_port" size="3" maxlength="5" onBlur="valid_range(this,1,65535,'Port')" value='<% nvram_get("wl_radius_port"); %>' />
</div><div class="setting">
<div class="label">Shared Key</div>
<input name="wl_radius_key" size="20" maxlength="79" value='<% nvram_get("wl_radius_key"); %>' /></div>