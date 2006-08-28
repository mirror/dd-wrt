<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Diagnostics</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + diag.titl;

function to_submit(F, I) {
	if(!valid(F, I)) return;
	F.submit_type.value = I;
	F.submit_button.value = "Ping";
	F.change_action.value = "gozila_cgi";
	
	if (I == "start") 
		F.ping.value = sbutton.cmd;
	else if (I == "startup")
		F.startup.value = sbutton.saving;
	else if (I == "firewall")
		F.startup.value = sbutton.saving;
	
	apply(F);
}

function valid(F,I) {
	if(I == "start" && F.ping_ip.value == ""){
		alert(errmsg.err12);
		F.ping_ip.focus();
		return false;
	}
	return true;
}
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
											<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
											<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
											<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
											<li><a href="Log.asp"><% tran("bmenu.adminLog"); %></a></li>
											<li><span><% tran("bmenu.adminDiag"); %></span></li>
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
						<form name="ping" action="apply.cgi" method="post" >
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="submit_type" value="start" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="ping_times" value="1" />
							<input type="hidden" name="next_page" value="Diagnostics.asp" />
							<h2><% tran("diag.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("diag.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("diag.cmd"); %></div>
									<textarea id="ping_ip" name="ping_ip" rows="6" cols="40" style="font-family:Courier, Courier New"><% nvram_get("ping_ip"); %></textarea>
								</div>
									<script type="text/javascript">
									var table = new Array(<% dump_ping_log(""); %>);
									if(table.length > 0 && location.href.indexOf("Diagnostics.asp") == -1) {
										document.write("<br /><pre style=\"margin:0\">" + table.join("\n") + "</pre>");
									}
									</script>
							</fieldset><br />
							
							<% nvram_match("rc_startup", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.startup"); %></legend>
								<pre id="startup" style="margin:0"><% nvram_get("rc_startup"); %></pre><br />
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"button_start\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('startup').firstChild.data\" />")</script>
								</div>
							</fieldset><br />
							<% nvram_match("rc_startup", "", "-->"); %>
							
							<% nvram_match("rc_firewall", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.firewall"); %></legend>
								<pre id="firewall" style="margin:0"><% nvram_get("rc_firewall"); %></pre><br />
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"button_firewall\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('firewall').firstChild.data\" />")</script>
								</div>
							</fieldset><br />
							<% nvram_match("rc_firewall", "", "-->"); %>
							
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"ping\" value=\"" + sbutton.runcmd + "\" onclick=\"to_submit(this.form, 'start')\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"startup\" value=\"" + sbutton.startup + "\" onclick=\"to_submit(this.form, 'startup')\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"firewall\" value=\"" + sbutton.firewall + "\" onclick=\"to_submit(this.form, 'firewall')\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("diag.cmd"); %>:</dt>
							<dd class="definition"><% tran("hdiag.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HDiagnostics.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>