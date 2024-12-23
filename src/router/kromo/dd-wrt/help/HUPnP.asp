<% do_hpagehead("upnp.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("upnp.h2"); %></h2>
			<dl>
				<% tran("hupnp.page1"); %>
				<dt><% tran("upnp.legend"); %></dt>
				<% tran("hupnp.page2"); %>
				<dt><% tran("upnp.serv"); %></dt>
				<% tran("hupnp.page3"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
			</ul>
		</div>
	</body>
</html>
