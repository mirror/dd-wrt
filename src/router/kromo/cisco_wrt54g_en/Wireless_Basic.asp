<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless</title>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + wl_basic.titl;

var EN_DIS = '<% nvram_else_match("wl0_gmode","-1","0","1"); %>';
function SelWL(num,F) {
	if ( num == 0)
		I = '0';
	else
		I = '1';
	wl_enable_disable(F,I);
}
function wl_enable_disable(F,I) {
	EN_DIS = I;
	if (F.wl_ssid && F.wl0_channel)
	{
	if( I == "0"){
		choose_disable(F.wl_ssid);
		choose_disable(F.wl0_channel);
		<% nvram_match("wl_mode", "ap", "choose_disable(F.wl_closed[0]);"); %>
		<% nvram_match("wl_mode", "ap", "choose_disable(F.wl_closed[1]);"); %>
	} else {
		choose_enable(F.wl_ssid);
		choose_enable(F.wl0_channel);
		<% nvram_match("wl_mode", "ap", "choose_enable(F.wl_closed[0]);"); %>
		<% nvram_match("wl_mode", "ap", "choose_enable(F.wl_closed[1]);"); %>
	}
	}
}

function vifs_add_submit(F,I) {
	F.iface.value = I;
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "add_vifs";
 	F.action.value = "Apply";
	F.submit();
}
function vifs_remove_submit(F,I) {
	F.iface.value = I;
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "remove_vifs";
 	F.action.value = "Apply";
	F.submit();
}

function to_submit(F) {
	if(F.wl_ssid)
	if(F.wl_ssid.value == ""){
		alert("You must input a SSID!");
		F.wl_ssid.focus();
		return false;
	}
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "save";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	apply(F);
}
function init() {
	wl_enable_disable(document.wireless,'<% nvram_else_match("wl0_gmode","-1","0","1"); %>');
}
		</script>
	</head>

   <body class="gui" onload="init()"> <% showad(); %>
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
                <li class="current"><span><% tran("bmenu.wireless"); %></span>
                  <div id="menuSub">
                      <ul id="menuSubList">
                      <li><span><% tran("bmenu.wirelessBasic"); %></span></li>
                      <li><a href="Wireless_radauth.asp"><% tran("bmenu.wirelessRadius"); %></a></li>
                      <li><a href="WL_WPATable.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
                      <li><a href="Wireless_MAC.asp"><% tran("bmenu.wirelessMac"); %></a></li>
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
		  <input type="hidden" name="submit_button" value="Wireless_Basic" />
		  <input type="hidden" name="submit_type" />
		  <input type="hidden" name="change_action" />
		  <input type="hidden" name="iface" />
		  <input type="hidden" name="action" value="Apply" />
		        <% show_wireless(); %>
		    <br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" /><input type="reset" value="Cancel Changes" /></div>
                  </form>
               </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">Wireless Network Mode: </dt>
                     <dd class="definition">If you wish to exclude Wireless-G clients, choose <em>B-Only</em> mode. If you would like to disable wireless access, choose <em>Disable</em>.
                     </dd>
                  </dl><br />
                  <a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWireless.asp')">More...</a>
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
