<% do_hpagehead("ddns.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("ddns.legend"); %></h2>
			<dl>
				<dt><% tran("hddns.page1"); %></dt>
				<dt><% tran("ddns.srv"); %></dt>
				<dt><% tran("hddns.page2"); %></dt>
				<dt><% tran("ddns.typ"); %></dt>
				<dt><% tran("hddns.page3"); %></dt>
				<dt><% tran("ddns.wildcard"); %></dt>
				<dt><% tran("hddns.page4"); %></dt>
				<dt><% tran("ddns.forceupd"); %></dt>
				<dt><% tran("hddns.page5"); %></dt>
				<dt><% tran("ddns.statu"); %></dt>
				<dt><% tran("hddns.page6"); %></dt>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
			</ul>
		</div>
	</body>
</html>
