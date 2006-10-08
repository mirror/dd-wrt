<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Incoming Log Table</title>
		<script type="text/javascript">
//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + log_in.titl;

		
//]]>
</script>
	</head>
	<body>
		<div class="popup">
			<form>
				<h2><% tran("log_in.h2"); %></h2>
				<table class="table">
					<tr>
						<th><% tran("log_in.th_ip"); %></th>
						<th><% tran("share.proto"); %></th>
						<th><% tran("log_in.th_port"); %></th>
						<th><% tran("share.rule"); %></th>
					</tr>
					<% dumplog("incoming"); %>
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