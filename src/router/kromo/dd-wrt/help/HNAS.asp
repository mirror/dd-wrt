<% do_hpagehead("nas.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("nas.titl"); %></h2>
			<dl>
				
				<dt><% tran("nas.proftpd_srv"); %></dt>
				<dd>FTP Server enables you to share files : <br/>
					<ul>
						<li>Over the Internet - WAN</li>
						<li>On local network</li>
					</ul>
				</dd>
				<dt><% tran("nas.dlna_legend"); %></dt>
				<dd>DLNA Server enables you to share media : <br/>
					<ul>
						<li>You need a dlna capable client e.g. TV to view files served by the router.</li>
					</ul>
				</dd>
				<dt><% tran("nas.samba3"); %></dt>
				<dd>SAMBA Server enables you to acces files : <br/>
					<ul>
						<li>On router from local network through file explorer</li>
					</ul>
				</dd> 
				<dt><% tran("share.option"); %></dt>
				<dd><br/>
					<ul>
						<li>Path: Path to mounted partition. See currently mounted disks under <a href="../USB.asp">Disk Info</a> </li>
						<li>Subdir: Directory name on given partition e.g. public or folder/sub_folder/sub_sub_folder </li>
						<li>Name: Share name displayed when browsing the network shares e.g. \\router\name </li>
						<li>Public: Everyone can access this share. No user account required.</li>
					</ul>
				</dd> 
				<dt>Note</dt>
				<dd><br/>
					For Windows users: a connected USB drive shows up in Windows under D: and contains two directories <i>public, secret</i>.</br>
					You want to share <i>D:\public</i>. To do this connect the usb drive and lookup or specify a mountpoint under <a href="../USB.asp">USB Settings</a>.</br>
					D: equals a mount point under linux. In order to share <i>D:\public</i>, select current mount point and specify subdir <i>public</i></br>
					give it a name e.g. Guest and setup access permissions.					
				</dd> 
				
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
</html>
