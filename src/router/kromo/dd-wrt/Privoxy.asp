<% do_pagehead("privoxy.titl"); %>
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
	show_layer_ext(document.setup.privoxy_enable, 'privoxyconfig', <% nvem("privoxy_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.privoxy_transp_enable, 'privoxyex', <% nvem("privoxy_transp_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.privoxy_advanced, 'privoxyadvanced', <% nvem("privoxy_advanced", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.privoxy_advanced, 'privoxywhite', <% nvem("privoxy_advanced", "0", "1", "0"); %> == 1);
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
					<% do_menu("Services.asp", "Privoxy.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Privoxy" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							<% show_modules(".webproxy"); %>
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
							<dt class="term"><% tran("privoxy.server"); %></dt>
							<dd class="definition"><% tran("hprivoxy.right2"); %></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HPrivoxy.asp')"><% tran("share.more"); %></a>
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
