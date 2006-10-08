<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Site Survey</title>
		<script type="text/javascript">
//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + survey.titl;

function do_join (F,SSID) {
	F.wl_ssid.value = SSID;
	<% nvram_invmatch("wl_mode", "ap", "/"); %><% nvram_invmatch("wl_mode", "ap", "/"); %>F.wl_mode.value="<% nvram_match("wl_mode", "ap", "sta"); %>"

	if (F.wl_ssid.value == "") {
//		alert("invalid SSID.");
		alert(errmsg.err47);
		return false;
	}
	F.submit_button.value = "Join";

	F.action.value = "Apply";
	apply(F);
}
		
//]]>
</script>
	</head>

	<body>
		<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="commit" value="1" />
			<input type="hidden" name="action" />
			<input type="hidden" name="wl_ssid" />
			<input type="hidden" name="wl_mode" />
			<h2><% tran("survey.h2"); %></h2>
        	<table class="center table" cellspacing="5">
				<tr>
				   <th width="31%"><% tran("share.ssid"); %></th>
				   <th width="20%"><% tran("share.mac"); %></th>
				   <th width="7%"><% tran("share.channel"); %></th>
				   <th width="7%"><% tran("share.rssi"); %></th>
				   <th width="7%"><% tran("share.noise"); %></th>
				   <th width="7%"><% tran("share.beacon"); %></th>
				   <th width="7%"><% tran("share.openn"); %></th>
				   <th width="7%"><% tran("share.dtim"); %></th>
				   <th width="7%"><% tran("share.rates"); %></th>
				   <th width="10%"><% tran("survey.thjoin"); %></th>
				</tr>
	<script type="text/javascript">
//<![CDATA[


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
		document.write("<\/tr>");
	}
}
	
//]]>
</script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript">
//<![CDATA[

document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />");

//]]>
</script>
				<script type="text/javascript">
//<![CDATA[

document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />");

//]]>
</script>
			</div>
		</form>
	</body>
</html>