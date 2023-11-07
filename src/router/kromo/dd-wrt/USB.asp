<% do_pagehead("usb.titl"); %>
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

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	show_layer_ext(document.setup.usb_enable, 'idusb', <% nvem("usb_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.usb_storage, 'idusbstor', <% nvem("usb_storage", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.usb_automnt, 'idusbmnt', <% nvem("usb_automnt", "1", "1", "0"); %> == 1);
	
	update = new StatusUpdate("USB.live.asp", <% nvg("refresh_time"); %>);
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

	//]]>
	</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Services.asp", "USB.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="USB" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>

							<% show_modules(".webusb"); %>

							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								 var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								 submitFooterButton(1,1,0,autoref);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HUSB.asp')"><% tran("share.more"); %></a>
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
