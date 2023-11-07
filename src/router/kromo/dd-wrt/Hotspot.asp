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
	F.apply_button.value = sbutton.applied;
	if(F._hotss_customsplash) {
		F.hotss_customsplash.value = F._hotss_customsplash.checked ? 1 : 0;
	}
	applytake(F);
}

function handle_hotss(F,value) {
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
		<% ifndef("HAVE_HOTSPOT","/*"); %>
		show_layer_ext(this, 'idhotspotsys', true);
		<% ifndef("HAVE_HOTSPOT","*/"); %>		
		show_layer_ext(this, 'idchilli', false);
	} else {
		setElementsActive("chilli_enable", "chilli_additional", true);
		<% ifndef("HAVE_HOTSPOT","/*"); %>
		show_layer_ext(this, 'idhotspotsys', false);
		<% ifndef("HAVE_HOTSPOT","*/"); %>
		show_layer_ext(this, 'idchilli', <% nvem("chilli_enable", "1", "1", "0"); %> == 1);
		if (<% nvem("chilli_def_enable", "0", "1", "0"); %> == 1) {
			show_layer_ext(this, 'idchilli', false);
			setElementsActive("chilli_enable", "chilli_additional", false);
		}
	}
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	//show_layer_ext(document.setup.apd_enable, 'idsputnik', <% nvem("apd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_enable, 'idwifidog', <% nvem("wd_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_sslavailable, 'idwifidogssl', <% nvem("wd_sslavailable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_auth, 'idauth', <% nvem("wd_auth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_radius, 'idradius', <% nvem("wd_radius", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.wd_radius, 'idauthsrv', <% nvem("wd_radius", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_enable, 'idchilli', <% nvem("chilli_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_nowifibridge, 'idchillidhcp', <% nvem("chilli_nowifibridge", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.chilli_macauth, 'idmacauth', <% nvem("chilli_macauth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.http_redirect_enable, 'idhttpredirect', <% nvem("http_redirect_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.NC_enable, 'idnocat', <% nvem("NC_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.ND_enable, 'idnodog', <% nvem("ND_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.shat_enable, 'idzeroip', <% nvem("shat_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.smtp_redirect_enable, 'smtpredirect', <% nvem("smtp_redirect_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_enable, 'idhotspotsys', <% nvem("hotss_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_uamenable, 'idhotssuam', <% nvem("hotss_uamenable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.hotss_nowifibridge, 'idhotssdhcp', <% nvem("hotss_nowifibridge", "1", "1", "0"); %> == 1);
	setHotss("<% nvem("hotss_enable", "1", "1", "0"); %>");
	if (document.setup.hotss_loginonsplash) {
		handle_hotss(document.setup, <% nvem("hotss_loginonsplash", "1", "1", "0"); %> == 1);
	}
		
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
					<% do_menu("Services.asp","Hotspot.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Hotspot" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							<input type="hidden" name="hotss_preconfig" value="0"/>
							<input type="hidden" name="hotss_customsplash" />
							<h2><% tran("hotspot.h2"); %></h2>
							<% show_modules(".webhotspot"); %>
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
							<dt class="term"><% tran("hotspot.legend"); %> <% tran("hotspotsys.legend"); %></dt>
							<dd class="definition"><% tran("hstatus_hots.right1"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HHotspot.asp');"><% tran("share.more"); %></a>
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
