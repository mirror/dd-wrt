<% do_hpagehead(); %>
		<title>Help - Wireless Security</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Wireless Security</h2>
			<dl>
				<dd>The router supports different types of security settings for your network. Wi-Fi Protected Access (WPA) Pre-Shared key, WPA Remote Access Dial In User Service (RADIUS), RADIUS, and Wire Equivalence Protection (WEP), which can be selected from the list next to Security Mode. To disable security settings, keep the default setting, <i>Disable</i>.</dd>
				<dt>WPA Pre-Shared Key</dt>
				<dd>TKIP stands for Temporal Key Integrity Protocol, which utilizes a stronger encryption method than WEP, and incorporates Message Integrity Code (MIC) to provide protection against packet tampering. AES stands for Advanced Encryption System, which utilizes a symmetric 128-Bit block data encryption and MIC. You should choose AES if your wireless clients supports it.<br /><br />
					To use WPA Pre-Shared Key, enter a password in the <i>WPA Shared Key</i> field between 8 and 63 characters long. You may also enter a <i>Group Key Renewal Interval</i> time between 0 and 99,999 seconds.</dd>
				<dt>WPA RADIUS</dt>
				<dd>WPA RADIUS uses an external RADIUS server to perform user authentication. To use WPA RADIUS, enter the IP address of the RADIUS server, the RADIUS Port (default is 1812) and the shared secret from the RADIUS server.</dd>
				<dt>WPA2 Only</dt>
				<dd>WPA2 uses 802.11i to provide additional security beyond what is provided in WPA. AES is required under WPA2, and you may need additional updates to your OS and/or wireless drivers for WPA2 support. Please note WPA2/TKIP is not a supported configuration. Aditionally the WPA2 security mode is not supported under WDS.</dd>
				<dt>WPA2 Mixed</dt>
				<dd>This mode allows for mixing WPA2 and WPA clients. If only some of your clients support WPA2 mode, then you should choose WPA2 Mixed. For maximum interoperability, you should choose WPA2 Mixed/TKIP+AES.</dd>
				<dt>RADIUS</dt>
				<dd>RADIUS utilizes either a RADIUS server for authentication or WEP for data encryption. To utilize RADIUS, enter the IP address of the RADIUS server and its shared secret. Select the desired encryption bit (64 or 128) for WEP and enter either a passphrase or a manual WEP key.</dd>
				<dt>WEP</dt>
				<dd>There are two levels of WEP encryption, 64-bit (40-bit) and 128-bit. To utilize WEP, select the desired encryption bit, and enter a passphrase or up to four WEP key in hexadecimal format. If you are using 64-bit (40-bit), then each key must consist of exactly 10 hexadecimal characters. For 128-bit, each key must consist of exactly 26 hexadecimal characters. Valid hexadecimal characters are "0"-"9" and "A"-"F". Check your wireless clients to see which encryption level it supports.<br /><br />
					Use of WEP is discouraged due to security weaknesses, and one of the WPA modes should be used whenever possible. Only use WEP if you have clients that can only support WEP (usually older, 802.11b-only clients).
				</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click </i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="Hradauth.asp">Radius Authentification</a></li>
				<li><a href="HWirelessMAC.asp">Wireless MAC Filter</a></li>
				<li><a href="HWirelessAdvanced.asp">Advanced Wireless Settings</a></li>
			</ul>
		</div>
	</body>
</html>
