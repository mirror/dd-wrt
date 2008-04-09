<% do_hpagehead("status_wireless.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("status_wireless.h2"); %></h2>
			<dl>
				<dd>This status screen displays the router's wireless status and configuration. All information is read-only.</dd>
				
				<dt><% tran("share.mac"); %></dt>
				<dd>The MAC Address of the wireless interface is displayed here.</dd>
				
				<dt><% tran("share.mode"); %></dt>
				<dd>The Mode of the wireless network is displayed here.</dd>
				
				<dt><% tran("share.ssid"); %></dt>
				<dd>The SSID of the wireless network is displayed here.</dd>
				
				<dt><% tran("share.channel"); %></dt>
				<dd>The channel of the wireless network is displayed here.</dd>
				
				<dt><% tran("wl_basic.TXpower"); %></dt>
				<dd>The transfer power of the wireless device is displayed here.</dd>
				
				<dt><% tran("share.rates"); %></dt>
				<dd>The current wireless transfer rate is displayed here.</dd>
				
				<dt><% tran("share.encrypt"); %></dt>
				<dd>The status of the encryption is displayed here.</dd>
				
				<dd>Click the <i>Survey</i> button to show all wireless networks in your neighbourhood reachable by your router.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">Router Status</a></li>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
			</ul>
		</div>
	</body>
</html>