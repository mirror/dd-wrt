<% do_hpagehead("service.pptp_legend"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("service.pptp_legend"); %></h2>
			<dl>
				<dd>PPTP Server Configuration</dd>
				
				<dt><% tran("share.srvip"); %><dt>
				<dd>Defines the Local IP of the PPTP Server. This should be almost the Local LAN IP.<dd>
				
				<dt><% tran("service.pptp_client"); %><dt>
				<dd>Must be a space separated list of IPs or a range of definition which will be used as pool for client ip assignment.<dd>
				<div class="note">
					<h4>Note</h4><br />
					<div>May be a list of IP addresses (for example 192.168.0.2,192.168.0.3), a range (for example 192.168.0.1-254 or 192.168.0-255.2) or some combination (for example 192.168.0.2,192.168.0.5-8).</div>
				</div>
				
				<dt><% tran("service.pptp_chap"); %><dt>
				<dd>Must be a list of secrets in standard linux CHAP secrets format.<dd>
				<div class="note">
					<h4>Note</h4><br />
					<div>myuser * mypassword *</div>
				</div>			
				
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
			
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
