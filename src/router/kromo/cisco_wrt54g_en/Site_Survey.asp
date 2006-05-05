<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Site Survey</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
document.title = "<% nvram_get("router_name"); %>" + survey.titl;

function do_join (F,SSID) {
	F.wl_ssid.value = SSID;
	<% nvram_invmatch("wl_mode", "ap", "//"); %>F.wl_mode.value="<% nvram_match("wl_mode", "ap", "sta"); %>"

	if (F.wl_ssid.value == "") {
//		alert("invalid SSID.");
		alert(errmsg.err47);
		return false;
	}
	F.submit_button.value = "Join";

	F.action.value = "Apply";
	apply(F);
}
		</script>
	</head>

	<body>
		<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="commit" value="1" />
			<input type="hidden" name="action" />
			<input type="hidden" name="wl_ssid" />
			<input type="hidden" name="wl_mode" />
			<h2><script type="text/javascript">Capture(survey.h2)</script></h2>
        	<table class="center table" cellspacing="5">
				<tr>
				   <th width="31%"><script type="text/javascript">Capture(share.ssid)</script></th>
				   <th width="20%"><script type="text/javascript">Capture(share.mac)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.channel)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.rssi)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.noise)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.beacon)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.openn)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.dtim)</script></th>
				   <th width="7%"><script type="text/javascript">Capture(share.rates)</script></th>
				   <th width="10%"><script type="text/javascript">Capture(survey.thjoin)</script></th>
				</tr>
	<script language="JavaScript">

var table = new Array(
<% dump_site_survey(""); %>
);

if (table.length == 0) {
	document.write("<tr><td colspan=\"10\" align=\"center\">" + share.none + "</td></tr>");
} else {
	for (var i = 0; i < table.length; i = i + 9) {
		document.write("<tr>");
		document.write("<td>"+table[i]+"</td>");
		document.write("<td align=\"center\" style=\"cursor:pointer\" title=\"OUI Search\" onclick=\"getOUIFromMAC('" + table[i+1] + "')\" >"+table[i+1]+"</td>");
		document.write("<td align=\"center\">"+table[i+2]+"</td>");
		document.write("<td align=\"center\">"+table[i+3]+"</td>");
		document.write("<td align=\"center\">"+table[i+4]+"</td>");
		document.write("<td align=\"center\">"+table[i+5]+"</td>");
		document.write("<td align=\"center\">"+table[i+6]+"</td>");
		document.write("<td align=\"center\">"+table[i+7]+"</td>");
		document.write("<td align=\"center\">"+table[i+8]+"</td>");
		document.write("<td align=\"center\"><input type=\"button\" value=\"" + sbutton.join + "\" onclick='do_join(this.form,\"" + table[i] + "\")' /></td>");
		document.write("</tr>");
	}
}
	</script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript">document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />")</script>
				<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />")</script>
			</div>
		</form>
	</body>
</html>