<% do_hpagehead(); %>
		<title>Help - Factory Defaults</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("factdef.h2"); %></h2>
			<dl>
				<dd>If you have problems with your router, which might be a result from changing some settings, but you are unsure what settings exactly, you can restore the factory defaults.</dd>
				<dt><% tran("factdef.legend"); %></dt>
				<dd>Click the <i>Yes</i> button to reset all configuration settings to their default values. Then click the <i>Save Settings</i> button.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Any settings you have saved will be lost when the default settings are restored. After restoring the router is accessible under the default IP address 192.168.1.1 and the default password <tt>admin</tt>.</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HBackup.asp"><% tran("bmenu.adminBackup"); %></a></li>
			</ul>
		</div>
	</body>
</html>
