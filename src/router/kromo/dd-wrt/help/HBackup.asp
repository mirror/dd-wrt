<% do_hpagehead("config.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("config.h2"); %> / <% tran("config.h22"); %></h2>
			<dl>
				<!--dd>You may backup your current configuration in case you need to reset the router back to its factory default settings.</dd-->
				<dt><% tran("hconfig.page1"); %></dt>
				<dt><% tran("config.legend"); %></dt>
				<!--dd>Click the <i>Backup</i> button to download your current router configuration to your PC.</dd-->
				<dt><% tran("hconfig.page2"); %></dt>
        <dt><% tran("config.legend2"); %></dt>
        <!--dd>Click the <i>Browse</i> button to browse for the configuration file that is currently saved on your PC. Click <i>Restore</i> to overwrite all current configurations with the ones in the configuration file<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Only restore configurations with files backed up using the same firmware and the same model of router.</dd>
					</div>
				</dd-->
				<dt><% tran("hconfig.page3"); %></dt>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HDefault.asp"><% tran("bmenu.adminFactory"); %></a></li>
			</ul>
		</div>
	</body>
</html>
