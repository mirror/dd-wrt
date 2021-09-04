<% do_hpagehead("wpa.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>Wireless Security</h2>
			<dl>
				<% tran("hwpa.page1"); %>
				<dt>WPA Personal</dt>
				<% tran("hwpa.page2"); %>
				<dt>WPA Enterprise</dt>
				<% tran("hwpa.page3"); %>
				<dt>WPA2 Personal</dt>
				<% tran("hwpa.page4"); %>
				<dt>WPA2 Personal Mixed</dt>
				<% tran("hwpa.page5"); %>
				<dt>RADIUS</dt>
				<% tran("hwpa.page6"); %>
				<dt>WEP</dt>
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
