<% do_hpagehead("lighttpd.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("lighttpd.titl"); %></h2>
			<dl>
				<dt><% tran("lighttpd.server"); %></dt>
				<dd>Is a secure, fast, compliant, and very flexible web server that has been optimized for high-performance environments.<br/>
					lighttpd uses memory and CPU efficiently and has lower resource use than other popular web servers.<br/>
					It's advanced feature-set (FastCGI, CGI, Auth, Output-Compression, URL-Rewriting and much more) make lighttpd the perfect web server for all systems, small and large.<br/>
				</dd>
				<dt><% tran("lighttpd.wan"); %></dt>
				<dd>When enabled allows access to the webserver from the Internet.<br/>
					When disabled only clients from your internal network can access the Webserver.
				</dd>
				<dt><% tran("privoxy.custom"); %></dt>
				<dd>To override the default configuration for lighttpd and PHP you can store a custom <br/>
				lighttpd.conf and php.ini in the <b>/jffs/etc</b> directory which you will need to create.
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
