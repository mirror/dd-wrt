<% do_hpagehead("radius.legend"); %>
	<body class"help-bg">
	<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("radius.h2"); %></h2>
			<dl>
				<% tran("hradauth.page1"); %>
				
				<dt><% tran("radius.label2"); %></dt>
				<% tran("hradauth.page2"); %>
				
				<dt><% tran("radius.label3"); %> - <% tran("radius.label4"); %></dt>
				<% tran("hradauth.page3"); %>
				
				<dt><% tran("radius.label5"); %></dt>
				<% tran("hradauth.page4"); %>
				
				<dt><% tran("radius.label6"); %></dt>
				<% tran("hradauth.page5"); %>
				
				<dt><% tran("radius.label7"); %></dt>
				<% tran("hradauth.page6"); %>
				
				<dt><% tran("radius.label8"); %></dt>
				<% tran("hradauth.page7"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
				<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
				<li><a href="HWirelessAdvanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
			</ul>
		</div>
	</body>
</html>
