<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Port Range Forwarding</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + prforward.titl;

function forward_add_submit(F) {
	F.submit_type.value = "add_forward";
	F.submit();
}

function forward_remove_submit(F) {
	F.submit_type.value = "remove_forward";
	F.submit();
}

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
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
					<% do_menu("Forward.asp","Forward.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="portRange" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Forward" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HForward.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>