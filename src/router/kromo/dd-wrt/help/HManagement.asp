<% do_hpagehead("management.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("management.h2"); %></h2>
			<dl>
				<% tran("hmanagement.page1"); %>
				
				<dt><% tran("management.psswd_pass"); %></dt>
				<% tran("hmanagement.page2"); %>
				
				<dt><% tran("management.remote_legend"); %></dt>
				<% tran("hmanagement.page3"); %>
				<% tran("bmenu.servicesServices"); %>
				<% tran("hmanagement.page4"); %>
				
				<dt><% tran("management.web_legend"); %></dt>
				<% tran("hmanagement.page5"); %>
				
				<dt><% tran("management.boot_legend"); %></dt>
				<% tran("hmanagement.page6"); %>
				
				<dt><% tran("management.cron_legend"); %></dt>
				<% tran("hmanagement.page7"); %>
				
				<dt><% tran("management.loop_legend"); %></dt>
				<% tran("hmanagement.page8"); %>
				
				<dt><% tran("management.wifi_legend"); %></dt>
				<% tran("hmanagement.page9"); %>
				
				<dt><% tran("management.rst_legend"); %></dt>
				<% tran("hmanagement.page10"); %>
				
				<dt><% tran("management.routing_legend"); %></dt>
				<% tran("hmanagement.page11"); %>
				
				<dt><% tran("management.net_legend"); %></dt>
				<% tran("hmanagement.page12"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a></li>
			</ul>
		</div>
	</body>
</html>
