<% do_hpagehead("wpa.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("wpa.titl"); %></h2>
			<dl>
				<% tran("hwpa.page1"); %>
				<dt><% tran("wpa.psk"); %></dt>
				<% tran("hwpa.page2"); %>
				<dt><% tran("wpa.wpa"); %></dt>
				<% tran("hwpa.page3"); %>
				<dt><% tran("wpa.psk2"); %></dt>
				<% tran("hwpa.page4"); %>
				<dt>WPA2-PSK/WPA-PSK</dt>
				<% tran("hwpa.page5"); %>
				<dt><% tran("wpa.radius"); %></dt>
				<% tran("hwpa.page6"); %>
				<dt>WEP</dt>
				<% tran("hwpa.page7"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp"><% tran("sas.wireless_settings"); %></a></li>
				<li><a href="Hradauth.asp"><% tran("service.pppoesrv_radauth"); %></a></li>
				<li><a href="HWirelessMAC.asp"><% tran("wl_mac.h2"); %></a></li>
				<li><a href="HWirelessAdvanced.asp"><% tran("wl_adv.titl"); %></a></li>
			</ul>
		</div>
	</body>
</html>
