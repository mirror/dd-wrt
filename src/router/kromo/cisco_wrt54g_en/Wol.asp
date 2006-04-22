<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html>
	<head> 
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - WOL</title>
		<link type="text/css" rel="stylesheet" href="/style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]> <link type="text/css" rel="stylesheet" href="../style/<% nvram_get("router_style"); %>/style_ie.css" /> <![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript"> 

function submit_static(mac, ip) {
	F = document.forms["ping"];
	cmd = F.ping_ip.value;
	cmd = cmd + ip + " -p ";
	cmd = cmd + F.local_wol_port.value + " ";
	cmd = cmd + mac;
	F.ping_ip.value = cmd;
	F.submit_type.value = "start";
	F.submit_button.value = "Ping";
	F.change_action.value = "gozila_cgi";
	apply(F);
}

function to_submit(F, I) {
	if(!valid(F, I)) return;
	F.local_wol_mac.value = F.local_wol_mac.value.replace("\n", " ");
	cmd = F.ping_ip.value;
	cmd = cmd + F.local_wol_network.value + " -p ";
	cmd = cmd + F.local_wol_port.value + " ";
	cmd = cmd + F.local_wol_mac.value + ";";
	cmd = cmd + "nvram set local_wol_mac=\""+F.local_wol_mac.value+"\";";
	cmd = cmd + "nvram set local_wol_network="+F.local_wol_network.value+";";
	cmd = cmd + "nvram set local_wol_port="+F.local_wol_port.value+";";
	cmd = cmd + "nvram commit;";
	F.ping_ip.value = cmd;
	F.submit_type.value = I;
	F.submit_button.value = "Ping";
	F.change_action.value = "gozila_cgi";
	apply(F);
}

function valid(F,I) {
	if(I == "start" && F.local_wol_mac.value == ""){
//		alert("You must input a MAC address to run.");
		alert(errmsg.err35);
		F.local_wol_mac.focus();
		return false;
	}
	if(I == "start" && F.local_wol_network.value == ""){
//		alert("You must input a network broadcast address to run.");
		alert(errmsg.err36);
		F.local_wol_network.focus();
		return false;
	}
	if(I == "start" && F.local_wol_port.value == ""){
//		alert("You must input a UDP port to run.");
		alert(errmsg.err37);
		F.local_wol_port.focus();
		return false;
	}
	return true;
}

function display_static_leases() {
	var static_leases = "<% nvram_get("static_leases"); %>".split(" ");
	while (static_leases.length > 0) {
		var lease = static_leases.shift().split("=")
		var mac = lease[0];
		var host = lease[1];
		var ip = lease[2];
		if(mac!=undefined && host!=undefined && ip!=undefined) {
			document.write("<tr>");
			document.write("<td width=30>"+mac+"</td>");
			document.write("<td width=30>"+host+"</td>");
			document.write("<td width=30>"+ip+"</td>");
			ip = ip.substring(0,ip.lastIndexOf("."))+".255";
			document.write("<td width=30>");
			document.write("<input type=button value=\"" + sbutton.wol + "\" onclick=\"submit_static('"+mac+"','"+ip+"');\" />");
			document.write("</td>");
			document.write("</tr>");
		} 
	}
}

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
								<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
								<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.admin)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.adminManagement)</script></a></li>
											<li><a href="Hotspot.asp"><script type="text/javascript">Capture(bmenu.adminHotspot)</script></a></li>
											<li><a href="Services.asp"><script type="text/javascript">Capture(bmenu.adminServices)</script></a></li>
											<li><a href="Alive.asp"><script type="text/javascript">Capture(bmenu.adminAlive)</script></a></li>
											<li><a href="Log.asp"><script type="text/javascript">Capture(bmenu.adminLog)</script></a></li>
											<li><a href="Diagnostics.asp"><script type="text/javascript">Capture(bmenu.adminDiag)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminWol)</script></span></li>
											<li><a href="Factory_Defaults.asp"><script type="text/javascript">Capture(bmenu.adminFactory)</script></a></li>
											<li><a href="Upgrade.asp"><script type="text/javascript">Capture(bmenu.adminUpgrade)</script></a></li>
											<li><a href="config.asp"><script type="text/javascript">Capture(bmenu.adminBackup)</script></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ping" action="apply.cgi" method="post">
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Ping" />
							<input type="hidden" name="submit_type" value="start" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="ping_times" value="1" />
							<input type="hidden" name="next_page" value="user/Wol.asp" />
							<input type="hidden" name="ping_ip" value="/usr/sbin/wol -v -i " />
							<h2><script type="text/javascript">Capture(wol.h2)</script></h2>
							<% nvram_selmatch("static_leases","","<!--"); %>
							<fieldset>
								<legend><script type="text/javascript">Capture(wol.legend)</script></legend>
								<div class="setting">
									<table cellspacing=0 cellpadding=0>
										<script type="text/javascript"> display_static_leases(); </script>
									</table>
									<script type="text/javascript">
										var table = new Array(<% dump_ping_log(""); %>);
										if(table.length > 0 && location.href.indexOf("Wol.asp") == -1) {
											document.write("<br /><pre style=\"margin:0\">" + table.join("\n") + "</pre>");
										}
									</script>
								</div>
							</fieldset><br />
							<% nvram_selmatch("static_leases","","-->"); %>
							<fieldset> 
								<legend><script type="text/javascript">Capture(wol.legend2)</script></legend>
									<div class="setting">
										<div class="label"><script type="text/javascript">Capture(wol.mac)</script></div>
										<textarea id="local_wol_mac" name="local_wol_mac" rows="3"  cols="20"><% nvram_get("local_wol_mac"); nvram_selmatch("local_wol_mac","","00:00:00:00:00:00"); %></textarea><br />
										<div class="label"><script type="text/javascript">Capture(wol.broadcast)</script></div>
										<input maxlength=15 id="local_wol_network" name="local_wol_network" size="20" value='<% nvram_get("local_wol_network"); nvram_selmatch("local_wol_network","","192.168.1.255"); %>'/><br>
										<div class="label"><script type="text/javascript">Capture(wol.udp)</script></div>
										<input maxlength=15 id="local_wol_port" name="local_wol_port" size="20" value='<% nvram_get("local_wol_port"); nvram_selmatch("local_wol_port","","7"); %>'/>
									</div>
									<script type="text/javascript">
										var table = new Array(<% dump_ping_log(""); %>);
										if(table.length == 0 && location.href.indexOf("Wol.asp") == -1) {
											table = document.forms[0].local_wol_mac.value.split(" ");
											document.write("<br /><pre style=\"margin:0\">");
											while(table.length > 0) {
											document.write(wol.msg1 + " " +table.shift()+ wol.msg2 + " " +document.forms[0].local_wol_network.value+":"+document.forms[0].local_wol_port.value+" ...\n");
										}
										document.write("</pre>");
									}
								</script>
							</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"ping\" value=\"" + sbutton.wol + "\" onclick=\"to_submit(this.form, 'start')\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div>
						<dl> 
							<dt class="term"><script type="text/javascript">Capture(hwol.right1)</script></dt>
							<dd class="definition"><script type="text/javascript">Capture(hwol.right2)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HWol.asp');"><script type="text/javascript">Capture(share.more)</script></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>