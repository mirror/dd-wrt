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

function ken_key(index)
{
	F.keyindex.value = keyindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gen_key";
	apply(F);
}

function changeproto(F, index, value, brvalue)
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

if (value == 2) {
	show_layer_ext(F, "idbridged" + index, false);
	show_layer_ext(F, "idlocalip" + index, false);
} else {
	show_layer_ext(F, "idlocalip" + index, true);
	if (brvalue == 1) {
	    show_layer_ext(F, "idbridged" + index, false);
	} else {
	    show_layer_ext(F, "idbridged" + index, true);
	}
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
		
		changeproto(document.eop.oet1_proto, 1, <% nvram_get("oet1_proto"); %>, <% nvram_get("oet1_bridged"); %>);
		changeproto(document.eop.oet2_proto, 2, <% nvram_get("oet2_proto"); %>, <% nvram_get("oet2_bridged"); %>);
		changeproto(document.eop.oet3_proto, 3, <% nvram_get("oet3_proto"); %>, <% nvram_get("oet3_bridged"); %>);
		changeproto(document.eop.oet4_proto, 4, <% nvram_get("oet4_proto"); %>, <% nvram_get("oet4_bridged"); %>);
		changeproto(document.eop.oet5_proto, 5, <% nvram_get("oet5_proto"); %>, <% nvram_get("oet5_bridged"); %>);
		changeproto(document.eop.oet6_proto, 6, <% nvram_get("oet6_proto"); %>, <% nvram_get("oet6_bridged"); %>);
		changeproto(document.eop.oet7_proto, 7, <% nvram_get("oet7_proto"); %>, <% nvram_get("oet7_bridged"); %>);
		changeproto(document.eop.oet8_proto, 8, <% nvram_get("oet8_proto"); %>, <% nvram_get("oet8_bridged"); %>);
		changeproto(document.eop.oet9_proto, 9, <% nvram_get("oet9_proto"); %>, <% nvram_get("oet9_bridged"); %>);
		changeproto(document.eop.oet10_proto, 10, <% nvram_get("oet10_proto"); %>, <% nvram_get("oet10_bridged"); %>);

		
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
							<input type="hidden" name="keyindex" />
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