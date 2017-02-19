<% do_hpagehead("service.pptp_legend"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("service.pptp_legend"); %></h2>
			<dl>
				<dt><% tran("service.pptp_legend"); %> Server</dt>
				<!--dd>A VPN technology by Microsoft and remote access vendors, it is implemented in multiple OS's both desktop and mobile. Configuring this allows you to access your LAN at home remotely.
					<ul class="wide">
						<li>Server IP &ndash; The IP address of your router</li>
						<li>Client IP &ndash; A list or range of IP addresses for remotely connected machines. This range should not overlap with the DHCP range (for example 192.168.0.2,192.168.0.3), a range (192.168.0.1-254 or 192.168.0-255.2) or some combination (192.168.0.2,192.168.0.5-8).</li>
						<li>CHAP-Secrets &ndash; A list of usernames and passwords for the VPN login, one user per line (Example: joe * joespassword *). For more details look up the pppd main page.</li>
					</ul>
				</dd-->
				<% tran("hstatus_vpn.page1"); %>
				
				<dt><% tran("service.pptpd_legend"); %></dt>
				<!--dd>A VPN client that enables you to connect to VPN servers by Microsoft and remote access vendors. Configuring this allows the router to VPN into a remote network.
					<ul class="wide">
						<li>Server IP or DNS Name &ndash; The IP address or DNS Name of the VPN server that you would like to connect to (Example: www.MyServer.com). </li>
						<li>Remote Subnet &ndash; Remote Subnet of the network you are connecting to (Example: 192.168.2.0). </li>
						<li>Remote Subnet Mask &ndash; Remote Subnet Mask of the network you are connecting to (Example: 255.255.255.0). </li>
						<li>MPPE Encryption  &ndash; The type of security to use for the connection. If you are connecting to another DD-WRT router you need (Example: mppe required). But if you are connecting to a Windows VPN server you need (Example: mppe required,no40,no56,stateless) or (Example: mppe required,no40,no56,stateful) </li>
						<li>MTU &ndash; Maximum Transmission Unit (Default: 1450) </li>
						<li>MRU &ndash; Maximum Receiving Unit (Default: 1450) </li>
						<li>User Name &ndash; Enter the username that you will use to connect to the VPN server. If you are connecting to another Linux based PPTP server you just need to enter the username. But if you are connecting to a Windows VPN server you need to enter the servername and username (Example: DOMAIN\\UserName). </li>
						<li>Password &ndash; Enter the password of the for the username </li>
					</ul>
				</dd>

				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd-->
				<% tran("hstatus_vpn.page2"); %>
			
			</dl>
						
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
