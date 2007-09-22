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
function milkfish_sip_message_submit(F) {
        F.change_action.value="gozila_cgi";
        F.submit_type.value = "send_message";
        F.submit();
}

		
		//]]>
		</script>
	</head>
	
	<body>
		<form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		<input type="hidden" name="submit_button" value="Milkfish_messaging" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
			<div id="main">
				<div id="contentsInfo">
					<h2>Milkfish SIP messaging</h2>
                                                <div class="setting">
                                                     <div class="label"><% tran("service.milkfish_destination"); %></div>
                                                          <textarea id="sip_message_dest" name="sip_message_dest" rows="1" cols="50" style="font-family:Courier, Courier New" wrap="off"></textarea>
                                                </div>
                                                <br/>
                                                <div class="setting">
                                                     <div class="label"><% tran("service.milkfish_sipmessage"); %></div>
                                                          <textarea id="sip_message" name="sip_message" rows="1" cols="50" style="font-family:Courier, Courier New" wrap="off"></textarea>
                                                </div>
                                                <br/>
                        <div class="center">
                                <script type="text/javascript">
                                //<![CDATA[
                                   document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"Send\" onclick=\"milkfish_sip_message_submit(this.form);\" />");
//]]>
                                </script>
                        </div>

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
</html
