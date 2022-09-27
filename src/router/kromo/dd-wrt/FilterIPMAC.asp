<% do_pagehead("filterIP.titl"); %>
		<script type="text/javascript">
		//<![CDATA[
function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.apply_button.value = sbutton.applied;
	applytake(F);
}
		//]]>
		</script>
	</head>

	<body class="popup_bg">
		<form name="ipfilter" action="apply.cgi" method="post" spellcheck="false">
			<input type="hidden" name="submit_button" value="FilterIPMAC" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />

			<input type="hidden" name="filter_ip_value" />
			<input type="hidden" name="filter_mac_value" />
			<h2><% tran("filterIP.h2"); %></h2>
			<fieldset>
				<legend><% tran("filterIP.legend1"); %></legend>
				<div class="setting">
					<div class="label">MAC 01</div>
					<input class="num" size="20" maxlength="17" name="mac0" onblur="valid_macs_all(this)" value="<% filter_mac_get(0); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 02</div>
					<input class="num" size="20" maxlength="17" name="mac1" onblur="valid_macs_all(this)" value="<% filter_mac_get(1); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 03</div>
					<input class="num" size="20" maxlength="17" name="mac2" onblur="valid_macs_all(this)" value="<% filter_mac_get(2); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 04</div>
					<input class="num" size="20" maxlength="17" name="mac3" onblur="valid_macs_all(this)" value="<% filter_mac_get(3); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 05</div>
					<input class="num" size="20" maxlength="17" name="mac4" onblur="valid_macs_all(this)" value="<% filter_mac_get(4); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 06</div>
					<input class="num" size="20" maxlength="17" name="mac5" onblur="valid_macs_all(this)" value="<% filter_mac_get(5); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 07</div>
					<input class="num" size="20" maxlength="17" name="mac6" onblur="valid_macs_all(this)" value="<% filter_mac_get(6); %>" />
				</div>
				<div class="setting">
					<div class="label">MAC 08</div>
					<input class="num" size="20" maxlength="17" name="mac7" onblur="valid_macs_all(this)" value="<% filter_mac_get(7); %>" />
				</div>
			</fieldset><br />
			<fieldset>
				<legend><% tran("filterIP.legend2"); %></legend>
				<div class="setting">
					<div class="label">IP 01</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip0" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",0); %>" />
				</div>
				<div class="setting">
					<div class="label">IP 02</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip1" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",1); %>" />
				</div>
				<div class="setting">
					<div class="label">IP 03</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip2" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",2); %>" />
				</div>
				<div class="setting">
					<div class="label">IP 04</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip3" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",3); %>" />
				</div>
				<div class="setting">
					<div class="label">IP 05</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip4" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",4); %>" />
				</div>
				<div class="setting">
					<div class="label">IP 06</div>
					<% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxlength="3" name="ip5" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip",5); %>" />
				</div>
			</fieldset><br />
			<fieldset>
				<legend><% tran("filterIP.legend3"); %></legend>
				<div class="setting">
					<div class="label" style="width:8em;"><% tran("filterIP.ip_range"); %> 01</div>
					<input class="num" size="3" maxlength="3" name="ip_range0_0" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_0",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_1" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_1",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_2" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_2",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_3" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip_range0_3",6); %>" />~
					<input class="num" size="3" maxlength="3" name="ip_range0_4" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_4",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_5" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_5",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_6" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range0_6",6); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range0_7" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip_range0_7",6); %>" />
				</div>
				<div class="setting">
					<div class="label" style="width: 8em;"><% tran("filterIP.ip_range"); %> 02</div>
					<input class="num" size="3" maxlength="3" name="ip_range1_0" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_0",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_1" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_1",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_2" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_2",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_3" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip_range1_3",7); %>" />~
					<input class="num" size="3" maxlength="3" name="ip_range1_4" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_4",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_5" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_5",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_6" onblur="valid_range(this,0,255,'IP')" value="<% filter_ip_get("ip_range1_6",7); %>" />.
					<input class="num" size="3" maxlength="3" name="ip_range1_7" onblur="valid_range(this,0,254,'IP')" value="<% filter_ip_get("ip_range1_7",7); %>" />
				</div>
			</fieldset><br />
			<div id="footer" class="submitFooter nostick">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(1,1,0,0,0,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>
