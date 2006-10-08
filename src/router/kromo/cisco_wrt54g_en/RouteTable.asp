<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Routing Table</title>
		<script type="text/javascript"><![CDATA[
document.title = "<% nvram_get("router_name"); %>" + routetbl.titl;

		]]></script>
	</head>

	<body>
		<form>
			<h2><% tran("routetbl.h2"); %></h2>
			<table>
				<tr>
					<th><% tran("routetbl.th1"); %></th>
					<th><% tran("share.subnet"); %></th>
					<th><% tran("share.gateway"); %></th>
					<th><% tran("share.intrface"); %></th>
				</tr>
				<script type="text/javascript"><![CDATA[
					var table = new Array(<% dump_route_table(""); %>);
					
					if(table.length == 0) {
						document.write("<tr><td align=\"center\" colspan=\"4\">- " + share.none + " -</td></tr>");
					} else {
						for(var i = 0; i < table.length; i = i+4) {
							if(table[i+3] == "LAN")
								table[i+3] = "LAN &amp; WLAN";
							else if(table[i+3] == "WAN")
								table[i+3] = "WAN";
							document.write("<tr><td>"+table[i]+"</td><td>"+table[i+1]+"</td><td>"+table[i+2]+"</td><td>"+table[i+3]+"</td></tr>");
						}
					}
				]]></script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />");
]]></script>
				<script type="text/javascript"><![CDATA[
document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />");
]]></script>
			</div>
		</form>
	</body>
</html>