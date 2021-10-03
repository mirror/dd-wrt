<% do_hpagehead("dmz.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("dmz.h2"); %></h2>
			<dl>
				<dt><% tran("hdmz.page1"); %></dt>
				<dt><% tran("dmz.host"); %></dt>
				<dt><% tran("hdmz.page2"); %></dt>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HFirewall.asp"><% tran("bmenu.firwall"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
			</ul>
		</div>
	</body>
</html>
