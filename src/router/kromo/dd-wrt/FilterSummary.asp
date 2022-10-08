<% do_pagehead_nopwc("filterSum.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function filter_del(F) {
	F.submit_type.value = "delete";
	apply(F);
}
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
});
	//]]>
	</script>
	</head>

	<body class="popup_bg" onunload="top.opener.window.location.href='Filters.asp'">
		<form action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="FilterSummary" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<h2><% tran("filterSum.h2"); %></h2>
			<table class="table">
				<thead>
					<tr>
						<th class="center"><% tran("filterSum.polnum"); %></th>
						<th class="center"><% tran("filter.polname"); %></th>
						<th class="center"><% tran("filter.legend2"); %></th>
						<th class="center"><% tran("filterSum.polday"); %></th>
						<th class="center"><% tran("share.del"); %></th>
					</tr>
				</thead>
				<tbody>
					<% filter_summary_show(); %>
				</tbody>
			</table><br />
			<div id="footer" class="submitFooter">
				<script type="text/javascript">
					//<![CDATA[
					document.write("<input class=\"button\" type=\"button\" value=\"" + share.del + "\" onclick=\"filter_del(this.form);\" />");
					document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.clos + "\" onclick=\"self.close();\" />");
					//]]>
				</script>
			</div>
		</form>
	</body>
</html>
