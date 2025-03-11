<% do_hpagehead("firewall.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("bmenu.firwall"); %></h2>
			<dl>
				<dt><% tran("firewall.proxy"); %></dt>
				<% tran("hfirewall.page1"); %>
				<dt><% tran("firewall.cookies"); %></dt>
				<% tran("hfirewall.page2"); %>
				<dt><% tran("firewall.applet"); %></dt>
				<% tran("hfirewall.page3"); %>
				<dt><% tran("firewall.activex"); %></dt>
				<% tran("hfirewall.page4"); %>
				<dt><% tran("firewall.ping"); %></dt>
				<% tran("hfirewall.page5"); %>
				<dt><% tran("firewall.portscan"); %></dt>
				<% tran("hfirewall.page14"); %>
				<dt><% tran("firewall.tarpit"); %></dt>
				<% tran("hfirewall.page15"); %>
				<dt><% tran("firewall.syncflood"); %></dt>
				<% tran("hfirewall.page16"); %>
				<dt><% tran("firewall.udpflood"); %></dt>
				<% tran("hfirewall.page17"); %>
				<dt><% tran("firewall.pod"); %></dt>
				<% tran("hfirewall.page18"); %>
				<dt><% tran("firewall.muticast"); %></dt>
				<% tran("hfirewall.page6"); %>
				<dt><% tran("filter.nat"); %></dt>
				<% tran("hfirewall.page7"); %>
				<dt><% tran("filter.port113"); %></dt>
				<% tran("hfirewall.page8"); %>
			</dl>
			<h2><% tran("log.h2"); %></h2>
			<dl>
				<% tran("hfirewall.page9"); %>
				<dt><% tran("log.legend"); %></dt>
				<% tran("hfirewall.page10"); %>
				<dt><% tran("log.lvl"); %></dt>
				<% tran("hfirewall.page11"); %>
				<dt><% tran("sbutton.log_in"); %></dt>
				<% tran("hfirewall.page12"); %>
				<dt><% tran("sbutton.log_out"); %></dt>
				<% tran("hfirewall.page13"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
