<% do_hpagehead("upnp.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("upnp.h2"); %></h2>
			<dl>
				<!--dd>Universal Plug and Play (UPnP) is a set of computer network protocols. This Microsoft technology is for automatic configuration of devices. The goals of UPnP are to allow devices to connect seamlessly and to simplify the implementation of networks in the home and corporate environments. UPnP achieves this by defining and publishing UPnP device control protocols built upon open, Internet-based communication standards.</dd-->
				<% tran("hupnp.page1"); %>
				
				<dt><% tran("upnp.legend"); %></dt>
				<!--dd>The UPnP forwards table shows all open ports forwarded automatically by the UPnP process. You can delete forwards by clicking the trash can or click the <em>Delete All</em> button to clear all forwards.</dd-->
				<% tran("hupnp.page2"); %>
				
				<dt><% tran("upnp.serv"); %></dt>
				<!--dd>Allows applications to automatically setup port forwardings.</dd-->
				<% tran("hupnp.page3"); %>
				
				<dt><% tran("upnp.clear"); %></dt>
				<!--dd>If enabled, all UPnP port forwardings are deleted when the router starts up.</dd-->
				</div>
				</dd>
				
				<dd>Click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd-->
				<% tran("hupnp.page5"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
			</ul>
		</div>
	</body>
</html>
