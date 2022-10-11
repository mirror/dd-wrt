<% do_pagehead("trforward.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function trigger_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_trigger";
	apply(F);
}

function trigger_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.del_value.value = I;
	F.submit_type.value = "del_trigger";
	apply(F);
}

function to_submit(F) {
	F.submit_type.value = "";
	F.change_action.value = "";
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.submit_type.value = "";
	F.change_action.value = "";
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
					<% do_menu("ForwardSpec.asp","Triggering.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="trigger" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Triggering" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="del_value" />
							<input type="hidden" name="port_trigger" value="10" />
							<h2><% tran("trforward.h2"); %></h2>
							<fieldset>
								<legend><% tran("trforward.legend"); %></legend>
								<table class="table" cellspacing="5" summary="ports triggering table">
									<thead>
									<tr>
										<td></td>
										<th class="center" colspan="2" width="30%"><% tran("trforward.trrange"); %></th>
										<th class="center" colspan="3" width="30%"><% tran("trforward.fwdrange"); %></th>
										<td></td>
									</tr>
									<tr>
										<th><% tran("trforward.app"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th><% tran("share.proto"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th class="center" width="10%"><% tran("share.enable"); %></th>
										<th class="center" width="10%"><% tran("share.actiontbl"); %></th>
									</tr>
								</thead>
								<tbody>
									<% show_triggering(); %>
									<tr>
										<td colspan="7">&nbsp;</td>
										<td class="center">
											<script type="text/javascript">
												//<![CDATA[
												document.write("<input class=\"add\" type=\"button\" name =\"add_button\" aria-label=\"" + sbutton.add + "\" onclick=\"trigger_add_submit(this.form);\" />");
												//]]>
											</script>
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
							<dt class="term"><% tran("trforward.app"); %>:</dt>
							<dd class="definition"><% tran("htrforward.right2"); %></dd>
							<dt class="term"><% tran("trforward.trrange"); %>:</dt>
							<dd class="definition"><% tran("htrforward.right4"); %></dd>
							<dt class="term"><% tran("trforward.fwdrange"); %>:</dt>
							<dd class="definition"><% tran("htrforward.right6"); %></dd>
							<dt class="term"><% tran("share.start"); %>:</dt>
							<dd class="definition"><% tran("htrforward.right8"); %></dd>
							<dt class="term"><% tran("share.end"); %>:</dt>
							<dd class="definition"><% tran("htrforward.right10"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HTrigger.asp')"><% tran("share.more"); %></a>
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
