<% do_pagehead("bmenu.servicesMilkfish"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	checked(F);
	apply(F);
}

addEvent(window, "load", function() {

		show_layer_ext(document.setup.milkfish_enabled, 'idmilkfish', <% nvram_else_match("milkfish_enabled", "1", "1", "0"); %> == 1);
		
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
					<% do_menu("Services.asp","Milkfish.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Milkfish" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<h2><% tran("bmenu.servicesMilkfish"); %></h2>

	<fieldset>
		<legend><% tran("bmenu.servicesMilkfish"); %></legend>
			<div class="setting">
			<div class="label"><% tran("bmenu.servicesMilkfish"); %></div>
				<input class="spaceradio" type="radio" name="milkfish_enabled" value="1" <% nvram_checked("milkfish_enabled", "1"); %> onclick="show_layer_ext(this, 'idmilkfish', true)" /><% tran("share.enable"); %>&nbsp;
				<input class="spaceradio" type="radio" name="milkfish_enabled" value="0" <% nvram_checked("milkfish_enabled", "0"); %> onclick="show_layer_ext(this, 'idmilkfish', false)" /><% tran("share.disable"); %>
			</div>
	</fieldset><br />
	
<div id="idmilkfish">

	<fieldset>
		<legend><% tran("bmenu.servicesMilkfish"); %></legend>

	</fieldset>
	
</div>

<br/>

							
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
						<br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HMilkfish.asp');"><% tran("share.more"); %></a>
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