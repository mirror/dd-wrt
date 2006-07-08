<% do_hpagehead(); %>
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
				<dt>Packet Scheduler</dt>
				<dd>
					<ul class="wide">
						<li>HFSC - Hierarchical Fair Service Curve. Queues attached to an interface build a tree, thus each queue can have further child queues. Each queue can have a priority and a bandwidth assigned. Priority mainly controls the time packets take to get sent out, while bandwidth has primarily effects on throughput. </li>
						<li>HTB - Hierarchical Token Bucket, it is a faster replacement for the CBQ qdisc in Linux. HTB helps in controlling the use of the outbound bandwidth on a given link. HTB allows you to use one physical link to simulate several slower links and to send different kinds of traffic on different simulated links. In both cases, you have to specify how to divide the physical link into simulated links and how to decide which simulated link to use for a given packet to be sent. In other words, HTB is useful for limiting a client's download/upload rates, thereby preventing his monopolization of the available bandwidth.</li>
					</ul>
				</dd>
				<dt>Uplink / Downlink</dt>
				<dd>In order to use bandwidth management (QoS) you must enter bandwidth values for your uplink and downlink. These are generally 80% to 90% of your maximum bandwidth. </dd>
				<dt>Classification</td>
				<dd>Bandwidth classification based on the four categories will be enabled first on the hardware ports, then on MAC addresses, then netmasks and finally services. For example, if you enable classification based on a MAC address, this will override netmask and service classifications. However, the LAN port based classification will work together with MAC, netmask and service classifications, and will not override them.
					<ul class="wide">
						<li>Exempt - This class tries to keep the bandwith and packet flow untouched.</li>
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
