<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>   
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Sputnik Agent Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function init() {
	<% show_status("onload");%>
}
		</script>
	</head>
   <body class="gui" onload="init()">
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
                        <li><a href="Filters.asp">Access Restrictions</a></li>
                        <li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
                        <li><a href="Management.asp">Administration</a></li>
                        <li class="current"><span>Status</span>
			    <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Status_Router.asp">Router</a></li>
                                 <li><a href="Status_Lan.asp">LAN</a></li>
                                 <li><a href="Status_Wireless.asp">Wireless</a></li>
                                 <% nvram_invmatch("status_auth","1","<!--"); %>
								 <li><a href="Info.htm">Sys-Info</a></li>
								 <% nvram_invmatch("status_auth","1","-->"); %>
                                 <li><span>Sputnik&copy; Agent</span></li>
                              </ul>
                           </div>
                        </li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
	      <div id="contents">
		  <h2>Sputnik&reg; Agent&trade; Information</h2>
                     <div>
                        <div class="setting">
                           <div class="label">Managed By</div><% sputnik_apd_status("scc_server"); %>
                        </div>
                        <div class="setting">
                           <div class="label">State</div><% sputnik_apd_status("phase"); %>
                        </div>
                        <div class="setting">
                           <div class="label">SCC License No.</div><% sputnik_apd_status("lsk_serial"); %>
                        </div>
            	    </div>
		</div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">Sputnik Agent Status: </dt>
                     <dd class="definition">This screen displays the status of the Sputnik Agent process.</dd>

                     <dt class="term">Managed By: </dt>
                     <dd class="definition">The Sputnik Control Center that this access point is connected to. </dd> 
                     <dt class="term">State: </dt>
                     <dd class="definition">The current Agent status.</dd>
                     <dt class="term">SCC License No: </dt>
                     <dd class="definition">The license number of your Sputnik Control Center.</dd>
                  </dl><br />
                  <a href="javascript:openHelpWindow('HSputnikStatus.asp')">More...</a>
               </div>
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
