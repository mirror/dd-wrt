<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Wireless Settings</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Wireless Settings</h2>
			<dl>
				<dt>Wireless Mode</dt>
				<dd>The wireless part of your router can run in different modes:
					<ul class="wide">
						<li>AP mode &ndash; This is the default mode, also called Infrastructure mode. Your router acts as an central connection point, which wireless clients can connect to.</li>
						<li>Client mode &ndash; This mode turns the router into a wireless client (like a wirless PCI card or PCMCIA card). This in turn connects to a Wireless access point. The client is isolated from other Wireless clients. Also only one ethernet device can be connected though it could be a router in AP mode.</li>
						<li>Client Bridged mode &ndash; Wireless Bridging is used to connect 2 LAN segments via a wireless link. The 2 segments will be in the same subnet and looks like 2 ethernet switches connected by a cable, to all computers on the subnet. Since the computers are on the same subnet, broadcasts will reach all machines allowing DHCP clients in one segment to get their addresses from a DHCP server in a different segment. You could use a Wireless Bridge to transparently connect computer(s) in one room to computer(s) in a different room when you could not, or did not want to run an ethernet cable between the rooms.</li>
						<li>Ad-Hoc mode &ndash; This is for peer to peer wireless connections. Clients running in Ad-Hoc mode can connect to each other as required without involving central access points.</li>
					</ul><br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Note that <a href="HWDS.asp">WDS</a> is only available in AP mode.</div>
					</div>
				</dd>
				<dt>Wireless Network Mode</dt>
				<dd>If you have Wireless-G and 802.11b devices in your network, then keep the default setting, <i>Mixed</i>. If you have only Wireless-G devices, select <i>G-Only</i>. If you would like to limit your network to only 802.11b devices, then select <i>B-Only</i>. If you want to disable wireless networking, select <i>Disable</i>. Note that <i>B-Only</i> mode is not supported under WDS. 
</dd>
				<dt>Wireless Network Name (SSID)</dt>
				<dd>The SSID is the network name shared among all devices in a wireless network. The SSID must be identical for all devices in the wireless network. It is case-sensitive and must not exceed 32 alphanumeric characters, which may be any keyboard character. Make sure this setting is the same for all devices in your wireless network.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>For added security, it is recommended to change the default SSID <tt>linksys</tt> to a unique name of your choice.</div>
					</div>
				</dd>
				<dt>Wireless Channel</dt>
				<dd>Select the appropriate channel from the list provided to correspond with your network settings (in North America between channel 1 and 11, in Europe 1 and 13, in Japan all 14 channels). All devices in your wireless network must use the same channel in order to function correctly. Try to avoid conflicts with other wireless networks by choosing a channel where the upper and lower three channels are not in use.</dd>
				<dt>Wireless SSID Broadcast</dt>
				<dd>When wireless clients survey the local area for wireless networks to associate with, they will detect the SSID broadcast by the router. To broadcast the router SSID, keep the default setting, <i>Enable</i>. If you do not want to broadcast the router SSID, then select <i>Disable</i>.</dd>
				<dt>Sensitivity Range</dt>
				<dd>Adjusts the ack timing in Atheros typical way based on the maximum distance in meters
					<ul class="wide">
						<li> 0 disables ack timing completely</li>
						<li> 1 - 999999 adjusts ack timing</li>
					</ul>
				</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HWPA.asp">Wireless Security</a></li>
				<li><a href="HWirelessAdvanced.asp">Advanced Wireless Settings</a></li>
			</ul>
		</div>
	</body>
</html>