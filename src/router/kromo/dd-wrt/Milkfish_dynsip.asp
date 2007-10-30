<% do_pagehead("bmenu.servicesMilkfish"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	applytake(F);
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button" value="Milkfish_dynsip" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
			<div id="main">
				<div id="contentsInfo">
					<h2><% tran("service.milkfish_dynsip"); %></h2>
					<br/>
					<b>Leaving all settings empty will automatically default to the Milkfish HomeSIP Service.<br /><br />If unsure, leave everything unset.</b><br />
					<br>					
           				
		            		    <div class="setting">
				        	<div class="label"><% tran("service.milkfish_dynsipdomain"); %></div>
						<input size="27" name="milkfish_dynsipdomain" value="<% nvram_get("milkfish_dynsipdomain"); %>" />
					    </div>
					    <div class="setting">
					        <div class="label"><% tran("service.milkfish_dynsipurl"); %></div>
					        <input size="27" name="milkfish_dynsipurl" value="<% nvram_get("milkfish_dynsipurl"); %>" />
					    </div>
					    <div class="setting">
					        <div class="label"><% tran("service.milkfish_dsusername"); %></div>
						<input size="27" name="milkfish_dsusername" value="<% nvram_get("milkfish_dsusername"); %>" />
					    </div>
					    <div class="setting">
						<div class="label"><% tran("service.milkfish_dspassword"); %></div>
					        <input size="27" name="milkfish_dspassword" value="<% nvram_get("milkfish_dspassword"); %>" />
					    </div>
					
					<br />
				<br />
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
