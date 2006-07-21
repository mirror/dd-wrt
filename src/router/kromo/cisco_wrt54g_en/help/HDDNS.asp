<% do_hpagehead(); %>
		<title>Help - Dynamic DNS (DDNS)</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Dynamic DNS (DDNS)</h2>
			<dl>
				<dd>The router offers a Dynamic Domain Name System (DDNS) feature. DDNS lets you assign a fixed host and domain name to a dynamic Internet IP address. It is useful when you are hosting your own website, FTP server, or other server behind the router. Before using this feature, you need to sign up for DDNS service at <a href="http://www.dyndns.org" target="_new">www.dyndns.org</a>, a DDNS service provider.</dd>

				<dt>DDNS Service</dt>
				<dd>To disable DDNS service, keep the default setting, <i>Disable</i>. To enable DDNS service, follow these instructions:
					<ol class="wide">
						<li>Sign up for DDNS service at <a href="http://www.dyndns.org" target="_new">www.dyndns.org</a>, and write down your User Name, Password, and Host Name information.</li>
						<li>On the DDNS screen, select <i>Enable</i>.</li>
						<li>Complete the <i>User Name</i>, <i>Password</i>, and <i>Host Name</i> fields.</li>
						<li>Click the <i>Save Settings</i> button to save your changes. Click the <i>Cancel Changes</i> button to cancel unsaved changes.</li>
					</ol><br />
					You can now access your router from the Internet with the domain you have chosen.
				</dd>
				
				<dt>Type</dt>
				<dd>The <em>Static</em> DNS service is similar to the <em>Dynamic</em> DNS service, in that it allows a hostname such as yourname.dyndns.org to point to your IP address. Unlike a <em>Dynamic</em> DNS host, a <em>Static</em> DNS host does not expire after 35 days without updates, but updates take longer to propagate though the DNS system.<br/>
				DynDNS' <em>Custom</em> DNS service provides a managed primary DNS solution, giving you complete control over an entire domain name and providing a unified primary/secondary DNS service. A web-based interface provides two levels of control over your domain, catering to average or power users.</dd>
				
				<dt>Wildcard</dt>
				<dd>Enabling the wildcard feature for your host causes *.yourhost.dyndns.org to be aliased to the same IP address as yourhost.dyndns.org. This feature is useful if you want to be able to use, for example, www.yourhost.dyndns.org and still reach your hostname</dd>

				<dt>Status</dt>
				<dd>The status of the DDNS service connection is displayed here.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HManagement.asp">Management</a></li>
				<li><a href="HStatus.asp">Router Status</a></li>
			</ul>
		</div>
	</body>
</html>