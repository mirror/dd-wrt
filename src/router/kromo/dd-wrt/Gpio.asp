<% do_pagehead("gpio.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function to_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gpios_save";
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gpios_save";
	F.apply_button.value = sbutton.applied;
	applytake(F);
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);

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
					<% do_menu("Status_Router.asp","Gpio.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="gpios_save" id="gpios_save" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Gpio" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="remove" />
							<h2><% tran("gpio.h2"); %></h2>
							<fieldset>
								<legend><% tran("gpio.oplegend"); %></legend>
								<div class="setting">
									<% show_status_gpio_output(); %><br>
								</div>
								<legend><% tran("gpio.iplegend"); %></legend>
								<div class="setting">
								<% show_status_gpio_input(); %><br>
								</div>
							</fieldset><br />
							<div id="footer" class="submitFooter">
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
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term"><% tran("upnp.legend"); %>:</dt>
							<dd class="definition"><% tran("hupnp.right2"); %></dd>
							<dt class="term"><% tran("upnp.serv"); %>:</dt>
							<dd class="definition"><% tran("hupnp.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HUPnP.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"<% get_firmware_version_href(); %>\"><% get_firmware_version(); %></a>");
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
