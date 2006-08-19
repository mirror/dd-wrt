<% do_hpagehead(); %>
		<title>Help - Wireless MAC Filter</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Wireless MAC Filter</h2>
			<dl>
				<dd>The Wireless MAC Filter allows you to control which wireless-equipped PCs may or may not communicate with the router depending on their MAC addresses. For information how to get MAC addresses from Windows-PCs, see <a href="HWanMAC.asp">MAC Address Cloning</a> for detailed instructions.</dd>
				<dd>To set up a filter, click <i>Enable</i>, and follow these instructions:
					<ol class="wide">
						<li>If you want to block specific wireless-equipped PCs from communicating with the router, then keep the default setting, <i>Prevent PCs listed from accessing the wireless network</i>. If you want to allow specific wireless-equipped PCs to communicate with the router, then click the radio button next to <i>Permit only PCs listed to access the wireless network</i>.</li>
						<li>Click the <i>Edit MAC Filter List</i> button. Enter the appropriate MAC addresses into the MAC fields.<br /><br />
							<div class="note">
								<b>Note:</b><br />
								The MAC address should be entered in this format: xxxxxxxxxxxx (the x's represent the actual characters of the MAC address).
							</div>
						</li>
						<li>Click the <i>Save Settings</i> button to save your changes. Click the <i>Cancel Changes</i> button to cancel your unsaved changes. Click the <i>Close</i> button to return to the previous screen without saving changes.</li>
					</ol><br />
					To disable the Wireless MAC Filter, keep the default setting, <i>Disable</i>.
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">Wireless Settings</a></li>
				<li><a href="HWPA.asp">Wireless Security</a></li>
			</ul>
		</div>
	</body>
</html>