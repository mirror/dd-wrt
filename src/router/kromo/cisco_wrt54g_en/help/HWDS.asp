<% do_hpagehead(); %>
		<title>Help - WDS</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>WDS</h2>
			<dl>
				<dd>WDS (Wireless Distribution System) is a Wireless Access Point mode that enables wireless bridging in which WDS APs communicate only with each other only (without allowing for wireless clients or stations to access them), and/or wireless repeating in which APs communicate both with each other and with wireless stations (at the expense of half the throughput). This firmware currently supports two types of WDS, LAN and Point to Point.</dd>
				<dt>LAN-type WDS</dt>
				<dd>This is the easiest, and currently most common, type of WDS used for linking LANs. It is very simple to setup and requires no extra routing protocols or knowledge of networking. Simply put, it is pure bridging. A simple example would be extending the range of an existing AP by setting up a 2nd AP and connecting it to the first using LAN-type WDS.
					<ol class="wide">
						<li>Make sure you are using the same <a href="HWireless.asp">Wireless Settings</a> on both routers and not any type of <a href="HWPA.asp">Wireless Security</a>.</li>
						<li>Find a drop-down selection that has <i>Disabled</i> displayed. Click this and select <i>LAN</i>, do the same on the other router.</li>
						<li>On the first router, take the numbers next to <i>Wireless MAC</i> and enter them in to the second router on the same line that you set to "LAN".</li>
						<li>Take the Wireless MAC from the second router and enter them on the first router.</li>
						<li>Check for any typing errors and then click <i>Save Settings</i>.</li>
						<li>Go to the <a href="HStatusWireless.asp">Wireless Status</a> page. You should see <i>WDS Link</i> and the Wireless MAC of the other router listed, with a signal reading. If the signal is "0dBm" then there may be something wrong. Check your antenna connections and configuration settings, and try again.</li>
						<li>Once you have a good signal (-70dBm to -30dBm, -70dBm being lowest), you can change the <i>Internet Connection Type</i> on the <a href="HSetup.asp">Basic Setup</a> page of the second router to <i>Disabled</i> and set the <em>Gateway</em> to the LAN IP Address of the first router. You can now run normal tests to check if you are connected (like <tt>ping</tt>).</li>
						<li>It is strongly recommended to enable <a href="HWPA.asp">Wireless Security</a>. WPA Pre-shared Key with AES is recommended as it is secure and easy.</li>
					</ol><br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>WDS is only available in <em>AP</em> mode. Also Wireless encryption <em>WPA2</em> and Wireless network mode <em>B-Only</em> are not supported under WDS.</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="HWPA.asp">Wireless Security</a></li>
				<li><a href="HStatusWireless.asp">Wireless Status</a></li>
			</ul>
		</div>
	</body>
</html>