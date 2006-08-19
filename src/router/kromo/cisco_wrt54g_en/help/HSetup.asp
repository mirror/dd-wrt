<% do_hpagehead(); %>
		<title>Help - Basic Setup</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Basic Setup</h2>
			<dl>
				<dd>The Setup screen is the first screen you will see when accessing the router. Most users will be able to configure the router and get it working properly using only the settings on this screen. Some Internet Service Providers (ISPs) will require that you enter specific information, such as User Name, Password, IP Address, Default Gateway Address, or DNS IP Address. This information can be obtained from your ISP, if required.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>After you have configured these settings, you should set a new password for the router using the <a href="HManagement.asp">Management</a> screen. This will increase security, protecting the router from unauthorized changes. All users who try to access the router web-based utility or Setup Wizard will be prompted for the router's password.</div>
					</div>
				</dd>
				<dt>Host Name</dt>
				<dd>This entry is necessary for some ISPs and can be provided by them.</dd>
				<dt>Domain Name</dt>
				<dd>This entry is necessary for some ISPs and can be provided by them.</dd>
				<dt>Configuration Type</dt>
				<dd>The router supports four connection types:
					<ul>
						<li>Automatic Configuration DHCP</li>
						<li>Static IP</li>
						<li>PPPoE (Point-to-Point Protocol over Ethernet)</li>
						<li>PPTP (Point-to-Point Tunneling Protocol)</li>
					</ul><br />
					These types can be selected from the drop-down menu next to <i>Internet Connection</i>. The information required and available features will differ depending on what kind of connection type you select.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Some cable providers require a specific MAC address for connection to the Internet. To learn more about this, click the System tab. Then click the Help button, and read about the <a href="HWanMAC.asp">MAC Address Cloning</a> feature.</div>
					</div>
				</dd>
				<dt>Internet IP Address and Subnet Mask</dt>
				<dd>This is the router's IP Address and Subnet Mask as seen by external users on the Internet (including your ISP). If your Internet connection requires a static IP address, then your ISP will provide you with a Static IP Address and Subnet Mask.</dd>
				<dt>Default Gateway</dt>
				<dd>Your ISP will provide you with the Gateway IP Address.</dd>
				<dt>DNS (Domain Name Server)</dt>
				<dd>Your ISP will provide you with at least one DNS IP Address.</dd>
				<dt>User Name and Password</dt>
				<dd>Enter the User Name and Password you use when logging onto your ISP through a PPPoE or PPTP connection.</dd>
				<dt>Connect on Demand</dt>
				<dd>You can configure the router to disconnect your Internet connection after a specified period of inactivity (Max Idle Time). If your Internet connection has been terminated due to inactivity, Connect on Demand enables the router to automatically re-establish your connection as soon as you attempt to access the Internet again. If you wish to activate Connect on Demand, click the radio button. If you want your Internet connection to remain active at all times, enter 0 in the Max Idle Time field. Otherwise, enter the number of minutes you want to have elapsed before your Internet connection terminates.</dd>
				<dt>Keep Alive Option</dt>
				<dd>This option keeps you connected to the Internet indefinitely, even when your connection sits idle. To use this option, click the radio button next to <i>Keep Alive</i>. The default Redial Period is 30 seconds (in other words, the router will check the Internet connection every 30 seconds).</dd>
				<dt>MTU</dt>
				<dd>MTU is the Maximum Transmission Unit. It specifies the largest packet size permitted for Internet transmission. Keep the default setting, <i>Auto</i>, to have the router select the best MTU for your Internet connection. To specify a MTU size, select <i>Manual</i>, and enter the value desired (default is 1400). You should leave this value in the 1200 to 1500 range.</dd>
				<dt>IP Address and Subnet Mask</dt>
				<dd>This is the router IP Address and Subnet Mask as seen on the internal LAN. The default value is 192.168.1.1 for IP Address and 255.255.255.0 for Subnet Mask.</dd>
				<dt>DHCP Server</dt>
				<dd>Keep the default, <i>Enable</i>, to enable the router's DHCP server option. If you already have a DHCP server on your network or you do not want a DHCP server, then select <i>Disable</i>.</dd>
				<dt>Starting IP Address</dt>
				<dd>Enter a numerical value for the DHCP server to start with when issuing IP addresses. Do not start with 192.168.1.1 (the router's own IP address).</dd>
				<dt>Maximum Number of DHCP Users</dt>
				<dd>Enter the maximum number of PCs that you want the DHCP server to assign IP addresses to. The absolute maximum is 253, possible if 192.168.1.2 is your starting IP address.</dd>
				<dt>Client Lease Time</dt>
				<dd>The Client Lease Time is the amount of time a network user will be allowed connection to the router with their current dynamic IP address. Enter the amount of time, in minutes, that the user will be "leased" this dynamic IP address.</dd>
				<dt>Static DNS 1-3</dt>
				<dd>The Domain Name System (DNS) is how the Internet translates domain or website names into Internet addresses or URLs. Your ISP will provide you with at least one DNS Server IP address. If you wish to utilize another, enter that IP address in one of these fields. You can enter up to three DNS Server IP addresses here. The router will utilize these for quicker access to functioning DNS servers.</dd>
				<dt>WINS</dt>
				<dd>The Windows Internet Naming Service (WINS) manages each PC's interaction with the Internet. If you use a WINS server, enter that server's IP address here. Otherwise, leave this blank.</dd>
				<dt>Time Zone</dt>
				<dd>Select the time zone for your location. To use local time, leave the checkmark in the box next to <i>Use local time</i>.</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes. You can test the settings by connecting to the Internet.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWanMAC.asp">MAC Address Cloning</a></li>
				<li><a href="HManagement.asp">Management</a></li>
				<li><a href="HStatus.asp">Router Status</a></li>
			</ul>
		</div>
	</body>
</html>
