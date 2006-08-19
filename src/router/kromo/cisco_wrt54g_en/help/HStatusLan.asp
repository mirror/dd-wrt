<% do_hpagehead(); %>
		<title>Help - LAN Status</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>LAN Status</h2>
			<dl>
				<dd>This status screen displays the LAN status and configuration. All information is read-only.</dd>
				<dt>MAC Address</dt>
				<dd>The MAC Address of the LAN interface is displayed here.</dd>
				<dt>IP Address and Subnet Mask</dt>
				<dd>The current IP Address and Subnet Mask of the router, as seen by users on your local area network (LAN), are displayed here.</dd>
				<dt>DHCP Server</dt>
				<dd>The status of the router's DHCP server function is displayed here.</dd>
				<dt>Start/End IP Address</dt>
				<dd>The first and the last IP address the DHCP server can hand out to clients.</dd>
				<dt>DHCP Client List</dt>
				<dd>To show the current IP address leases by the DHCP server, click the <i>DHCP Clients Table</i> button.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HLog.asp">Log</a></li>
				<li><a href="HStatus.asp">Router Status</a></li>
				<li><a href="HStatusWireless.asp">Wireless Status</a></li>
			</ul>
		</div>
	</body>
</html>