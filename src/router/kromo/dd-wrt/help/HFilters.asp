<% do_hpagehead("filter.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("filter.h2"); %></h2>
			<dl>
				<dd>This screen allows you to block or allow specific kinds of Internet usage. You can set up Internet access policies for specific PCs and set up filters by using network port numbers.</dd>
				<dt><% tran("filter.legend"); %></dt>
				<% tran("hfilter.page1"); %>
				<dt><% tran("sbutton.summary"); %></dt>
				<% tran("hfilter.page2"); %>
			</dl>
		</div>
	</body>
<% footer(); %></html>
