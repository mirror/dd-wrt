<% do_hpagehead("bmenu.networking"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>Generic Networking and VLAN</h2>
			<dl>
				<dt><% tran("networking.h2"); %></dt>
				<!--dd>Allows you to transfer different independent network streams by using just one interface. This is done by inserting a small TAG within the ethernet header. By identifying this tag these ethernet packets can be split up again on the other side to create new interface out of it. Using this option together with the bridging option allows you to create different transfer networks which can be bridged to a Wireless Interface to separate the Router Management network from the network accessible by the User. This is useful for large ISP networks.</dd-->
				<% tran("hnetworking.page1"); %>
				<dt><% tran("networking.legend"); %></dt>
				<!--dd>Allows you to create a new VLAN interface out of a standard interface by filtering the interface using a defined TAG number.<dd-->
				<% tran("hnetworking.page2"); %>
				<dt><% tran("networking.h22"); %> - <% tran("networking.legend2"); %></dt>
				<!--dd>Creates a new empty network bridge for later use. STP means Spanning Tree Protocol and with PRIO you're able to set the bridge priority order. The lowest number has the highest priority.<dd-->
				<% tran("hnetworking.page3"); %>
				<dt><% tran("networking.h22"); %> - <% tran("networking.legend3"); %></dt>
				<!--dd>Allows you to assign any valid interface to a network bridge. Consider setting the Wireless Interface options to Bridged if you want to assign any Wireless Interface here. Any system specific bridge setting can be overridden here in this field. <dd>
				<dd>Click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes.</dd-->
				<% tran("hnetworking.page4"); %>
			</dl>
		</div>
	</body>
</html>
