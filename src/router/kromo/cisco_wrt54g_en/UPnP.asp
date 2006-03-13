<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - UPnP Forward</title>
		<style type="text/css">
		<!--
			A{color: #000000; text-decoration: underline;}
			A:link{color: #000000; text-decoration: underline;}
			A:hover{color: #000000; text-decoration: none;}
			A:visited{color: #000000; text-decoration: underline;}
			.dis th {color: #A04040;}
		-->
		</style>
		<link type="text/css" rel="stylesheet" href="style.css" />
		<!--
		UPnP
		11.2005	tofu10		init
		Intergated to DD-WRT by LawnMowerGuy1
		-->
	
		<script type="text/JavaScript" src="common.js">{}</script>
		<script language="JavaScript">

function to_submit(F) 
{
        F.submit_button.value = "UPnP"; 
        F.action.value = "Apply"; 
        F.save_button.value = "Saved";
        F.save_button.disabled = true;
        F.submit(); 
} 

var upnpForwards = new Array(<% tf_upnp(); %>);
var data = new Array ();
var mouHi = ('<% nvram_get("mourowhi"); %>' == '1');

function parseForwards()
{
	// wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc
	data = [];
	for (var i=0; i<upnpForwards.length; ++i) {
		if (upnpForwards[i] !== '' && upnpForwards[i].match(/^(\d+-\d+)>(.*?):(\d+-\d+),(.*?),(.*?),(.*)/)) {
			var e = {};
			e.wanPorts = RegExp.$1;
			e.lanIP = RegExp.$2;
			e.lanPorts = RegExp.$3;
			e.proto = RegExp.$4;
			e.enabled = (RegExp.$5 == 'on');
			e.desc = RegExp.$6;

			if ((e.wanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.wanPorts = RegExp.$1;
				else e.wanPorts = RegExp.$1 + "-<br/>" + RegExp.$2;
			if ((e.lanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.lanPorts = RegExp.$1;
				else e.lanPorts = RegExp.$1 + "-<br/>" + RegExp.$2;
			data.push(e);
		}
		else {
			data.push("null");
		}
	}
	//data.sort(sorter);
}

//Get rid of sort function for now
//var sortby = 4;
//var sortrev = 0;

//function sorter(a, b)
//{
//	var aa, bb, ax, bx, i;
//
//	if (sortrev) {
//		bb = b;
//		b = a;
//		a = bb;
//	}
//	switch (sortby) {
//	case 0:
//		aa = parseInt(a.wanPorts);
//		bb = parseInt(a.wanPorts);
//		if (aa < bb) return -1;
//		if (aa > bb) return 1;
//		break;
//	case 1:
//		aa = parseInt(a.lanPorts);
//		bb = parseInt(a.lanPorts);
//		if (aa < bb) return -1;
//		if (aa > bb) return 1;
//		break;
//	case 2:
//		aa = (a.lanIP + '....').split('.');
//		bb = (b.lanIP + '....').split('.');
//		for (i = 0; i < 4; ++i) {
//			ax = parseInt('0' + aa[i]);
//			bx = parseInt('0' + bb[i]);
//			if (ax < bx) return -1;
//			if (ax > bx) return 1;
//		}
//		break;
//	case 3:
//		if (a.proto < b.proto) return -1;
//		if (a.proto > b.proto) return 1;
//		break;
//	}
//	if (a.desc < b.desc) return -1;
//	if (a.desc > b.desc) return 1;
//	return 0;
//}

//function resort(by)
//{
//	if (sortby == by) sortrev ^= 1; else sortrev = 0;
//	sortby = by;
//	data.sort(sorter);
//	show();
//}

function show()
{
	var c = document.getElementById("theforwards");
	if (c) c.innerHTML = makeTable();
}

function makeTable()
{
	var s;
	var dataLen = data.length;
	var nullCount = 0;

	s = "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"3\">";
	s += "<tr><th width=\"5%\" >From (WAN)</th><th width=\"5%\">To (LAN)</th><th width=\"20%\">IP Address</th><th width=\"10%\">Protocol</th><th width=\"55%\">Description</th><th width=\"5%\">Delete</th></tr>";
	s += "<tr><tr/>";
	for (var i=0; i<dataLen; ++i) {
		var e = data[i];
		if (e !== 'null') {
			var c = "row" + (i & 1) + (e.enabled ? "" : " dis");
			s += "<tr height='15' class='" + c + "'" + (mouHi ? ("onmouseover='this.className=\"" + c + " hov\"' onmouseout='this.className=\"" + c + "\"'") : "") + "><th valign='top'>" + e.wanPorts + "</th><th valign='top'>" + e.lanPorts + "</th><th valign='top'>" + e.lanIP + "</th><th valign='top'>" + e.proto + "</th><th valign='top'>" + ((e.desc.length > 20) ? ("<small>" + e.desc + "</small>") : e.desc) + "</th><th class=\"bin\" title=\"Click to delete entry\" onclick='unmap("+i+")'></th></tr>";
		}
		else {
			nullCount++;
		}
	}
	if (nullCount == dataLen) {
 		 s += "<tr><th colspan=5 align='center' valign='center'>- No Forwards -</th></tr>";
	}
	s += "</table><br/>";

	return s;
}

function unmap(x)
{
	if (x != 'all') {
		var e = data[x];
		if (!confirm("Delete " + e.desc + "? [" + e.wanPorts + "->" + e.lanPorts + " " + e.lanIP + " " + e.proto + "]")) return;
	}
	else {
		if (!confirm("Delete all entries?")) return;
	}
	var fupnp = document.getElementById("fupnp");
	
	fupnp.submit_button.value = "UPnP"; 
	fupnp.action.value = "Apply"; 
	fupnp.remove.value = x;
	fupnp.save_button.value = "Deleted";
	fupnp.save_button.disabled = true;
	fupnp.submit();
	
}

parseForwards();

		</script>
	</head>
	
	<body class="gui">
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
								<li class="current"><span>Applications&nbsp;&amp;&nbsp;Gaming</span>
									<div id="menuSub">
										<ul id="menuSubList"> 
											<li><a href="Forward.asp">Port Range Forward</a></li>
											<li><a href="ForwardSpec.asp">Port Forwarding</a></li>
											<li><a href="Triggering.asp">Port Triggering</a></li>
											<li><span>UPnP Forward</span></li>
											<li><a href="DMZ.asp">DMZ</a></li>
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
						<form name="fupnp" id="fupnp" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="commit" value="1"/>
							<input type="hidden" name="remove"/>
							<h2>Universal Plug and Play</h2>
							<fieldset>
								<legend>UPnP Forwards</legend>
								<div id="theforwards">
									<script language="Javascript">document.write(makeTable())</script>
								</div>
								<input type="button" value="Delete All" onclick="unmap('all')"/>
								<input type="button" value="Refresh" onclick="window.location.replace('UPnP.asp')"/>
							</fieldset>
							<br />
							<fieldset>
								<legend>UPnP Configuration</legend>
								<div class="setting">
									<div class="label">UPnP Service</div>
									<input type="radio" name="upnp_enable" value="1" <% nvram_selmatch("upnp_enable","1","checked"); %> />Enable
									<input type="radio" name="upnp_enable" value="0" <% nvram_selmatch("upnp_enable","0","checked"); %> />Disable
								</div>
								<% nvram_invmatch("upnp_enable", "1", "<!--"); %><div class="setting">
									<div class="label">Clear port forwards at startup</div>
									<input type="radio" name="upnpcas" value="1" <% nvram_selmatch("upnpcas","1","checked"); %> />Enable
									<input type="radio" name="upnpcas" value="0" <% nvram_selmatch("upnpcas","0","checked"); %> />Disable
								</div>
								<div class="setting">
									<div class="label">Send presentation URL</div>
									<input type="radio" name="upnpmnp" value="1" <% nvram_selmatch("upnpmnp","1","checked"); %> />Enable
									<input type="radio" name="upnpmnp" value="0" <% nvram_selmatch("upnpmnp","0","checked"); %> />Disable
								</div>
							<% nvram_invmatch("upnp_enable", "1", "-->"); %></fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button"  value="Save Settings" onclick="to_submit(this.form)"/>
								<input type="reset" value="Cancel Changes" />
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
							<dt class="term">UPnP Service:</dt>
							<dd class="definition">Allows applications to automatically setup port forwardings. Click the trash can to delete an individual entry.</dd>
						</dl>
						<br/>
						<a target="_blank" href="help/HUPnP.asp">More...</a>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>