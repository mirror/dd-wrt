<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Administration</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
var EN_DIS1 = '<% nvram_get("remote_management"); %>'
var wan_proto = '<% nvram_get("wan_proto"); %>'

function SelPort(num,F)	{
	if(num == 1 && F.PasswdModify.value == 1){
		 if(ChangePasswd(F) == true)
			port_enable_disable(F,num);
	} else
		port_enable_disable(F,num);
}

function port_enable_disable(F,I) {
	EN_DIS2 = I;
	if ( I == "0" ){
		if (F.http_wanport)
		choose_disable(F.http_wanport);
		if (F._remote_mgt_https)
		choose_disable(F._remote_mgt_https);
	} else {
		if (F.http_wanport)
		choose_enable(F.http_wanport);
		if (F._remote_mgt_https)
		choose_enable(F._remote_mgt_https);
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
		if(confirm("The Router is currently set to its default password. As a security measure, you must change the password before the Remote Management feature can be enabled. Click the OK button to change your password.  Click the Cancel button to leave the Remote Management feature disabled.")) {
			//window.location.replace('Management.asp');
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
		alert("Confirmed password did not match Entered Password. Please re-enter password");
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}

	return true;
}

function to_reboot(F) {
	F.action.value="Reboot";
	F.submit();
	return true;
}

function to_submit(F) {
	if( F.http_passwd.value != F.http_passwdConfirm.value ) {
		alert("Password confirmation doesn't match !");
		return false;
	}
	
	F.action.value="Apply";

	valid_password(F);

	if(F.remote_management[0].checked == true){
		if(!ChangePasswd(F))
			return false;
	}
	if(F._remote_mgt_https){
		if(F.http_enable.checked == true && F.https_enable.checked == false)
			F._remote_mgt_https.checked == false;
		if(F.http_enable.checked == false && F.https_enable.checked == true)
			F._remote_mgt_https.checked == true;
		if(F._remote_mgt_https.checked == true) F.remote_mgt_https.value = 1;
		else 	 F.remote_mgt_https.value = 0;
	} 
	if(F._https_enable){
		if(F._https_enable.checked == true)
			F.https_enable.value = 1;
		else
			F.https_enable.value = 0;
	}

	if(F._http_enable){
		if(F._http_enable.checked == true)
			F.http_enable.value = 1;
		else
			F.http_enable.value = 0;
	}
	if(F._info_passwd){
		if(F._info_passwd.checked == true)
			F.info_passwd.value = 1;
		else
			F.info_passwd.value = 0;
	}
	F.submit_button.value = "Management";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
	return true;
}

function handle_https(F)
{
	if(F._https_enable.checked == true && F.remote_management[0].checked == true) {
		choose_enable(F._remote_mgt_https);
	}
	else {
		choose_disable(F._remote_mgt_https);
	}
} 

function init() {
	port_enable_disable(document.setup, '<% nvram_get("remote_management"); %>');
}

		</script>
	</head>
	
	<body class="gui" onload="init()"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1>
					</div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li><a href="Wireless_Basic.asp">Wireless</a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li class="current"><span>Administration</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span>Management</span></li>
											<li><a href="Hotspot.asp">Hotspot</a></li>
											<li><a href="Services.asp">Services</a></li>
											<li><a href="Alive.asp">Keep Alive</a></li>
											<li><a href="Log.asp">Log</a></li>
											<li><a href="Diagnostics.asp">Diagnostics</a></li>
											<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
											<li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Firmware Upgrade</a>","<a href="Upgrade.asp">Firmware Upgrade</a>"); %></li> 
											<li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Backup</a>","<a href="config.asp">Backup</a>"); %></li>
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="reboot_button"/>
							<input type="hidden" name="commit" value="1"/>
							<input type="hidden" name="PasswdModify" value='<% nvram_else_match("http_passwd", "admin", "1", "0"); %>'/>
							<input type="hidden" name="remote_mgt_https"/>
							<input type="hidden" name="http_enable"/>
							<input type="hidden" name="info_passwd"/>
							<input type="hidden" name="https_enable"/>
							<h2>Router Management</h2>
							<% show_modules(".webconfig"); %>
							<% show_modules(".webconfig_release"); %>
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)"/>
								<input type="reset" value="Cancel Changes"/>
								<input type="button" value="Reboot Router" onclick="to_reboot(this.form)"/>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<br/>
						<a href="javascript:openHelpWindow('HManagement.asp');">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>