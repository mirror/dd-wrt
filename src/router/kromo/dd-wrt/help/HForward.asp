<% do_hpagehead("prforward.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("prforward.h2"); %></h2>
			<dl>
				<dd>To add a new Port Range Forwarding rule, click <i>Add</i> and fill in the fields below. To remove the last rule, click <i>Remove</i>.</dd>
				<% tran("hprforward.page1"); %>
				<dt><% tran("prforward.app"); %></dt>
				<% tran("hprforward.page2"); %>
				<dt><% tran("share.start"); %></dt>
				<% tran("hprforward.page3"); %>
				<dt><% tran("share.end"); %></dt>
				<% tran("hprforward.page4"); %>
				<dt><% tran("share.proto"); %></dt>
				<% tran("hprforward.page5"); %>
				<dt><% tran("share.ip"); %></dt>
				<% tran("hprforward.page6"); %>
				<dt><% tran("share.enable"); %></td>
				<% tran("hprforward.page7"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HUPnP.asp"><% tran("bmenu.applicationsUpnp"); %></a></li>
				<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
