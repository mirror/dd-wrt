<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Quality of Service (QoS)</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Quality of Service (QoS)</h2>
			<dl>
				<dd>Bandwidth management prioritizes the traffic on your router. Interactive traffic (telephony, browsing, telnet, etc.) gets priority and bulk traffic (file transfer, P2P) gets low priority. The main goal is to allow both types to live side-by side without unimportant trafic disturbing more critical things. All of this is more or less automatic.<br /><br />
					QoS allows control of the bandwidth allocation to different services, netmasks, MAC addresses and the four LAN ports. QoS is divided into four bandwidth classes called Premium, Express, Standard, and Bulk. Unclassified services will use the Standard bandwidth class.</dd>
				<dt>Port</dt>
				<dd>You must choose whether to apply bandwidth limits to the WAN device or the LAN &amp; wireless LAN device. (The LAN and wireless LAN ports are bonded internally into a single virtual device).</dd>
				<dt>Uplink / Downlink</dt>
				<dd>In order to use bandwidth management (QoS) you must enter bandwidth values for your uplink and downlink. These are generally 80% to 90% of your maximum bandwidth. </dd>
				<dt>Classification</td>
				<dd>Bandwidth classification based on the four categories will be enabled first on the hardware ports, then on MAC addresses, then netmasks and finally services. For example, if you enable classification based on a MAC address, this will override netmask and service classifications. However, the LAN port based classification will work together with MAC, netmask and service classifications, and will not override them.
					<ul class="wide">
						<li>Premium - The top bandwidth class. By default handshaking and icmp packets fall into this class. This class should be used sparingly. Occasionally VoIP service may be placed in this class so that voice receives top priority.</li>
						<li>Express - The Express class is for interactive applications that require bandwidth above standard services so that interactive apps run smoothly.</li>
						<li>Standard - All services that are not specifically classed will fall under the standard class.</li>
						<li>Bulk - The bulk class is only allocated bandwidth when the remaining classes are idle. Use this class for P2P services and downloading services like FTP.</li>
					</ul>
				</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click the <i>Cancel Changes</i> button to cancel your unsaved changes.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Remember to save your changes before adding another QoS rule.</div>
					</div>
				</dd>
			</dl>
		</div>
	</body>
</html>