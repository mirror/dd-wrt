<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Filter Summary Table</title>
		<link type="text/css" rel="stylesheet" href="style.css" />
		<script type="text/JavaScript" src="common.js">{}</script>
		<script language="JavaScript">

function filter_del(F) {
	F.submit_button.value = "FilterSummary";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "delete";
	F.submit();
}

function init() {
	window.focus();
}
		</script>
	</head>
	
	<body onload="init()" onUnload="top.opener.window.location.href='Filters.asp'">
		<form action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" />
			<h2>Internet Policy Summary</h2>
			<table>
			<tbody>
				<tr>
					<th>No.</th>
					<th>Policy Name</th>
					<th>Days</th>
					<th>Time of Day</th>
					<th>&nbsp;</th>
				</tr>
				<% filter_summary_show(); %>
				<tr>
					<td colspan="3">&nbsp;</td>
					<td colspan="2" align="right"><input type="button" value="Delete" onClick="filter_del(this.form)" />&nbsp;<input onclick="self.close()" type="reset" value="Close"/></td>
				</tr>
				</tbody>
			</table>
		</form>
   </body>
</html>