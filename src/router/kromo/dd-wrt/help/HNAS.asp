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
				<dd>FTP Server enables you to serve files : <br/>
					<ul>
						<li>Possibility to serve files over the Internet</li>
						<li>Possibility to serve files on local network</li>
					</ul>
				</dd>
				<dd>SAMBA Server enables you to acces files : <br/>
					<ul>
						<li>Possibility to access files on router from local network</li>
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
