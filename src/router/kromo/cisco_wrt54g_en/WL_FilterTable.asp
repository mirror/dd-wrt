<%% do_pagehead(); %%>
		<title><%% nvram_get("router_name"); %%> - [%s] MAC Address Filter List</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<%% nvram_get("router_name"); %%>" + wl_filter.titl;

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="macfilter" action="apply.cgi" method="<%% get_http_method(); %%>">
			<input type="hidden" name="submit_button" value="WL_FilterTable" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="ifname" value="%s" />
			<input type="hidden" name="%s_mac_list" />
			<div id="main">
				<div id="contentsInfo">
					<h2><%% tran("wl_filter.h2"); %%></h2>
					<table width="100%%" >
						<tr>
							<td align="left"><%% tran("wl_filter.h3"); %%></td>
							<td align="right"><script type="text/javascript">
							//<![CDATA[
							document.write("<input class=\"button\" type=\"button\" name=\"table_button\" value=\"" + sbutton.wl_client_mac + "\" onclick=\"openWindow('WL_ActiveTable.asp', 650, 450);\" />");
							//]]>
							</script></td>
						</tr>
					</table><br/>
					<%% wireless_filter_table("input","%s"); %%>
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