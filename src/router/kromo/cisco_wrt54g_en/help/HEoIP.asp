<% do_hpagehead(); %>
		<title>Help - EoIP</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("service.eoip_legend"); %></h2>
			<dl>
				
				<dt><% tran("service.eoip_legend"); %></dt>
				<dd>Ethernet over IP (EoIP) Tunneling enable you to create an Ethernet tunnel between two routers on top of an IP connection. The EoIP interface appears as an Ethernet interface. When the bridging function of the router is enabled, all Ethernet traffic (all Ethernet protocols) will be bridged just as if there where a physical Ethernet interface and cable between the two routers (with bridging enabled).<br/>Network setups with EoIP interfaces : <br/>
					<ul>
						<li>Possibility to bridge LANs over the Internet</li>
						<li>Possibility to bridge LANs over encrypted tunnels</li>
						<li>Possibility to bridge LANs over 802.11b 'ad-hoc' wireless networks</li>
					</ul>
				</dd> 
				
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
