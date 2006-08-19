<% do_hpagehead(); %>
		<title>Help - Status</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Wireless Status</h2>
			<dl>
				<dd>This status screen displays the router's wireless status and configuration. All information is read-only.</dd>
				<dt>MAC Address</dt>
				<dd>The MAC Address of the wireless interface is displayed here.</dd>
				<dt>Mode</dt>
				<dd>The Mode of the wireless network is displayed here.</dd>
				<dt>SSID</dt>
				<dd>The SSID of the wireless network is displayed here.</dd>
				<dt>DHCP Server</dt>
				<dd>The status of the Router's DHCP server function is displayed here.</dd>
				<dt>Channel</dt>
				<dd>The channel of the wireless network is displayed here.</dd>
				<dt>Xmit</dt>
				<dd>The transfer power of the wireless device is displayed here.</dd>
				<dt>Rate</dt>
				<dd>The current wireless transfer rate is displayed here.</dd>
				<dt>Encryption</dt>
				<dd>The status of the encryption is displayed here.</dd>
				<dt>Survey</dt>
				<dd>Click the <i>Survey</i> button to show all wireless networks in your neighbourhood reachable by your router.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">Router Status</a></li>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
			</ul>
		</div>
	</body>
</html>