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
						<th><% tran("log_in.th_ip"); %></th>
						<th><% tran("share.proto"); %></th>
						<th><% tran("log_in.th_port"); %></th>
						<th><% tran("share.rule"); %></th>
					</tr>
				</thead>
				<tbody>
					<% dumplog("incoming"); %>
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
