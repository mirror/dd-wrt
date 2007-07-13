<% do_pagehead("bmenu.servicesMilkfish"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button" value="Milkfish_status" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" value="gozila_cgi" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
			<div id="main">
				<div id="contentsInfo">
					<h2>Milkfish - SIP Status</h2>
                                        <% exec_milkfish_service("milkfish_services status"); %>
                                        <div class="submitFooter">
						<script type="text/javascript">
						//<![CDATA[
						//submitFooterButton(1,1,0,0,0,1);
						//]]>
						</script>
					</div>
				</div>
			</div>
		</form>
	</body>
</html>
