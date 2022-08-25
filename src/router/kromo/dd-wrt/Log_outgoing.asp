<% do_pagehead("log_out.titl"); %>
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
				<h2><% tran("log_out.h2"); %></h2>
				<fieldset>
				<legend><% tran("log.legend"); %></legend>
				<table class="table">
					<tr>
						<th><% tran("log_out.th_lanip"); %></th>
						<th><% tran("log_out.th_wanip"); %></th>
						<th><% tran("share.proto"); %></th>
						<th><% tran("log_out.th_port"); %></th>
						<th><% tran("share.rule"); %></th>
					</tr>
					<% dumplog("outgoing"); %>
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
