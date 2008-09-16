<% do_pagehead("diag.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F, I) {
	if(!valid(F, I)) return;
	F.submit_type.value = I;
	var applytype=0;
	
	if (I == "start") {
		F.ping.value = sbutton.cmd;
		applytype=1;
	}
	else if (I == "startup")
		F.startup.value = sbutton.saving;
	else if (I == "firewall")
		F.firewall.value = sbutton.saving;
	else if (I == "custom")
		F.custom.value = sbutton.saving;
		
	if (applytype)
		applytake(F);
	else
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
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Management.asp","Diagnostics.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ping" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="next_page" value="Diagnostics.asp" />
							<h2><% tran("diag.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("diag.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("diag.cmd"); %></div>
									<textarea id="ping_ip" name="ping_ip" rows="6" cols="40" style="font-family:Courier, Courier New" wrap="off"><% nvram_get("ping_ip"); %></textarea>
								</div>
									<script type="text/javascript">
									//<![CDATA[
									var table = new Array(<% dump_ping_log(""); %>);
									var h = Math.floor(windo.getWindoSize().height * 0.4);

									if(table.length > 0 && location.href.indexOf("Diagnostics.asp") == -1) {
										document.write("<br /><br /><pre style=\"height:" + ((h > 200) ? h : 200) + "px;\">" + table.join("\n") + "</pre>");
									}
									//]]>
									</script>
							</fieldset><br />
							
							<% nvram_match("rc_startup", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.startup"); %></legend>
								<pre id="startup"><% nvram_get("rc_startup"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_start\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('startup').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvram_match("rc_startup", "", "-->"); %>
							
							<% nvram_match("rc_firewall", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.firewall"); %></legend>
								<pre id="firewall"><% nvram_get("rc_firewall"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_firewall\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('firewall').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvram_match("rc_firewall", "", "-->"); %>

							<% nvram_match("rc_custom", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.custom"); %></legend>
								<pre id="custom"><% nvram_get("rc_custom"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_custom\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('custom').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvram_match("rc_custom", "", "-->"); %>
														
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"ping\" value=\"" + sbutton.runcmd + "\" onclick=\"to_submit(this.form, 'start');\" />")
								document.write("<input class=\"button\" type=\"button\" name=\"startup\" value=\"" + sbutton.startup + "\" onclick=\"to_submit(this.form, 'startup');\" />")
								document.write("<input class=\"button\" type=\"button\" name=\"firewall\" value=\"" + sbutton.firewall + "\" onclick=\"to_submit(this.form, 'firewall');\" />")
								document.write("<input class=\"button\" type=\"button\" name=\"custom\" value=\"" + sbutton.custom + "\" onclick=\"to_submit(this.form, 'custom');\" />")
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
							<dt class="term"><% tran("diag.cmd"); %>:</dt>
							<dd class="definition"><% tran("hdiag.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDiagnostics.asp');"><% tran("share.more"); %></a>
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