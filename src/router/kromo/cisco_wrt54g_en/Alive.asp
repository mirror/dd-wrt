<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Keep Alive</title>
		<script type="text/javascript">
		//<![CDATA[
		
document.title = "<% nvram_get("router_name"); %>" + alive.titl;

function to_submit(F) {
	F.submit_button.value = "Alive";
	F.save_button.value = sbutton.saving;

	F.action.value="Apply";
	apply(F);
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

function init() {
	show_layer_ext(document.setup.squid_watchdog_enable, 'idsquid_watchdog', <% nvram_else_match("squid_watchdog_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.schedule_enable, 'idschedule', <% nvram_else_match("schedule_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wds_watchdog_enable, 'idwds_watchdog', <% nvram_else_match("wds_watchdog_enable", "1", "1", "0"); %> == 1);
	setWDS(<% nvram_get("wds_watchdog_enable"); %>);
	setPXY(<% nvram_get("squid_watchdog_enable"); %>);
	setAlive();
}
		
	//]]>
</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Management.asp","Alive.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="commit" value="1" />
							<h2><% tran("alive.h2"); %></h2>
							<% show_modules(".webalive"); %>
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form);\" />");								
								document.write("<input type=\"button\" name=\"reset_button\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.reload();\" />");
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HAlive.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>