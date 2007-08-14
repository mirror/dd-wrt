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
				<dt>Server IP<dt>
				<dd>Defines the Local IP of the PPTP Server. this should be almost the Local LAN IP<dd>
				<dt>Client IP(s)<dt>
				<dd>Must be a space separated list of IPs or a range of definition which will be used as pool for client ip assignment<dd>
				<div class="note">
					<h4>example:</h4>
					<div>192.168.1.2-99</div>
				</div>
				<dt>CHAP-Secrets<dt>
				<dd>Must be a list of secrets in standard linux CHAP secrets format<dd>
				<div class="note">
					<h4>example:</h4>
					<div>myuser * mypassword *</div>
				</div>			
				<dt><% tran("service.pptp_legend"); %></dt>
				<dd>Click <i>Save Settings</i> to save your settings or click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
