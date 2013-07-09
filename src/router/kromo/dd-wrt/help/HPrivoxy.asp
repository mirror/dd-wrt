<% do_hpagehead("privoxy.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("privoxy.titl"); %></h2>
			<dl>
				
				<dt><% tran("privoxy.server"); %></dt>
				<dd>Privoxy enables you to filter common ads. <br/>
					<ul>
					<br/>
					</ul>
				</dd>
				<dt><% tran("privoxy.transp"); %></dt>
				<dd>Transparent Mode : <br/>
					<ul>
						<li>Traffic to Port is 80 is intercepted by privoxy even if client did not configure any proxy settings</li>
						<li>Thus you can enforce filtering</li>
						<li>Transparent mode cannot intercept https connections. So all https traffic will still pass privoxy</li>
					</ul>
				</dd>
				<dt><% tran("privoxy.custom"); %></dt>
				<dd>Custom configuration: <br/>
					<ul>
						<li>Allows you to specify custom settings and paths to custom filters on external media e.g. usb disk</li>
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
