<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - DHCP Active IP Table</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = '<% nvram_get("router_name"); %>'+dhcp.titl;

function DHCPAct(F) {
	F.submit_button.value = "DHCPTable";
	F.submit_type.value = "delete";
	F.change_action.value = "gozila_cgi";
	F.submit();
}
		</script>
   </head>

	<body onload="{window.focus();}">
		<form action="apply.cgi" method="<% get_http_method(); %>" >
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<h2><script type="text/javascript">Capture(dhcp.h2)</script></h2>
			<div class="center"><script type="text/javascript">Capture(dhcp.server)</script>&nbsp;<% nvram_get("lan_ipaddr"); %></div><br />
			<table cellspacing="5">
				<tr>
					<th><script type="text/javascript">Capture(dhcp.tclient)</script></th>
					<th><script type="text/javascript">Capture(share.ip)</script></th>
					<th><script type="text/javascript">Capture(share.mac)</script></th>
					<th><script type="text/javascript">Capture(share.expires)</script></th>
					<th><script type="text/javascript">Capture(share.del)</script></th>
				</tr>
<script language="JavaScript">

var table = new Array(
<% dumpleases(0); %>
);
var i = 0;
if(table.length == 0) {
	document.write("<tr>");
	document.write("<td colspan=\"5\" align=\"center\">- " + share.none + " -</td>");
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
		document.write("</tr>");
	}
}
</script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript">document.write("<input type=\"button\" name=\"B1\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />")</script>
				<script type="text/javascript">document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.del + "\" onclick=\"DHCPAct(this.form)\" />")</script>
				<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />")</script>
			</div>
		</form>
	</body>
</html>