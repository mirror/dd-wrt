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
		<style type="text/css">
		<!--
			A{color: #000000; text-decoration: underline;}
			A:link{color: #000000; text-decoration: underline;}
			A:hover{color: #000000; text-decoration: none;}
			A:visited{color: #000000; text-decoration: underline;}
			.dis td {color: #FF0000;}
		-->
		</style>
		<script type="text/javascript">

function to_submit(F)
{
	F.submit_button.value = "UPnP";
	F.save_button.value = "Saved";
        
	F.action.value = "Apply";
	apply(F);
}

var upnpForwards = new Array(<% tf_upnp(); %>);
var data = new Array ();

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
			e.proto = RegExp.$4.toUpperCase();
			e.enabled = (RegExp.$5 == 'on');
			e.desc = RegExp.$6;

			if ((e.wanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.wanPorts = RegExp.$1;
				else e.wanPorts = RegExp.$1 + "-<br />" + RegExp.$2;
			if ((e.lanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.lanPorts = RegExp.$1;
				else e.lanPorts = RegExp.$1 + "-<br />" + RegExp.$2;
			data.push(e);
		}
		else {
			data.push('null');
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

	s = "<table class=\"table center\" width=\"100%\" cellspacing=\"5\">";
	s += "<tr><th width=\"40%\">Description</th><th width=\"15\" >From&nbsp;(WAN)</th><th width=\"15%\">To&nbsp;(LAN)</th><th width=\"20%\">IP&nbsp;Address</th><th width=\"10%\">Protocol</th><th width=\"5%\">Delete</th></tr>";
	for (var i=0; i<dataLen; ++i) {
		var e = data[i];
		if (e !== 'null') {
			var c = e.enabled ? "" : "dis";
			s += "<tr height=\"15\" class='" + c + "'" + "><td valign=\"top\">" + ((e.desc.length > 20) ? ("<small>" + e.desc + "</small>") : e.desc) + "</td><td valign=\"top\">" + e.wanPorts + "</td><td valign=\"top\">" + e.lanPorts + "</td><td valign=\"top\">" + e.lanIP + "</td><td valign=\"top\">" + e.proto + "</td><td class=\"bin\" title=\"Click to delete entry\" onclick='unmap("+i+")'></td></tr>";
		}
		else {
			nullCount++;
		}
	}
	if (dataLen == nullCount) {
 		 s += "<tr><td height=\"15\" colspan=\"6\" align=\"center\">- None -</td></tr>";
	}
	return s + "</table>";
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
	fupnp.delete_button.value = "Deleted";
	fupnp.save_button.disabled = true;
	fupnp.delete_button.disabled = true;
	fupnp.submit();

}

parseForwards();

var update;

addEvent(window, "load", function() {
	update = new StatusUpdate("UPnP.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("upnp_forwards", function(u) {
		upnpForwards = eval("new Array(" + u.upnp_forwards + ")");
		parseForwards();
		setElementContent("theforwards", makeTable());
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
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
								<li><a href="index.asp"><script>Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script>Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script>Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script>Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script>Capture(bmenu.accrestriction)</script></a></li>
								<li class="current"><span><script>Capture(bmenu.applications)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Forward.asp"><script>Capture(bmenu.applicationsprforwarding)</script></a></li>
											<li><a href="ForwardSpec.asp"><script>Capture(bmenu.applicationspforwarding)</script></a></li>
											<li><a href="Triggering.asp"><script>Capture(bmenu.applicationsptriggering)</script></a></li>
											<li><span><script>Capture(bmenu.applicationsUpnp)</script></span></li>
											<li><a href="DMZ.asp"><script>Capture(bmenu.applicationsDMZ)</script></a></li>
											<li><a href="QoS.asp"><script>Capture(bmenu.applicationsQoS)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp"><script>Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script>Capture(bmenu.statu)</script></a></li>
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
								<span id="theforwards"><script type="text/javascript">document.write(makeTable())</script></span><br />
								<div class="center">
									<input type="button" name="delete_button" value="Delete All" onclick="unmap('all')" />
									<input type="button" name="refresh_button" value="<% nvram_else_match("refresh_time","0","Refresh","Auto-Refresh ON"); %>" onclick="window.location.reload()" />
								</div>
							</fieldset><br />
							<fieldset>
								<legend>UPnP Configuration</legend>
								<div class="setting">
									<div class="label">UPnP Service</div>
									<input type="radio" name="upnp_enable" value="1" <% nvram_selmatch("upnp_enable","1","checked"); %> />Enable
									<input type="radio" name="upnp_enable" value="0" <% nvram_selmatch("upnp_enable","0","checked"); %> />Disable
								</div>
								<% nvram_invmatch("upnp_enable", "1", "<!--"); %>
								<div class="setting">
									<div class="label">Clear port forwards at startup</div>
									<input type="radio" name="upnpcas" value="1" <% nvram_selmatch("upnpcas","1","checked"); %> />Enable
									<input type="radio" name="upnpcas" value="0" <% nvram_selmatch("upnpcas","0","checked"); %> />Disable
								</div>
								<div class="setting">
									<div class="label">Send presentation URL</div>
									<input type="radio" name="upnpmnp" value="1" <% nvram_selmatch("upnpmnp","1","checked"); %> />Enable
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
						</dl>
						<br/>
						<a href="javascript:openHelpWindow('HUPnP.asp')">More...</a>
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
