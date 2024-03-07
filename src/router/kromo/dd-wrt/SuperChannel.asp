<% do_pagehead_nopwc(); %>
	<title><% nvg("router_name"); %> - SuperChannel</title>
	<script type="text/javascript">//
	//<![CDATA[

document.title = "<% nvg("router_name"); %> - SuperChannel";

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
						<form name="register" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="SuperChannel" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" value="Apply" />
							<fieldset>
								<legend><% tran("superchan.legend"); %></legend>
								<h2><% tran("superchan.h2feat"); %></h2>
								<p>SuperChannel allows the use of special frequencies on <b>capable devices only</b>.<br />These frequencies range between <% get_low_2ghz(); %> Mhz to <% get_high_2ghz(); %> Mhz for the 802.11g band and <% get_low_5ghz(); %> Mhz to <% get_high_5ghz(); %> Mhz for the 802.11a band.<br /><b>This feature is not yet enabled.</b></p>
								<h2><% tran("superchan.h2disc"); %></h2>
								<% tran("superchan.lgltxt"); %>
								<div class="setting">
									<div class="label"><% tran("superchan.lsyskey"); %></div>
									<textarea cols="80" rows="5" id="sysvalue" name="sysvalue" readonly="true" >
										<% getregcode(); %>
									</textarea>
								</div>
								<div class="setting">
									<div class="label"><% tran("superchan.lactkey"); %></div>
									<textarea cols="80" rows="5" id="regvalue" name="regvalue"> </textarea>
									<script type="text/javascript">
									//<![CDATA[
									var regvalue = fix_cr( '<% nvg("regvalue"); %>' );
									document.getElementById("regvalue").value = regvalue;
									//]]>
									</script>
								</div>
							</fieldset><br />
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"register\" value=\"Activate\" onclick=\"to_submit(this.form)\" />");
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
						<!-- Hide more... there is no help page here https://svn.dd-wrt.com/ticket/7478
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSuperChannel.asp')"><% tran("share.more"); %></a>
						-->
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
