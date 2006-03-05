<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Log</title>
		<link type="text/css" rel="stylesheet" href="style.css"/>
		<script type="text/JavaScript" src="common.js">{}</script>
		<script language="JavaScript">

function to_submit(F) {
	F.submit_button.value = "Log";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
	return true;
}

function SelLog(num,F) {
	log_enable_disable(F,num);
}

function log_enable_disable(F,I) {
        EN_DIS = I;
        if ( I == "0" ){
                choose_disable(F.log_incoming);
                choose_disable(F.log_outgoing);
                choose_disable(F.log_dropped);
                choose_disable(F.log_accepted);
                choose_disable(F.log_rejected);
		
        }
        else{
                choose_enable(F.log_incoming);
                choose_enable(F.log_outgoing);
                choose_enable(F.log_dropped);
                choose_enable(F.log_accepted);
                choose_enable(F.log_rejected);
        }
}

function ViewLogIn() {
	self.open('Log_incoming.asp','inLogTable','alwaysRaised,resizable,scrollbars,width=580,height=600').focus();
}

function ViewLogOut() {
	self.open('Log_outgoing.asp','outLogTable','alwaysRaised,resizable,scrollbars,width=760,height=600').focus();
}

function ViewLog() {
	self.open('Log_all.asp','inLogTable','alwaysRaised,resizable,scrollbars,width=580,height=600').focus();
}
function init() {               
        log_enable_disable(document.log,"<% nvram_get("log_enable"); %>");
}

		</script>
	</head>
	
	<body class="gui" onload="init();"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li><a href="Wireless_Basic.asp">Wireless</a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li class="current"><span>Administration</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp">Management</a></li>
											<li><a href="Hotspot.asp">Hotspot</a></li>
											<li><a href="Services.asp">Services</a></li>
											<li><a href="Alive.asp">Keep Alive</a></li>
											<li><span>Log</span></li>
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
						<form name="log" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="action"/>
							<h2>Log Management</h2>
							<fieldset>
							<legend>Log</legend>
							<div class="setting">
								<div class="label">Log</div>
								<input type="radio" value="1" name="log_enable" <% nvram_match("log_enable", "1", "checked"); %> onclick="SelLog(1,this.form)">Enable</input>
								<input type="radio" value="0" name="log_enable" <% nvram_match("log_enable", "0", "checked"); %> onclick="SelLog(0,this.form)">Disable</input>
							</div>
							<div class="setting">
								<div class="label">Log Level</div>
								<select name="log_level">
									<option value="0" <% nvram_match("log_level", "0", "selected"); %>>Low</option>
									<option value="1" <% nvram_match("log_level", "1", "selected"); %>>Medium</option>
									<option value="2" <% nvram_match("log_level", "2", "selected"); %>>High</option>
								</select>
							</div>
							</fieldset>
							<br/>
							<fieldset>
								<legend>Log Type</legend>
								<div class="setting">
									<div class="label">Dropped</div>
									<select name="log_dropped">
										<option value="0" <% nvram_invmatch("log_dropped", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_dropped", "1", "selected"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label">Rejected</div>
									<select name="log_rejected">
										<option value="0" <% nvram_invmatch("log_rejected", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_rejected", "1", "selected"); %>>On</option>
									</select>
								</div>
								<div class="setting">
									<div class="label">Accepted</div>
									<select name="log_accepted">
										<option value="0" <% nvram_invmatch("log_accepted", "1", "selected"); %>>Off</option>
										<option value="1" <% nvram_match("log_accepted", "1", "selected"); %>>On</option>
									</select>
								</div>
								<% support_invmatch("SYSLOG_SUPPORT", "1", "<!--"); %>
								<div class="setting">
									<name>IP Address</name><% prefix_ip_get("lan_ipaddr",1); %>
									<input class="num" size="3" maxlength="3" name="log_ipaddr" onblur="valid_range(this,0,254,'IP')" value="<% nvram_get("log_ipaddr"); %>" />
								</div>
								<% support_invmatch("SYSLOG_SUPPORT", "1", "-->"); %>
								<input type="button" value="Incoming Log" name="log_incoming" onclick="ViewLogIn()" id="button1"/>
								<input type="button" value="Outgoing Log" name="log_outgoing" onclick="ViewLogOut()"/>
								<% support_match("SYSLOG_SUPPORT", "1", "<input onclick=\"ViewLog()\" type=\"button\" value=\"System Log\"/>"); %>
							</fieldset>
							<br/>
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)"/>
								<input type="button" value="Cancel Changes" onclick="window.location.replace('../Log.asp')"/>
							</div>
						</form>
					</div>
				</div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<% nvram_match("wan_proto","disabled","<!--"); %>
					<div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
					<% nvram_match("wan_proto","disabled","-->"); %>
                    <div class="info"><% nvram_match("wan_proto","disabled","WAN disabled"); %></div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<br/>
						<a target="_blank" href="help/HLog.asp">More...</a>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>