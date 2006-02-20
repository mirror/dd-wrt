<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
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
				<dd>There are two encryption options for WPA Pre-Shared Key, TKIP and AES. TKIP stands for Temporal Key Integrity Protocol. TKIP utilizes a stronger encryption method and incorporates Message Integrity Code (MIC) to provide protection against hackers. AES stands for Advanced Encryption System, which utilizes a symmetric 128-Bit block data encryption.<br /><br />
					To use WPA Pre-Shared Key, enter a password in the <i>WPA Shared Key</i> field between 8 and 63 characters long. You may also enter a <i>Group Key Renewal Interval</i> time between 0 and 99,999 seconds.</dd>
				<dt>WPA RADIUS</dt>
				<dd>WPA RADIUS uses an external RADIUS server to perform user authentication. To use WPA RADIUS, enter the IP address of the RADIUS server, the RADIUS Port (default is 1812) and the shared secret from the RADIUS server.</dd>
				<dt>RADIUS</dt>
				<dd>RADIUS utilizes either a RADIUS server for authentication or WEP for data encryption. To utilize RADIUS, enter the IP address of the RADIUS server and its shared secret. Select the desired encryption bit (64 or 128) for WEP and enter either a passphrase or a manual WEP key.</dd>
				<dt>WEP</dt>
				<dd>There are two levels of WEP encryption, 64-bit and 128-bit. The higher the encryption bit, the more secure your network, however, speed is sacrificed at higher bit levels. To utilize WEP, select the desired encryption bit, and enter a passphrase or a WEP key in hexadecimal format.<br /><br />
					If you are using 64-bit, then each key must consist of exactly 10 hexadecimal characters in length. For 128-bit, each key must consist of exactly 26 hexadecimal characters. Valid hexadecimal characters are "0"-"9" and "A"-"F".<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>It has been found that WEP is not as secure as once believed. Therefore you should try to use WPA Pre-Shared Key first.</div>
					</div>
				</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click </i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="Hradauth.asp">Radius Authentification</a></li>
				<li><a href="HWirelessMAC.asp">Wireless MAC Filter</a></li>
				<li><a href="HWirelessAdvanced.asp">Advanced Wireless Settings</a></li>
			</ul>
		</div>
	</body>
</html>
