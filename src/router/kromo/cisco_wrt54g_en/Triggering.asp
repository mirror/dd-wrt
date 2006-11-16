<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Port Triggering</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + trforward.titl;

function trigger_add_submit(F) {
	F.submit_type.value = "add_trigger";
	F.submit();
}

function trigger_remove_submit(F) {
	F.submit_type.value = "remove_trigger";
	F.submit();
}

function to_submit(F) {
	F.submit_type.value = "";
	F.change_action.value = "";
	F.save_button.value = sbutton.saving;
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
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Forward.asp","Triggering.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="trigger" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Triggering" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" />
							
							<input type="hidden" name="port_trigger" value="10" />
							<h2><% tran("trforward.h2"); %></h2>
							<fieldset>
								<legend><% tran("trforward.legend"); %></legend>
								<table class="table center" cellspacing="5" summary="ports triggering table">
									<tr>
										<td></td>
										<th colspan="2"><% tran("trforward.trrange"); %></th>
										<th colspan="2"><% tran("trforward.fwdrange"); %></th>
										<td></td>
									</tr>
									<tr>
										<th><% tran("trforward.app"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th><% tran("share.enable"); %></th>
									</tr>
									<% show_triggering(); %>
								</table><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name =\"add_button\" value=\"" + sbutton.add + "\" onclick=\"trigger_add_submit(this.form);\" />");
									document.write("<input class=\"button\" type=\"button\" name =\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"trigger_remove_submit(this.form);\" />");
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HTrigger.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>