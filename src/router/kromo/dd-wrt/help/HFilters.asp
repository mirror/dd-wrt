<% do_hpagehead("filter.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("filter.h2"); %></h2>
			<dl>
				<% tran("hfilter.pageintro"); %>
				<dt><% tran("filter.legend"); %></dt>
				<% tran("hfilter.page1"); %>
				<dt><% tran("sbutton.summary"); %></dt>
				<% tran("hfilter.page2"); %>
			</dl>
		</div>
	</body>
</html>
