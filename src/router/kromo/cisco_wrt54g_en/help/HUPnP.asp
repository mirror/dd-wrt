<% do_hpagehead(); %>
		<title>Help -  UPnP</title>
	</head>
	
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Universal Plug and Play (UPnP)</h2>
			<dl>
				<dd>Universal Plug and Play (UPnP) is a set of computer network protocols. This Microsoft technology is for automatic configuration of devices. The goals of UPnP are to allow devices to connect seamlessly and to simplify the implementation of networks in the home and corporate environments. UPnP achieves this by defining and publishing UPnP device control protocols built upon open, Internet-based communication standards.</dd>
				<dt>Forwards</dt>
			  <dd>The UPnP forwards table shows all open ports forwarded automatically by the UPnP process. You can delete forwards by clicking the trash can or click the <em>Delete All</em> button to clear all forwards.</dd>
				<dt>UPnP Service</dt>
				<dd>Allows applications to automatically setup port forwardings.<br/><br/>
				</dd>
				<dt>Clear port forwards at startup</dt>
				<dd>If enabled, all UPnP port forwardings are deleted when the router starts up.</dd>
				<dt>Send presentation URL</dt>
				<dd>If enabled, a presentation url tag is sent with the device description. This allows the router to show up in <em>Windows's My Network Places</em>. <br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>When enabling this option you may need to reboot your PC.</div>
					</div>
				</dd>
				<dd>Click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
			</ul>
		</div>
	</body>
</html>
