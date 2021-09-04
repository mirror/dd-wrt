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
				<% tran("hprivoxy.page1"); %>
				<dt><% tran("privoxy.pac"); %></dt>
				<% tran("hprivoxy.page2"); %>
				<dt><% tran("privoxy.transp"); %></dt>
				<% tran("hprivoxy.page3"); %>
				<dt><% tran("privoxy.custom"); %></dt>
				<% tran("hprivoxy.page4"); %>
				
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
			</ul>
		</div>
	</body>
<% footer(); %></html>
