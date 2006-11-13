	<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - EoIP Tunnel</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + eoip.titl;

function to_submit(F) {
	F.submit_button.value = "eop-tunnel";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
		show_layer_ext(document.eop.oet1_en, 'idoet1', <% nvram_else_match("oet1_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet1_bridged, 'idbridged1', <% nvram_else_match("oet1_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet2_en, 'idoet2', <% nvram_else_match("oet2_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet2_bridged, 'idbridged2', <% nvram_else_match("oet2_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet3_en, 'idoet3', <% nvram_else_match("oet3_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet3_bridged, 'idbridged3', <% nvram_else_match("oet3_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet4_en, 'idoet4', <% nvram_else_match("oet4_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet4_bridged, 'idbridged4', <% nvram_else_match("oet4_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet5_en, 'idoet5', <% nvram_else_match("oet5_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet5_bridged, 'idbridged5', <% nvram_else_match("oet5_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet6_en, 'idoet6', <% nvram_else_match("oet6_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet6_bridged, 'idbridged6', <% nvram_else_match("oet6_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet7_en, 'idoet7', <% nvram_else_match("oet7_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet7_bridged, 'idbridged7', <% nvram_else_match("oet7_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet8_en, 'idoet8', <% nvram_else_match("oet8_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet8_bridged, 'idbridged8', <% nvram_else_match("oet8_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet9_en, 'idoet9', <% nvram_else_match("oet9_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet9_bridged, 'idbridged9', <% nvram_else_match("oet9_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet10_en, 'idoet10', <% nvram_else_match("oet10_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet10_bridged, 'idbridged10', <% nvram_else_match("oet10_bridged", "1", "0", "1"); %> == 1);
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
					<% do_menu("index.asp","eop-tunnel.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="eop" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("eoip.legend"); %></h2>
							
								<% show_eop_tunnels(); %>
				
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
							<div><h2><% tran("share.help"); %></h2></div>
							<dl>
							</dl><br />
							<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HEoIP.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>