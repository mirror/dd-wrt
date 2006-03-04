<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
	   <META http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		  <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		  <title><% nvram_get("router_name"); %> - DHCP Active IP Table</title>
		  <link type="text/css" rel="stylesheet" href="style.css"/>
		  <script type="text/JavaScript" src="common.js">{}</script>
		  <script language="JavaScript">
			
function DHCPAct(F) {
	F.submit_button.value = "DHCPTable";
	F.submit_type.value = "delete";
	F.change_action.value = "gozila_cgi";
	F.submit();
}
		</script>
   </head>
    
   <body onload="{window.focus();}">
   	<form action="apply.cgi" method="<% get_http_method(); %>">
	  	<input type="hidden" name="submit_button"/>
		<input type="hidden" name="change_action"/>
		<input type="hidden" name="submit_type"/>
		<h2>DHCP Active IP Table</h2>
         <table>
		 	<tbody>
				<tr>
				   <td colspan="2"><strong>DHCP Server IP Address</strong></td>
				   <td colspan="2"><% nvram_get("lan_ipaddr"); %></td>
				   <td>&nbsp;</td>
				</tr>
				<tr>
					<td colspan="5">&nbsp;</td>
				</tr>
				<tr>
				   <th>Client Host Name</th>
				   <th>IP Address</th>
				   <th>MAC Address</th>
				   <th>Expires</th>
				   <td>&nbsp;</td>
				</tr>

<script language="JavaScript">
var table = new Array(
<% dumpleases(0); %>
);
var i = 0;
var count = 0;
for(;;){
	if(!table[i]){
		if(i == 0){
			document.write("<tr>");
			document.write("<td>None</td>");
			document.write("<td>None</td>");
			document.write("<td>None</td>");
			document.write("<td>None</td>");
			document.write("<td>&nbsp;</td>");
			document.write("</tr>");
		}
		break;
	}
	document.write("<tr>");
	document.write("<td>"+table[i]+"</td>");
	document.write("<td>"+table[i+1]+"</td>");
	document.write("<td>"+table[i+2]+"</td>");
	document.write("<td>"+table[i+3]+"</td>");
	document.write("<td><input type=\"checkbox\" name=\"d_"+count+"\" value=\""+table[i+4]+"\" /></td>");
	document.write("<tr>");
	count ++;
	i = i + 5;
}
</script>
				<tr>
					<td colspan="5">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2"><input name="button" type="button" onclick="window.location.reload()" value=" Refresh " /></td>
					<td align="right" colspan="3"><input type="button" name="action" value="Delete" onclick="DHCPAct(this.form)"/>&nbsp;<input onclick="self.close()" type="reset" value="Close"/></td>
				</tr>
			</tbody>
        </table>
      </form>
   </body>
</html>