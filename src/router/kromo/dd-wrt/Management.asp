<% do_pagehead("management.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function SelPort(num,F)	{
	if(num == 1 && F.PasswdModify.value == 1){
		 if(ChangePasswd(F) == true)
			port_enable_disable(F,num);
	} else {
		port_enable_disable(F,num);
	}
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
		if (F._remote_mgt_https) {
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

	if (!F.http_passwd.value || F.http_passwd.value == "" || F.http_passwd.value.length == 0) {
		alert(errmsg.err6);
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}
	return true;
}

function handle_https(F) {
	if(F._https_enable.checked == true && F.remote_management[0].checked == true) {
		choose_enable(F._remote_mgt_https);
	} else {
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
	if(F._fix_freq) {
	    if (F._fix_freq.checked == true) {
		    F.fix_freq.value = 1;
	    } else {
		    F.fix_freq.value = 0;
	    }
	}
	if(F._remote_mgt_https) {
		if(F.http_enable.checked == true && F.https_enable.checked == false) {
			F._remote_mgt_https.checked == false;
		}
		if(F.http_enable.checked == false && F.https_enable.checked == true) {
			F._remote_mgt_https.checked == true;
		}
		if(F._remote_mgt_https.checked == true) {
			F.remote_mgt_https.value = 1;
		} else {
			F.remote_mgt_https.value = 0;
		}
	}

	if(F._https_enable) {
		if(F._https_enable.checked == true) {
			F.https_enable.value = 1;
		} else {
			F.https_enable.value = 0;
		}
	}

	if(F._http_enable) {
		if(F._http_enable.checked == true) {
			F.http_enable.value = 1;
		} else {
			F.http_enable.value = 0;
		}
	}

	if(F._info_passwd) {
		if(F._info_passwd.checked == true) {
			F.info_passwd.value = 1;
		} else {
			F.info_passwd.value = 0;
		}
	}
	if (F.remote_ip_any)
	if (F.remote_ip_any[1].checked == true){
		if(F.remote_ip_0.value == "0" && F.remote_ip_1.value == "0" && F.remote_ip_2.value == "0" && F.remote_ip_3.value == "0" ) {
			alert(errmsg.err111);
			return false;
		}
		if(parseInt(F.remote_ip_3.value) > parseInt(F.remote_ip_4.value)) {
			alert(errmsg.err110);
			F.remote_ip_4.focus();
			return false;
		}
	}
	return true;
}

function to_submit(F) {
	if (submitcheck(F)) {
		F.save_button.value = sbutton.saving;
		apply(F);
	}
}

function to_apply(F) {
	if (submitcheck(F)) {
		F.apply_button.value = sbutton.applied;
		applytake(F);
	}
}

var update;

addEvent(window, "load", function() {
	port_enable_disable(document.setup, "<% nvg("remote_management"); %>");
	if (document.setup.remote_mgt_ssh)
		selSSH("<% nvg("sshd_enable"); %>", 1);
	if (document.setup.remote_mgt_telnet)
		selTelnet("<% nvg("telnetd_enable"); %>", 1);
	stickControl(<% nvg("sticky_footer"); %>);
	show_layer_ext(document.setup.remote_mgt_ssh, 'idssh', <% nvem("remote_mgt_ssh", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.remote_mgt_telnet, 'idtelnet', <% nvem("remote_mgt_telnet", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.remote_ip_any, 'idremip', <% nvem("remote_ip_any", "1", "0", "1"); %> == 1);
	show_layer_ext(document.setup.remote_management, 'idhttpd', <% nvem("remote_management", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.status_auth, 'idsysinfo', <% nvem("status_auth", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.cron_enable, 'idcron', <% nvem("cron_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.samba_mount, 'idsamba', <% nvem("samba_mount", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.enable_jffs2, 'idjffs2', <% nvem("enable_jffs2", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mmc_enable0, 'idmmc', <% nvem("mmc_enable0", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.mmc_gpio, 'idmmcgpio', <% nvem("mmc_gpio", "1", "1", "0"); %> == 1);

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
					<% do_menu("Management.asp","Management.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="post" autocomplete="new-password" spellcheck="false">
							<input type="hidden" name="submit_button" value="Management" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />

							<input type="hidden" name="PasswdModify" value="<% nvem("http_passwd", "admin", "1", "0"); %>" />
							<input type="hidden" name="remote_mgt_https" />
							<input type="hidden" name="http_enable" />
							<input type="hidden" name="fix_freq" />
							<input type="hidden" name="info_passwd" />
							<input type="hidden" name="https_enable" />
							<h2><% tran("management.h2"); %></h2>
							<% show_modules(".webconfig"); %>
							<% show_modules(".webconfig_release"); %>
							<div id="footer" class="submitFooter">
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
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term"><% tran("hmanagement.right1"); %></dt>
							<dd class="definition"><% tran("hmanagement.right2"); %></dd>
							<dt class="term"><% tran("hmanagement.right3"); %></dt>
							<dd class="definition"><% tran("hmanagement.right4"); %></dd>
							<dt class="term"><% tran("hmanagement.right5"); %></dt>
							<dd class="definition"><% tran("hmanagement.right6"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HManagement.asp');"><% tran("share.more"); %></a>
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
