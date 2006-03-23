<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<% nvram_match("meta_refresh","1","<META HTTP-EQUIV=Refresh CONTENT=3; URL=Status_Wireless.asp >"); %>    
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Wireless Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function popup_survey() {
	w=760;
	h=700;
	l = (screen.availWidth-10 - w) / 2;
	t = (screen.availHeight-20 -h) / 2;

	popup = window.open("Site_Survey.asp", 'survey', 'resizable=yes, scrollbars=yes, width=' + w + ', height=' + h + ',top=' + t + ',left=' + l);
	popup.focus();
}
      </script>
   </head>
   
   <body class="gui"> <% showad(); %>
      <div id="wrapper">
         <div id="content">
            <div id="header">
               <div id="logo">
                  <h1><% show_control(); %></h1>
               </div>
               <div id="menu">
                  <div id="menuMain">
                     <ul id="menuMainList">
                        <li><a href="index.asp">Setup</a></li>
                        <li><a href="Wireless_Basic.asp">Wireless</a></li>
			<% nvram_invmatch("sipgate","1","<!--"); %>
			<li><a href="Sipath.asp">SIPatH</a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
                        <li><a href="Firewall.asp">Security</a></li>
                        <li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
                        <li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
                        <li><a href="Management.asp">Administration</a></li>
                        <li class="current"><a class="current" href="Status_Router.asp">Status</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Status_Router.asp">Router</a></li>
                                 <li><a href="Status_Lan.asp">LAN</a></li>
                                 <li><span>Wireless</span></li>
                                 <% nvram_invmatch("status_auth","1","<!--"); %>
								 <li><a href="Info.htm">Sys-Info</a></li>
								 <% nvram_invmatch("status_auth","1","-->"); %>
								 <% show_sputnik(); %>
                              </ul>
                           </div>
                        </li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <h2>Wireless</h2>
                  <div>
                    <fieldset>
                      <legend>Wireless Status</legend>
                     <div class="setting">
                        <div class="label">MAC Address</div><% nvram_get("wl0_hwaddr"); %>
                     </div>
                     <div class="setting">
                        <div class="label">Mode</div><% nvram_match("wl_mode", "wet", "Client Bridge"); %><% nvram_match("wl_mode", "ap", "AP"); %><% nvram_match("wl_mode", "sta", "Client"); %><% nvram_match("wl_mode", "infra", "Adhoc"); %><% nvram_match("wl_mode", "apsta", "Repeater"); %>
                     </div>
                     <div class="setting">
                        <div class="label">Network</div><% nvram_match("wl_net_mode", "mixed", "Mixed"); %><% nvram_match("wl_net_mode", "g-only", "G-Only"); %><% nvram_match("wl_net_mode", "disabled","Disabled"); %><% nvram_match("wl_net_mode", "b-only", "B-Only"); %>
                     </div>
                     <div class="setting">
                        <div class="label">SSID</div><% nvram_get("wl_ssid"); %>
                     </div>
                     <div class="setting">
                        <div class="label">DHCP Server</div><% nvram_match("lan_proto", "dhcp", "Enabled"); %><% nvram_match("lan_proto", "static", "Disabled"); %>
                     </div>
                     <div class="setting">
                        <div class="label">Channel</div><% get_curchannel(); %>
                     </div>
                     <div class="setting">
                        <div class="label">Xmit</div><% nvram_get("txpwr"); %> mW
                     </div>
                     <div class="setting">
                        <div class="label">Rate</div><% get_currate(); %> Mbps
                     </div>
                     <div class="setting">
                        <div class="label">Encryption</div>
                     		<% nvram_match("security_mode", "disabled", "Disabled"); %>
                     		<% nvram_invmatch("security_mode", "disabled", "Enabled, "); %>
                     		<% nvram_match("security_mode", "psk", "WPA Pre-shared Key"); %>
                     		<% nvram_match("security_mode", "wpa", "WPA RADIUS"); %>
                     		<% nvram_match("security_mode", "psk2", "WPA2 Pre-Shared Key Only"); %>
                     		<% nvram_match("security_mode", "wpa2", "WPA2 RADIUS Only"); %>
                     		<% nvram_match("security_mode", "psk psk2", "WPA2 Pre-Shared Key Mixed"); %>
                     		<% nvram_match("security_mode", "wpa wpa2", "WPA2 RADIUS Mixed"); %>
                     		<% nvram_match("security_mode", "radius", "RADIUS"); %>
                     		<% nvram_match("security_mode", "wep", "WEP"); %>     
                    </div>
                    <div class="setting">
                      <div class="label">PPTP Status</div><% nvram_match("pptpd_connected","0","Disconnected"); %> <% nvram_match("pptpd_connected","1","Connected"); %>
                    </div>
                    </fieldset>
                    <br/>
                    
                    <% active_wireless(0); %>
                    <% active_wds(0); %>
                    </div>
                    <br/>
                    <form>
                      <input onclick="popup_survey()" type="button" value="Survey" />
                    </form>
                    <br/>
                    <form>
                      <div class="submitFooter">
                        <input onclick="window.location.replace('Status_Wireless.asp')" type="button" value="Refresh" />
                      </div>
                    </form>
                  </div>
                </div>
                <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">MAC Address: </dt>
                     <dd class="definition">This is the Router's MAC Address, as seen on your local, wireless network.</dd>
                     <dt class="term">Network: </dt>
                     <dd class="definition">As selected from the Wireless tab, this will display the wireless mode (Mixed, G-Only, or Disabled) used by the network.</dd>
                  </dl><br /><a target="_blank" href="help/HStatusWireless.asp">More...</a></div>
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
