<% do_pagehead("vpn.titl"); %>
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
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<% do_menu("Firewall.asp","VPN.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="VPN" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("vpn.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("vpn.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("vpn.ipsec"); %></div>
									<input class="spaceradio" type="radio" value="1" name="ipsec_pass" <% nvram_checked("ipsec_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="ipsec_pass" <% nvram_checked("ipsec_pass","0"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("vpn.pptp"); %></div>
									<input class="spaceradio" type="radio" value="1" name="pptp_pass" <% nvram_checked("pptp_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="pptp_pass" <% nvram_checked("pptp_pass","0"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("vpn.l2tp"); %></div>
									<input class="spaceradio" type="radio" value="1" name="l2tp_pass" <% nvram_checked("l2tp_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="l2tp_pass" <% nvram_checked("l2tp_pass","0"); %> /><% tran("share.disable"); %>
								</div>
							</fieldset><br/>
							
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
							<dd class="definition"><% tran("hvpn.right1"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HVPN.asp');"><% tran("share.more"); %></a>
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