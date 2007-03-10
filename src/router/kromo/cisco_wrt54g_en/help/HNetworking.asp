<% do_hpagehead(); %>
		<title>Help -  Networking</title>
	</head>
	
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Generic Networking and VLAN</h2>
			<dl>
				<dd>VLAN tagging allows you to transfer different independent network streams by using just one interface. This is done by inserting a small TAG within the ethernet header. By identifying this tag these ethernet packets can be split up again on the other side to create new interface out of it. Using this option together with the bridging option allows you to create different transfer networks which can be bridged to a Wireless Interface to separate the Router Management network from the network accessible by the User. This is useful for large ISP networks.</dd>
				<dt>Tagging</dt>
				<dd>Allows you to create a new VLAN interface out of a standard interface by filtering the interface using a defined TAG number.<dd>
				<dt>Bridging - Create Bridge</dt>
				<dd>Creates a new empty network bridge for later use. STP means Spanning Tree Protocol and with PRIO you're able to set the bridge priority order. The lowest number has the highest priority.<dd>
				<dt>Bridging - Assign to Bridge</dt>
				<dd>Allows you to assign any valid interface to a network bridge. Consider setting the Wireless Interface options to Bridged if you want to assign any Wireless Interface here. Any system specific bridge setting can be overridden here in this field. <dd>
				<dd>Click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
	</body>
</html>
