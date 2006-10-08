<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - MAC Filter</title>
		<script type="text/javascript"><![CDATA[

document.title = "<% nvram_get("router_name"); %>" + wl_mac.titl;

function to_submit(F) {
	F.submit_button.value = "Wireless_MAC";
	F.change_action.value = "apply_cgi";
	F.action.value = "Apply";
	
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	apply(F);
}

addEvent(window, "load", function() {
	show_layer_ext(document.wireless.wl_macmode1, 'idmac', "<% nvram_else_match("wl_macmode1", "other", "other", "disabled"); %>" == "other");
});
		]]></script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Wireless_Basic.asp","Wireless_MAC.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><% tran("wl_mac.h2"); %></h2>
							<fieldset>
								<legend><% tran("wl_mac.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("wl_mac.label"); %></div>
									<input class="spaceradio" type="radio" value="other" name="wl_macmode1" <% nvram_checked("wl_macmode1","other"); %> onclick="show_layer_ext(this, 'idmac', true)"/><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="disabled" name="wl_macmode1" <% nvram_checked("wl_macmode1","disabled"); %> onclick="show_layer_ext(this, 'idmac', false)"/><% tran("share.disable"); %>
								</div>
								<div class="setting" id="idmac">
									<div class="label"><% tran("wl_mac.label2"); %><br />&nbsp;</div>
									<input class="spaceradio" type="radio" value="deny" name="wl_macmode" <% nvram_invmatch("wl_macmode","allow","checked"); %> /><% tran("wl_mac.deny"); %>
									<br />
									<input class="spaceradio" type="radio" value="allow" name="wl_macmode" <% nvram_checked("wl_macmode","allow"); %> /><% tran("wl_mac.allow"); %>
								</div><br />
								<div class="center">
									<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"mac_filter_button\" value=\"" + sbutton.filterMac + "\" onclick=\"openWindow('WL_FilterTable.asp', 880, 730)\" />");
]]></script>
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");
]]></script>
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"cancel\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.reload()\" />");
]]></script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWirelessMAC.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>