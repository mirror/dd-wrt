<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Active IP Connections Table</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + status_conn.titl;

		//]]>
		</script>
	</head>
	<body>
		<div class="popup">
			<form>
				<h2><% tran("status_conn.h2"); %></h2>
					<table class="table">
						<tr>
							<th><% tran("share.proto"); %></th>
							<th><% tran("share.timeout"); %></th>
							<th><% tran("share.src"); %></th>
							<th><% tran("share.dst"); %></th>
							<th><% tran("share.srv"); %></th>
							<th><% tran("share.state"); %></th>
							<th><% tran("share.name_resolution"); %></th>
						</tr>
						<% ip_conntrack_table(); %>
				</table><br />
				<div class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />");
					//]]>
					</script>
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />");
					//]]>
					</script>
				</div>
			</form>
		</div>
	</body>
</html>