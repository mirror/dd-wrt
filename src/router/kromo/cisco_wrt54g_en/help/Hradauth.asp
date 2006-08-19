<% do_hpagehead(); %>
		<title>Help - Radius Authentification</title>
	</head>
	<body>
	<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Radius Authentification</h2>
			<dl>
				<dd>RADIUS (Remote Authentication Dial-In User Service) is a security service for authenticating and authorizing dial-up users. A typical enterprise network may have an access server attached to a modem pool, along with a RADIUS server to provide authentication services. Remote users dial into the access server, and the access server sends authentication requests to the RADIUS server. The RADIUS server authenticates users and authorizes access to internal network resources. Remote users are clients to the access server and the access server is a client to the RADIUS server.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>RADIUS is only available in <em>AP</em> mode.</div>

					</div>
				</dd>
				<dt>MAC Format</dt>
				<dd>When sending the authentication request to the RADIUS server, the wireless client use the MAC address as the username. This would be received by the RADIUS server in the following format :
					<ul class="wide">
						<li>aabbcc-ddeeff</li>
						<li>aabbccddeeff</li>
						<li>aa-bb-cc-dd-ee-ff</li>
					</ul>
				</dd>				
				<dt>Radius Server IP and Port</dt>
				<dd>The radius server IP address and TCP port.</dd>
				<dt>Maximum Unauthenticated Users</dt>
				<dd>Sets a amount of users which ran access without any valid radius authentifaction</dd>
				<dt>Password Format</dt>
				<dd>Sets the property which radius password should be used, the shared key or the mac adress itself</dd>
				<dt>RADIUS Shared Secret</dt>
				<dd>Transactions between the client and RADIUS accounting server are authenticated through the use of a shared secret, which is never sent over the network.</dd>
				<dt>Override Radius if Server is unavailable</dt>
				<dd>If the Radius server becomes unavailable, the radius authentication will be disabled until it becoms reachable again. This allows wireless remote administration of a Access Point in fail scenarios.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="HWPA.asp">Wireless Security</a></li>
				<li><a href="HWirelessAdvanced.asp">Advanced Wireless Settings</a></li>
			</ul>
		</div>
	</body>
</html>