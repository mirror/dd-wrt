<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless Status</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + status_wireless.titl;

		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li class="current"><span><% tran("bmenu.statu"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Status_Router.asp"><% tran("bmenu.statuRouter"); %></a></li>
											<li><a href="Status_Lan.asp"><% tran("bmenu.statuLAN"); %></a></li>
											<li><a href="Status_Wireless.asp"><% tran("bmenu.statuWLAN"); %></a></li>
											<% show_sputnik(); %>
											<li><span>VPN</span></li>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm"><% tran("bmenu.statuSysInfo"); %></a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
										</ul>
									</div>
								</li>
							</ul>
						</div>
					</div>
				</div>

				<div id="main">
					<div id="contents">
					<% show_openvpn_status(); %>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div>
						<br />
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
