<% do_hpagehead(); %>
		<title>Help - Router Status</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Status</h2>
			<dl>
				<dd>This status screen displays the router's current status and configuration. All information is read-only.</dd>
				<dt>Firmware Version </dt>
				<dd>The version number of the firmware currently installed is displayed here. Firmware should only be upgraded from the System screen if you experience problems with the router. Visit <a href="http://www.dd-wrt.com" target="_new">www.dd-wrt.com</a> to find out if there is updated firmware.</dd>
				<dt>Current Time</dt>
				<dd>The current date and time are displayed here.</dd
				<dt>MAC Address </dt>
				<dd>The MAC Address of the Internet interface is displayed here.</dd>
				<dt>Router Name</dt>
				<dd>Shows the configured name of the router</dd>
				<dt>Router Model</dt>
				<dd>Shows the router vendor and the model</dd>
				<dt>CPU</dt>
				<dd>Shows the CPU type and revision</dd>
				<dt>CPU Clock</dt>
				<dd>Shows the current CPU clock</dd>
				<dt>Host Name</dt>
				<dd>The Host Name is the name of the router. This entry is necessary for some ISPs.</dd>
				<dt>Configuration Type</dt>
				<dd>The current Internet connection type is displayed here.</dd>
				<dt>IP Address, Subnet Mask, and Default Gateway</dt>
				<dd>The Internet IP Address, Subnet Mask, and Default Gateway IP Address of the router, as seen by external users on the Internet, are displayed here.</dd>
				<dt>DNS</dt>
				<dd>The DNS (Domain Name System) IP Addresses currently used by the router are shown here. Multiple DNS IP settings are common. In most cases, the first available DNS entry is used.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
				<li><a href="HStatusWireless.asp">Wireless Status</a></li>
			</ul>
		</div>
	</body>
</html>