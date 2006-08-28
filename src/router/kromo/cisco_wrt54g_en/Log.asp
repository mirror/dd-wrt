<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Log</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + log.titl;

function to_submit(F) {
	F.submit_button.value = "Log";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
	show_layer_ext(document.log.log_enable, 'idlog1', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.log.log_enable, 'idlog2', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
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
								<li class="current"><span><% tran("bmenu.admin"); %></span></li>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><% tran("bmenu.adminManagement"); %></a></li>
											<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
											<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
											<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
											<li><span><% tran("bmenu.adminLog"); %></span></li>
											<li><a href="Diagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
											<li><a href="Wol.asp"><% tran("bmenu.adminWol"); %></a></li>
											<li><a href="Factory_Defaults.asp"><% tran("bmenu.adminFactory"); %></a></li>
								<script type="text/javascript">
										https_visit = "<% support_elsematch("HTTPS","1","1","0"); %>";
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
						<form name="log" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><% tran("log.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("log.legend"); %></legend>
								<div class="setting">
									<div class="label">Log</div>
									<input class="spaceradio" type="radio" value="1" name="log_enable" <% nvram_checked("log_enable", "1"); %> onclick="show_layer_ext(this, 'idlog1', true);show_layer_ext(this,'idlog2', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="log_enable" <% nvram_checked("log_enable", "0"); %> onclick="show_layer_ext(this, 'idlog1', false);show_layer_ext(this,'idlog2', false)" /><% tran("share.disable"); %>
								</div>
							<div id="idlog1">
								<div class="setting">
									<div class="label"><% tran("log.lvl"); %></div>
									<select name="log_level">
										<script type="text/javascript">document.write("<option value=\"0\" <% nvram_selected("log_level", "0", "js"); %> >" + share.low + "</option>");</script>
										<script type="text/javascript">document.write("<option value=\"1\" <% nvram_selected("log_level", "1", "js"); %> >" + share.medium + "</option>");</script>
										<script type="text/javascript">document.write("<option value=\"2\" <% nvram_selected("log_level", "2", "js"); %> >" + share.high + "</option>");</script>
									</select>
								</div>
							</div>
							</fieldset><br />
							
							<div id="idlog2">
								<fieldset>
									<legend><% tran("share.option"); %></legend>
									<div class="setting">
										<div class="label"><% tran("log.drop"); %></div>
										<select name="log_dropped">
											<option value="0" <% nvram_selected("log_dropped", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_dropped", "1"); %>>On</option>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.reject"); %></div>
										<select name="log_rejected">
											<option value="0" <% nvram_selected("log_rejected", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_rejected", "1"); %>>On</option>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.accept"); %></div>
										<select name="log_accepted">
											<option value="0" <% nvram_selected("log_accepted", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_accepted", "1"); %>>On</option>
										</select>
									</div>
								</fieldset><br />

								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"log_incoming\" value=\"" + sbutton.log_in + "\" onclick=\"openWindow('Log_incoming.asp', 580, 600)\" />")</script>
									<script type="text/javascript">document.write("<input type=\"button\" name=\"log_outgoing\" value=\"" + sbutton.log_out + "\" onclick=\"openWindow('Log_outgoing.asp', 760, 600)\" />")</script>
								</div><br />
							</div>
							
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"setLog('<% nvram_get("log_enable"); %>')\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HLog.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>