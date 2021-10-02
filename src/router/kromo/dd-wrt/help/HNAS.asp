<% do_hpagehead("nas.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("nas.titl"); %></h2>
			<dl>
				
				<dt><% tran("nas.proftpd_srv"); %></dt>
				<% tran("hnas.page1"); %>
				<dt><% tran("nas.dlna_legend"); %></dt>
				<% tran("hnas.page2"); %>
				<dt><% tran("nas.samba3"); %></dt>
				<% tran("hnas.page3"); %>
				<% tran("hnas.page4"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
</html>
