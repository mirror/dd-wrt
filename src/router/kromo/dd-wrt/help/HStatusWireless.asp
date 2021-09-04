<% do_hpagehead("status_wireless.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("status_wireless.h2"); %></h2>
			<dl>
				<% tran("hstatus_wireless.page1"); %>
				
				<dt><% tran("share.mac"); %></dt>
				<% tran("hstatus_wireless.page2"); %>
				
				<dt><% tran("share.mode"); %></dt>
				<% tran("hstatus_wireless.page3"); %>
				
				<dt><% tran("share.ssid"); %></dt>
				<% tran("hstatus_wireless.page4"); %>
				
				<dt><% tran("share.channel"); %></dt>
				<% tran("hstatus_wireless.page5"); %>
				
				<dt><% tran("wl_basic.TXpower"); %></dt>
				<% tran("hstatus_wireless.page6"); %>
				
				<dt><% tran("share.rates"); %></dt>
				<% tran("hstatus_wireless.page7"); %>
				
				<dt><% tran("share.encrypt"); %></dt>
				<% tran("hstatus_wireless.page8"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">Router Status</a></li>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
			</ul>
		</div>
	</body>
<% footer(); %></html>
