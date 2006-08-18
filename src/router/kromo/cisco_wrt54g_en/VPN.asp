<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - VPN</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + vpn.titl;

function to_submit(F) {
	F.submit_button.value = "VPN";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}
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
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li class="current"><span><% tran("bmenu.security"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Firewall.asp"><% tran("bmenu.firwall"); %></a></li>
											<li><span><% tran("bmenu.vpn"); %></span></li>
										</ul>
									</div>
								</li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="action"/>
							<h2><% tran("vpn.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("vpn.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("vpn.ipsec"); %></div>
									<input class="spaceradio" type="radio" value="1" name="ipsec_pass" <% nvram_checked("ipsec_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="ipsec_pass" <% nvram_checked("ipsec_pass","0"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("vpn.pptp"); %></div>
									<input class="spaceradio" type="radio" value="1" name="pptp_pass" <% nvram_checked("pptp_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="pptp_pass" <% nvram_checked("pptp_pass","0"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("vpn.l2tp"); %></div>
									<input class="spaceradio" type="radio" value="1" name="l2tp_pass" <% nvram_checked("l2tp_pass","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="l2tp_pass" <% nvram_checked("l2tp_pass","0"); %> /><% tran("share.disable"); %>
								</div>
							</fieldset><br/>
							
							<div class="submitFooter">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
									<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
								</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dd class="definition"><% tran("hvpn.right1"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HVPN.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script type="text/javascript">document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>