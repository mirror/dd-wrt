<% do_hpagehead(); %>
		<title>Help - Management</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("management.h2"); %></h2>
			<dl>
				<dd>The Management screen allows you to change the router's settings. On this page you will find most of the configurable items of the DD-WRT router code.</dd>
				
				<dt><% tran("management.psswd_pass"); %></dt>
				<dd>The new password must not exceed 32 characters in length and must not include any spaces. Enter the new password a second time to confirm it.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Default username is <tt>root</tt><br/>
						It is strongly recommended that you change the factory default password of the router, which is <tt>admin</tt>. All users who try to access the router's web-based utility or Setup Wizard will be prompted for the router's password.</div>
					</div>
				</dd>
				
				<dt><% tran("management.remote_legend"); %></dt>
				<dd>This feature allows you to manage the router from a remote location, via the Internet. To disable this feature, keep the default setting, <em>Disable</em>. To enable this feature, select <em>Enable</em>, and use the specified port (default is 8080) on your PC to remotely manage the router. You must also change the router's default password to one of your own, if you haven't already.<br /><br />
					To remotely manage the router, enter <tt>http://xxx.xxx.xxx.xxx:8080</tt> (the x's represent the router's Internet IP address, and 8080 represents the specified port) in your web browser's address field. You will be asked for the router's password.<br /><br />
					If you use https you need to specify the url as <tt>https://xxx.xxx.xxx.xxx:8080</tt> (not all DD-WRT firmwares does support this without rebuilding with SSL support).<br /><br />
					You can also enable <em>SSH</em>&nbsp; to remotely access the router by Secure Shell. Note that SSH daemon needs to be enable in <a href="HServices.asp"><% tran("bmenu.adminServices"); %></a> page.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>If the Remote Router Access feature is enabled, anyone who knows the router's Internet IP address and password will be able to alter the router's settings.</div>
					</div>
				</dd>
				
				<!--<dt>AP Watchdog</dt>
				<dd>The AP Watchdog enables a timer that will check to see if any clients are connected in the interval seconds given. If no clients are attached, the watchdog assumes the AP needs to be reset. When clients are connected this reset will not occur. The watchdog is intended for situations where the AP becomes unavailable due to interference or internal chip problems.</dd>-->
				
				<dt><% tran("management.web_legend"); %></dt>
				<dd>This feature allows you to manage the router using either HTTP protocol or the HTTPS protocol. If you choose to disable this feature, a manual reboot will be required.<br/>
				You can also activate or not the router information web page. It's now possible to password protect this page (same username and password than above).<br/>
				MAC Masquerading allows you to truncate MAC addresses in the Webinterface.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>If MAC Masquerading is enabled, all the MAC addresses will be posted in this format: xx:xx:xx:xx:AA:BB. MAC masquerading only applies to the Sys-Info page.</div>
					</div>
				</dd>
				
				<dt><% tran("management.boot_legend"); %></dt>
				<dd>Boot Wait is a feature you will hopefully never need. It introduces a short delay while booting (5s). During this delay you can initiate the download of a new firmware if the one in the flash rom is not broken. Obviously this is only necessary if you can no longer reflash using the web interface because the installed firmware will not boot. See the DD-WRT documentation for more information.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>It is recommended that you enable the Boot Wait feature. This will help you recover in the future should you flash your router improperly.</div>
					</div>
				</dd>
				
				<dt><% tran("management.cron_legend"); %></dt>
				<dd>The cron subsystem schedules execution of Linux commands. You'll need to use the command line or startup scripts to actually use this.</dd>
				
				<dt><% tran("management.loop_legend"); %></dt>
				<dd>Enable / disable the loopback interface. The loopback interface makes your internal clients appear as if they are external. This is useful for testing things like DynDNS names. The loopback is an option because enabling it will break PPTP and Windows machine browsing by wireless clients.</dd>
				
				<dt><% tran("management.wifi_legend"); %></dt>
				<dd>A limited 802.1x server needed to fulfill WPA handshake requirements to allow Windows XP clients to work with WPA.</dd>
				
				<dt><% tran("management.ntp_legend"); %></dt>
				<dd>Synchronize the clock of the router with an NTP time server.</dd>
				
				<dt><% tran("management.rst_legend"); %></dt>
				<dd>This feature controls the resetbuttond process. The reset button initiates actions depending on how long you press it.
					<ul>
						<li>Short press &ndash; Reset the router (reboot)</li>
						<li>Long press (&gt;5s) &ndash; Reboot and restore the factory default configuration.</li>
					</ul>
				</dd>
				
				<dt><% tran("management.routing_legend"); %></dt>
				<dd>Routing enables the OSPF and RIP routing daemons if you have set up OSPF or RIP routing in the Advanced Routing page.</dd>
				
				<dt><% tran("management.net_legend"); %></dt>
				<dd>If you have any peer-to-peer (P2P) applications running on your network please increase the maximum ports and lower the TCP/UDP timeouts. This is necessary to maintain router stability because peer-to-peer applications open many connections and don't close them properly. Consider using these:
					<ul>
						<li>Maximum Ports: 4096</li>
						<li>TCP Timeout: 120 sec</li>
						<li>UDP Timeout: 120 sec</li>
					</ul>
				</dd>
				<dd>Check all values and click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes. Click <em>Reboot router</em> to reboot your router immediately.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HServices.asp"><% tran("bmenu.adminServices"); %></a></li>
			</ul>
		</div>
	</body>
</html>
