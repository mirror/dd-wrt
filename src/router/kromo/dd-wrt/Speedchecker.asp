<% do_pagehead("speedchecker.titl"); %>
	<script type="text/javascript">
	//<![CDATA[
function to_submit(F) {
	F.submit_type.value = "speedchecker";
	apply(F);
}
function to_apply(F) {
	F.submit_type.value = "speedchecker";
	apply(F);
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	speedchecker_enable_onClick(<% nvg("speedchecker_enable"); %>);

	document.getElementById("main").style.float="none";
	document.getElementById("contents").style.width="98%";

	update = new StatusbarUpdate();
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

function speedchecker_enable_onClick(value) {
	var scope = document.getElementById('speedcheckerconfig');
	var RID = '<% nvg("speedchecker_uuid"); %>';
	var secret = '<% nvg("speedchecker_uuid2"); %>';
	speedchecker_toggle_desc(value);

	if (value == 1) {
		scope.innerHTML = '<iframe width="100%" height="250" frameborder="0" scrolling="no" src="https://speedchecker.dd-wrt.com/registration.html?RID=' + RID + '&secret=' + secret + '"></iframe>';
		show_layer_ext(document.setup.speedchecker_enable, 'speedcheckerconfig', true);
	} else {
		scope.innerHTML = '';
		show_layer_ext(document.setup.speedchecker_enable, 'speedcheckerconfig', false);
		scope = document.getElementById('scdesc');
		scope.innerHTML = '<iframe width="100%" height="200" frameborder="0" scrolling="no" src="https://speedchecker.dd-wrt.com/header.html"></iframe>';
	};
}

function speedchecker_toggle_desc(value) {
	var val = <% nvg("speedchecker_enable"); %>;
	var scope = document.getElementById('scdesc');

	if (val==1) {
		scope.innerHTML = '<iframe width="100%" height="200" frameborder="0" scrolling="no" src="/speedtest.asp"></iframe>';
	} else {
		scope.innerHTML = '<iframe width="100%" height="200" frameborder="0" scrolling="no" src="https://speedchecker.dd-wrt.com/header.html"></iframe>';
	}
}
	//]]>
	</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Services.asp", "Speedchecker.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Speedchecker" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>

							<fieldset>
								<div id="scdesc" size="100%"></div>
								<div class="setting">
					 				<div class="label"><% tran("speedchecker.server"); %></div>
					 				<input class="spaceradio" type="radio" name="speedchecker_enable" value="1" <% nvc("speedchecker_enable", "1"); %> onclick="speedchecker_enable_onClick(1)" /><% tran("share.enable"); %>&nbsp;
					 				<input class="spaceradio" type="radio" name="speedchecker_enable" value="0" <% nvc("speedchecker_enable", "0"); %> onclick="speedchecker_enable_onClick(0)"/><% tran("share.disable"); %>
								</div>
								<div id="speedcheckerconfig"></div>
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
