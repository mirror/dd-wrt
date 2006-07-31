<% do_hpagehead(); %>
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
				<dd>The default value is 1. This value, between 1 and 255, indicates the interval of the Delivery Traffic Indication Message (DTIM). A DTIM field is a countdown field informing clients of the next window for listening to broadcast and multicast messages. When the router has buffered broadcast or multicast messages for associated clients, it sends the next DTIM with a DTIM Interval value. Its clients hear the beacons and awaken to receive the broadcast and multicast messages.</dd>
				<dt>Fragmentation Threshold</dt>
				<dd>This value should remain at its default setting of 2346. The range is 256-2346 bytes. It specifies the maximum size for a packet before data is fragmented into multiple packets. If you experience a high packet error rate, you may slightly increase the Fragmentation Threshold. Setting the Fragmentation Threshold too low may result in poor network performance. Only minor modifications of this value are recommended.</dd>
				<dt>RTS Threshold</dt>
				<dd>This value should remain at its default setting of 2347. The range is 0-2347 bytes. Should you encounter inconsistent data flow, only minor modifications are recommended. If a network packet is smaller than the preset RTS threshold size, the RTS/CTS mechanism will not be enabled. The router sends Request to Send (RTS) frames to a particular receiving station and negotiates the sending of a data frame. After receiving an RTS, the wireless station responds with a Clear to Send (CTS) frame to acknowledge the right to begin transmission.</dd>
				<dt>AP Isolation</dt>
				<dd>The default value is <i>Off</i>. This setting isolates wireless clients so access to and from other wireless clients are stopped.</dd>
				<dt>TX Antenna / RX Antenna</dt>
				<dd>Values are <i>Auto</i>, <i>Left</i>, <i>Right</i>, default value is <i>Auto</i>. This is used in conjunction with external antennas to give them optimum performance. On some router models left and right antennas may be reversed depending on you point of view.</dd>
				<dt>Preamble</dt>
				<dd>Values are <i>Long</i> and <i>Short</i>, default value is <i>Long</i>. If your wireless device supports the short preamble and you are having trouble getting it to communicate with other 802.11b devices, make sure that it is set to use the long preamble.</dd>
				<dt>Xmit Power</dt>
				<dd>This value ranges from 1 - 251 mw, default value is 28mw. A safe increase of up to 70 would be suitable for most users. Higher power settings are not recommended for users due to excess heat generated by the radio chipset, which can affect the life of the router.</dd>
				<dt>Afterburner</dt>
				<dd>The default value is <i>Off</i>. This should only be used with WRT54GS Models and only in conjunction with other Linksys "GS" wireless clients that also support Linksys "Speedbooster" technology.</dd>
				<dt>Wireless GUI Access</dt>
				<dd>The default value is <i>Enabled</i>. The setting allows access to the routers setup (GUI) from wireless clients. Disable this if you wish to block all wireless clients from accessing the setup pages.</dd>
				<dt>Radio Times Restrictions</dt>
				<dd>The <em>Radio Times Restriction</em> facility constitutes a time switch for the radio. By default, the time switch is not active and the WLAN is permanently on. Enable the time switch, if you want to turn off the WLAN during some hours of the day. Hours during which the WLAN is on are marked in green, while red indicates that the radio is off. Clicking on the respective hour toggles between on and off.</dd>
				<dt>WMM Support</dt>
				<dd>Enable support of Wi-Fi Multimedia feature. Configuring QoS options consists of setting parameters on existing queues for different types of wireless traffic. You can configure different minimum and maximum wait times for the transmission of packets in each queue based on the requirements of the media being sent. Queues automatically provide minimum transmission delay for Voice, Video, multimedia, and mission critical applications, and rely on best-effort parameters for traditional IP data.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>As an Example, time-sensitive Voice & Video, and multimedia are given effectively higher priority for transmission (lower wait times for channel access), while other applications and traditional IP data which are less time-sensitive but often more data-intensive are expected to tolerate longer wait times.</div>
					</div>
				</dd>
				<dt>No-Acknowledgement</dt>
				<dd>This refers to the acknowledge policy used at the MAC level. Enabling no-acknowledgement can result in more efficient throughput but higher error rates in a noisy Radio Frequency (RF) environment.</dd>
				<dt>EDCA AP Parameters (AP to Client)</dt>
				<dd>This affects traffic flowing from the access point to the client station.</dd>
				<dt>EDCA STA Parameters (Client to AP)</dt>
				<dd>This affects traffic flowing from the client station to the access point.</dd>
				<dt>Background</dt>
				<dd>Priority is low.<BR />
					High throughput. Bulk data that requires maximum throughput and is not time-sensitive is sent to this queue (FTP data, for example).</dd>
				<dt>Best Effort</dt>
				<dd>Priority is Medium.<BR />
					Medium throughput and delay. Most traditional IP data is sent to this queue.</dd>
				<dt>Video</dt>
				<dd>Priority is High.<BR />
					Minimum delay. Time-sensitive video data is automatically sent to this queue.</dd>
				<dt>Voice</dt>
				<dd>Priority is High.<BR />
					Time-sensitive data like VoIP and streaming media are automatically sent to this queue.</dd>
				<dt>CWmin</dt>
				<dd>Minimum Contention Window. This parameter is input to the algorithm that determines the initial random backoff wait time ("window") for retry of a transmission. The value specified here in the Minimum Contention Window is the upper limit (in milliseconds) of a range from which the initial random backoff wait time is determined.<BR />
					The first random number generated will be a number between 0 and the number specified here. If the first random backoff wait time expires before the data frame is sent, a retry counter is incremented and the random backoff value (window) is doubled. Doubling will continue until the size of the random backoff value reaches the number defined in the Maximum Contention Window. Valid values for the "cwmin" are 1, 3, 7, 15, 31, 63, 127, 255, 511, or 1024. The value for "cwmin" must be lower than the value for "CWmax".</dd>
				<dt>CWmax</dt>
				<dd>Maximum Contention Window. The value specified here in the Maximum Contention Window is the upper limit (in milliseconds) for the doubling of the random backoff value. This doubling continues until either the data frame is sent or the Maximum Contention Window size is reached. Once the Maximum Contention Window size is reached, retries will continue until a maximum number of retries allowed is reached. Valid values for the "cwmax" are 1, 3, 7, 15, 31, 63, 127, 255, 511, or 1024. The value for "cwmax" must be higher than the value for "CWmin".</dd>				
				<dt>AIFSN</dt>
				<dd>The Arbitration Inter-Frame Spacing Number specifies a wait time (in milliseconds) for data frames.</dd>
				<dt>TXOP(b) and TXOP (a/g)</dt>
				<dd>Transmission Opportunity for "a" "b" and "g" modes is an interval of time when a WME AP has the right to initiate transmissions onto the wireless medium (WM). This value specifies (in milliseconds) the Transmission Opportunity (TXOP) for AP; that is, the interval of time when the WMM AP has the right to initiate transmissions on the wireless network.</dd>
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