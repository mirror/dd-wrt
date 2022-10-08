<% do_pagehead_nopwc("routetbl.titl"); %>
	<script type="text/javascript">
	//<![CDATA[
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
});
	//]]>
	</script>
	</head>

	<body class="popup_bg">
		<form>
			<h2><% tran("routetbl.h2"); %></h2>
			<fieldset>
			<legend><% tran("route.titl"); %></legend>
			<table class="table" cellspacing="4" id="routing_table" summary="routing table">
			<tbody>
				<tr>
					<th sortdir="up"> <% tran("routetbl.th1"); %></th>
					<th sortdir="up"> <% tran("share.gateway"); %></th>
					<th sortdir="up"> <% tran("routetbl.table"); %></th>
					<th sortdir="up"> <% tran("routetbl.scope"); %></th>
					<th sortdir="up"> <% tran("route.metric"); %></th>
					<th sortdir="up"> <% tran("share.iftbl"); %></th>
					<th sortdir="up"> <% tran("routetbl.src"); %></th>
				</tr>
				<script type="text/javascript">
				//<![CDATA[
					var table = new Array(<% dump_route_table(); %>);
					
					if(table.length == 0) {
						document.write("<tr><td class=\"center\" colspan=\"4\">- " + share.none + " -</td></tr>");
					} else {
						for(var i = 0; i < table.length; i = i+7) {
							if(table[i+5] == "LAN")
								table[i+5] = "LAN &amp; WLAN";
							else if(table[i+5] == "WAN")
								table[i+5] = "WAN";
							document.write("<tr><td>"+table[i]+"</td><td>"+table[i+1]+"</td><td>"+table[i+2]+"</td><td>"+table[i+3]+"</td><td>"+table[i+4]+"</td><td>"+table[i+5]+"</td><td>"+table[i+6]+"</td></tr>");
						}
					}
				//]]>
				</script>
			</tbody>
			</table>
			</fieldset><br />
			<script type="text/javascript">
			//<![CDATA[
			var t = new SortableTable(document.getElementById('routing_table'), 4000);
			//]]>
			</script>				
			<div id="footer" class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(0,0,0,0,1,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>
