<% do_pagehead("status_openvpn.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

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
					<% do_menu("Status_Router.asp","Status_OpenVPN.asp"); %>
				</div>
				<div id="main">
					<div id="contentsInfo">
					<h2><% tran("status_openvpn.titl"); %></h2>
						<% show_openvpn_status(); %>
						<div id="footer" class="submitFooter">
							<script type="text/javascript">
							//<![CDATA[
							submitFooterButton(0,0,0,0,1,0);
							//]]>
							</script>
						</div>
					</div>
				</div>
				<!--<div id="helpContainer">
					<div id="help">
							<h2><% tran("share.help"); %></h2>
					</div>
				</div>-->
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
