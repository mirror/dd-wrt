<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - MAC Address Filter List</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + wl_filter.titl;

function SelPage(num,F) {
	F.submit_button.value = "WL_FilterTable";
	F.change_action.value = "gozila_cgi";
	F.wl_filter_page.value=F.wl_filter_page.options[num].value;
	F.submit();
}

function to_submit(F) {
	F.submit_button.value = "WL_FilterTable";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="macfilter" action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="action" />
			<input type="hidden" name="wl_mac_list" />
			<input type="hidden" name="small_screen" />
			<div id="main">
				<div id="contentsInfo">
					<h2><% tran("wl_filter.h2"); %></h2>
					<table width="100%" >
						<tr>
							<td align="left"><% tran("wl_filter.h3"); %></td>
							<td align="right"><script type="text/javascript">
							//<![CDATA[
							document.write("<input class=\"button\" type=\"button\" name=\"table_button\" value=\"" + sbutton.wl_client_mac + "\" onclick=\"openWindow('WL_ActiveTable.asp', 650, 450);\" />");
							//]]>
							</script></td>
						</tr>
					</table><br/>
					<% wireless_filter_table("input"); %>
					<div class="submitFooter">
						<script type="text/javascript">
						//<![CDATA[
						submitFooterButton(1,1,0,0,0,1);
						//]]>
						</script>
					</div>
				</div>
			</div>
		</form>
	</body>
</html>