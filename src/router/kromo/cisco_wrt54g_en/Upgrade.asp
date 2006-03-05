<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Firmware Upgrade</title>
		<link type="text/css" rel="stylesheet" href="style.css"/>
		<script type="text/JavaScript" src="common.js">{}</script>
		<script language="JavaScript">

/*var showchar = '|';
var maxchars = 60;
var delay_time = 1000;
var counter=0;
var num=0;

function progress(){
	var F = document.forms[0];
	if(num == 4){
		clearTimeout(timerID);
		alert("Upgrade failed !");
		return false;
	}
	if (counter < maxchars)	{
		counter++;
		var tmp = '';
		for (var i=0; i < counter; i++)
			tmp = tmp + showchar;
		F.process.value = tmp;
		timerID = setTimeout('progress()',delay_time);
	} else {
		counter = 0;
		num ++;
		progress();
    }
}*/

function process_aborted(F) {
  bar1.togglePause();
  alert("Upgrade failed !");
  return false;
}

function stop(){
	if(ie4)
  		document.all['style0'].style.visibility = 'hidden';
}

function upgrade(F){
	var len = F.file.value.length;
	var ext = new Array('.','b','i','n');
	if (F.file.value == '')	{
		alert("Please select a file to upgrade !");
		return false;
	}
	var IMAGE = F.file.value.toLowerCase();
	for (i=0; i < 4; i++)	{
		if (ext[i] != IMAGE.charAt(len-4+i)){
			alert("Incorrect image file !");
			return false;
		}
	}

	if(ns4)
	  delay_time = 1500;
		choose_disable(F.Upgrade_b);
		F.Upgrade_b.value = " Upgrading ";
		F.submit_button.value = "Upgrade";
		F.submit();
		//document.onstop = stop;
		//progress();
		bar1.togglePause();
}
		</script>
	</head>
	
	<body class="gui"> <% showad(); %>
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
												<li><a href="Log.asp">Log</a></li>
												<li><a href="Diagnostics.asp">Diagnostics</a></li>
												<li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
												<li><span>Firmware Upgrade</span></li>
												<li><a href="config.asp">Backup</a></li>
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
							<form name="firmware" method="post" action="upgrade.cgi" encType="multipart/form-data">
								<input type="hidden" name="submit_button"/>
								<input type="hidden" name="action"/>
								<input type="hidden" name="change_action"/>
								<h2>Firmware Upgrade</h2>
								<div>After flashing, reset to:
								<br/>
									<div class="setting">
										<input type="radio" value="0" name="erase" checked/> No reset
									</div>
									<div class="setting">
										<input type="radio" value="1" name="erase"/> Default settings
									</div>
									<!--<div class="setting">
										<input type="radio" value="2" name="erase"/> Factory Defaults
									</div>-->
								</div>
								<br/>Please select a file to upgrade: <input type="file" name="file" size="20"/><br/><br/>
								<div class="warning"><em>Warning:</em> Upgrading firmware may take a few minutes, please don't turn off the power or press the reset button.</div>
								<br/><hr width="90%" /><br/>
								<div class="warning"><p><center>
									<script language="javascript">
										var bar1=createBar(500,15,100,15,50,"process_aborted(this.form)");
										bar1.togglePause();
									</script></center></p>
									<br/>
									<em>Upgrade must NOT be interrupted! (please wait 2 mins.)</em>
								</div>
								<br/>
								<div class="submitFooter">
									 <input type="button" name="Upgrade_b" value=" Upgrade " onclick="upgrade(this.form)"/>
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
								<dd class="definition">Click on the browse button to select the firmware file to be uploaded to the router.</dd>
								<dd class="definition">Click the Upgrade button to begin the upgrade process. Upgrade must not be interrupted.</dd>
							</dl>
							<br/>
							<a target="_blank" href="help/HUpgrade.asp">More...</a>
						</div>
					</div>
				</div>
			</div>
		</body>
	</html>