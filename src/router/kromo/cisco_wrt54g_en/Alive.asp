<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Keep Alive</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + alive.titl;

function to_reboot(F) {
	F.action.value = "Reboot";
	F.submit();
	return true;
}

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
		</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li class="current"><span><% tran("bmenu.admin"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><% tran("bmenu.adminManagement"); %></a></li>
											<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
											<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
											<li><span><% tran("bmenu.adminAlive"); %></span></li>
											<li><a href="Log.asp"><% tran("bmenu.adminLog"); %></a></li>
											<li><a href="Diagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
											<li><a href="Wol.asp"><% tran("bmenu.adminWol"); %></a></li>
											<li><a href="Factory_Defaults.asp"><% tran("bmenu.adminFactory"); %></a></li>
								<script type="text/javascript">
										https_visit = <% support_elsematch("HTTPS","1","1","0"); %>;
										if (https_visit =="1") {
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminUpgrade + "</em></a></li>");
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminBackup + "</em></a></li>");
										} else {
											document.write("<li><a href=\"Upgrade.asp\">" + bmenu.adminUpgrade + "</a></li>");
											document.write("<li><a href=\"config.asp\">" + bmenu.adminBackup + "</a></li>");
										}											
								</script>
<!--										<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											<li><a href="config.asp">Backup</a></li>
 -->
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="reboot_button" />
							<input type="hidden" name="commit" value="1" />
							<h2><% tran("alive.h2"); %></h2>
							<% show_modules(".webalive"); %>
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"init()\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div>
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