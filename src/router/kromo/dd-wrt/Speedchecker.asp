<% do_pagehead("speedchecker.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "speedchecker";
	apply(F);
}
function to_apply(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "speedchecker";
	apply(F);
}

var update;

addEvent(window, "load", function() {
	show_layer_ext(document.setup.speedchecker_enable, 'speedcheckerconfig', <% nvem("speedchecker_enable", "1", "1", "0"); %> == 1);

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
							
<h2><% tran("speedchecker.legend"); %></h2>
<fieldset>
<legend><% tran("speedchecker.server"); %></legend>
<div class="setting">
           <div class="label"><% tran("speedchecker.server"); %></div>
           <input class="spaceradio" type="radio" name="speedchecker_enable" value="1" <% nvram_checked("speedchecker_enable", "1"); %> onclick="show_layer_ext(this, 'speedcheckerconfig', true)" /><% tran("share.enable"); %>&nbsp;
           <input class="spaceradio" type="radio" name="speedchecker_enable" value="0" <% nvram_checked("speedchecker_enable", "0"); %> onclick="show_layer_ext(this, 'speedcheckerconfig', false)"/><% tran("share.disable"); %>
</div>
<div id="speedcheckerconfig">
  <div class="setting">
           <div class="label"><% tran("speedchecker.regtitle"); %></div>
<% nvm("speedchecker_uuid", "", "<!--"); %>
<a href="https://speedchecker.dd-wrt.com/registration.html?RID=<% nvram_get("speedchecker_uuid"); %>&redirect_back=<% nvram_get("lan_ipaddr") %>/Speedchecker.asp">Registration Link</a>
<% nvm("speedchecker_uuid", "", "-->"); %>
<% nvram_invmatch("speedchecker_uuid", "", "<!--"); %>
<% tran("speedchecker.savemessage"); %>
<% nvram_invmatch("speedchecker_uuid", "", "-->"); %>

  </div>
</div>
</fieldset><br />

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
							<dt class="term"><% tran("speedchecker.server"); %></dt>
    							<dd class="definition"><% tran("hSpeedchecker.right2"); %></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSpeedchecker.asp')"><% tran("share.more"); %></a>
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
