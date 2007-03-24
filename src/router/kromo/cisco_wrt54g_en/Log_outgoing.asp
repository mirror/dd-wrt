<% do_pagehead("log_out.titl"); %>

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