<%% do_pagehead("wl_filter.titl"); %%>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.save_button.value = sbutton.saving;
	applytake(F);
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="macfilter" action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="WL_FilterTable-%s" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="ifname" value="%s" />
			<input type="hidden" name="wl_mac_list" />
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
