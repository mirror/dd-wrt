<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Diagnostics</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
function ViewPing() {
	self.open('Ping.asp','PingTable','alwaysRaised,resizable,scrollbars,width=720,height=500').focus();
}
</script></head>
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
                        <li class="current"><a class="current" href="Management.asp">Administration</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Management.asp">Management</a></li>
				 <li><a href="Hotspot.asp">Hotspot</a></li>
				 <li><a href="Services.asp">Services</a></li>
				 <li><a href="Alive.asp">Keep Alive</a></li>
                                 <li><a href="Log.asp">Log</a></li>
                                 <li><span>Diagnostics</span></li>
                                 <li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
				 <li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Firmware Upgrade</a>","<a href=Upgrade.asp >Firmware Upgrade</a>"); %></li> 
				 <li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Backup</a>","<a href=config.asp >Backup</a>"); %></li>
                              </ul>
                           </div>
                        </li>
                        <li><a href="Status_Router.asp">Status</a></li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="diag" action="apply.cgi" method="<% get_http_method(); %>">
                     <h2>Command Shell</h2>
                     <fieldset>
                        <legend>Command Shell Parameters</legend>
                        <div class="setting"><input type="button" value="Run" onclick=ViewPing(); /></div>
                     </fieldset><br /></form>
               </div>
            </div>
            <div id="statusInfo">
               <div class="info">Firmware: <% get_firmware_version(); %></div>
               <div class="info">Time: <% get_uptime(); %></div>
               <div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div><br /><a target="_blank" href="help/HSetup.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>