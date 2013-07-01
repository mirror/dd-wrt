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
				
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
</html>
