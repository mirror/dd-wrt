<% do_hpagehead(); %>
		<title>Help - Port Range Forwarding</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("prforward.h2"); %></h2>
			<dl>
				<dd>Port Range Forwarding allows you to set up public services on your network, such as web servers, ftp servers, e-mail servers, or other specialized Internet applications. Specialized Internet applications are any applications that use Internet access to perform functions such as videoconferencing or online gaming. When users send this type of request to your network via the Internet, the router will forward those requests to the appropriate PC.<br /><br />
					If you only want to forward a single port, see <a href="HForward.asp">Port Forwarding</a>.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Any PC whose port is being forwarded must have a static IP address assigned to it because its IP address may change when using the DHCP function.</div>
					</div>
				</dd>
				<dd>To add a new Port Range Forwarding rule, click <i>Add</i> and fill in the fields below. To remove the last rule, click <i>Remove</i>.</dd>
				
				<dt><% tran("prforward.app"); %></dt>
				<dd>Enter the name of the application in the field provided.</dd>
				
				<dt><% tran("share.start"); %></dt>
				<dd>Enter the number of the first port of the range you want to seen by users on the Internet and forwarded to your PC.</dd>
				
				<dt><% tran("share.end"); %></dt>
				<dd>Enter the number of the last port of the range you want to seen by users on the Internet and forwarded to your PC.</dd>
				
				<dt><% tran("share.proto"); %></dt>
				<dd>Chose the right protocol <i>TCP</i>, <i>UDP</i> or <i>Both</i>. Set this to what the application requires.</dd>
				
				<dt><% tran("share.ip"); %></dt>
				<dd>Enter the IP Address of the PC running the application.</dd>
				
				<dt><% tran("share.enable"); %></td>
				<dd>Click the <i>Enable</i> checkbox to enable port forwarding for the application.</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click the <i>Cancel Changes</i> button to cancel your unsaved changes.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Remember to save your changes before adding another forwarding rule.</div>
					</div>
				</dd>
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
