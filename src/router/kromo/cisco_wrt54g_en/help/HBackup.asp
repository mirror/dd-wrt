<% do_hpagehead(); %>
		<title>Help - Backup</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("config.h2"); %> / <% tran("config.h22"); %></h2>
			<dl>
				<dd>You may backup your current configuration in case you need to reset the router back to its factory default settings.</dd>
				<dt><% tran("config.legend"); %></dt>
				<dd>Click the <i>Backup</i> button to download your current router configuration to your PC.</dd>
        <dt><% tran("config.legend2"); %></dt>
        <dd>Click the <i>Browse</i> button to browse for the configuration file that is currently saved on your PC. Click <i>Restore</i> to overwrite all current configurations with the ones in the configuration file<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Only restore configurations with files backed up using the same firmware and the same model of router.</dd>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HDefault.asp">Factory Defaults</a></li>
			</ul>
		</div>
	</body>
</html>
