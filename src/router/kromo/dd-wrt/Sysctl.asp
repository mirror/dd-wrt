<% do_pagehead("sysctl.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function to_submit(F) {
	F.change_action.value="gozila_cgi";
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
				<% do_menu("Management.asp","Sysctl.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Sysctl" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" value="save" />
							<input type="hidden" name="commit" value="1" />
							<h2>Sysctl Configuration</h2>
							<% sysctl(); %>
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
						</dl><br />
						<!-- Hide more... there is no help page here https://svn.dd-wrt.com/ticket/7478
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSysctl.asp');"><% tran("share.more"); %></a>
					  -->
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
