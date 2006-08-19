<% do_hpagehead(); %>
		<title>Help - Advanced Routing</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Advanced Routing</h2>
			<dl>
				<dd>On the Routing screen, you can set the routing mode and settings of the router. <i>Gateway</i> mode is recommended for most users.</dd>
				<dt>Operating Mode</dt>
				<dd>Choose the correct working mode. Keep the default setting, Gateway, if the router is hosting your network's connection to the Internet. Select router if the router exists on a network with other routers.</dd>
				<dt>Dynamic Routing (RIP)</dt>
				<dd>Dynamic Routing enables the router to automatically adjust to physical changes in the network's layout and exchange routing tables with other routers. The router determines the network packets&#8217; route based on the fewest number of hops between the source and destination.<br /><br />
					To enable the Dynamic Routing feature for the WAN side, select <i>WAN</i>. To enable this feature for the LAN and wireless side, select <i>LAN &amp; WLAN</i>. To enable the feature for both the WAN and LAN, select <i>Both</i>. To disable the Dynamic Routing feature for all data transmissions, keep the default setting, <i>Disable</i>.<br /><br />
					<div class="note">
						<b>Note:</b><br />
						Dynamic Routing is not available in Gateway mode.
					</div>
				</dd>
				<dt>Static Routing</dt>
				<dd>A static route is a pre-determined pathway that network information must travel to reach a specific host or network.<br /><br />
					To set up a static route between the router and another network:
					<ol class="wide">
						<li>Select a number from the Static Routing drop-down list.</li>
						<li>Enter the following data:
							<ul>
								<li>Destination IP Address &ndash; The Destination IP Address is the address of the network or host to which you want to assign a static route.</li>
								<li>Subnet Mask &ndash; The Subnet Mask determines which portion of an IP address is the network portion, and which portion is the host portion.</li>
								<li>Gateway &ndash; This is the IP address of the gateway device that allows for contact between the router and the network or host.</li>
							</ul>
						</li>
						<li>Depending on where the Destination IP Address is located, select <i>LAN &amp; WLAN</i> or <i>WAN</i> from the Interface drop-down menu. </li>
						<li>Click the <i>Apply</i> button to save your changes. To cancel your unsaved changes, click the <i>Cancel</i> button. For additional static routes, repeat steps 1-4.</li>
					</ol><br />
					To delete a static route entry:
					<ol class="wide">
						<li>From the Static Routing drop-down list, select the entry number of the static route.</li>
						<li>Click the <i>Delete This Entry</i> button.</li>
						<li>To save a deletion, click the <i>Apply</i> button. To cancel a deletion, click the <i>Cancel</i> button.</li>
					</ol>
				</dd>
				<dt>Show Routing Table</dt>
				<dd>Click the <i>Show Routing Table</i> button to view all of the valid route entries in use. The following data will be displayed for each entry.
					<ul class="wide">
						<li>Destination IP Address &ndash; The Destination IP Address is the address of the network or host to which the static route is assigned.</li>
						<li>Subnet Mask &ndash; The Subnet Mask determines which portion of an IP address is the network portion, and which portion is the host portion.</li>
						<li>Gateway &ndash; This is the IP address of the gateway device that allows for contact between the router and the network or host.</li>
						<li>Interface &ndash; This interface tells you whether the Destination IP Address is on the LAN &amp; WLAN (internal wired and wireless networks), the WAN (Internet), or Loopback (a dummy network in which one PC acts like a network, necessary for certain software programs).</li>
					</ul><br />
					Click the <i>Refresh</i> button to refresh the data displayed. Click the <i>Close</i> button to return to the Routing screen.
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp">Management</a></li>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
			</ul>
		</div>
	</body>
</html>
