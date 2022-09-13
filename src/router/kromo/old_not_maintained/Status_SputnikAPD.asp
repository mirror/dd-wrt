<% do_pagehead("status_sputnik.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

var update;

addEvent(window, "load", function() {
	<% show_status("onload");%>
	
	update = new StatusUpdate("Status_SputnikAPD.live.asp", <% nvg("refresh_time"); %>);
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

		//]]>
		</script>
	</head>
	<body class="gui">
		
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Status_Router.asp","Status_SputnikAPD.asp"); %>
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

				<div class="submitFooter nostick">
					<script type="text/javascript">
											//<![CDATA[
											var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
											submitFooterButton(0,0,0,autoref);
											//]]>
											</script>
				</div>
			 </div>
		  </div>
		  <div id="helpContainer">
			<div id="help">
			   <h2><% tran("share.help"); %></h2>
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
			   
			   <!--<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HSputnikStatus.asp');"><% tran("share.more"); %></a>-->
			</div>
		  </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
					//]]>
					</script>
				</div>
				<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
				<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>
