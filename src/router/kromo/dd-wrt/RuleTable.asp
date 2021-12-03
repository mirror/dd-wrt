<% do_pagehead("routetbl.titl"); %>

	</head>

	<body>
		<form>
			<h2><% tran("routetbl.h3"); %></h2>
			<table class="table" cellspacing="4" id="pbr_table" summary="pbr table">
				<tr>
					<th sortdir="up"><% tran("routetbl.priority"); %></th>
					<th sortdir="up"><% tran("routetbl.not"); %></th>
					<th sortdir="up"><% tran("routetbl.from"); %></th>
					<th sortdir="up"><% tran("routetbl.to"); %></th>
					<th sortdir="up"><% tran("routetbl.tos"); %></th>
					<th sortdir="up"><% tran("routetbl.fwmark"); %></th>
					<th sortdir="up"><% tran("routetbl.ipproto"); %></th>
					<th sortdir="up"><% tran("routetbl.sport"); %></th>
					<th sortdir="up"><% tran("routetbl.dport"); %></th>
					<th sortdir="up"><% tran("routetbl.iif"); %></th>
					<th sortdir="up"><% tran("routetbl.oif"); %></th>
					<th sortdir="up"><% tran("routetbl.table"); %></th>
					<th sortdir="up"><% tran("routetbl.nat"); %></th>
				</tr>
				<script type="text/javascript">
				//<![CDATA[
					var table = new Array(<% dump_pbr_table(); %>);
					
					if(table.length == 0) {
						document.write("<tr><td align=\"center\" colspan=\"4\">- " + share.none + " -</td></tr>");
					} else {
						for(var i = 0; i < table.length; i = i+13) {
							if(table[i+8] == "LAN")
								table[i+8] = "LAN &amp; WLAN";
							else if(table[i+8] == "WAN")
								table[i+8] = "WAN";
							if(table[i+9] == "LAN")
								table[i+9] = "LAN &amp; WLAN";
							else if(table[i+9] == "WAN")
								table[i+9] = "WAN";
							document.write("<tr><td>"+table[i]+"</td><td>"+table[i+1]+"</td><td>"+table[i+2]+"</td><td>"+table[i+3]+"</td><td>"+table[i+4]+"</td><td>"+table[i+5]+"</td><td>"+table[i+6]+"</td><td>"+table[i+7]+"</td><td>"+table[i+8]+"</td><td>"+table[i+9]+"</td><td>"+table[i+10]+"</td><td>"+table[i+11]+"</td><td>"+table[i+12]+"</td></tr>");
						}
					}
				//]]>
				</script>
			</table><br />
			<script type="text/javascript">
			//<![CDATA[
			var t = new SortableTable(document.getElementById('pbr_table'), 4000);
			//]]>
			</script>				
			<div class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(0,0,0,0,1,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>