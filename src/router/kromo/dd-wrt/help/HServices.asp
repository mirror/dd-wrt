<% do_hpagehead("service.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>Services</h2>
			<dl>
				
				<dt><% tran("service.dhcp_legend2"); %></dt>
				<% tran("hservice.page1"); %>
				
				<dt><% tran("service.dnsmasq_legend"); %></dt>
				<% tran("hservice.page2"); %>
				
				<dt><% tran("service.dnsmasq_dnssec"); %></dt>
				<% tran("hservice.dnsmasq_dnssec"); %>
				
				<dt><% tran("service.dnsmasq_dnssec_proxy"); %></dt>
				<% tran("hservice.dnsmasq_dnssec_proxy"); %>
				
				<dt><% tran("service.dnsmasq_dnssec_cu"); %></dt>
				<% tran("hservice.dnsmasq_dnssec_cu"); %>
				
				<dt><% tran("service.dnsmasq_no_dns_rebind"); %></dt>
				<% tran("hservice.dnsmasq_no_dns_rebind"); %>
				
				<dt><% tran("service.dnsmasq_strict"); %></dt>
				<% tran("hservice.dnsmasq_strict"); %>
				
				<dt><% tran("service.dnsmasq_add_mac"); %></dt>
				<% tran("hservice.dnsmasq_add_mac"); %>
				
				<dt><% tran("service.dnsmasq_rc"); %></dt>
				<% tran("hservice.dnsmasq_rc"); %>
				
				<dt><% tran("service.dnsmasq_cachesize"); %></dt>
				<% tran("hservice.dnsmasq_cachesize"); %>
				
				<dt><% tran("service.kaid_legend"); %></dt>
				<% tran("hservice.page3"); %>
				<% tran("service.kaid_locdevnum"); %>
				<% tran("hservice.page4"); %>
				<% tran("service.kaid_uibind"); %>
				<% tran("hservice.page5"); %>
				
				<dt><% tran("service.rflow_legend"); %></dt>
				<% tran("hservice.page6"); %>
				
				<dt><% tran("service.ssh_legend"); %></dt>
				<% tran("hservice.page7"); %>
				
				<dt><% tran("service.syslog_legend"); %></dt>
				<% tran("hservice.page8"); %>
				
				<dt><% tran("service.telnet_legend"); %></dt>
				<% tran("hservice.page9"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
