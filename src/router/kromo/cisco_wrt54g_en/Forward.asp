<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Port Range Forwarding</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + prforward.titl;

function forward_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Forward";
	F.submit_type.value = "add_forward";
	
 	F.action.value = "Apply";
	F.submit();
}

function forward_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Forward";
	F.submit_type.value = "remove_forward";
	
 	F.action.value = "Apply";
	F.submit();
}

function to_submit(F) {
	F.submit_button.value = "Forward";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}
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
								<li class="current"><span><% tran("bmenu.applications"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span><% tran("bmenu.applicationsprforwarding"); %></span></li>
											<li><a href="ForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
											<li><a href="Triggering.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
											<li><a href="UPnP.asp"><% tran("bmenu.applicationsUpnp"); %></a></li>
											<li><a href="DMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
											<li><a href="QoS.asp"><% tran("bmenu.applicationsQoS"); %></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="portRange" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<input type="hidden" name="forward_port" value="13" />
							<h2><% tran("prforward.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("prforward.legend"); %></legend>
								<table class="table center" cellspacing="5">
									<tr>
										<th><% tran("prforward.app"); %></th>
										<th><% tran("share.start"); %></th>
										<th><% tran("share.end"); %></th>
										<th><% tran("share.proto"); %></th>
										<th><% tran("share.ip"); %></th>
										<th><% tran("share.enable"); %></th>
									</tr>
									<% show_forward(); %>
								</table><br />
								<div class="center">
									<script type="text/javascript">document.write("<input class=\"btn\" type=\"button\" value=\"" + sbutton.add + "\" onclick=\"forward_add_submit(this.form)\">");</script>
									<script type="text/javascript">document.write("<input class=\"btn\" type=\"button\" value=\"" + sbutton.remove + "\" onclick=\"forward_remove_submit(this.form)\">");</script>
								</div>
						 	</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\">");</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\">");</script>
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
							<dt class="term"><% tran("prforward.h2"); %>:</dt>
							<dd class="definition"><% tran("hprforward.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HForwardRange.asp')"><% tran("share.more"); %></a>
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