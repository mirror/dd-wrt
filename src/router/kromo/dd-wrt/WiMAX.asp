<% do_pagehead("wl_wimax.titl"); %>
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

var update;

addEvent(window, "load", function() {
	<% list_mac_layers(); %>
	
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
					<% do_menu("Wireless_Basic.asp","WiMAX.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="wireless" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="WiMAX" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("wl_wimax.h2"); %></h2>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.mode"); %></div>
							    <select name="ofdm_mode">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<option value=\"disabled\" <% nvram_selmatch("ofdm_mode", "disabled", "selected"); %> >Disabled</option>");
								document.write("<option value=\"sta\" <% nvram_selmatch("ofdm_mode", "sta", "selected"); %> >WAN Router</option>");
								document.write("<option value=\"bridge\" <% nvram_selmatch("ofdm_mode", "bridge", "selected"); %> >Bridge</option>");
								document.write("<option value=\"router\" <% nvram_selmatch("ofdm_mode", "router", "selected"); %> >LAN Router</option>");
								//]]>
								</script>
							    </select>
							</div>
							<% showbridgesettings("ofdm","0"); %>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.downstream"); %></div>
							    <input size="27" name="ofdm_downstream" value="<% nvram_get("ofdm_downstream"); %>" />&nbsp;KHz
							</div>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.upstream"); %></div>
							    <input size="27" name="ofdm_upstream" value="<% nvram_get("ofdm_upstream"); %>" />&nbsp;KHz
							</div>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.width"); %></div>
							    <select name="ofdm_width">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<option value=\"1.75\" <% nvram_selmatch("ofdm_width", "1.75", "selected"); %> >1.75</option>");
								document.write("<option value=\"3\" <% nvram_selmatch("ofdm_width", "3", "selected"); %> >3</option>");
								document.write("<option value=\"3.5\" <% nvram_selmatch("ofdm_width", "3.5", "selected"); %> >3.5</option>");
								document.write("<option value=\"5\" <% nvram_selmatch("ofdm_width", "5", "selected"); %> >5</option>");
								document.write("<option value=\"7\" <% nvram_selmatch("ofdm_width", "7", "selected"); %> >7</option>");
								document.write("<option value=\"10\" <% nvram_selmatch("ofdm_width", "10", "selected"); %> >10</option>");
								//]]>
								</script>
							    </select>&nbsp;MHz
							</div>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.duplex"); %></div>
							    <select name="ofdm_duplex">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<option value=\"TDD\" <% nvram_selmatch("ofdm_duplex", "TDD", "selected"); %> >Time Divison Duplex</option>");
								document.write("<option value=\"H-FDD\" <% nvram_selmatch("ofdm_duplex", "H-FDD", "selected"); %> >Half-Duplex Frequency Division Duplex</option>");
								//]]>
								</script>
							    </select>
							</div>
							<div class="setting">
							    <div class="label"><% tran("wl_wimax.mac"); %></div>
							    <input size="27" name="ofdm_macaddr" value="<% nvram_get("ofdm_macaddr"); %>" />
							</div>
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
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWiMAX.asp')"><% tran("share.more"); %></a>
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
