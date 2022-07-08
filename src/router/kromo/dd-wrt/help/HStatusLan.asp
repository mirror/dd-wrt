<% do_hpagehead("status_lan.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("status_lan.h2"); %></h2>
			<dl>
				<% tran("hstatus_lan.page1"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp"><% tran("status_router.titl"); %></a></li>
				<li><a href="HStatusWireless.asp"><% tran("status_wireless.titl"); %></a></li>
			</ul>
		</div>
	</body>
</html>
