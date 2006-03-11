<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
	<title><% nvram_get("router_name"); %> - Router Status</title>
	<link type="text/css" rel="stylesheet" href="style.css"/>
	<script type="text/JavaScript" src="common.js">{}</script>
	<script language="JavaScript">
  
function DHCPAction(F,I) {
	F.submit_type.value = I;
	F.submit_button.value = "Status_Router";
	F.change_action.value = "gozila_cgi";
	F.submit();
}

function Connect(F,I) {
	F.submit_type.value = I;
	F.submit_button.value = "Status_Router";
	F.change_action.value = "gozila_cgi";
	F.submit();
}

function init() {
	<% show_status("onload");%>
}

function ShowAlert(M) {
	var str = "";
	var mode = "";
	var wan_ip = "<% nvram_status_get("wan_ipaddr"); %>";
	var wan_proto = "<% nvram_safe_get("wan_proto"); %>";

	if(document.status.wan_proto.value == "pppoe")
		mode = "PPPoE";
	else if(document.status.wan_proto.value == "heartbeat")
		mode = "HBS";
	else
		mode = "PPTP";

	if(M == "AUTH_FAIL" || M == "PAP_AUTH_FAIL" || M == "CHAP_AUTH_FAIL")
		str = mode + " authentication fail";
	else if(M == "IP_FAIL" || (M == "TIMEOUT" && wan_ip == "0.0.0.0"))
		str = "Can not get a IP address from " + mode + " server";
	else if(M == "NEG_FAIL")
		str = mode + " negotication fail";
	else if(M == "LCP_FAIL")
		str = mode + " LCP negotication fail";
	else if(M == "TCP_FAIL" || (M == "TIMEOUT" && wan_ip != "0.0.0.0" && wan_proto == "heartbeat"))
		str = "Can not build a TCP connection to " + mode + " server";
	else
		str = "Can not connect to " + mode + " server";

	alert(str);

	Refresh();
}

var value=0;

function Refresh() {
	var refresh_time = "<% show_status("refresh_time"); %>";
	if(refresh_time == "")	refresh_time = 60000;
	if (value>=1) {
		window.location.replace("Status_Router.asp");
	}
	value++;
	timerID=setTimeout("Refresh()",refresh_time);
}

function ViewDHCP() {
	dhcp_win = self.open('DHCPTable.asp','inLogTable','alwaysRaised,resizable,scrollbars,width=720,height=600');
	dhcp_win.focus();
}

var mem_info = new Array('mem:'<% dumpmeminfo(); %>);

var mem_total = parseInt(mem_info[19]);
var mem_free = parseInt(mem_info[22]);
var mem_used = mem_total - mem_free;
var mem_buffer = parseInt(mem_info[28]);
var mem_cached = parseInt(mem_info[31]);
var mem_active = parseInt(mem_info[37]);
var mem_inactive = parseInt(mem_info[40]);

with(Math) { var mem_system = pow(2,(ceil(log(mem_total)/LN2))); };

var mem_total_f = mem_total / mem_system * 100;
var mem_free_f = mem_free / mem_total * 100;
var mem_used_f = mem_used / mem_total * 100;
var mem_buffer_f = mem_buffer / mem_used * 100;
var mem_cached_f = mem_cached / mem_used * 100;
var mem_active_f = mem_active / mem_used * 100;
var mem_inactive_f = mem_inactive / mem_used * 100;

var mem_total_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_total_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_total_f.toFixed(1) + ' %</div></div></div>' + mem_total + ' kB / ' + mem_system + ' kB';
var mem_free_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_free_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_free_f.toFixed(1) + ' %</div></div></div>' + mem_free + ' kB / ' + mem_total + ' kB';
var mem_used_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_used_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_used_f.toFixed(1) + ' %</div></div></div>' + mem_used + ' kB / ' + mem_total + ' kB';
var mem_buffer_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_buffer_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_buffer_f.toFixed(1) + ' %</div></div></div>' + mem_buffer + ' kB / ' + mem_total + ' kB';
var mem_cached_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_cached_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_cached_f.toFixed(1) + ' %</div></div></div>' + mem_cached + ' kB / ' + mem_total + ' kB';
var mem_active_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_active_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_active_f.toFixed(1) + ' %</div></div></div>' + mem_active + ' kB / ' + mem_total + ' kB';
var mem_inactive_str = '<div class=\"meter\"><div class=\"bar\" style=\"width:' + mem_inactive_f.toFixed(1) + '%;\"><div class=\"text\">' + mem_inactive_f.toFixed(1) + ' %</div></div></div>' + mem_inactive + ' kB / ' + mem_total + ' kB';

var uptime_raw = '<% get_uptime(); %>';
var uptime_up = uptime_raw.substring(uptime_raw.indexOf('up') + 3,uptime_raw.indexOf('load') - 2);
var uptime_load = uptime_raw.substring(uptime_raw.indexOf('average') + 9);

		</script>
	</head>
	
	<body class="gui" onload="init()"> <% showad(); %>
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
								<li><a href="Management.asp">Administration</a></li>
								<li class="current"><span>Status</span>
								<div id="menuSub">
									<ul id="menuSubList">
										<li><span>Router</span></li>
									    <li><a href="Status_Lan.asp">LAN</a></li>
										<li><a href="Status_Wireless.asp">Wireless</a></li>
										<% nvram_invmatch("status_auth","1","<!--"); %>
										<li><a href="Info.htm">Sys-Info</a></li>
										<% nvram_invmatch("status_auth","1","-->"); %>
										<% show_sputnik(); %>
									</ul>
								</div>
							</li>
						</ul>
					</div>
				</div>
			</div>
            <div id="main">
				<div id="contents">
					<form name="status" action="apply.cgi" method="<% get_http_method(); %>">
						<input type="hidden" name="submit_button"/>
						<input type="hidden" name="submit_type"/>
						<input type="hidden" name="change_action"/>
						<input type="hidden" name="action"/>
						<input type="hidden" name="wan_proto" value='<% nvram_get("wan_proto"); %>' />
						<h2>Router Information</h2>
						<fieldset>
							<legend>System</legend>
							<div class="setting">
							   <div class="label">Router Name</div><% nvram_get("router_name"); %>
							</div>
							<div class="setting">
							   <div class="label">Router Model</div><% nvram_get("DD_BOARD"); %>
							</div>
							<div class="setting">
							   <div class="label">MAC Address</div><% nvram_get("wan_hwaddr"); %>
							</div>
							<div class="setting">
							   <div class="label">Firmware Version</div><% get_firmware_version(); %>
							</div>
							<div class="setting">
							   <div class="label">Host Name</div><% nvram_get("wan_hostname"); %>
							</div>
							<div class="setting">
							   <div class="label">Domain Name</div><% nvram_get("wan_domain"); %>
							</div>
							<div class="setting">
							   <div class="label">Current Time</div><% localtime(); %>
							</div>
							<div class="setting">
							   <div class="label">Uptime</div><script type="text/JavaScript">document.write(uptime_up);</script>
							</div>
							<div class="setting">
							   <div class="label">Load Average</div><script type="text/JavaScript">document.write(uptime_load);</script>
							</div>
							</fieldset><br>
							<fieldset>
							<legend>CPU</legend>
							<div class="setting">
							   <div class="label">CPU Model</div><% show_cpuinfo(); %>
							</div>
							<div class="setting">
							   <div class="label">CPU Clock</div><% get_clkfreq(); %> MHz
							</div>
						</fieldset><br>
						<fieldset>
							<legend>Memory</legend>
							<div class="setting">
								<div class="label">System</div><script type="text/JavaScript">document.write(mem_system);</script> kB
							</div>
							<div class="setting">
								<div class="label">Total Available</div>
								<script type="text/JavaScript">document.write(mem_total_str);</script>
							</div>
							<div class="setting">
								<div class="label">Free</div>
								<script type="text/JavaScript">document.write(mem_free_str);</script>
							</div>
							<div class="setting">
								<div class="label">Used</div>
								<script type="text/JavaScript">document.write(mem_used_str);</script>
							</div>
							
							<div class="setting">
								<div class="label">Buffers</div>
								<script type="text/JavaScript">document.write(mem_buffer_str);</script>
							</div>
							
							<div class="setting">
								<div class="label">Cached</div>
								<script type="text/JavaScript">document.write(mem_cached_str);</script>
							</div>
							
							<div class="setting">
								<div class="label">Active</div>
								<script type="text/JavaScript">document.write(mem_active_str);</script>
							</div>
							
							<div class="setting">
								<div class="label">Inactive</div>
								<script type="text/JavaScript">document.write(mem_inactive_str);</script>
							</div>
						</fieldset><br/>
						<h2>Internet</h2>
						<fieldset>
							<legend>Configuration Type</legend>
							<div class="setting">
								<div class="label">Login Type</div>
								<% nvram_match("wan_proto","dhcp","Automatic Configuration - DHCP"); %>
								<% nvram_match("wan_proto","static","Static"); %>
								<% nvram_match("wan_proto","pppoe","PPPoE"); %>
								<% nvram_match("wan_proto","pptp","PPTP"); %>
								<% nvram_match("wan_proto","heartbeat","HeartBeatSignal"); %>
								<% nvram_match("wan_proto","disabled","Disabled"); %>
							</div>
							<% show_status_setting(); %>
							<% nvram_match("wan_proto", "dhcp", "<input onclick=DHCPAction(this.form,'release') type=button value='DHCP Release'/><input onclick=DHCPAction(this.form,'renew') type=button value='DHCP Renew'/>"); %>
						</fieldset><br/>
						<div class="submitFooter">
							<input type="button" value="Refresh" onclick="window.location.replace('Status_Router.asp')"/>
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
                  <dl>
                     <dt class="term">Firmware Version: </dt>
                     <dd class="definition">This is the Router's current firmware.</dd>
                     <dt class="term">Current Time: </dt>
                     <dd class="definition">This shows the time, as you set on the Setup Tab.</dd>
                     <dt class="term">MAC Address: </dt>
                     <dd class="definition">This is the Router's MAC Address, as seen by your ISP.</dd>
                     <dt class="term">Router Name: </dt>
                     <dd class="definition">This is the specific name for the Router, which you set on the Setup Tab.</dd>
                     <dt class="term">Configuration Type: </dt>
                     <dd class="definition">This shows the information required by your ISP for connection to the Internet. This information was entered on the Setup Tab. You can <em>Connect</em> or <em>Disconnect</em> your connection here by clicking on that button.</dd>
                  </dl><br /><a target="_blank" href="help/HStatus.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>
