<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - OpenVPN Status</title>
		<script type="text/javascript"><![CDATA[
document.title = "<% nvram_get("router_name"); %>" + status_openvpn.titl;

		]]></script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Status_Router.asp","Status_OpenVPN.asp"); %>
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
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>
