<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - UPnP</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<!--
		UPnP
		11.2005	tofu10		init
		Intergated to DD-WRT by LawnMowerGuy1
		-->
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "UPnP";
	F.save_button.value = "Saved";
	F.action.value = "Apply";
	update.stop();
	apply(F);
}

var data = new Array();

function parseForwards(upnpForwards) {
	// wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc
	data = [];
	for (var i = 0; i < upnpForwards.length; ++i) {
		if (upnpForwards[i] == '' || !upnpForwards[i].match(/^(\d+-\d+)>(.*?):(\d+-\d+),(.*?),(.*?),(.*)/)) continue;
		var e = {};
		e.index = i;
		e.wanPorts = RegExp.$1;
		e.lanIP = RegExp.$2;
		e.lanPorts = RegExp.$3;
		e.proto = RegExp.$4.toUpperCase();
		e.enabled = (RegExp.$5 == 'on');
		e.desc = RegExp.$6;

		if ((e.wanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.wanPorts = RegExp.$1;
			else e.wanPorts = RegExp.$1 + "&nbsp;-&nbsp;" + RegExp.$2;
		if ((e.lanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.lanPorts = RegExp.$1;
			else e.lanPorts = RegExp.$1 + "&nbsp;-&nbsp;" + RegExp.$2;
		data.push(e);
	}
}

function setUPnPTable(forwards) {
	parseForwards(forwards);
	var table = document.getElementById("upnp_table");
	for(var i = table.rows.length - 1; i > 0 ; i--) {
		table.deleteRow(i);
	}
	if(data.length == 0) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 6;
		cell.align = "center";
		cell.innerHTML = "- None - ";
		return;
	}
	for(var i = 0; i < data.length; i++) {
		var row = table.insertRow(-1);
		row.style.height = "15px";
		row.className = (data[i].enabled ? '' : 'disabled');
		row.insertCell(-1).innerHTML = data[i].desc;
		row.insertCell(-1).innerHTML = data[i].wanPorts;
		row.insertCell(-1).innerHTML = data[i].lanPorts;
		row.insertCell(-1).innerHTML = data[i].lanIP;
		row.insertCell(-1).innerHTML = data[i].proto;
		var cell = row.insertCell(-1);
		cell.className = "bin";
		cell.title = "Click to delete lease";
		cell.innerHTML = " ";
		eval("addEvent(cell, 'click', function() { deleteForward(" + i + ") })");
	}
}

function deleteForward(x) {
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
	fupnp.remove.value = (x == 'all' ? 'all' : e.index);
	fupnp.delete_button.value = "Deleted";
	fupnp.save_button.disabled = true;
	fupnp.delete_button.disabled = true;
	update.stop();
	fupnp.submit();
}

var update;

addEvent(window, "load", function() {
	setUPnPTable(new Array(<% tf_upnp(); %>));

	update = new StatusUpdate("UPnP.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("upnp_forwards", function(u) {
		setUPnPTable(eval("new Array(" + u.upnp_forwards + ")"));
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
		</script>
		<style type="text/css">
tr.disabled td {
	text-decoration:line-through;
	color:#999;
}
		</style>
	</head>

	<body class="gui"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.applications)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applicationsprforwarding)</script></a></li>
											<li><a href="ForwardSpec.asp"><script type="text/javascript">Capture(bmenu.applicationspforwarding)</script></a></li>
											<li><a href="Triggering.asp"><script type="text/javascript">Capture(bmenu.applicationsptriggering)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.applicationsUpnp)</script></span></li>
											<li><a href="DMZ.asp"><script type="text/javascript">Capture(bmenu.applicationsDMZ)</script></a></li>
											<li><a href="QoS.asp"><script type="text/javascript">Capture(bmenu.applicationsQoS)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="fupnp" id="fupnp" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="remove" />
							<h2>Universal Plug and Play (UPnP)</h2>
							<fieldset>
								<legend>Forwards</legend>
								<table class="table center" cellspacing="6" id="upnp_table">
									<tr>
										<th width="40%">Description</th>
										<th width="15%">From&nbsp;(WAN)</th>
										<th width="15%">To&nbsp;(LAN)</th>
										<th width="20%">IP&nbsp;Address</th>
										<th width="10%">Protocol</th>
										<th>Delete</th>
									</tr>
								</table><br />
								<div class="center">
									<input type="button" name="delete_button" value="Delete All" onclick="deleteForward('all')" />&nbsp;
									<input type="button" name="refresh_button" value="<% nvram_else_match("refresh_time","0","Refresh","Auto-Refresh ON"); %>" onclick="window.location.reload()" />
								</div>
							</fieldset><br />
							<fieldset>
								<legend>UPnP Configuration</legend>
								<div class="setting">
									<div class="label">UPnP Service</div>
									<input type="radio" name="upnp_enable" value="1" <% nvram_selmatch("upnp_enable","1","checked"); %> />Enable&nbsp;
									<input type="radio" name="upnp_enable" value="0" <% nvram_selmatch("upnp_enable","0","checked"); %> />Disable
								</div>
								<% nvram_invmatch("upnp_enable", "1", "<!--"); %>
								<div class="setting">
									<div class="label">Clear port forwards at startup</div>
									<input type="radio" name="upnpcas" value="1" <% nvram_selmatch("upnpcas","1","checked"); %> />Enable&nbsp;
									<input type="radio" name="upnpcas" value="0" <% nvram_selmatch("upnpcas","0","checked"); %> />Disable
								</div>
								<div class="setting">
									<div class="label">Send presentation URL</div>
									<input type="radio" name="upnpmnp" value="1" <% nvram_selmatch("upnpmnp","1","checked"); %> />Enable&nbsp;
									<input type="radio" name="upnpmnp" value="0" <% nvram_selmatch("upnpmnp","0","checked"); %> />Disable
								</div>
								<% nvram_invmatch("upnp_enable", "1", "-->"); %>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button"  value="Save Settings" onclick="to_submit(this.form)" />
								<input type="reset" value="Cancel Changes" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Forwards:</dt>
							<dd class="definition">Click the trash can to delete an individual entry.</dd>
							<dt class="term">UPnP Service:</dt>
							<dd class="definition">Allows applications to automatically setup port forwardings.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HUPnP.asp')">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <a href="javascript:openAboutWindow()"><% get_firmware_version(); %></a></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
