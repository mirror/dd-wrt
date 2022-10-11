<% do_pagehead("freeradius.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save_radius_user";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save_radius_user";
	F.apply_button.value = sbutton.applied;
	applytake(F);
}

function generate_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "generate_certificate";
	F.submit();
}

function user_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_radius_user";
	F.submit();
}

function user_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_radius_user";
	F.del_value.value=I;
	F.submit();
}

function client_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_radius_client";
	F.submit();
}

function client_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_radius_client";
	F.del_value.value=I;
	F.submit();
}

function make_client_cert(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "make_client_cert";
	F.index_value.value=I;
	F.submit();
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	show_layer_ext(document.radius.radius_enabled, 'idradius', <% nvem("radius_enabled", "1", "1", "0"); %> == 1);
	update = new StatusUpdate("FreeRadius.live.asp", <% nvg("refresh_time"); %>);
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
					<% do_menu("Services.asp","FreeRadius.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="radius" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="FreeRadius" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="del_value" />
							<input type="hidden" name="index_value" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />

							<h2><% tran("freeradius.h2"); %></h2>
							<fieldset>
								<legend><% tran("freeradius.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("freeradius.srv"); %></div>
									<input class="spaceradio" type="radio" name="radius_enabled" value="1" <% nvc("radius_enabled", "1"); %> onclick="show_layer_ext(this, 'idradius', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="radius_enabled" value="0" <% nvc("radius_enabled", "0"); %> onclick="show_layer_ext(this, 'idradius', false)" /><% tran("share.disable"); %>
								</div>
							</fieldset><br />
							<div id="idradius">
								<fieldset>
									<legend><% tran("wl_basic.legend"); %></legend>
									<div class="setting">
										<div class="label"><% tran("freeradius.port"); %></div>
										<input class="num" maxlength="5" size="5" name="radius_port" onblur="valid_range(this,1,65535,freeradius.port)" value="<% nvg("radius_port"); %>" />
										<span class="default">
											<script type="text/javascript">
											//<![CDATA[
											document.write("(" + share.deflt + ": 1812)");
											//]]>
											</script>
										</span>
									</div>
								</fieldset><br />
								<fieldset>
									<legend><% tran("freeradius.certificate"); %></legend>
									<div class="setting">
										<div class="label"><% tran("freeradius.countrycode"); %></div>
										<input size="3" name="radius_country" value="<% nvg("radius_country"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.state"); %></div>
										<input size="32" name="radius_state" value="<% nvg("radius_state"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.locality"); %></div>
										<input size="32" name="radius_locality" value="<% nvg("radius_locality"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.organisation"); %></div>
										<input size="32" name="radius_organisation" value="<% nvg("radius_organisation"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.email"); %></div>
										<input size="32" name="radius_email" value="<% nvg("radius_email"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.common"); %></div>
										<input size="32" name="radius_common" value="<% nvg("radius_common"); %>" />
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.expiration"); %></div>
										<input class="num" maxlength="5" size="5" name="radius_expiration" value="<% nvg("radius_expiration"); %>" />
										<span class="default">
											<script type="text/javascript">
											//<![CDATA[
											document.write("(" + share.deflt + ": 365)");
											//]]>
											</script>
										</span>
									</div>
									<div class="setting">
										<div class="label"><% tran("freeradius.passphrase"); %></div>
										<input size="32" name="radius_passphrase" value="<% nvg("radius_passphrase"); %>" />
									</div>
									<div class="center"><br />
										<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" name=\"generate_button\" value=\"" + freeradius.cert + "\" onclick=\"generate_submit(this.form);\" />");
										//]]>
										</script>
									</div>
								</fieldset><br />
								<fieldset>
									<legend><% tran("freeradius.cert_status"); %></legend>
									<div class="setting">
										<span id="certificate_status"><% show_certificate_status(); %></span>&nbsp;
									</div>
								</fieldset><br />
								<fieldset>
									<legend><% tran("freeradius.clients"); %></legend>
									<% show_radius_clients(); %>
								</fieldset><br />
								<fieldset>
									<legend><% tran("freeradius.users"); %></legend>
									<% show_radius_users(); %>
								</fieldset><br />
							</div>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(1,1,0,autoref);
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
							<dt class="term"><% tran("freeradius.legend"); %>&nbsp;<% tran("share.requiremts"); %>:</dt>
							<dd class="definition"><% tran("hfreeradius.right2"); %></dd>
						</dl><br />
						<!-- Hide more... there is no help page here https://svn.dd-wrt.com/ticket/7478
							<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HFreeRadius.asp');"><% tran("share.more"); %></a>
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
