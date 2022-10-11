<% do_pagehead("bmenu.setupipv6"); %>
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
	show_layer_ext(document.setup.ipv6_enable,'idipv6', <% nvem("ipv6_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.dhcp6c_custom,'iddhcp6c_custom', <% nvem("dhcp6c_custom", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.dhcp6s_enable,'iddhcp6s_enabled', <% nvem("dhcp6s_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.dhcp6s_custom,'iddhcp6s_custom', <% nvem("dhcp6s_custom", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.radvd_enable,'idradvd', <% nvem("radvd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.radvd_custom,'idradvd_custom', <% nvem("radvd_custom", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.ipv6_typ,'idipv6_native', false);
	show_layer_ext(document.setup.ipv6_typ,'idipv6_6in4', false);

	if( "<% nvram_gozila_get("ipv6_typ"); %>" == "ipv6native" ){
		show_layer_ext(document.setup.ipv6_typ,'idipv6_native', true);
	} else if ( "<% nvram_gozila_get("ipv6_typ"); %>" == "ipv6in4" ) {
		show_layer_ext(document.setup.ipv6_typ,'idipv6_native', true);
		show_layer_ext(document.setup.ipv6_typ,'idipv6_6in4', true);
	}
	
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
					<% do_menu("index.asp", "IPV6.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="IPV6" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							<% show_modules(".ipv6config"); %>
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
							<dt class="term"><% tran("management.ipv6_h2"); %>:</dt>
							<dd class="definition"><% tran("hipv6.right2"); %></dd>
						</dl>
						<br />
						<!-- Hide more... there is no help page here https://svn.dd-wrt.com/ticket/7478
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HIPV6.asp')"><% tran("share.more"); %></a>
					  -->
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
