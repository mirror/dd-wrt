<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% charset(); %>
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<title><% nvram_get("router_name"); %> - Firewall</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + firewall.titl;

function to_submit(F) {
	F.submit_button.value = "Firewall";
	F.block_wan.value = (F._block_wan.checked == true) ? 1 : 0;
	if(F._block_loopback){
		F.block_loopback.value = (F._block_loopback.checked == true) ? 1 : 0;
	}
	if(F._block_cookie){
		F.block_cookie.value = (F._block_cookie.checked == true) ? 1 : 0;
	}
	if(F._block_java){
		F.block_java.value = (F._block_java.checked == true) ? 1 : 0;
	}
	if(F._block_proxy){
		F.block_proxy.value = (F._block_proxy.checked == true) ? 1 : 0;
	}
	if(F._block_activex){
		F.block_activex.value = (F._block_activex.checked == true) ? 1 : 0;
	}
	if(F._ident_pass){
		F.ident_pass.value = (F._ident_pass.checked == true) ? 0 : 1;
	}
	if(F._block_multicast) {
		F.multicast_pass.value = (F._block_multicast.checked == true) ? 0 : 1;
	}
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

function setFirewall(val) {
	setElementsActive("_block_proxy", "_ident_pass", val == "on");
}

addEvent(window, "load", function() {
	setFirewall("<% nvram_get("filter"); %>");
});

		</script>
	</head>

	<body class="gui">
	<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.security)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span><script type="text/javascript">Capture(bmenu.firwall)</script></span></li>
											<li><a href="VPN.asp"><script type="text/javascript">Capture(bmenu.vpn)</script></a></li>
										</ul>
									</div>
								</li>
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
					<form name="firewall" action="apply.cgi" method="<% get_http_method(); %>" >
						<input type="hidden" name="submit_button" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="action" />
						<input type="hidden" name="block_wan" />
						<input type="hidden" name="block_loopback" />
						<input type="hidden" name="multicast_pass" value="0" />
						<input type="hidden" name="ident_pass" />
						<input type="hidden" name="block_cookie" value="0" />
						<input type="hidden" name="block_java" value="0" />
						<input type="hidden" name="block_proxy" value="0" />
						<input type="hidden" name="block_activex" value="0" />
						<h2><script type="text/javascript">Capture(firewall.h2)</script></h2>
						
						<fieldset>
							<legend><script type="text/javascript">Capture(firewall.legend)</script></legend>
							<div class="setting">
								<div class="label"><script type="text/javascript">Capture(firewall.firewall)</script></div>
								<input class="spaceradio" type="radio" value="on" name="filter" <% nvram_checked("filter", "on"); %> onclick="setFirewall(this.value)" ><script type="text/javascript">Capture(share.enable)</script></input>&nbsp;
								<input class="spaceradio" type="radio" value="off" name="filter" <% nvram_checked("filter", "off"); %> onclick="setFirewall(this.value)" ><script type="text/javascript">Capture(share.disable)</script></input>
							</div>
						</fieldset><br />
						
						<fieldset>
							<legend><script type="text/javascript">Capture(firewall.legend2)</script></legend>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_proxy" <% nvram_checked("block_proxy", "1"); %> ><script type="text/javascript">Capture(firewall.proxy)</script></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_cookie" <% nvram_checked("block_cookie", "1"); %> ><script type="text/javascript">Capture(firewall.cookies)</script></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_java" <% nvram_checked("block_java", "1"); %> ><script type="text/javascript">Capture(firewall.applet)</script></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_activex" <% nvram_checked("block_activex", "1"); %> ><script type="text/javascript">Capture(firewall.activex)</script></input>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><script type="text/javascript">Capture(firewall.legend3)</script></legend>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_wan" <% nvram_checked("block_wan", "1"); %> ><script type="text/javascript">Capture(firewall.ping)</script></input>
									</div>
									<% support_invmatch("MULTICAST_SUPPORT", "1", "<!--"); %>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="0" name="_block_multicast" <% nvram_checked("multicast_pass", "0"); %> ><script type="text/javascript">Capture(firewall.muticast)</script></input>
									</div>
									<% support_invmatch("MULTICAST_SUPPORT", "1", "-->"); %>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="0" name="_block_loopback" <% nvram_checked("block_loopback", "1"); %> ><script type="text/javascript">Capture(filter.nat)</script></input>
									</div>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_ident_pass" <% nvram_checked("ident_pass", "0"); %> ><script type="text/javascript">Capture(filter.port113)</script></input>
									</div>
								</fieldset><br />
								<div class="submitFooter">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
									<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"setFirewall('<% nvram_get("filter"); %>')\" />")</script>
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
							<dt class="term"><script type="text/javascript">Capture(firewall.legend)</script>:</dt>
							<dd class="definition"><script type="text/javascript">Capture(hfirewall.right2)</script></dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HFirewall.asp');"><script type="text/javascript">Capture(share.more)</script></a>
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