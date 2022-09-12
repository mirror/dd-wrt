<% do_pagehead(); %>
		<title><% nvg("router_name"); %> - Activation</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvg("router_name"); %> - Activation";

//]]>
</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><strong><% tran("bmenu.setup"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.wireless"); %></strong></span></li>
								<% nvim("sipgate","1","<!--"); %>
								<li><span><strong><% tran("bmenu.sipath"); %></strong></span></li>
								<% nvim("sipgate","1","-->"); %>
								<li><span><strong><% tran("bmenu.security"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.accrestriction"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.applications"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.admin"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.statu"); %></strong></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<form name="register" action="apply.cgi" method="post" spellcheck="false">
						<input type="hidden" name="submit_button" value="Register" />
						<input type="hidden" name="submit_type" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="action" value="Apply" />
						<div id="contentsInfo">
							<dl>
								<dd><% tran("reg.reg_aok"); %><br></dd>
							</dl>
							<div id="footer" class="submitFooter"></div>
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
