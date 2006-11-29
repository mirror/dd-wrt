<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - MAC Filter</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + wl_mac.titl;

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}

addEvent(window, "load", function() {
	<% list_mac_layers(); %>
});
		
		//]]>
		</script>
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
							<input type="hidden" name="submit_button" value="Wireless_MAC" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("wl_mac.h2"); %></h2>
							<% show_macfilter(); %>
							<br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1);
								//]]>
								</script>
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