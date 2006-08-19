<% do_hpagehead(); %>
		<title>Help - MAC Address Cloning</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("wanmac.h2"); %></h2>
			<dl>
				<dd>The router's MAC address is a 12-digit code assigned to a unique piece of hardware for identification. Some ISPs require that you register the MAC address of your network card/adapter, which was connected to your cable or DSL modem during installation.</dd>
				
				<dt><% tran("wanmac.legend"); %></dt>
					<dd>To clone your network adapter's MAC address onto the router, select <i>Enable</i> and enter your adapter's MAC address in the <i>Clone WAN MAC</i> field or click <i>Get Current PC MAC Address</i> to fill in the MAC address of your PC you are using. Then save your changes by clicking on the <i>Save Settings</i> button.
					<br/><br/>
					To disable MAC address cloning, keep the default setting, <i>Disable</i>.
					</dd>
					<dd>Find your adapter's MAC address by following the instructions for your PC's operating system.
						<br/><br/>
						<b>Windows 98 and Millennium:</b>
						<ol class="wide">
							<li>Click the Start button, and select <i>Run</i>.</li>
							<li>Type <tt>winipcfg</tt> in the field provided, and press the OK key.</li>
							<li>Select the Ethernet adapter you are using.</li>
							<li>Click More Info.</li>
							<li>Write down your adapter's MAC address.</li>
						</ol><br />
						<b>Windows 2000 and XP:</b>
						<ol class="wide">
							<li>Click the Start button, and select Run.</li>
							<li>Type <tt>cmd</tt> in the field provided, and press the OK key.</li>
							<li>At the command prompt, run <tt>ipconfig /all</tt>, and look at your adapter's physical address.</li>
							<li>Write down your adapter's MAC address.</li>
						</ol>
					</dd>
					<dd>Check all values and click <i>Save Settings</i> to save your settings. Click </i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
			</ul>
		</div>
	</body>
</html>
