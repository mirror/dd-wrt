<% do_hpagehead(); %>
		<title>Help - Firewall</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Firewall</h2>
			<dl>
				
				<dt><% tran("firewall.proxy"); %></dt>
				<dd>Blocks HTTP requests containing the "<i>Host:</i>" string.</dd>
				
				<dt><% tran("firewall.cookies"); %></dt>
				<dd>Identifies HTTP requests that contain the "<i>Cookie:</i>" string and mangle the cookie. Attempts to stop cookies from being used.</dd>
				
				<dt><% tran("firewall.applet"); %></dt>
				<dd>Blocks HTTP requests containing a URL ending in "<i>.js</i>" or "<i>.class</i>".</dd>
				
				<dt><% tran("firewall.activex"); %></dt>
				<dd>Blocks HTTP requests containing a URL ending in "<i>.ocx</i>" or "<i>.cab</i>".</dd>
				
				<dt><% tran("firewall.ping"); %></dt>
				<dd>Stops the router from responding to "pings" from the WAN.</dd>
				
				<dt><% tran("firewall.muticast"); %></dt>
				<dd>Prevents multicast packets from reaching the LAN.</dd>
				
				<dt><% tran("filter.nat"); %></dt>
				<dd>Prevents hosts on LAN from using WAN address of router to contact servers on the LAN (which have been configured using port redirection).</dd>
				
				<dt><% tran("filter.port113"); %></dt>
				<dd>Prevents WAN access to port 113.</dd>
								
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HForward.asp">Port Forwarding</a></li>
				<li><a href="HDMZ.asp">DMZ</a></li>
			</ul>
		</div>
	</body>
</html>
