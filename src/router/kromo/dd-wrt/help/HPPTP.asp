<% do_hpagehead("service.pptp_legend"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("service.pptp_legend"); %></h2>
			<dl>
				<dt><% tran("service.pptp_legend"); %> Server</dt>
				<% tran("hstatus_vpn.page1"); %>
				
				<dt><% tran("service.pptpd_legend"); %></dt>
				<% tran("hstatus_vpn.page2"); %>
			
			</dl>
						
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
