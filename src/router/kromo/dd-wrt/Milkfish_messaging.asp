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
                                       <input id="sip_message_dest" name="sip_message_dest" type="text" size="50" maxlength="50" value="sip:"> 
                                    </div>
                                    <br/>
				    <div class="setting">
					<div class="label"><% tran("service.milkfish_sipmessage"); %></div>
					<textarea cols="40" rows="4" id="sip_message" name="sip_message"></textarea>
					<script type="text/javascript">
					//<![CDATA[
					    var sip_message = fix_cr( '<% nvram_get("sip_message"); %>' );
					    document.getElementById("sip_message").value = sip_message;
					//]]>
					</script>
				    </div>
                                    <br/>
                                    <div class="center">
                                       <script type="text/javascript">
                                       //<![CDATA[
                                        document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"Send\" onclick=\"milkfish_sip_message_submit(this.form);\" />");
				//]]>
                                       </script>
                                    </div>
				    <br/><br/>
                                    <b>Please note: Not all SIP Clients (Soft- or Hardphones) support SIP Messaging.<br /><br />Please refer to your SIP Clients Datasheet or Documentation to learn more about SIP Messaging capabilities.</b><br /><br />
                                    <br/>
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
