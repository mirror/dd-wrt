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
	else { 
		if (I == "startup") {
			F.startup.value = sbutton.saving;
		} 
		if (I == "shutdown") {
			F.shutdown.value = sbutton.saving;
		} 
		if (I == "firewall") {
			F.firewall.value = sbutton.saving;
		}  
		if (I == "custom") {
			F.custom.value = sbutton.saving;
		}
		if (I == "usb") {
			F.usb.value = sbutton.saving;
		}
	}
		
	if (applytype) {
		applytake(F);
	}else {
		apply(F);
	}
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
				<% do_menu("Management.asp","Diagnostics.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ping" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Diagnostics" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<h2><% tran("diag.h2"); %></h2>

							<fieldset>
								<legend><% tran("diag.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("diag.cmd"); %></div>
									<textarea id="ping_ip" name="ping_ip" rows="6" cols="80" style="font-family: Courier, 'Courier New', monospace" wrap="off"><% nvg("ping_ip"); %></textarea>
								</div>
									<script type="text/javascript">
									//<![CDATA[
									var table = new Array(<% dump_ping_log(""); %>);
									var h = Math.floor(windo.getWindoSize().height * 0.4);

									if(table.length > 0) {
										document.write("<div class=\"pre_label\">" + diag.output + "</div>");
										document.write("<pre style=\"height:" + ((h > 200) ? h : 200) + "px;\">" + table.join("\n") + "</pre>");
									}
									//]]>
									</script>
							</fieldset><br />
							<% nvm("rc_startup", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.startup"); %></legend>
								<pre id="startup"><% nvg("rc_startup"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_start\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('startup').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvm("rc_startup", "", "-->"); %>

							<% nvm("rc_shutdown", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.shutdown"); %></legend>
								<pre id="shutdown"><% nvg("rc_shutdown"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_shutdown\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('shutdown').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvm("rc_shutdown", "", "-->"); %>

							<% nvm("rc_firewall", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.firewall"); %></legend>
								<pre id="firewall"><% nvg("rc_firewall"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_firewall\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('firewall').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvm("rc_firewall", "", "-->"); %>
							<% ifndef("USB_rc", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.usb"); %></legend>
								<pre id="usb"><% nvg("rc_usb"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_usb\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('usb').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% ifndef("USB_rc", "-->"); %>
							<% nvm("rc_custom", "", "<!--"); %>
							<fieldset>
								<legend><% tran("diag.custom"); %></legend>
								<pre id="custom"><% nvg("rc_custom"); %></pre><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"button_custom\" value=\"" + sbutton.cptotext + "\" onclick=\"this.form.ping_ip.value = document.getElementById('custom').firstChild.data\" />")
									//]]>
									</script>
								</div>
							</fieldset><br />
							<% nvm("rc_custom", "", "-->"); %>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button green_btn\" type=\"button\" name=\"ping\" value=\"" + sbutton.runcmd + "\" onclick=\"to_submit(this.form, 'start');\" />");
								document.write("<input class=\"button\" type=\"button\" name=\"startup\" value=\"" + sbutton.startup + "\" onclick=\"to_submit(this.form, 'startup');\" />");
								document.write("<input class=\"button\" type=\"button\" name=\"shutdown\" value=\"" + sbutton.shutdown + "\" onclick=\"to_submit(this.form, 'shutdown');\" />");
								document.write("<input class=\"button\" type=\"button\" name=\"firewall\" value=\"" + sbutton.firewall + "\" onclick=\"to_submit(this.form, 'firewall');\" />");
								<% ifndef("USB", "/*"); %>
								document.write("<input class=\"button\" type=\"button\" name=\"usb\" value=\"" + sbutton.usb + "\" onclick=\"to_submit(this.form, 'usb');\" />");
								<% ifndef("USB", "*/"); %>
								document.write("<input class=\"button\" type=\"button\" name=\"custom\" value=\"" + sbutton.custom + "\" onclick=\"to_submit(this.form, 'custom');\" />");
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
							<dt class="term"><% tran("diag.cmd"); %>:</dt>
							<dd class="definition"><% tran("hdiag.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDiagnostics.asp');"><% tran("share.more"); %></a>
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
