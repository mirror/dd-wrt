<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - SuperChannel</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %> - SuperChannel";

function to_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "SuperChannel";
	F.submit_type.value = "activate";
	F.register.value = "Activating";
	F.action.value = "Apply";
	apply(F);
}


//]]>
</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
				        <% do_menu("Wireless_Basic.asp","SuperChannel.asp"); %>
				</div>
				<div id="main">
				<div id="contents">
                    		<form name="register" action="apply.cgi" method="post">
                  			<input type="hidden" name="submit_button" value="SuperChannel" />
                  			<input type="hidden" name="submit_type" />
                  			<input type="hidden" name="next_page" value="Wireless_Basic.asp" />
                  			<input type="hidden" name="change_action" />
					<input type="hidden" name="action" value="Apply" />
					<fieldset>
					<legend>SuperChannel feature activation</legend>
					SuperChannel allows you to use special frequencies from 2192 Mhz - 2732 Mhz (802.11g capable devices only) and 4915 Mhz - 6100 Mhz (802.11a capable devices only). This feature is not yet enabled.<br />
					<h2>Disclaimer</h2>
					Consider that in many countries it is not allowed to use these frequencies. DD-WRT / NewMedia-NET GmbH assumes no liability whatsoever, expressed or implied, for the use of this feature.<br />		    			    
						<div class="setting">
						    <div class="label">System Key</div>
						    <% getregcode(); %>
						</div>
						<div class="setting">		
						<div class="label">Activation Key</div>
						<textarea cols="80" rows="5" id="regvalue" name="regvalue"> </textarea>
						<script type="text/javascript">
						//<![CDATA[
						var regvalue = fix_cr( '<% nvram_get("regvalue"); %>' );
						document.getElementById("regvalue").value = regvalue;
						//]]>
						</script>
						</div>
					</fieldset>
					<div class="submitFooter">
							<script type="text/javascript">
							//<![CDATA[
							document.write("<input type=\"button\" name=\"register\" value=\"Activate\" onclick=\"to_submit(this.form)\" />");
							//]]>
							</script>
						</div>
					</form>
				</div>
				</div>
			<div id="helpContainer">
				<div id="help">
					<div><h2><% tran("share.help"); %></h2></div>
					<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSuperChannel.asp')"><% tran("share.more"); %></a>
				</div>
			</div>
			<div id="floatKiller"></div>
			<div id="statusInfo">
			<div class="info"><% tran("share.firmware"); %>: 
				<script type="text/javascript">
				//<![CDATA[
				document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
				//]]>
				</script>
			</div>
			<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
			<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
			</div>
			</div>
		</div>
	</body>
</html>