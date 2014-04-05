<% do_pagehead("prforward.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function forward_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "add_forward";
	apply(F);
}

function forward_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "remove_forward";
	apply(F);
}

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
	
var update;

addEvent(window, "load", function() {
	
	update = new StatusbarUpdate();
	update.start();

});

addEvent(window, "unload", function() {
	update.stop();

});
	
		//]]>
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("ForwardSpec.asp","Forward.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="portRange" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Forward" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<input type="hidden" name="forward_port" value="13" />
							<h2><% tran("prforward.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("prforward.legend"); %></legend>
								<table class="table center" cellspacing="5" summary="port forwarding table">
									<tr>
										<th><% tran("prforward.app"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th><% tran("share.proto"); %></th>
										<th><% tran("share.ip"); %></th>
										<th><% tran("share.enable"); %></th>
									</tr>
									<% show_forward(); %>
								</table><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"forward_add_submit(this.form);\" />");
									document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"forward_remove_submit(this.form);\" />");
									//]]>
									</script>
								</div>
							</fieldset><br />
							<div class="submitFooter">
							<script type="text/javascript">
							//<![CDATA[
							submitFooterButton(1,1);
							//]]>
							</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("prforward.h2"); %>:</dt>
							<dd class="definition"><% tran("hprforward.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HForward.asp')"><% tran("share.more"); %></a>
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