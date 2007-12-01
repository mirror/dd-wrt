<% do_pagehead("bmenu.servicesMilkfish"); %>
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
		<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button" value="Milkfish_phonebook" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
			<div id="main">
				<div id="contentsInfo">
					<h2><% tran("service.milkfish_phonebook"); %></h2>
        <% exec_milkfish_service("milkfish_services ddactive"); %>
        <fieldset>
                <legend><% tran("service.milkfish_registrations"); %></legend>
                        <table class="table center" summary="chap secrets table">
                        <tr>
                                <th width="30%"><% tran("share.user"); %></th>
                                <th width="30%"><% tran("service.milkfish_contact"); %></th>
                                <th width="30%"><% tran("service.milkfish_agent"); %></th>
                        </tr>
                         <% exec_show_registrations(); %>
                        </table><br />
        </fieldset>

                                        <div class="submitFooter">
						<script type="text/javascript">
						//<![CDATA[
						submitFooterButton(0,0,0,0,0,1);
						//]]>
						</script>
					</div>
				</div>
			</div>
		</form>
	</body>
</html>
