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
function milkfish_user_add_submit(F) {
        F.change_action.value="gozila_cgi";
        F.submit_type.value = "add_milkfish_user";
        F.submit();
}

function milkfish_user_remove_submit(F) {
        F.change_action.value="gozila_cgi";
        F.submit_type.value = "remove_milkfish_user";
        F.submit();
}
		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button" value="Milkfish_database" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
                <input type="hidden" name="milkfish_ddsubscribers" />
			<div id="main">
				<div id="contentsInfo">
					<h2><% tran("service.milkfish_database"); %></h2>
					<br />
        <fieldset>
                <legend><% tran("service.database_subscribers"); %></legend>
                        <table class="table center" summary="chap secrets table">
                        <tr>
                                <th width="30%"><% tran("share.user"); %></th>
                                <th width="30%"><% tran("share.passwd"); %></th>
                        </tr>
                        <% exec_show_subscribers(); %> 
                        </table><br />
                        <div class="center">
                                <script type="text/javascript">
                                //<![CDATA[
                                   document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"milkfish_user_add_submit(this.form);\" />"); 
                                   document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"milkfish_user_remove_submit(this.form);\" />");
//]]>
                                </script>
                        </div>
        </fieldset>


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
