<% do_hpagehead(); %>
		<title>Help - Command Shell</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("diag.h2"); %></h2>
			<dl>
				<dd>You are able to run command lines directly via the Webinterface.</dd>
				<dt><% tran("diag.legend"); %></dt>
				<dd>Fill the text area with your command click <em>Run Commands</em> to submit.</dd>				
				<dt><% tran("diag.startup"); %></dt>
				<dd>You can save some command lines to be executed at startup's router. Fill the text area with commands (only one command by row) and click <em>Save Startup</em>.</dd>
				<dt><% tran("diag.firewall"); %></dt>
				<dd>Each time the firewall is started, it can run some custom iptables instructions. Fill the text area with firewall's instructions (only one command by row) and click <em>Save Firewall</em>.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>
							<ul class="wide">
								<li>Startup commands are stored in nvram rc_startup variable</li>
								<li>Firewall commands are stored in nvram rc_firewall variable</li>
							</ul>
						</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HManagement.asp">Management</a></li>
			</ul>
		</div>
	</body>
</html>
