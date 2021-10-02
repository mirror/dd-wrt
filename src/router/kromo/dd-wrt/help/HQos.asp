<% do_hpagehead("qos.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("qos.h2"); %></h2>
			<dl>
				<% tran("hqos.page1"); %>
				<dt><% tran("share.port"); %></dt>
				<% tran("hqos.page2"); %>
				<dt><% tran("qos.type"); %></dt>
				<% tran("hqos.page3"); %>
				<dt><% tran("qos.uplink"); %> / <% tran("qos.dnlink"); %></dt>
				<% tran("hqos.page4"); %>
				<dt><% tran("share.priority"); %></td>
				<% tran("hqos.page5"); %>
			</dl>
		</div>
	</body>
</html>
