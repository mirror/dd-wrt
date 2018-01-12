<% do_pagehead("eoip.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	F.save_button.value = sbutton.saving;
	applytake(F);
}

function changeproto(F, index, value)
{
if (value == 1) {
	show_layer_ext(F, "idmtik" + index, true);
} else {
	show_layer_ext(F, "idmtik" + index, false);
}

if (value == 2) {
	show_layer_ext(F, "idwireguard" + index, true);
	show_layer_ext(F, "idl2support" + index, false);
} else {
	show_layer_ext(F, "idwireguard" + index, false);
	show_layer_ext(F, "idl2support" + index, true);
}

}

var update;

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


		show_layer_ext(document.eop.oet1_proto, 'idmtik1', <% nvram_else_match("oet1_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet2_proto, 'idmtik2', <% nvram_else_match("oet2_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet3_proto, 'idmtik3', <% nvram_else_match("oet3_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet4_proto, 'idmtik4', <% nvram_else_match("oet4_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet5_proto, 'idmtik5', <% nvram_else_match("oet5_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet6_proto, 'idmtik6', <% nvram_else_match("oet6_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet7_proto, 'idmtik7', <% nvram_else_match("oet7_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet8_proto, 'idmtik8', <% nvram_else_match("oet8_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet9_proto, 'idmtik9', <% nvram_else_match("oet9_proto", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet10_proto, 'idmtik10', <% nvram_else_match("oet10_proto", "1", "1", "0"); %> == 1);

		show_layer_ext(document.eop.oet1_proto, 'idwireguard1', <% nvram_else_match("oet1_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet2_proto, 'idwireguard2', <% nvram_else_match("oet2_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet3_proto, 'idwireguard3', <% nvram_else_match("oet3_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet4_proto, 'idwireguard4', <% nvram_else_match("oet4_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet5_proto, 'idwireguard5', <% nvram_else_match("oet5_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet6_proto, 'idwireguard6', <% nvram_else_match("oet6_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet7_proto, 'idwireguard7', <% nvram_else_match("oet7_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet8_proto, 'idwireguard8', <% nvram_else_match("oet8_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet9_proto, 'idwireguard9', <% nvram_else_match("oet9_proto", "2", "2", "0"); %> == 2);
		show_layer_ext(document.eop.oet10_proto, 'idwireguard10', <% nvram_else_match("oet10_proto", "2", "2", "0"); %> == 2);

		show_layer_ext(document.eop.oet1_proto, 'idl2support1', <% nvram_else_match("oet1_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet2_proto, 'idl2support2', <% nvram_else_match("oet2_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet3_proto, 'idl2support3', <% nvram_else_match("oet3_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet4_proto, 'idl2support4', <% nvram_else_match("oet4_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet5_proto, 'idl2support5', <% nvram_else_match("oet5_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet6_proto, 'idl2support6', <% nvram_else_match("oet6_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet7_proto, 'idl2support7', <% nvram_else_match("oet7_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet8_proto, 'idl2support8', <% nvram_else_match("oet8_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet9_proto, 'idl2support9', <% nvram_else_match("oet9_proto", "2", "0", "2"); %> == 2);
		show_layer_ext(document.eop.oet10_proto, 'idl2support10', <% nvram_else_match("oet10_proto", "2", "0", "2"); %> == 2);
		
		update = new StatusbarUpdate();
		update.start();
		
});

addEvent(window, "unload", function() {
	update.stop();

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
						<form name="eop" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="eop-tunnel" />
							<input type="hidden" name="action" value="Apply" />
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
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
					//]]>
					</script>
				</div>
				<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
				<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>