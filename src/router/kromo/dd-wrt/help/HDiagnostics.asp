<% do_hpagehead("diag.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("diag.h2"); %></h2>
			<dl>
				<dt><% tran("hdiag.page1"); %></dt>
				<dt><% tran("diag.legend"); %></dt>
				<dt><% tran("hdiag.page2"); %></dt>
				<dt><% tran("diag.startup"); %></dt>
				<dt><% tran("hdiag.page3"); %></dt>
				<dt><% tran("diag.firewall"); %></dt>
				<dt><% tran("hdiag.page4"); %></dt>
				<dt><% tran("diag.custom"); %></dt>
				<dt><% tran("hdiag.page5"); %></dt>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
