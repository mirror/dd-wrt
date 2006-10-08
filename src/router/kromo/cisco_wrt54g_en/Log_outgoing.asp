<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Outgoing Log Table</title>
		<script type="text/javascript"><![CDATA[
document.title = "<% nvram_get("router_name"); %>" + log_out.titl;

		]]></script>
	</head>
	<body>
		<div class="popup">
			<form>
				<h2><% tran("log_out.h2"); %></h2>
				<table class="table">
					<tr>
						<th><% tran("log_out.th_lanip"); %></th>
						<th><% tran("log_out.th_wanip"); %></th>
						<th><% tran("share.proto"); %></th>
						<th><% tran("log_out.th_port"); %></th>
						<th><% tran("share.rule"); %></th>
					</tr>
					<% dumplog("outgoing"); %>
				</table><br />
				<div class="submitFooter">
					<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />");
]]></script>
					<script type="text/javascript"><![CDATA[
document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />");
]]></script>
				</div>
			</form>
		</div>
	</body>
</html>