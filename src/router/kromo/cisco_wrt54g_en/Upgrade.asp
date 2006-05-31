<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% charset(); %>
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<title><% nvram_get("router_name"); %> - Firmware Upgrade</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + upgrad.titl;

function process_aborted(F) {
	bar1.togglePause();
//	alert("Upgrade failed.");
	alert(fail.mess2);
	window.location.replace("Upgrade.asp")
	return false;
}

function upgrade(F,id) {
	if (F.file.value == "")	{
//		alert("Please select a file to upgrade.");
		alert(errmsg.err60);
		return false;
	}
	var len = F.file.value.length;
	var ext = new Array('.','b','i','n');
	var IMAGE = F.file.value.toLowerCase();
	for (i=0; i < 4; i++)	{
		if (ext[i] != IMAGE.charAt(len-4+i)){
//			alert("Incorrect image file.");
			alert(errmsg.err61);
			return false;
		}
	}

	choose_disable(F.Upgrade_b);
	bar1.togglePause();
	change_style(id,'textblink');
	F.Upgrade_b.value = sbutton.upgrading;
	
	F.submit_button.value = "Upgrade";
	apply(F);
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
											<li><a href="Wol.asp"><script type="text/javascript">Capture(bmenu.adminWol)</script></a></li>
											<li><a href="Factory_Defaults.asp"><script type="text/javascript">Capture(bmenu.adminFactory)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.adminUpgrade)</script></span></li>
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
						<form name="firmware" method="post" action="upgrade.cgi" enctype="multipart/form-data">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<h2><script type="text/javascript">Capture(upgrad.h2)</script></h2>
							
							<fieldset>
								<legend><script type="text/javascript">Capture(upgrad.legend)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(upgrad.info1)</script></div>
									<input class="spaceradio" type="radio" value="0" name="erase" checked="checked" /><script type="text/javascript">Capture(upgrad.resetOff)</script>&nbsp;
									<input class="spaceradio" type="radio" value="1" name="erase" /><script type="text/javascript">Capture(upgrad.resetOn)</script>
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(upgrad.file)</script></div>
									<input type="file" name="file" size="40"/>
								</div>
							</fieldset><br />
							
							<div class="warning">
								<p><div id="warning_text"><b><script type="text/javascript">Capture(upgrad.warning)</script></b></div></p>
								<p><script type="text/javascript">Capture(upgrad.mess1)</script></p>
								<div align="center"><script type="text/javascript">
									var bar1 = createBar(500,15,100,15,50,"process_aborted(this.form)");
									bar1.togglePause();
								</script></div><br />
							</div><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"Upgrade_b\" value=\"" + sbutton.upgrade + "\" onclick=\"upgrade(this.form,'warning_text')\" />")</script>
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
							<dt class="term"><script type="text/javascript">Capture(upgrad.legend)</script>: </dt>
							<dd class="definition"><script type="text/javascript">Capture(hupgrad.right2)</script></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow('HUpgrade.asp');"><script type="text/javascript">Capture(share.more)</script></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><script type="text/javascript">Capture(share.firmware)</script>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
