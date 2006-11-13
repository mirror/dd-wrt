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
					submitFooterButton(0,0,0,0,1,1);
					//]]>
					</script>
				</div>
			</form>
		</div>
	</body>
</html>