<% do_hpagehead("usb.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("usb.titl"); %></h2>
			<dl>
				
				<dt><% tran("usb.usb_core"); %></dt>
				<dd>
					<ul>
						<li>Enable USB Support</li>
						
					</ul>
				</dd>
				<dt><% tran("usb.usb_printer"); %></dt>
				<dd>
					<ul>
						<li>Enable Printer Support</li>
					</ul>
				</dd>
				<dt><% tran("usb.usb_storage"); %></dt>
				<dd>
					<ul>
						<li>Enable support for external drives</li>
					</ul>
				</dd>
				<dt><% tran("usb.usb_automnt"); %></dt>
				<dd>
					<ul>
						<li>Auto mount connected drives</li>
					</ul>
				</dd>
				<dt>Options</dt>
				<dd>
					<ul>
						<li>Run script from the specified path whenever a drive is mounted by automount</li>
						<li>Mount partition with given UUID to specified mount point e.g. /opt</li>
						<li>Use SES Button to un mount drives before disconnecting them</li>
					</ul>
				</dd>
				<dt><% tran("usb.usb_diskinfo"); %></dt>
				<dd>
					<ul>
						<li>Displays disk info e.g. partition size, volume name if set, as well as UUID ( e.g. B965FA66-CC65-4DK8-1254-DD0A78D19A90) for all connected drives</li>

					</ul>
				</dd>
				</br>
				<dt>Note:</dt> </br>
				Automount by default mounts all drives to /mnt/<devname> e.g. /mnt/sda1 </br> 
				If your volume name is opt the partition will be mounted to /opt, if set to jffs -> /jffs.</br> 
				You can override this by entering a partitions UUID in the option fields.</br> </br>
				

		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
</html>
