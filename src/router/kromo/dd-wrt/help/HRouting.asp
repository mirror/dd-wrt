<% do_hpagehead("route.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("route.h2"); %></h2>
			<dl>
				<% tran("hroute.page1"); %>
				<dt><% tran("route.mod"); %></dt>
				<% tran("hroute.page2"); %>
				<dt><% tran("route.gateway_legend"); %></dt>
				<% tran("hroute.page3"); %>
				<dt><% tran("route.static_legend"); %></dt>
				<% tran("hroute.page4"); %>
				<dt><% tran("sbutton.routingtab"); %></dt>
				<% tran("hroute.page5"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HStatusLan.asp"><% tran("bmenu.statuLAN"); %></a></li>
			</ul>
		</div>
	</body>
</html>
