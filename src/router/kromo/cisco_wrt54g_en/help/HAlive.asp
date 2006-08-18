<% do_hpagehead(); %>
		<title>Help - Keep Alive</title>
	</head>

	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig">
				<a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a>
			</div>
		</div>
		<div id="content">
			<h2>Keep Alive</h2>
			<dl>
				<dt><% tran("alive.legend"); %></dt>
				<dd>You can schedule regular reboots for the router : 
					<ul>
						<li>Regularly after xxx seconds.</li>
						<li>At a specific date time each week or everyday.</li>
					</ul><br/>
					<div class="note">
						<h4>Note</h4>
						<div>For date based reboots Cron must be activated. See <a href="HManagement.asp">Management</a> for Cron activation.</div>
					</div>
				</dd>
				<dt><% tran("alive.legend2"); %></dt>
				<dd></dd>
				<dd>Check all values and click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes. Click <em>Reboot router</em> to reboot your router immediately.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HWDS.asp">WDS</a></li>
				<li><a href="HManagement.asp">Management</a></li>
			</ul>
		</div>
	</body>
</html>