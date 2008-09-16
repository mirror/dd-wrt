<% do_pagehead("filterSum.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function filter_del(F) {
	F.submit_type.value = "delete";
	apply(F);
}
		
		//]]>
		</script>
	</head>

	<body onunload="top.opener.window.location.href='Filters.asp'">
		<form action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="FilterSummary" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<h2><% tran("filterSum.h2"); %></h2>
			<table>
				<tbody>
					<tr>
						<th><% tran("filterSum.polnum"); %></th>
						<th><% tran("filter.polname"); %></th>
						<th><% tran("filter.legend2"); %></th>
						<th><% tran("filterSum.polday"); %></th>
						<th><% tran("share.del"); %></th>
					</tr>
					<% filter_summary_show(); %>
				</tbody>
			</table>
			<div align="right">
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