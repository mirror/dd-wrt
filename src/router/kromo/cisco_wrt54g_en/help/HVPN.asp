<% do_hpagehead(); %>
		<title>Help - VPN Passthrough</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>VPN Passthrough</h2>
			<dl>
				<dd>Virtual Private Networking (VPN) is typically used for work-related networking. For VPN tunnels, the router supports IPSec Passthrough, PPTP Passthrough and L2TP Passthrough.</dd>
				<dt>IPSec</dt>
				<dd>Internet Protocol Security (IPSec) is a suite of protocols used to implement secure exchange of packets at the IP layer. To allow IPSec tunnels to pass through the router, IPSec Passthrough is enabled by default. To disable IPSec Passthrough, select <i>Disable</i>. </dd>
				<dt>PPTP</dt>
				<dd>Point-to-Point Tunneling Protocol is the method used to enable VPN sessions to a Windows NT 4.0 or 2000 server. To allow PPTP tunnels to pass through the router, PPTP Passthrough is enabled by default. To disable PPTP Passthrough, select <i>Disable</i>.</dd>
				<dt>L2TP</dt>
				<dd>Layer Two (2) Tunneling Protocol, an extension to the PPP protocol that enables ISPs to operate Virtual Private Networks (VPNs). L2TP merges the best features of two other tunneling protocols: PPTP from Microsoft and L2F from Cisco Systems. To allow L2TP tunnels to pass through the router, L2TP Passthrough is enabled by default. To disable L2TP Passthrough, select <i>Disable</i>.</dd>
				<dd>Check all the values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div id="also">
			<b>See also:</b><br />
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
