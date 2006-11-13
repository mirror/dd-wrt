<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Access Restrictions Summary</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + filterSum.titl;

function filter_del(F) {
	F.submit_button.value = "FilterSummary";
	F.change_action.value = "gozila_cgi";
	
	F.submit_type.value = "delete";
	F.submit();
}
		
		//]]>
		</script>
	</head>

	<body onunload="top.opener.window.location.href='Filters.asp'">
		<form action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" />
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
					<tr>
						<td colspan="3">&nbsp;</td>
						<td colspan="2" align="right">
							<script type="text/javascript">
							//<![CDATA[
							document.write("<input class=\"button\" type=\"button\" value=\"" + share.del + "\" onclick=\"filter_del(this.form);\" />");
							document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.clos + "\" onclick=\"self.close();\" />");
							//]]>
							</script>
						</td>
					</tr>
				</tbody>
			</table>
		</form>
	</body>
</html>