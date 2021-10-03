<% do_hpagehead("alive.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("alive.titl"); %></h2>
			<dl>
				<dt><% tran("alive.legend"); %></dt>
				<% tran("halive.page1"); %>
				<dt><% tran("alive.legend2"); %></dt>
				<% tran("halive.page2"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
