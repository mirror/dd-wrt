<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Activation</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %> - Activation";



//]]>
</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><% tran("bmenu.setup"); %></span></li>
								<li><span><% tran("bmenu.wireless"); %></span></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><span><% tran("bmenu.sipath"); %></span></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><span><% tran("bmenu.security"); %></span></li>
								<li><span><% tran("bmenu.accrestriction"); %></span></li>
								<li><span><% tran("bmenu.applications"); %></span></li>
								<li><span><% tran("bmenu.admin"); %></span></li>
								<li><span><% tran("bmenu.statu"); %></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
                    		<form name="register" action="apply.cgi" method="post">
                  			<input type="hidden" name="submit_button" value="Registered" />
                  			<input type="hidden" name="submit_type" />
                  			<input type="hidden" name="change_action" />
					<input type="hidden" name="action" value="Apply" />

					<div id="contentsInfo">
			<dl>
				<dd>Activation Done, the System will now reboot.<br></dd>
			</dl>
						<div class="submitFooter">
						</div>
					</div>
				</form>
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