<% do_pagehead("status_band.titl"); %>
		<script type="text/javascript">
		//<![CDATA[
		
var update;

addEvent(window, "load", function() {
	if(document.getElementsByName("refresh_button")) {
		document.getElementsByName("refresh_button")[0].disabled = true;
		document.getElementsByName("refresh_button")[0].style.background = '#DADADA';
		document.getElementsByName("refresh_button")[0].style.cursor = "default";
	}
	
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
				<% do_menu("Status_Router.asp","Status_Bandwidth.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="status_band" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<% show_bandwidth(); %>
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
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
							<dt class="term"><% tran("status_band.h2"); %>:</dt>
							<dd class="definition"><% tran("hstatus_band.svg"); %></dd>
							<dt class="term"><% tran("status_band.chg_unit"); %>:</dt>
							<dd class="definition"><% tran("hstatus_band.right1"); %></dd>
							<dt class="term"><% tran("status_band.chg_scale"); %>:</dt>
							<dd class="definition"><% tran("hstatus_band.right2"); %></dd>
						</dl><br />
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