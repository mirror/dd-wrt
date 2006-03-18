<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - DHCP Active IP Table</title>
		<link type="text/css" rel="stylesheet" href="style.css"/>
		<script type="text/JavaScript" src="common.js"></script>
		<script type="text/JavaScript">

function DHCPAct(F) {
	//function DHCPAct(F, lease) {
	//if (!confirm("Delete " + lease + " ? ")) return;
	
	F.submit_button.value = "DHCPTable";
	F.submit_type.value = "delete";
	F.change_action.value = "gozila_cgi";
	F.submit();
}
		</script>
   </head>
   
	<body onload="{window.focus();}">
		<form action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<h2>DHCP Active IP Table</h2>
			<div class="center">DHCP Server IP Address : <% nvram_get("lan_ipaddr"); %></div><br />
			<table cellspacing="5">
				<tr>
					<th>Client Host Name</th>
					<th>IP Address</th>
					<th>MAC Address</th>
					<th>Expires</th>
					<th>Delete</th>
				</tr>
<script language="JavaScript">

var table = new Array(
<% dumpleases(0); %>
);
var i = 0;
if(table.length == 0) {
	document.write("<tr>");
	document.write("<td colspan=\"5\" align=\"center\">- None -</td>");
	document.write("</tr>");
}
else {
	for(var i = 0; i < table.length; i = i + 5) {
		document.write("<tr>");
		document.write("<td>" + table[i] + "</td>");
		document.write("<td>" + table[i + 1] + "</td>");
		document.write("<td>" + table[i + 2] + "</td>");
		document.write("<td>" + table[i + 3] + "</td>");
		document.write("<td><input type=\"checkbox\" name=\"d_" + Math.floor(i / 5) + "\" value=\"" + table[i + 4] + "\" /></td>");
		//document.write("<td class=\"bin\" title=\"Click to delete lease\" name=\"d_" + Math.floor(i / 5) + "\" value=\"" + table[i + 4] + "\" onclick='DHCPAct(this.form,\""+ table[i + 1] +"\")' /></td>");
		document.write("</tr>");
	}
}
</script>
			</table><br />
			<div class="submitFooter">
				<input name="button" type="button" onclick="window.location.reload()" value="Refresh" />
				<input type="button" name="action" value="Delete" onclick="DHCPAct(this.form)" />
				<input onclick="self.close()" type="reset" value="Close" />
			</div>
		</form>
	</body>
</html>