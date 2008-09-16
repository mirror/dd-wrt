<% do_pagehead("alive.titl"); %>
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

function setWDS(val) {
	setElementsActive("wds_watchdog_interval_sec", "wds_watchdog_ips", val == 1);
}

function setPXY(val) {
	setElementsActive("squid_watchdog_interval_sec", "squid_proxy_server_port", val == 1);
}

function setAlive() {
	alive = document.getElementsByName('schedule_enable');
	if (alive[0].checked) {			// enable
		time = document.getElementsByName('schedule_hour_time');
		if (time[0].checked) { 		// Time
			setElementsActive("schedule_hour_time", "schedule_time", true);
			setElementActive("schedule_hour_time", true);
			setElementsActive("schedule_hours", "schedule_weekdays", false);
		} else { 					//At a set Time
			setElementsActive("schedule_hour_time", "schedule_weekdays", true);
			setElementActive("schedule_time", false);
		}
	} else { 						// disable
		setElementsActive("schedule_hour_time", "schedule_weekdays", false);
	}
}


var update;

addEvent(window, "load", function() {
	show_layer_ext(document.setup.squid_watchdog_enable, 'idsquid_watchdog', <% nvram_else_match("squid_watchdog_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.schedule_enable, 'idschedule', <% nvram_else_match("schedule_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wds_watchdog_enable, 'idwds_watchdog', <% nvram_else_match("wds_watchdog_enable", "1", "1", "0"); %> == 1);
	setWDS(<% nvram_get("wds_watchdog_enable"); %>);
	setPXY(<% nvram_get("squid_watchdog_enable"); %>);
	setAlive();
	
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
				<% do_menu("Management.asp","Alive.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Alive" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<h2><% tran("alive.h2"); %></h2>
							<% show_modules(".webalive"); %>
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
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("alive.hour"); %>:</dt>
							<dd class="definition"><% tran("halive.right2"); %></dd>
							<dt class="term"><% tran("alive.IP"); %>:</dt>
							<dd class="definition"><% tran("halive.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HAlive.asp');"><% tran("share.more"); %></a>
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