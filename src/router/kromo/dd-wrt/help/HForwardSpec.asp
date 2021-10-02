<% do_hpagehead("pforward.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("pforward.h2"); %></h2>
			<dl>
				<% tran("hpforward.page1"); %>
				<% tran("bmenu.applicationsprforwarding"); %>				<% tran("hpforward.page2"); %>
				<dt><% tran("pforward.app"); %></dt>
				<% tran("hpforward.page3"); %>
				<dt><% tran("share.proto"); %></dt>
				<% tran("hpforward.page4"); %>
				<dt><% tran("pforward.src"); %></dt>
				<% tran("hpforward.page5"); %>
				<dt><% tran("pforward.from"); %></dt>
				<% tran("hpforward.page6"); %>
				<dt><% tran("share.ip"); %></dt>
				<% tran("hpforward.page7"); %>
				<dt><% tran("pforward.to"); %></dt>
				<% tran("hpforward.page8"); %>
				<dt><% tran("share.enable"); %></dt>
				<% tran("hpforward.page9"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
				<li><a href="HUPnP.asp"><% tran("bmenu.applicationsUpnp"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
