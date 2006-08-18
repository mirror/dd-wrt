<% do_hpagehead(); %>
		<title>Help - DMZ</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("dmz.h2"); %></h2>
			<dl>
				<dd>The DMZ (DeMilitarized Zone) hosting feature allows one local user to be exposed to the Internet for use of a special-purpose service such as Internet gaming or videoconferencing. DMZ hosting forwards all the ports at the same time to one PC. The Port Forwarding feature is more secure because it only opens the ports you want to have opened, while DMZ hosting opens all the ports of one computer, exposing the computer so the Internet can see it.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Any PC whose port is being forwarded must should have a new static IP address assigned to it because its IP address may change when using the DHCP function.</div>
					</div>
				</dd>
				<dt><% tran("dmz.host"); %></dt>
				<dd>To expose one PC to the Internet, select <i>Enable</i> and enter the computer's IP address in the <i>DMZ Host IP Address</i> field.<br /><br />
					To disable the DMZ, keep the default setting, <i>Disable</i>.</dd>
				<dd>Click <i>Save Settings</i> to save your settings or click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HFirewall.asp">Firewall</a></li>
				<li><a href="HForward.asp">Port Forwarding</a></li>
				<li><a href="HTrigger.asp">Port Triggering</a></li>
			</ul>
		</div>
	</body>
</html>
