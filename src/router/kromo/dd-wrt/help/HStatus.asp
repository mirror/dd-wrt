<% do_hpagehead("status_router.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("bmenu.statu"); %></h2>
			<dl>
				<% tran("hstatus_router.page1"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatusLan.asp"><% tran("status_lan.titl"); %></a></li>
				<li><a href="HStatusWireless.asp"><% tran("status_wireless.titl"); %></a></li>
			</ul>
		</div>
	</body>
</html>
