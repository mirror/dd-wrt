<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - MAC Address Clone</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + wanmac.titl;

function to_submit(F) {
	F.submit_button.value = "WanMAC";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	apply(F);
}

function CloneMAC(F) {
	F.submit_button.value = "WanMAC";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "clone_mac";
	F.action.value = "Apply";
	F.submit();
}

function SelMac(val) {
	show_layer_ext('idmac', val == 1);
	setElementsActive("def_hwaddr", "def_whwaddr_5", val == "1");
}

addEvent(window, "load", function() {
	SelMac("<% nvram_get("mac_clone_enable"); %>");
	<% onload("MACClone", "document.mac.mac_clone_enable[0].checked = true; SelMac(1);"); %>
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
								<li class="current"><span><% tran("bmenu.setup"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
	  										<li><a href="index.asp"><% tran("bmenu.setupbasic"); %></a></li>
	  										<li><a href="DDNS.asp"><% tran("bmenu.setupddns"); %></a></li>
	  										<li><span><% tran("bmenu.setupmacclone"); %></span></li>
	  										<li><a href="Routing.asp"><% tran("bmenu.setuprouting"); %></a></li>
	  										<li><a href="Vlan.asp"><% tran("bmenu.setupvlan"); %></a></li>
  										</ul>
  									</div>
  								</li>
  								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
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
						<form name="mac" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<h2><% tran("wanmac.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("wanmac.legend"); %></legend>
								<div class="setting">
									<input class="spaceradio" type="radio" value="1" name="mac_clone_enable" onclick="SelMac(this.value)" <% nvram_checked("mac_clone_enable", "1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="mac_clone_enable" onclick="SelMac(this.value)" <% nvram_checked("mac_clone_enable", "0"); %> /><% tran("share.disable"); %>
								</div>
								<div id="idmac">
									<div class="setting">
										<div class="label"><% tran("wanmac.wan"); %></div>
										<input type="hidden" name="def_hwaddr" value="6" />
										<input class="num" size="2" maxlength="2" name="def_hwaddr_0" onblur="valid_mac(this,0)" value="<% get_clone_mac("0"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_1" onblur="valid_mac(this,1)" value="<% get_clone_mac("1"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_2" onblur="valid_mac(this,1)" value="<% get_clone_mac("2"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_3" onblur="valid_mac(this,1)" value="<% get_clone_mac("3"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_4" onblur="valid_mac(this,1)" value="<% get_clone_mac("4"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_5" onblur="valid_mac(this,1)" value="<% get_clone_mac("5"); %>" />
									</div>
									<div class="setting">
										<script type="text/javascript">document.write("<input type=\"button\" name=\"clone_b\" value=\"" + sbutton.wanmac + "\" onclick=\"CloneMAC(this.form)\" />")</script>
									</div>
									
									<hr width="90%"><br />
									
									<div class="setting">
										<div class="label"><% tran("wanmac.wlan"); %></div>
										<input type="hidden" name="def_whwaddr" value="6" />
										<input class="num" size="2" maxlength="2" name="def_whwaddr_0" onblur="valid_mac(this,0)" value="<% get_clone_wmac("0"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_1" onblur="valid_mac(this,1)" value="<% get_clone_wmac("1"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_2" onblur="valid_mac(this,1)" value="<% get_clone_wmac("2"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_3" onblur="valid_mac(this,1)" value="<% get_clone_wmac("3"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_4" onblur="valid_mac(this,1)" value="<% get_clone_wmac("4"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_5" onblur="valid_mac(this,1)" value="<% get_clone_wmac("5"); %>" />
									</div>
								</div>
							</fieldset><br />
							
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.replace('WanMAC.asp')\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
				<div id="help">
					<div id="logo">
							<h2><% tran("share.help"); %></h2>
					</div>
					<dl>
						<dt class="term"><% tran("wanmac.legend"); %>:</dt>
						<dd class="definition"><% tran("hwanmac.right2"); %></dd>
					</dl><br />
					<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HMAC.asp');"><% tran("share.more"); %></a>
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
