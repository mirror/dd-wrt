<% do_hpagehead("diag.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
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
				<dt><% tran("diag.custom"); %></dt>
				<dd>Custom script is stored in /tmp/custom.sh file. You can run it manually or use cron to call it. Fill the text area with script's instructions (only one command by row) and click <em>Save Firewall</em>.<br/><br/>
				
					<div class="note">
						<h4>Note</h4>
						<div>
							<ul class="wide">
								<li>Startup commands are stored in nvram rc_startup variable</li>
								<li>Firewall commands are stored in nvram rc_firewall variable</li>
								<li>Custom script is stored in nvram rc_custom variable</li>
							</ul>
						</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
