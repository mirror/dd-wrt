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
		<form name="macfilter" action="apply.cgi<% insertpageToken(); %>" method="post">
			<input type="hidden" name="submit_button" value="WL_FilterTable" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="ifname" value="%s" />
			<input type="hidden" name="%s_mac_list" />
			<div id="main">
				<div id="contentsInfo" style="padding-top: 0px;">
					<h2><%% tran("wl_filter.h2"); %%></h2>
					<table width="100%%" >
						<tr>
							<td align="left"><%% tran("wl_filter.h3"); %%></td>
							<td align="right"><script type="text/javascript">
							//<![CDATA[
							document.write("<input class=\"button\" type=\"button\" name=\"table_button\" value=\"" + sbutton.wl_client_mac + "\" onclick=\"openWindow('WL_ActiveTable-%s.asp', 650, 450, 'ActiveTable');\" />");
							//]]>
							</script></td>
						</tr>
					</table><br/>
					<%% wireless_filter_table("input","%s"); %%>
					<div id="submit_footer" class="submitFooter">
						<script type="text/javascript">
						//<![CDATA[
						submitFooterButton(1,1,0,0,0,1);
						var children = document.getElementById('submit_footer').childNodes;
						for(var i = 0; i < children.length; i++) {
							if(children[i].name == "apply_button") {
								document.getElementById('submit_footer').removeChild(children[i]);
							}
							if(children[i].name == "reset_button") {
								document.getElementById('submit_footer').childNodes[i].onclick = function(){
									var ref = document.forms[0].elements['submit_button'].value;
									if( document.forms[0].elements['ifname'].value) {
										ref = ref + '-' + document.forms[0].elements['ifname'].value;
									}
									document.location = ref + '.asp';
								};
							}
						}
						//]]>
						</script>
					</div>
				</div>
			</div>
		</form>
	</body>
</html>
