<% do_pagehead("factdef.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function applycheck(F) {
	F.apply_button.value = sbutton.applied;
}

function to_apply(F) {
	if( F.FactoryDefaults[0].checked == 1 ) {
		if(!confirm(factdef.mess1)) {
			return false;
		}
		applycheck(F);
		applyupdate(F);
		return true;
	}
}

var update;

addEvent(window, "load", function() {
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
					<% do_menu("Management.asp","Factory_Defaults.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="default" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Factory_Defaults" />
							<input type="hidden" name="action" value="Restore" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />

							<h2><% tran("factdef.h2"); %></h2>

							<fieldset>
								<legend><% tran("factdef.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("factdef.restore"); %></div>
									<input class="spaceradio" type="radio" name="FactoryDefaults" value="1" /><% tran("share.yes"); %>&nbsp;
									<input class="spaceradio" type="radio" name="FactoryDefaults" value="0" checked="checked" /><% tran("share.no"); %>
								</div>
							</fieldset><br/>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input title=\"" + sbutton.applytitle + "\" class=\"button green_btn\" type=\"button\" name=\"apply_button\" value=\"" + sbutton.apply + "\" onclick=\"to_apply(this.form);\" />");
								document.write("<input title=\"" + sbutton.canceltitle + "\" class=\"button brown_btn\" type=\"button\" name=\"reset_button\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.reload();\" />");
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
							<dd class="definition"><% tran("hfactdef.right1"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDefault.asp');"><% tran("share.more"); %></a>
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
