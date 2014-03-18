<% do_pagehead("hotspot.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function user_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_user";
	F.submit();
}

function iradius_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_iradius";
	F.submit();
}

function user_remove_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "remove_user";
	F.submit();
}

function to_submit(F) {
	<% show_iradius_check(); %>
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	if(F._hotss_customsplash) {
		F.hotss_customsplash.value = F._hotss_customsplash.checked ? 1 : 0;
	}
	apply(F);
}
function to_apply(F) {
	<% show_iradius_check(); %>
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	if(F._hotss_customsplash) {
		F.hotss_customsplash.value = F._hotss_customsplash.checked ? 1 : 0;
	}
	applytake(F);
}

function handle_hotss(F,value)
{
	if(value == 0) {
		choose_enable(F._hotss_customsplash);
	}
	else {
		F._hotss_customsplash.checked = false;
		choose_disable(F._hotss_customsplash);
	}
}


function setHotss(val) {
	if (val == "1") {
		//document.setup.chilli_enable[1].click();
		setElementsActive("chilli_enable", "chilli_additional", false);
		show_layer_ext(this, 'idhotspotsys', true);
		show_layer_ext(this, 'idchilli', false);
		}
	else {
		setElementsActive("chilli_enable", "chilli_additional", true);
		show_layer_ext(this, 'idhotspotsys', false);
		show_layer_ext(this, 'idchilli', <% nvram_else_match("chilli_enable", "1", "1", "0"); %> == 1);
		if (<% nvram_else_match("chilli_def_enable", "0", "1", "0"); %> == 1) {
			show_layer_ext(this, 'idchilli', false);
			setElementsActive("chilli_enable", "chilli_additional", false);
			}
		}
}


var update;

addEvent(window, "load", function() {
	show_layer_ext(document.setup.apd_enable, 'idsputnik', <% nvram_else_match("apd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_enable, 'idwifidog', <% nvram_else_match("wd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_sslavailable, 'idwifidogssl', <% nvram_else_match("wd_sslavailable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_auth, 'idauth', <% nvram_else_match("wd_auth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_radius, 'idradius', <% nvram_else_match("wd_radius", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_radius, 'idauthsrv', <% nvram_else_match("wd_radius", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_enable, 'idchilli', <% nvram_else_match("chilli_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_nowifibridge, 'idchillidhcp', <% nvram_else_match("chilli_nowifibridge", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_macauth, 'idmacauth', <% nvram_else_match("chilli_macauth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.http_redirect_enable, 'idhttpredirect', <% nvram_else_match("http_redirect_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.NC_enable, 'idnocat', <% nvram_else_match("NC_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.smtp_redirect_enable, 'smtpredirect', <% nvram_else_match("smtp_redirect_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_enable, 'idhotspotsys', <% nvram_else_match("hotss_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_uamenable, 'idhotssuam', <% nvram_else_match("hotss_uamenable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_nowifibridge, 'idhotssdhcp', <% nvram_else_match("hotss_nowifibridge", "1", "1", "0"); %> == 1);
	setHotss("<% nvram_else_match("hotss_enable", "1", "1", "0"); %>");
	if (document.setup.hotss_loginonsplash)
		handle_hotss(document.setup, <% nvram_else_match("hotss_loginonsplash", "1", "1", "0"); %> == 1);
		
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
					<% do_menu("Services.asp","Hotspot.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi<% insertpageToken(); %>" method="post">
							<input type="hidden" name="submit_button" value="Hotspot" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							<input type="hidden" name="hotss_preconfig" value="0"/>
							<input type="hidden" name="hotss_customsplash" />
							
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
						<dl>
							<dt class="term"><% tran("hotspot.legend"); %> <% tran("hotspotsys.legend"); %></dt>
							<dd class="definition"><% tran("hstatus_hots.right1"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HHotspot.asp');"><% tran("share.more"); %></a>
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
