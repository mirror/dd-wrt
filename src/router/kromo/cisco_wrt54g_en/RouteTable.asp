<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Routing Table</title>
		<link type="text/css" rel="stylesheet" href="style.css"/>
		<script type="text/JavaScript" src="common.js">{}</script>
	</head>
	
	<body>
		<form>
			<h2>Routing Table Entry List</h2>
			<table>
				<tbody>
					<tr>
						<th>Destination LAN IP</th>
						<th>Subnet Mask</th>
						<th>Gateway</th>
						<th>Interface</th>
					</tr>
<script language="JavaScript">

var table = new Array(
<% dump_route_table(""); %>
);

var i = 0;

for(;;) {
	if(!table[i]){
		if(i == 0) {
			document.write("<tr>");
			document.write("<td>None</td>");
			document.write("<td>None</td>");
			document.write("<td>None</td>");
			document.write("<td>None</td></tr>");
		}
		break;
	}
	if(table[i+3] == "LAN") {
		table[i+3] = "LAN &amp; Wireless";
	} else if(table[i+3] == "WAN") {
		table[i+3] = "WAN (Internet)";
	}
	document.write("<tr>");
	document.write("<td>"+table[i]+"</td>");
	document.write("<td>"+table[i+1]+"</td>");
	document.write("<td>"+table[i+2]+"</td>");
	document.write("<td>"+table[i+3]+"</td></tr>");
	i = i + 4;
}
</script>
					<tr>
						<td colspan="3">&nbsp;</td>
						<td><input name="button" type="button" onclick="window.location.reload()" value=" Refresh "/>
						<input onclick="self.close()" type="reset" value="Close"/></td>
					</tr>
				</tbody>
			</table>
		</form>
	</body>
</html>