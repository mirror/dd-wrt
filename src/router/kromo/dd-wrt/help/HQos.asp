<% do_hpagehead("qos.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("qos.h2"); %></h2>
			<dl>
				<dd>Bandwidth management prioritizes the traffic on your router. Interactive traffic (telephony, browsing, telnet, etc.) gets priority and bulk traffic (file transfer, P2P) gets low priority. The main goal is to allow both types to live side-by side without unimportant trafic disturbing more critical things. All of this is more or less automatic.<br /><br />
					QoS allows control of the bandwidth allocation to different services, netmasks, MAC addresses and the four LAN ports, LAN port availability will vary by router, if its not displayed, your hardware does not support it. QoS is divided into five bandwidth classes called Maximum, Premium, Express, Standard, and Bulk. Unclassified services will use the Standard bandwidth class.</dd>
				
				<dt><% tran("share.port"); %></dt>
				<dd>You must choose whether to apply QoS to the WAN, or the LAN &amp; WLAN port. (LAN and WLAN ports are bonded internally into a single virtual device). Most should select WAN for this option.</dd>
				
				<dt><% tran("qos.type"); %></dt>
				<dd>
					<ul class="wide">
						<li>HFSC - Hierarchical Fair Service Curve. Queues attached to an interface build a tree, thus each queue can have further child queues. Each queue can have a priority and a bandwidth assigned. Priority controls the time packets take to get sent out, while bandwidth effects throughput. HFSC is a little more resource demanding than that of HTB. </li>
						<li>HTB - Hierarchical Token Bucket, it is a faster replacement for the CBQ qdisc in Linux and is less resource demanding than HFSC. HTB helps in controlling the use of the outbound bandwidth on a given link. HTB allows you to use one physical link to simulate several slower links and to send different kinds of traffic on different simulated links. HTB is useful for limiting a client's download/upload rates, preventing their monopolization of the available bandwidth.</li>
					</ul>
				</dd>
				
				<dt><% tran("qos.uplink"); %> / <% tran("qos.dnlink"); %></dt>
				<dd>In order to use QoS you must enter bandwidth values for your uplink and downlink. These are generally 80% to 95% of your maximum bandwidth. If you only want QoS to apply to uplink bandwidth, enter 0 (no limit) for downlink. Do not enter 0 for uplink. </dd>
				
				<dt><% tran("share.priority"); %></td>
				<dd>Bandwidth classification based on the four categories will be enabled first on the hardware ports, then on MAC addresses, then netmasks and finally services. For example, if you enable classification based on a MAC address, this will override netmask and service classifications. However, the LAN port based classification will work together with MAC, netmask and service classifications, and will not override them.
					<ul class="wide">
						<li>Maximum - (60% - 100%) This class offers maximum priority and should be used sparingly.</li>
						<li>Premium - (25% - 100%) Second highest bandwidth class, by default handshaking and ICMP packets fall into this class. Most VoIP and video services will function good in this class if Express is insufficient.</li>
						<li>Express - (10% - 100%) The Express class is for interactive applications that require bandwidth above standard services so that interactive apps run smoothly.</li>
						<li>Standard - (5% - 100%) All services that are not specifically classed will fall under standard class.</li>
						<li>Bulk - (1% - 100%) The bulk class is only allocated remaining bandwidth when the remaining classes are idle. If the line is full of traffic from other classes, Bulk will only be allocated 1% of total set limit. Use this class for P2P and downloading services like FTP.</li>
					</ul>
				</dd>
				<dd>Check all values and click <i>Save</i> to save your settings without taking effect, or click <i>Apply Settings</i> to permanently save your changes taking effect immediately. Clicking the <i>Cancel Changes</i> button will cancel your unsaved changes.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Remember to save your changes before adding another QoS rule.</div>
					</div>
				</dd>
			</dl>
		</div>
	</body>
</html>
