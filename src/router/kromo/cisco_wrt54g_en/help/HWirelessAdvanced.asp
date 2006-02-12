<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Wireless Advanced Settings</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Advanced Wireless Settings</h2>
			<dl>
				<dd>The Wireless Advanced Settings screen allows you to customize data transmission settings. In most cases, the advanced settings on this screen should remain at their default values.</dd>
				<dt>Authentication Type</dt>
				<dd>The default is set to <i>Auto</i>, which allows either Open System or Shared Key authentication to be used. For Open System authentication, the sender and the recipient do NOT use a WEP key for authentication. For Shared Key authentication, the sender and recipient use a WEP key for authentication. If you want to use only Shared Key authentication, then select <i>Shared Key</i>.</dd>
				<dt>Basic Rate</dt>
				<dd>The default value is set to <i>Default</i>. Depending on the wireless mode you have selected, a default set of supported data rates will be selected. The default setting will ensure maximum compatibility with all devices. You may also choose to enable all data rates by selecting <i>ALL</i>. For compatibility with older Wireless-B devices, select <i>1-2Mbps</i>.</dd>
				<dt>Transmission Rate</dt>
				<dd>The default setting is <i>Auto</i>. The range is from 1 to 54Mbps. The rate of data transmission should be set depending on the speed of your wireless network. You can select from a range of transmission speeds, or keep the default setting, <i>Auto</i>, to have the router automatically use the fastest possible data rate and enable the Auto-Fallback feature. Auto-Fallback will negotiate the best possible connection speed between the router and a wireless client.</dd>
				<dt>CTS Protection Mode</dt>
				<dd>The default value is <i>Disabled</i>. When set to <i>Auto</i>, a protection mechanism will ensure that your Wireless-B devices will connect to the Wireless-G router when many Wireless-G devices are present. However, performance of your Wireless-G devices may be decreased.</dd>
				<dt>Frame Burst</dt>
				<dd>The default value is <i>Disabled</i>. Frame burst allows packet bursting which will increase overall network speed though this is only recommended for approx 1-3 wireless clients, Anymore clients and there can be a negative result and throughput will be affected.</dd>
				<dt>Beacon Interval</dt>
				<dd>The default value is 100. Enter a value between 1 and 65,535 milliseconds. The Beacon Interval value indicates the frequency interval of the beacon. A beacon is a packet broadcast by the router to synchronize the wireless network. 50 is recommended in poor reception.</dd>
				<dt>DTIM Interval</dt>
				<dd>The default value is 1. This value, between 1 and 255 milliseconds, indicates the interval of the Delivery Traffic Indication Message (DTIM). A DTIM field is a countdown field informing clients of the next window for listening to broadcast and multicast messages. When the router has buffered broadcast or multicast messages for associated clients, it sends the next DTIM with a DTIM Interval value. Its clients hear the beacons and awaken to receive the broadcast and multicast messages.</dd>
				<dt>Fragmentation Threshold</dt>
				<dd>This value should remain at its default setting of 2346. The range is 256-2346 bytes. It specifies the maximum size for a packet before data is fragmented into multiple packets. If you experience a high packet error rate, you may slightly increase the Fragmentation Threshold. Setting the Fragmentation Threshold too low may result in poor network performance. Only minor modifications of this value are recommended.</dd>
				<dt>RTS Threshold</dt>
				<dd>This value should remain at its default setting of 2347. The range is 0-2347 bytes. Should you encounter inconsistent data flow, only minor modifications are recommended. If a network packet is smaller than the preset RTS threshold size, the RTS/CTS mechanism will not be enabled. The router sends Request to Send (RTS) frames to a particular receiving station and negotiates the sending of a data frame. After receiving an RTS, the wireless station responds with a Clear to Send (CTS) frame to acknowledge the right to begin transmission.</dd>
				<dt>AP Isolation</dt>
				<dd>The default value is <i>Off</i>. This setting isolates wireless clients so access to and from other wireless clients are stopped.</dd>
				<dt>TX Antenna / RX Antenna</dt>
				<dd>Values are <i>Auto</i>, <i>Left</i>, <i>Right</i>, default value is <i>Auto</i>. This is used in conjunction with external antennas to give them optimum performance.</dd>
				<dt>Xmit Power</dt>
				<dd>This value ranges from 1 - 251 mw, default value is 28mw. A safe increase of up to 70 would be suitable for most users. Higher power settings are not recommended for users due to excess heat generated by the radio chipset, which can affect the life of the router.</dd>
				<dt>Afterburner</dt>
				<dd>The default value is <i>Off</i>. This should only be used with WRT54GS Models and only in conjunction with other Linksys "GS" wireless clients that also support Linksys "Speedbooster" technology.</dd>
				<dt>Wireless GUI Access</dt>
				<dd>The default value is <i>Enabled</i>. The setting allows access to the routers setup (GUI) from wireless clients. Disable this if you wish to block all wireless clients from accessing the setup pages.</dd>
				<dt>Preamble</dt>
				<dd>Values are <i>Long</i> and <i>Short</i>, default value is <i>Long</i>. If your wireless device supports the short preamble and you are having trouble getting it to communicate with other 802.11b devices, make sure that it is set to use the long preamble.</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your changes. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also:</h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="HWPA.asp">Wireless Security</a></li>
			</ul>
		</div>
	</body>
</html>