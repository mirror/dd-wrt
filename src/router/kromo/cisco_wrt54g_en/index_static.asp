<div class="setting">
	<div class="label">Internet IP Address</div>
	<input type="hidden" name="wan_ipaddr" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_ipaddr_0" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","0"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_1" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","1"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_2" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_ipaddr","2"); %>' />.<input class="num" maxlength="3" size="3" name="wan_ipaddr_3" onblur="valid_range(this,1,254,'IP')" value='<% get_single_ip("wan_ipaddr","3"); %>' />
</div>
<div class="setting">
	<div class="label">Subnet Mask</div>
	<input type="hidden" name="wan_netmask" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_netmask_0" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","0"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_1" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","1"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_2" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","2"); %>' />.<input class="num" maxlength="3" size="3" name="wan_netmask_3" onblur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("wan_netmask","3"); %>' />
</div>
<div class="setting">
	<div class="label">Gateway</div>
	<input type="hidden" name="wan_gateway" value="4" />
	<input class="num" maxlength="3" size="3" name="wan_gateway_0" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_gateway","0"); %>' />.<input class="num" maxlength="3" size="3" name="wan_gateway_1" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_gateway","1"); %>' />.<input class="num" maxlength="3" name="wan_gateway_2" size="3" onblur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wan_gateway","2"); %>' />.<input class="num" maxlength="3" name="wan_gateway_3" size="3" onblur="valid_range(this,0,254,'IP')" value='<% get_single_ip("wan_gateway","3"); %>' />
</div>
<div class="setting">
	<div class="label">Static DNS 1</div>
	<input type="hidden" name="wan_dns" value="4" />
	<input class="num" name="wan_dns0_0" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","0","0"); %>' />.<input class="num" name="wan_dns0_1" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","0","1"); %>' />.<input class="num" name="wan_dns0_2" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","0","2"); %>' />.<input class="num" name="wan_dns0_3" size="3" maxlength="3" onblur="valid_range(this,0,254,'DNS')" value='<% get_dns_ip("wan_dns","0","3"); %>' />
</div>
<div class="setting">
	<div class="label">Static DNS 2</div>
	<input class="num" name="wan_dns1_0" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","1","0"); %>' />.<input class="num" name="wan_dns1_1" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","1","1"); %>' />.<input class="num" name="wan_dns1_2" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","1","2"); %>' />.<input class="num" name="wan_dns1_3" size="3" maxlength="3" onblur="valid_range(this,0,254,'DNS')" value='<% get_dns_ip("wan_dns","1","3"); %>' />
</div>
<div class="setting">
	<div class="label">Static DNS 3</div>
	<input class="num" name="wan_dns2_0" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","2","0"); %>' />.<input class="num" name="wan_dns2_1" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","2","1"); %>' />.<input class="num" name="wan_dns2_2" size="3" maxlength="3" onblur="valid_range(this,0,255,'DNS')" value='<% get_dns_ip("wan_dns","2","2"); %>' />.<input class="num" name="wan_dns2_3" size="3" maxlength="3" onblur="valid_range(this,0,254,'DNS')" value='<% get_dns_ip("wan_dns","2","3"); %>' />
</div>