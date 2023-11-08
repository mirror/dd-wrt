<% do_pagehead(); %>
		<title><% nvg("router_name"); %> - Activation</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvg("router_name"); %> - Activation";

function to_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Register";
	F.submit_type.value = "activate";
	F.register.value = "Activating";
	F.action.value = "Apply";
	apply(F);
}
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
});

//]]>
</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><% tran("bmenu.setup"); %></span></li>
								<li><span><% tran("bmenu.wireless"); %></span></li>
								<% nvim("sipgate","1","<!--"); %>
								<li><span><% tran("bmenu.sipath"); %></span></li>
								<% nvim("sipgate","1","-->"); %>
								<% ifdef("HAVE_ANTAIRA_MINI","<!--"); %>
								<li><span><% tran("bmenu.security"); %></span></li>
								<% ifdef("HAVE_ANTAIRA_MINI","-->"); %>
								<% ifdef("HAVE_ANTAIRA_MINI","<!--"); %>
								<li><span><% tran("bmenu.accrestriction"); %></span></li>
								<% ifdef("HAVE_ANTAIRA_MINI","-->"); %>
								<li><span><% tran("bmenu.applications"); %></span></li>
								<li><span><% tran("bmenu.admin"); %></span></li>
								<li><span><% tran("bmenu.statu"); %></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<form name="register" action="apply.cgi" method="post" spellcheck="false">
						<input type="hidden" name="submit_button" value="Register" />
						<input type="hidden" name="submit_type" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="action" value="Apply" />
						<div id="contentsInfo">
							<dl>
								<dd><% tran("reg.not_reg"); %><br></dd>
							</dl>
							<div class="setting">
								<div class="label"><% tran("reg.sys_key"); %></div>
								<textarea cols="80" rows="5" id="sysvalue" name="sysvalue" readonly="true" ><% getregcode(); %></textarea>
							</div>
							<div class="setting">		
								<div class="label"><% tran("reg.act_key"); %></div>
								<textarea cols="80" rows="5" id="regvalue" name="regvalue"> </textarea>
								<script type="text/javascript">
								//<![CDATA[
								var regvalue = fix_cr( '<% nvg("regvalue"); %>' );
								document.getElementById("regvalue").value = regvalue;
								//]]>
								</script>
							</div>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"register\" value=\"Activate\" onclick=\"to_submit(this.form)\" />");
								//]]>
								</script>
							</div>
						</div>
					</form>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>:&nbsp;
						<script type="text/javascript">
						//<![CDATA[
						document.write("<a title=\"" + share.about + "\" href=\"<% get_firmware_version_href(); %>\"><% get_firmware_version(); %></a>");
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
