<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Hotspot</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + hotspot.titl;

function user_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Hotspot";
	F.submit_type.value = "add_user";
	
 	F.action.value = "Apply";
	F.submit();
}
function iradius_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Hotspot";
	F.submit_type.value = "add_iradius";
 	F.action.value = "Apply";
	F.submit();
}

function user_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Hotspot";
	F.submit_type.value = "remove_user";
	
 	F.action.value = "Apply";
	F.submit();
}

function to_reboot(F) {
	F.action.value="Reboot";
	F.submit();
	return true;
}

function to_submit(F) {
	<% show_iradius_check(); %>
	F.submit_button.value = "Hotspot";
	F.save_button.value = sbutton.saving;

	F.action.value="Apply";
	apply(F);
	return true;
}

addEvent(window, "load", function() {
	if (document.setup.apd_enable) show_layer_ext('idsputnik', <% nvram_get("apd_enable"); %> == 1);
	if (document.setup.chilli_enable) show_layer_ext('idchilli', <% nvram_get("chilli_enable"); %> == 1);
	if (document.setup.http_redirect_enable) show_layer_ext('idhttpredirect', <% nvram_get("http_redirect_enable"); %> == 1);
	if (document.setup.NC_enable) show_layer_ext('idnocat', <% nvram_get("NC_enable"); %> == 1);
	if (document.setup.smtp_redirect_enable) show_layer_ext('smtpredirect', <% nvram_get("smtp_redirect_enable"); %> == 1);	
});

		</script>
	</head>

	<body class="gui">
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
											<li><span><% tran("bmenu.adminHotspot"); %></span></li>
											<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
											<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
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
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="reboot_button"/>
							<input type="hidden" name="commit" value="1"/>
							<h2><% tran("hotspot.h2"); %></h2>
							<% show_modules(".webhotspot"); %>
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" value=\"" + sbutton.reboot + "\" onclick=\"to_reboot(this.form)\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div><br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HHotspot.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>