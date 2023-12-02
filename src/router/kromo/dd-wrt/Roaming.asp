<% do_pagehead("wpa.titl"); %>
	<script type="text/javascript">
	//<![CDATA[


function to_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
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
		<div id="wrapper">
		<div id="content">
		<div id="header">
			<div id="logo"><h1><% show_control(); %></h1></div>
			<% do_menu("Wireless_Basic.asp","Roaming.asp"); %>
		</div>
		<div id="main">
			<div id="contents">
			<form name="wpa" action="apply.cgi" method="post" autocomplete="new-password">
				<input type="hidden" name="submit_button" value="Roaming" />
				<input type="hidden" name="action" value="Apply" />
				<input type="hidden" name="change_action" value="gozila_cgi" />
				<input type="hidden" name="submit_type" value="save" />

				<% show_roaming(); %>

				<div id="footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(1);
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
			   </dl><br />
			   <a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWPA.asp')"><% tran("share.more"); %></a>
			</div>
			</div>
			<div id="floatKiller"></div>
			<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
