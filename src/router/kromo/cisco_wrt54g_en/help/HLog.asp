<% do_hpagehead(); %>
		<title>Help - Log</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("log.h2"); %></h2>
			<dl>
				<dd>The router can keep logs of all incoming or outgoing traffic for your Internet connection.</dd>
				
				<dt><% tran("log.legend"); %></dt>
				<dd>To keep activity logs, select <i>Enable</i>. To stop logging, select <i>Disable</i>.</dd>
				
				<dt><% tran("log.lvl"); %></dt>
				<dd>Set this to the required amount of information. Set <i>Log Level</i> higher to log more actions.</dd>
				
				<dt><% tran("sbutton.log_in"); %></dt>
				<dd>To see a temporary log of the Router's most recent incoming traffic, click the <i>Incoming Log</i> button.</td>
				
				<dt><% tran("sbutton.log_out"); %></dt>
				<dd>To see a temporary log of the Router's most recent outgoing traffic, click the <i>Outgoing Log</i> button.</dd>
				<dd>Click <i>Save Settings</i> to save your settings. Click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">Router Status</a></li>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
			</ul>
		</div>
	</body>
</html>
