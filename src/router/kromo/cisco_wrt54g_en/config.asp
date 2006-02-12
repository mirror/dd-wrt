<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Backup &amp; Restore</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script></head>
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
                                 <li><a href="Diagnostics.asp">Diagnostics</a></li>
                                 <li><a href="Factory_Defaults.asp">Factory Defaults</a></li>
                                 <li><a href="Upgrade.asp">Firmware Upgrade</a></li>
                                 <li><span>Backup</span></li>
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
                  <form name="nvramrestore" action="nvram.cgi" method="POST" encType="multipart/form-data">
                     <h2>Backup</h2><input onclick=window.location.href="/nvrambak.bin" type="button" value="Backup" name="B1" /><hr width="90%" />
                     <h2>Restore Configuration</h2>Please select a file to restore: <input type=file name=file size="20" /><br /><input type=submit value=" Restore " /><hr width="90%" />
                     <div class="warning"><em>Warning:</em> Only upload files backed up using<br />this firmware and from the same model of router.<br />Do not upload any files that weren't created by this interface!
                     </div>
                  </form>
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
                  </div>
                  <dl>
                     <dd class="definition">
                        <p>You may backup your current configuration in case you need to reset the router back to its factory default settings.</p>
                        <p>You may click the Back up button to backup your current configuration.</p>
                        <p>Click the Browse button to browse for a configuration file that is currently saved on your PC.</p>
                        <p>Click Restore to overwrite all current configurations with the ones in the configuration file</p>
                     </dd>
                  </dl>
               <br /><a target="_blank" href="help/HBackup.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>