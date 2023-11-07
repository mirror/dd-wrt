<% do_pagehead("pforwardip.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function forward_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_forward_ip";
	apply(F);
}

function forward_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.del_value.value = I;
	F.submit_type.value = "del_forward_ip";
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
	F.apply_button.value = sbutton.applied;
	applytake(F);
}
	
var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
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
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("ForwardSpec.asp","ForwardIP.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="portRange" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="ForwardIP" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="del_value" />
							<input type="hidden" name="forward_ip" value="13" />
							<h2><% tran("pforwardip.h2"); %></h2>
							<fieldset>
								<legend><% tran("pforwardip.legend"); %></legend>
								<table class="table" cellspacing="5" summary="ports forwarding table">
								<thead>
									<tr>
										<th><% tran("pforwardip.name"); %></th>
										<th><% tran("pforwardip.src"); %></th>
										<th><% tran("pforwardip.dest"); %></th>
										<th class="center" width="10%"><% tran("share.enable"); %></th>
										<th class="center" width="10%"><% tran("share.actiontbl"); %></th>
									</tr>
								</thead>
								<tbody>
									<% show_forward_ip(); %>
									<tr>
										<td colspan="4">&nbsp;</td>
										<td class="center">
											<div>
											<script type="text/javascript">
												//<![CDATA[
												document.write("<input class=\"add\" type=\"button\" name=\"add_button\" aria-label=\"" + sbutton.add + "\" onclick=\"forward_add_submit(this.form);\" />");
												//]]>
											</script>
											</div>
										</td>
									</tr>
								</tbody>
								</table>
							</fieldset><br />
							<div id="footer" class="submitFooter">
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
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term"><% tran("pforward.h2"); %>:</dt>
							<dd class="definition"><% tran("hpforward.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HForwardSpec.asp')"><% tran("share.more"); %></a>
					</div>
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
