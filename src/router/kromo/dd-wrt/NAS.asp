<% do_pagehead("nas.titl"); %>
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
	
	show_layer_ext(document.setup.ftpsrv_enable, 'ftpen', <% nvram_else_match("ftpsrv_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.sambasrv_enable, 'sambaen', <% nvram_else_match("sambasrv_enable", "1", "1", "0"); %> == 1);
	
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
					<% do_menu("Services.asp", "NAS.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="wireless" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="NAS" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
						<h2><% tran("nas.ftpsrv"); %></h2>
						
						<fieldset>
							<legend><% tran("nas.ftpsrv"); %></legend>
							<div class="setting">
								<div class="label"><% tran("nas.ftpsrv"); %></div>
								<input class="spaceradio" type="radio" name="ftpsrv_enable" value="1" <% nvram_checked("ftpsrv_enable", "1"); %> onclick="show_layer_ext(this, 'ftpen', true)" /><% tran("share.enable"); %>&nbsp;
								<input class="spaceradio" type="radio" name="ftpsrv_enable" value="0" <% nvram_checked("ftpsrv_enable", "0"); %> onclick="show_layer_ext(this, 'ftpen', false)" /><% tran("share.disable"); %>
							</div>
							<div id="ftpen">
							<% show_modules(".webftp"); %>
							</div>
						</fieldset><br />
						
						<h2><% tran("nas.sambasrv"); %></h2>
						
						<fieldset>
							<legend><% tran("nas.sambasrv"); %></legend>
							<div class="setting">
								<div class="label"><% tran("nas.sambasrv"); %></div>
								<input class="spaceradio" type="radio" name="sambasrv_enable" value="1" <% nvram_checked("sambasrv_enable", "1"); %> onclick="show_layer_ext(this, 'sambaen', true)" /><% tran("share.enable"); %>&nbsp;
								<input class="spaceradio" type="radio" name="sambasrv_enable" value="0" <% nvram_checked("sambasrv_enable", "0"); %> onclick="show_layer_ext(this, 'sambaen', false)" /><% tran("share.disable"); %>
							</div>
							<div id="sambaen">
							<% show_modules(".websamba"); %>
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
						<div><h2><% tran("share.help"); %></h2></div><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HNAS.asp')"><% tran("share.more"); %></a>
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
