<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% charset(); %>
		<title><% nvram_get("router_name"); %> - MAC Filter</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + wl_mac.titl;

function to_submit(F) {
	F.submit_button.value = "Wireless_MAC";
	F.change_action.value = "apply_cgi";
	F.action.value = "Apply";
	
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function setMAC(val) {
	setElementActive("wl_macmode", val == "other");
}

addEvent(window, "load", function() {
	setMAC("<% nvram_get("wl_macmode1"); %>");
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
							<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
        		            <li class="current"><span><script type="text/javascript">Capture(bmenu.wireless)</script></span>
                		    <div id="menuSub">
                        	  <ul id="menuSubList">
                              	<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wirelessBasic)</script></a></li>
                                <li><a href="Wireless_radauth.asp"><script type="text/javascript">Capture(bmenu.wirelessRadius)</script></a></li>
                                <li><a href="WL_WPATable.asp"><script type="text/javascript">Capture(bmenu.wirelessSecurity)</script></a></li>
                                <li><span><script type="text/javascript">Capture(bmenu.wirelessMac)</script></span></li>
                                <li><a href="Wireless_Advanced.asp"><script type="text/javascript">Capture(bmenu.wirelessAdvanced)</script></a></li>
                                <li><a href="Wireless_WDS.asp"><script type="text/javascript">Capture(bmenu.wirelessWds)</script></a></li>
                              </ul>
                            </div>
                            </li>
                        	<% nvram_invmatch("sipgate","1","<!--"); %>
                        	<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
                        	<% nvram_invmatch("sipgate","1","-->"); %>
                        	<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
                        	<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
                        	<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
                        	<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
                        	<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
						  </ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><script type="text/javascript">Capture(wl_mac.h2)</script></h2>
							<fieldset>
								<legend><script type="text/javascript">Capture(wl_mac.legend)</script></legend>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(wl_mac.label)</script></div>
									<input class="spaceradio" type="radio" value="other" name="wl_macmode1" onclick="setMAC(this.value)" <% nvram_checked("wl_macmode1","other"); %> /><script type="text/javascript">Capture(share.enable)</script>&nbsp;
									<input class="spaceradio" type="radio" value="disabled" name="wl_macmode1" onclick="setMAC(this.value)" <% nvram_checked("wl_macmode1","disabled"); %> /><script type="text/javascript">Capture(share.disable)</script>
								</div>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(wl_mac.label2)</script><br />&nbsp;</div>
									<input class="spaceradio" type="radio" value="deny" name="wl_macmode" <% nvram_invmatch("wl_macmode","allow","checked"); %> /><script type="text/javascript">Capture(wl_mac.deny)</script>
									<br />
									<input class="spaceradio" type="radio" value="allow" name="wl_macmode" <% nvram_checked("wl_macmode","allow"); %> /><script type="text/javascript">Capture(wl_mac.allow)</script>
								</div><br />
								<div class="center">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"mac_filter_button\" value=\"" + sbutton.filterMac + "\" onclick=\"openWindow('WL_FilterTable.asp', 880, 730)\" />");</script>
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"cancel\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.reload()\" />");</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div><br />
						<a href="javascript:openHelpWindow('HWirelessMAC.asp')"><script type="text/javascript">Capture(share.more)</script></a>
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