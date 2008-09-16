<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Activation</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %> - Activation";

function to_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Register";
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
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><% tran("bmenu.setup"); %></span></li>
								<li><span><% tran("bmenu.wireless"); %></span></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><span><% tran("bmenu.sipath"); %></span></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><span><% tran("bmenu.security"); %></span></li>
								<li><span><% tran("bmenu.accrestriction"); %></span></li>
								<li><span><% tran("bmenu.applications"); %></span></li>
								<li><span><% tran("bmenu.admin"); %></span></li>
								<li><span><% tran("bmenu.statu"); %></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
                    		<form name="register" action="apply.cgi" method="post">
                  			<input type="hidden" name="submit_button" value="Register" />
                  			<input type="hidden" name="submit_type" />
                  			<input type="hidden" name="change_action" />
					<input type="hidden" name="action" value="Apply" />

					<div id="contentsInfo">
			<dl>
				<dd>Your system is not activated. Please contact your local dealer for obtaining a valid Activation Key to the displayed System Key.<br></dd>
			</dl>

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
						<div class="submitFooter">
							<script type="text/javascript">
							//<![CDATA[
							document.write("<input type=\"button\" name=\"register\" value=\"Activate\" onclick=\"to_submit(this.form)\" />");
							//]]>
							</script>
						</div>
					</div>
				</form>
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