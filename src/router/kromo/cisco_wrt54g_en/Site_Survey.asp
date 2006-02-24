<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Site Survey</title>
		<link type="text/css" rel="stylesheet" href="style.css"/>
		<script type="text/JavaScript" src="common.js">{}</script>
		<script language="JavaScript">

var EN

function do_join(F,SSID) {
	F.wl_ssid.value = SSID;
	<% nvram_invmatch("wl_mode", "ap", "//"); %>F.wl_mode.value="<% nvram_match("wl_mode", "ap", "sta"); %>"

	if(F.wl_ssid.value == ""){
		alert("invalid SSID!");
		return false;
	}

	F.submit_button.value = "Join";
	F.action.value = "Apply";
	F.submit();
}
		</script>
	</head>
	
	<body onload="window.focus();">
		<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button"/>
		<input type="hidden" name="commit" value="1"/>
		<input type="hidden" name="action" value="Apply"/>
		<input type="hidden" name="wl_ssid"/>
		<input type="hidden" name="wl_mode"/>
         <table class="center table">
				<tr>
					<th colspan="8"><h2>Neighbor's Wireless Networks</h2></th>
					<th align="right"><input type="button" value=" Refresh " onclick="window.location.reload()"/></th>
				</tr>
				<tr>
				   <th width="31%">SSID</th>
				   <th width="20%">MAC</th>
				   <th width="7%">Channel</th>
				   <th width="7%">Rssi</th>
				   <th width="7%">noise</th>
				   <th width="7%">beacon</th>
				   <th width="7%">cap</th>
				   <th width="7%">dtim</th>
				   <th width="7%">rates</th>
				   <th width="10%">Join Site</th>
				</tr>
	<script language="JavaScript">
	
	var table = new Array(
	<% dump_site_survey(""); %>
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
				document.write("<td>None</td>");
				document.write("<td>None</td>");
				document.write("<td>None</td>");
				document.write("<td>None</td>");
				document.write("<td>None</td>");
				document.write("<td>None</td>");			
				document.write("</tr>");
			}
			break;
		}
		document.write("<tr>");
		document.write("<td>"+table[i]+"</td>");
		document.write("<td>"+table[i+1]+"</td>");
		document.write("<td>"+table[i+2]+"</td>");
		document.write("<td>"+table[i+3]+"</td>");
		document.write("<td>"+table[i+4]+"</td>");
		document.write("<td>"+table[i+5]+"</td>");
		document.write("<td>"+table[i+6]+"</td>");
		document.write("<td>"+table[i+7]+"</td>");
		document.write("<td>"+table[i+8]+"</td>");
		document.write("<td><input type=\"button\" value=\"Join\" onclick='do_join(this.form,\""+table[i]+"\")'/></td>");		
		document.write("</tr>");
		count ++;
		i = i + 9;
	}
	</script>
				<tr>
					<td colspan="9" align="center"><input type="reset" value="Close" onclick="self.close()"/></td>
				</tr>
			</table>
		</form>
	</body>
</html>