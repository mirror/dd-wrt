<% do_hpagehead("lighttpd.titl"); %>
	<body class"help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("lighttpd.titl"); %></h2>
			<dl>
				<dt><% tran("lighttpd.server"); %></dt>
				<dd>Lighttpd webserver with PHP5 support. <br/>
					<ul>
					<br/>
					</ul>
				</dd>
				<dt><% tran("lighttpd.wan"); %></dt>
				<dd>If enabled allows access to the webserver from the internet. <br/>
					If disabled only clients from your internal network can access the webserver.
				</dd>
				<dt><% tran("privoxy.custom"); %></dt>
				<dd>To override the default configuration for lighttpd and php you can store a custom <br/>
				lighttpd.conf and php.ini in directory /jffs/etc .
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
</html>
