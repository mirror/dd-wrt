<% do_pagehead("p2p.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.submit_button.value = "XXXXX";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	F.submit_button.value = "XXXXX";
	F.save_button.value = sbutton.saving;
	applytake(F);
}

var update;

addEvent(window, "load", function() {
	show_layer_ext(document.p2p.ctorrent_enable, 'idctorrent', <% nvram_else_match("ctorrent_enable", "1", "1", "0"); %> == 1);

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
					<% do_menu("ForwardSpec.asp","P2P.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="p2p" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("p2p.h2"); %></h2>
							
							<% show_modules(".p2pwebconfig"); %>
						 	
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
				<!--
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("p2p.h2"); %>:</dt>
							<dd class="definition"><% tran("hp2p.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HP2P.asp')"><% tran("share.more"); %></a>
					</div>
					-->
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