<div class="setting">
	<div class="label"><% tran("radius.label3"); %></div>
	<input type="hidden" name="wl_radius_ipaddr" value="4" />
	<input size="3" maxlength="3" name="wl_radius_ipaddr_0" onblur="valid_range(this,0,255,'IP')" class="num" value="<% get_single_ip("wl_radius_ipaddr","0"); %>" />.<input size="3" maxlength="3" name="wl_radius_ipaddr_1" onblur="valid_range(this,0,255,'IP')" class="num" value="<% get_single_ip("wl_radius_ipaddr","1"); %>" />.<input size="3" maxlength="3" name="wl_radius_ipaddr_2" onblur="valid_range(this,0,255,'IP')" class="num" value="<% get_single_ip("wl_radius_ipaddr","2"); %>" />.<input size="3" maxlength="3" name="wl_radius_ipaddr_3" onblur="valid_range(this,1,254,'IP')" class="num" value="<% get_single_ip("wl_radius_ipaddr","3"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("radius.label4"); %></div>
	<input class="num" name="wl_radius_port" size="5" maxlength="5" onblur="valid_range(this,1,65535,wpa.radius_port)" value="<% nvg("wl_radius_port"); %>" />
	<span class="default"><script type="text/javascript">
	//<![CDATA[
	document.write("(" + share.deflt + ": 1812)");
	//]]>
	</script></span>
</div>
<div class="setting">
	<div class="label"><% tran("radius.label7"); %></div>
	<input type="password" autocomplete="new-password" id="wl_radius_key" name="wl_radius_key" size="20" maxlength="79" value="<% nvg("wl_radius_key"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_wl_radius_unmask" value="0" onclick="setElementMask('wl_radius_key', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
