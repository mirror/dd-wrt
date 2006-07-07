<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Administration</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + management.titl;

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

function to_reboot(F) {
	F.submit_button.value = "Management";
	F.action.value="Reboot";
	F.submit();
	return true;
}

function to_submit(F) {
	if( F.http_passwd.value != F.http_passwdConfirm.value ) {
//		alert("Password confirmation doesn't match.");
		alert(errmsg.err52);
		F.http_passwd.focus();
		return false;
	}

	valid_password(F);

	if(F.remote_management[0].checked == true) {
		if(!ChangePasswd(F))
			return false;
	}
	
	if(F._remote_mgt_https) {
		if(F.http_enable.checked == true && F.https_enable.checked == false)
			F._remote_mgt_https.checked == false;
		if(F.http_enable.checked == false && F.https_enable.checked == true)
			F._remote_mgt_https.checked == true;
		if(F._remote_mgt_https.checked == true) F.remote_mgt_https.value = 1;
		else 	 F.remote_mgt_https.value = 0;
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

	F.submit_button.value = "Management";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value="Apply";
	apply(F);
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

addEvent(window, "load", function() {
	port_enable_disable(document.setup, "<% nvram_get("remote_management"); %>");
	if (document.setup.remote_mgt_ssh)
		selSSH("<% nvram_get("sshd_enable"); %>", 1);

	if (document.setup.ipv6_enable0) {
		show_layer_ext('idipv6', <% nvram_get("ipv6_enable0"); %> == 1);
	}
	if (document.setup.remote_mgt_ssh) {
		show_layer_ext('idssh', <% nvram_get("remote_mgt_ssh"); %> == 1);
	}
	
	show_layer_ext('idhttpd', <% nvram_get("remote_management"); %> == 1);
	show_layer_ext('idsysinfo', <% nvram_get("status_auth"); %> == 1);
	show_layer_ext('idntp', <% nvram_get("ntp_enable"); %> == 1);
	show_layer_ext('idsamba', <% nvram_get("samba_mount"); %> == 1);
	show_layer_ext('idwol', <% nvram_get("wol_enable"); %> == 1);
	show_layer_ext('idjffs2', <% nvram_get("enable_jffs2"); %> == 1)

});

		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li class="current"><span><% tran("bmenu.admin"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span><% tran("bmenu.adminManagement"); %></span></li>
											<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
											<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
											<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
											<li><a href="Log.asp"><% tran("bmenu.adminLog"); %></a></li>
											<li><a href="Diagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
											<li><a href="Wol.asp"><% tran("bmenu.adminWol"); %></a></li>
											<li><a href="Factory_Defaults.asp"><% tran("bmenu.adminFactory"); %></a></li>
								<script type="text/javascript">
										https_visit = <% support_elsematch("HTTPS","1","1","0"); %>;
										if (https_visit =="1") {
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminUpgrade + "</em></a></li>");
											document.write("<li><a style=\"cursor:pointer\" title=\"" + errmsg.err46 + "\" onclick=\"alert(errmsg.err45)\" ><em>" + bmenu.adminBackup + "</em></a></li>");
										} else {
											document.write("<li><a href=\"Upgrade.asp\">" + bmenu.adminUpgrade + "</a></li>");
											document.write("<li><a href=\"config.asp\">" + bmenu.adminBackup + "</a></li>");
										}											
								</script>
<!--										<li><a href="Upgrade.asp">Firmware Upgrade</a></li>
											<li><a href="config.asp">Backup</a></li>
 -->
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
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
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"reboot_button\" value=\"" + sbutton.reboot + "\" onclick=\"to_reboot(this.form)\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><% tran("share.help"); %></h2>
						</div>
						<dl>
							<dt class="term"><% tran("hmanagement.right1"); %></dt>
							<dd class="definition"><% tran("hmanagement.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HManagement.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>