<% do_pagehead("management.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function SelPort(num,F)	{
	if(num == 1 && F.PasswdModify.value == 1){
		 if(ChangePasswd(F) == true)
			port_enable_disable(F,num);
	} else
		port_enable_disable(F,num);
}

function port_enable_disable(F,I) {
	if ( I == "0" ){
		if (F.http_wanport) choose_disable(F.http_wanport);
		if (F._remote_mgt_https) choose_disable(F._remote_mgt_https);
	} else {
		if (F.http_wanport) choose_enable(F.http_wanport);
		if (F._remote_mgt_https) choose_enable(F._remote_mgt_https);
	}
	if(F._http_enable.checked == false)
	if(F._https_enable)
	if(F._https_enable.checked == true) {
		if (F._remote_mgt_https)
		{
		choose_disable(F._remote_mgt_https);
		F._remote_mgt_https.checked = true;
		}
	}
	if(F._https_enable)
	if(F._http_enable.checked == true && F._https_enable.checked == false)
		choose_disable(F._remote_mgt_https);
}

function ChangePasswd(F) {
	if((F.PasswdModify.value==1 && F.http_passwd.value == "d6nw5v1x2pc7st9m") || F.http_passwd.value == "admin") {
		if(confirm(errmsg.err51)) {
			F.remote_management[1].checked = true;
			return false;
		} else {
			F.remote_management[1].checked = true;
			return false;
		}
	}

	return true;
}

function valid_password(F) {
	if (F.http_passwd.value != F.http_passwdConfirm.value) {
		alert(errmsg.err10);
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}

	return true;
}

function handle_https(F)
{
	if(F._https_enable.checked == true && F.remote_management[0].checked == true) {
		choose_enable(F._remote_mgt_https);
	}
	else {
		F._remote_mgt_https.checked = false;
		choose_disable(F._remote_mgt_https);
	}
}

function selSSH(val, load) {
	if (load == 1) {
		sshd = document.getElementsByName('remote_mgt_ssh');
		setElementActive("remote_mgt_ssh", val == "1");
		setElementActive("sshd_wanport", val == "1" && sshd[0].checked);
	} else {
		setElementActive("sshd_wanport", val == "1");
	}
}

function selTelnet(val, load) {
	if (load == 1) {
		telnet = document.getElementsByName('remote_mgt_telnet');
		setElementActive("remote_mgt_telnet", val == "1");
		setElementActive("telnet_wanport", val == "1" && telnet[0].checked);
	} else {
		setElementActive("telnet_wanport", val == "1");
	}
}

function to_reboot(F) {
	F.action.value="Reboot";
	apply(F);
}

function submitcheck(F) {
	if( F.http_passwd.value != F.http_passwdConfirm.value ) {
		alert(errmsg.err52);
		F.http_passwd.focus();
		return false;
	}

	valid_password(F);

	if(F.remote_management)
	if(F.remote_management[0].checked == true) {
		if(!ChangePasswd(F))
			return false;
	}
	
	if(F._remote_mgt_https) {
		if(F.http_enable.checked == true && F.https_enable.checked == false)
			F._remote_mgt_https.checked == false;
		if(F.http_enable.checked == false && F.https_enable.checked == true)
			F._remote_mgt_https.checked == true;
		if(F._remote_mgt_https.checked == true)
			F.remote_mgt_https.value = 1;
		else
			F.remote_mgt_https.value = 0;
	}
	
	if(F._https_enable) {
		if(F._https_enable.checked == true)
			F.https_enable.value = 1;
		else
			F.https_enable.value = 0;
	}

	if(F._http_enable) {
		if(F._http_enable.checked == true)
			F.http_enable.value = 1;
		else
			F.http_enable.value = 0;
	}
	
	if(F._info_passwd) {
		if(F._info_passwd.checked == true)
			F.info_passwd.value = 1;
		else
			F.info_passwd.value = 0;
	}

	F.save_button.value = sbutton.saving;
	return true;
}
function to_submit(F) {
    if (submitcheck(F))
    apply(F);
}
function to_apply(F) {
    if (submitcheck(F))
    applytake(F);
}

var update;

addEvent(window, "load", function() {
	port_enable_disable(document.setup, "<% nvram_get("remote_management"); %>");
	if (document.setup.remote_mgt_ssh)
		selSSH("<% nvram_get("sshd_enable"); %>", 1);
	if (document.setup.remote_mgt_telnet)
		selTelnet("<% nvram_get("telnetd_enable"); %>", 1);

	show_layer_ext(document.setup.ipv6_enable0,'idipv6', <% nvram_else_match("ipv6_enable0", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.remote_mgt_ssh, 'idssh', <% nvram_else_match("remote_mgt_ssh", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.remote_mgt_telnet, 'idtelnet', <% nvram_else_match("remote_mgt_telnet", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.remote_management, 'idhttpd', <% nvram_else_match("remote_management", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.status_auth, 'idsysinfo', <% nvram_else_match("status_auth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.cron_enable, 'idcron', <% nvram_else_match("cron_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.samba_mount, 'idsamba', <% nvram_else_match("samba_mount", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.enable_jffs2, 'idjffs2', <% nvram_else_match("enable_jffs2", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mmc_enable0, 'idmmc', <% nvram_else_match("mmc_enable0", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mmc_gpio, 'idmmcgpio', <% nvram_else_match("mmc_gpio", "1", "1", "0"); %> == 1);
	
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
					<% do_menu("Management.asp","Management.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Management" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />

							<input type="hidden" name="PasswdModify" value="<% nvram_else_match("http_passwd", "admin", "1", "0"); %>" />
							<input type="hidden" name="remote_mgt_https" />
							<input type="hidden" name="http_enable" />
							<input type="hidden" name="info_passwd" />
							<input type="hidden" name="https_enable" />
							<h2><% tran("management.h2"); %></h2>
				<% show_modules(".webconfig"); %>
				<% show_modules(".webconfig_release"); %>
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1,1);
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
							<dt class="term"><% tran("hmanagement.right1"); %></dt>
							<dd class="definition"><% tran("hmanagement.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HManagement.asp');"><% tran("share.more"); %></a>
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