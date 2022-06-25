<% do_hpagehead("wl_basic.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("sas.wireless_settings"); %></h2>
			<dl>
				<dt><% tran("wl_basic.label"); %></dt>
				<% tran("hwl_basic.page1"); %>
				<dt><% tran("wl_basic.label2"); %></dt>
				<% tran("hwl_basic.page2"); %>
				<% tran("hwl_basic.page2_5g"); %>
				<dt><% tran("wl_basic.ssid"); %></dt>
				<% tran("hwl_basic.page3"); %>
				<dt><% tran("wl_basic.label4"); %></dt>
				<% tran("hwl_basic.page4"); %>
				<dt><% tran("wl_basic.label5"); %></dt>
				<% tran("hwl_basic.page5"); %>
				<% ifndef("ACK", "<!--"); %>
				<dt><% tran("wl_basic.label6"); %></dt>
				<% tran("hwl_basic.page6"); %>
				<% ifndef("ACK", "-->"); %>
				<% tran("hwl_basic.page7"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWPA.asp"><% tran("wpa.titl"); %></a></li>
				<li><a href="HWirelessAdvanced.asp"><% tran("wl_adv.titl"); %></a></li>
			</ul>
		</div>
	</body>
</html>
