<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Hotspot</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + hotspot.titl;

function user_add_submit(F) {
	F.submit_type.value = "add_user";
	F.submit();
}

function iradius_add_submit(F) {
	F.submit_type.value = "add_iradius";
	F.submit();
}

function user_remove_submit(F) {
	F.submit_type.value = "remove_user";
	F.submit();
}

function to_submit(F) {
	<% show_iradius_check(); %>
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	apply(F);
}

addEvent(window, "load", function() {
	show_layer_ext(document.setup.apd_enable, 'idsputnik', <% nvram_else_match("apd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_enable, 'idwifidog', <% nvram_else_match("wd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_sslavailable, 'idwifidogssl', <% nvram_else_match("wd_sslavailable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_enable, 'idchilli', <% nvram_else_match("chilli_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.http_redirect_enable, 'idhttpredirect', <% nvram_else_match("http_redirect_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.NC_enable, 'idnocat', <% nvram_else_match("NC_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.smtp_redirect_enable, 'smtpredirect', <% nvram_else_match("smtp_redirect_enable", "1", "1", "0"); %> == 1);
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
					<% do_menu("Management.asp","Hotspot.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Hotspot" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							
							<h2><% tran("hotspot.h2"); %></h2>
							<% show_modules(".webhotspot"); %>
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
						<div><h2><% tran("share.help"); %></h2></div><br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HHotspot.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>