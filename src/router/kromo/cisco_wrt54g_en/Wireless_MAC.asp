<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - MAC Filter</title>
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

addEvent(window, "load", function() {
	show_layer_ext(document.wireless.wl_macmode1, 'idmac', "<% nvram_else_match("wl_macmode1", "other", "other", "disabled"); %>" == "other");
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
							<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
        		            <li class="current"><span><% tran("bmenu.wireless"); %></span>
                		    <div id="menuSub">
                        	  <ul id="menuSubList">
                              	<li><a href="Wireless_Basic.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
                                <li><a href="Wireless_radauth.asp"><% tran("bmenu.wirelessRadius"); %></a></li>
                                <li><a href="WL_WPATable.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
                                <li><span><% tran("bmenu.wirelessMac"); %></span></li>
                                <li><a href="Wireless_Advanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
                                <li><a href="Wireless_WDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
                              </ul>
                            </div>
                            </li>
                        	<% nvram_invmatch("sipgate","1","<!--"); %>
                        	<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
                        	<% nvram_invmatch("sipgate","1","-->"); %>
                        	<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
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
						<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><% tran("wl_mac.h2"); %></h2>
							<fieldset>
								<legend><% tran("wl_mac.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("wl_mac.label"); %></div>
									<input class="spaceradio" type="radio" value="other" name="wl_macmode1" <% nvram_checked("wl_macmode1","other"); %> onclick="show_layer_ext(this, 'idmac', true)"/><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="disabled" name="wl_macmode1" <% nvram_checked("wl_macmode1","disabled"); %> onclick="show_layer_ext(this, 'idmac', false)"/><% tran("share.disable"); %>
								</div>
								<div class="setting" id="idmac">
									<div class="label"><% tran("wl_mac.label2"); %><br />&nbsp;</div>
									<input class="spaceradio" type="radio" value="deny" name="wl_macmode" <% nvram_invmatch("wl_macmode","allow","checked"); %> /><% tran("wl_mac.deny"); %>
									<br />
									<input class="spaceradio" type="radio" value="allow" name="wl_macmode" <% nvram_checked("wl_macmode","allow"); %> /><% tran("wl_mac.allow"); %>
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
							<h2><% tran("share.help"); %></h2>
						</div><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWirelessMAC.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>