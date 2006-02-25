<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
      <title><% nvram_get("router_name"); %> - Status</title>
      <link type="text/css" rel="stylesheet" href="style.css"/>
      <script type="text/JavaScript" src="common.js">{}</script>
      <script language="JavaScript">

var dhcp_win = null;

function ViewDHCP() {
	dhcp_win = self.open('DHCPTable.asp','inLogTable','alwaysRaised,resizable,scrollbars,width=720,height=600');
	dhcp_win.focus();
}

function exit() {
        closeWin(dhcp_win);
}
    </script>
  </head>
  
  <body class="gui" onunload="exit()"> <% showad(); %>
    <div id="wrapper">
      <div id="content">
        <div id="header">
          <div id="logo"><h1><% show_control(); %></h1></div>
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
                <li class="current">
                  <a class="current" href="Status_Router.asp">Status</a>
                  <div id="menuSub">
                    <ul id="menuSubList">
                      <li><a href="Status_Router.asp">Router</a></li>
                      <li><span>LAN</span></li>
                      <li><a href="Status_Wireless.asp">Wireless</a></li>
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
            <h2>Local Network</h2>
            <div>
              <div class="setting">
                <div class="label">MAC Address</div><% nvram_get("lan_hwaddr"); %>
              </div>
              <div class="setting">
                <div class="label">IP Address</div><% nvram_get("lan_ipaddr"); %>
              </div>
              <div class="setting">
                <div class="label">Subnet Mask</div><% nvram_get("lan_netmask"); %>
              </div>
              <div class="setting">
                <div class="label">DHCP Server</div><% nvram_match("lan_proto", "dhcp", "Enabled"); %><% nvram_match("lan_proto", "static", "Disabled"); %>
              </div>
              <div class="setting">
                <div class="label">Start IP Address</div><% prefix_ip_get("lan_ipaddr",1); %><% nvram_get("dhcp_start"); %>
              </div>
              <div class="setting">
                <div class="label">End IP Address</div>
<script language="javascript">
  var prefix = "<% prefix_ip_get("lan_ipaddr",1); %>";
  var start = <% nvram_get("dhcp_start"); %>;
	var num = <% nvram_get("dhcp_num"); %>;
	document.write(prefix);
	document.write(start+num-1);
</script>
              </div>
              </div>
              <form>
                <div class="setting">
                  <input id="button1" onclick="ViewDHCP()" type="button" value="DHCP Clients Table" name="dhcp_table"/>
                </div>
              </form>
              <br/>
              <form>
                <div class="submitFooter">
                  <input onclick="window.location.replace('Status_Lan.asp')" type="button" value="Refresh"/>
                </div>
              </form>
            </div>
          </div>
          <div id="statusInfo">
            <div class="info">Firmware: <% get_firmware_version(); %></div>
            <div class="info">Time: <% get_uptime(); %></div>
            <% nvram_match("wan_proto","disabled","<!--"); %>
            <div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
            <% nvram_match("wan_proto","disabled","-->"); %>
            <div class="info"><% nvram_match("wan_proto","disabled","WAN disabled"); %></div>
           </div>
           <div id="helpContainer">
            <div id="help">
              <div id="logo"><h2>Help</h2></div>
                <dl>
                  <dt class="term">MAC Address: </dt>
                  <dd class="definition">This is the Router's MAC Address, as seen on your local, Ethernet network.</dd>
                  <dt class="term">IP Address: </dt>
                  <dd class="definition">This shows the Router's IP Address, as it appears on your local, Ethernet network.</dd>
                  <dt class="term">Subnet Mask: </dt>
                  <dd class="definition">When the Router is using a Subnet Mask, it is shown here.</dd>
                  <dt class="term">DHCP Server: </dt>
                  <dd class="definition">If you are using the Router as a DHCP server, that will be displayed here.</dd>
                </dl>
                <br/>
                <a target="_blank" href="help/HStatusLan.asp">More...</a>
              </div>
            </div>
          </div>
        </div>
      </body>
</html>