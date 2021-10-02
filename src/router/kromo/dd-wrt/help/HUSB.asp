<% do_hpagehead("usb.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("usb.titl"); %></h2>
			<dl>
				
				<dt><% tran("usb.usb_core"); %></dt>
				<% tran("husb.page1"); %>
				<dt><% tran("usb.usb_printer"); %></dt>
				<% tran("husb.page2"); %>
				<dt><% tran("usb.usb_storage"); %></dt>
				<% tran("husb.page3"); %>
				<dt><% tran("usb.usb_automnt"); %></dt>
				<% tran("husb.page4"); %>
				<% tran("husb.page5"); %>
				<dt><% tran("usb.usb_diskinfo"); %></dt>
				<% tran("husb.page6"); %>
				

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
