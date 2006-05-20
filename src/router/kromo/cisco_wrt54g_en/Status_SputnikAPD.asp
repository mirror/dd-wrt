<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Sputnik Agent Status</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + status_sputnik.titl;

var update;

addEvent(window, "load", function() {
	<% show_status("onload");%>
	
	update = new StatusUpdate("Status_SputnikAPD.live.asp", <% nvram_get("refresh_time"); %>);
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
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
								<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
								<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li class="current"><span><script type="text/javascript">Capture(bmenu.statu)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statuRouter)</script></a></li>
											<li><a href="Status_Lan.asp"><script type="text/javascript">Capture(bmenu.statuLAN)</script></a></li>
											<li><a href="Status_Wireless.asp"><script type="text/javascript">Capture(bmenu.statuWLAN)</script></a></li>
											<li><span><script type="text/javascript">Capture(bmenu.statuSputnik)</script></span></li>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm"><script type="text/javascript">Capture(bmenu.statuSysInfo)</script></a></li>
											<% nvram_invmatch("status_auth","1","-->"); %>
										</ul>
									</div>
								</li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
					<h2><script type="text/javascript">Capture(status_sputnik.h2)</script></h2>
					
					<fieldset>
						<legend><script type="text/javascript">Capture(share.info)</script></legend>
                        <div class="setting">
                        	<div class="label"><script type="text/javascript">Capture(status_sputnik.manage)</script></div>
                        	<span id="sputnik_status"><% sputnik_apd_status("scc_server"); %></span>&nbsp;
                        </div>
                        <div class="setting">
                        	<div class="label"><script type="text/javascript">Capture(share.state)</script></div>
                        	<span id="sputnik_state"><% sputnik_apd_status("phase"); %></span>&nbsp;
                        </div>
                        <div class="setting">
                        	<div class="label"><script type="text/javascript">Capture(status_sputnik.license)</script></div>
                        	<span id="sputnik_serial"><% sputnik_apd_status("lsk_serial"); %></span>&nbsp;
                        </div>
                    
                    </fieldset><br />
                    
                    <div class="submitFooter">
                    	<script>document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");</script>
                    </div>
                </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                  	<h2><script type="text/javascript">Capture(share.help)</script></h2>
                  </div>
                  <dl>
                     <dt class="term"><script type="text/javascript">Capture(hstatus_sputnik.right1)</script>:</dt>
                     <dd class="definition"><script type="text/javascript">Capture(hstatus_sputnik.right2)</script></dd>
                     <dt class="term"><script type="text/javascript">Capture(status_sputnik.manage)</script>:</dt>
                     <dd class="definition"><script type="text/javascript">Capture(hstatus_sputnik.right4)</script></dd>
                     <dt class="term"><script type="text/javascript">Capture(share.state)</script>:</dt>
                     <dd class="definition"><script type="text/javascript">Capture(hstatus_sputnik.right6)</script></dd>
                     <dt class="term"><script type="text/javascript">Capture(status_sputnik.license)</script>:</dt>
                     <dd class="definition"><script type="text/javascript">Capture(hstatus_sputnik.right8)</script></dd>
                  </dl><br />
                  
                  <!--<a href="javascript:openHelpWindow('HSputnikStatus.asp');"><script type="text/javascript">Capture(share.more)</script></a>-->
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>
