<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
	<title><% nvram_get("router_name"); %> - DMZ</title>
	<link type="text/css" rel="stylesheet" href="style.css"/>
	<script type="text/JavaScript" src="common.js">{}</script>
	<script language="JavaScript">
	
var EN_DIS = '<% nvram_get("dmz_enable"); %>'

function to_submit(F) {
	if(F.dmz_enable[0].checked == true){
		if(F.dmz_ipaddr.value == "0"){
			alert("Illegal DMZ IP Address!");
			F.dmz_ipaddr.focus();
			return false;
		}
	}

	F.submit_button.value = "DMZ";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

        F.submit();
}

function dmz_enable_disable(F,I) {
	EN_DIS1 = I;
	if ( I == "0" ){
		choose_disable(F.dmz_ipaddr);
	}
	else{
		choose_enable(F.dmz_ipaddr);
	}
}

function SelDMZ(F,num) {
	dmz_enable_disable(F,num);
}

function init() {
	dmz_enable_disable(document.dmz,'<% nvram_get("dmz_enable"); %>');
}
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
								<li class="current"><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a><div id="menuSub">
									<ul id="menuSubList">
										<li><a href="Forward.asp">Port Range Forward</a></li>
										<li><a href="ForwardSpec.asp">Port Forwarding</a></li> 
										<li><a href="Triggering.asp">Port Triggering</a></li> 
										<li><a href="UPnP.asp">UPnP Forward</a></li> 
										<li><span>DMZ</span></li>
										<li><a href="QoS.asp">QoS</a></li>
									</ul>
								</div>
							</li>
							<li><a href="Management.asp">Administration</a></li>
							<li><a href="Status_Router.asp">Status</a></li>
						</ul>
					</div>
				</div>
			</div>
            <div id="main">
            	<div id="contents">
                	<form name="dmz" action="apply.cgi" method="<% get_http_method(); %>">
						<input type="hidden" name="submit_button" value="DMZ"/>
						<input type="hidden" name="change_action"/>
						<input type="hidden" name="action" value="Apply"/>
						<h2>Demilitarized Zone (DMZ)</h2>
						<div>
							<div class="setting">
								<input type="radio" value="1" name="dmz_enable" onclick="SelDMZ(this.form,1)" <% nvram_match("dmz_enable","1","checked"); %>>Enable</input>
								<input type="radio" value="0" name="dmz_enable" onclick="SelDMZ(this.form,0)" <% nvram_match("dmz_enable","0","checked"); %>>Disable</input>
							</div>
							<div class="setting">
								<div class="label">DMZ Host IP Address</div>
								<% prefix_ip_get("lan_ipaddr",1); %>
								<input class="num" maxLength="3" onblur="valid_range(this,0,254,'IP')" size="3" name="dmz_ipaddr" value='<% nvram_get("dmz_ipaddr"); %>'/>
							</div>
							</div>
							<br/>
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)"/>
								<input type="reset" value="Cancel Changes"/>
							</div>
						</form>
					</div>
				</div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">DMZ: </dt>
							<dd class="definition">Enabling this option will expose your router to the Internet. All ports will be accessible from the Internet.</dd>
						</dl>
						<br/>
						<a target="_blank" href="help/HDMZ.asp">More...</a>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>