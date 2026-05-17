<% do_pagehead_nopwc("log_in.titl"); %>
	<script type="text/javascript">
	//<![CDATA[
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
});
	//]]>
	</script>

	</head>
	<body class="popup_bg">
			<form>
				<h2><% tran("log_in.h2"); %></h2>
				<fieldset>
				<legend><% tran("log.legend"); %></legend>
				<table class="table">
					<thead>
					<tr>
						<th><% tran("qos.service"); %></th>
						<th><% tran("share.ip"); %></th>
						<th><% tran("share.attempts"); %></th>
						<th><% tran("share.state"); %></th>
						<th><% tran("share.first_seen"); %></th>
						<th><% tran("share.blocked_until"); %></th>
						<th><% tran("share.remember_till"); %></th>
					</tr>
				</thead>
				<tbody>
					<% dumpblocklist(); %>
				</tbody>
				</table>
				</fieldset><br />
				<div id="footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(0,0,0,0,1,1);
					//]]>
					</script>
				</div>
			</form>
	</body>
</html>
