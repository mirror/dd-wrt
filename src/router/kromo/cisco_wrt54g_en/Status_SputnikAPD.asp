<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Sputnik Agent Status</title>
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
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li class="current"><span><% tran("bmenu.statu"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Status_Router.asp"><% tran("bmenu.statuRouter"); %></a></li>
											<li><a href="Status_Lan.asp"><% tran("bmenu.statuLAN"); %></a></li>
											<li><a href="Status_Wireless.asp"><% tran("bmenu.statuWLAN"); %></a></li>
											<li><span><% tran("bmenu.statuSputnik"); %></span></li>
											<% show_openvpn(); %>
											<% nvram_invmatch("status_auth","1","<!--"); %>
											<li><a href="Info.htm"><% tran("bmenu.statuSysInfo"); %></a></li>
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
					<h2><% tran("status_sputnik.h2"); %></h2>
					
					<fieldset>
						<legend><% tran("share.info"); %></legend>
                        <div class="setting">
                        	<div class="label"><% tran("status_sputnik.manage"); %></div>
                        	<span id="sputnik_status"><% sputnik_apd_status("scc_server"); %></span>&nbsp;
                        </div>
                        <div class="setting">
                        	<div class="label"><% tran("share.state"); %></div>
                        	<span id="sputnik_state"><% sputnik_apd_status("phase"); %></span>&nbsp;
                        </div>
                        <div class="setting">
                        	<div class="label"><% tran("status_sputnik.license"); %></div>
                        	<span id="sputnik_serial"><% sputnik_apd_status("lsk_serial"); %></span>&nbsp;
                        </div>
                    
                    </fieldset><br />
                    
                    <div class="submitFooter">
                    	<script type="text/javascript">document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");</script>
                    </div>
                </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                  	<h2><% tran("share.help"); %></h2>
                  </div>
                  <dl>
                     <dt class="term"><% tran("hstatus_sputnik.right1"); %>:</dt>
                     <dd class="definition"><% tran("hstatus_sputnik.right2"); %></dd>
                     <dt class="term"><% tran("status_sputnik.manage"); %>:</dt>
                     <dd class="definition"><% tran("hstatus_sputnik.right4"); %></dd>
                     <dt class="term"><% tran("share.state"); %>:</dt>
                     <dd class="definition"><% tran("hstatus_sputnik.right6"); %></dd>
                     <dt class="term"><% tran("status_sputnik.license"); %>:</dt>
                     <dd class="definition"><% tran("hstatus_sputnik.right8"); %></dd>
                  </dl><br />
                  
                  <!--<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HSputnikStatus.asp');"><% tran("share.more"); %></a>-->
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
