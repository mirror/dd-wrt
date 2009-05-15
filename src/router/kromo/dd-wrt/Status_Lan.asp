<% do_pagehead("status_lan.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function deleteLease(val, val2) {
	document.forms[0].ip_del.value = val;
	document.forms[0].mac_del.value = val2;
	document.forms[0].change_action.value="gozila_cgi";
	document.forms[0].submit();
}

function setDHCPTable() {
	var val = arguments;
	var table = document.getElementById("dhcp_leases_table");
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 5;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 5) {
	
		var row = table.insertRow(-1);
		row.style.height = "15px";
		
		row.insertCell(-1).innerHTML = val[i];
		
		row.insertCell(-1).innerHTML = val[i+1];
		
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		cellmac.style.textDecoration = "underline";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];

		var cellbail = row.insertCell(-1);
		cellbail.innerHTML = val[i+3];
		
		var cell = row.insertCell(-1);
		cell.className = "bin";
		cell.title = errmsg.err58;
		eval("addEvent(cell, 'click', function() { deleteLease('" + val[i+1] + "','" + val[i+2] + "') })");
	}
}

function setARPTable() {
	var table = document.getElementById("active_clients_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 4;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 4) {
		var row = table.insertRow(-1);
		row.style.height = "15px";
		row.insertCell(-1).innerHTML = val[i];
		row.insertCell(-1).innerHTML = val[i+1];
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		cellmac.style.textDecoration = "underline";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];
		
		var cellcount = row.insertCell(-1);
		cellcount.style.textAlign = 'center';
		cellcount.innerHTML = val[i+3];
		
		setMeterBar(row.insertCell(-1), parseInt(val[i+3])/<% nvram_get("ip_conntrack_max"); %>*100, "");
	}
}

var update;

addEvent(window, "load", function() {
	setElementContent("dhcp_end_ip", "<% prefix_ip_get("lan_ipaddr",1); %>" + (parseInt("<% nvram_get("dhcp_start"); %>") + parseInt("<% nvram_get("dhcp_num"); %>") - 1));
	setDHCPTable(<% dumpleases(0); %>);
	setARPTable(<% dumparptable(0); %>);
	setElementVisible("dhcp_1", "<% nvram_get("lan_proto"); %>" == "dhcp");
	setElementVisible("dhcp_2", "<% nvram_get("lan_proto"); %>" == "dhcp");

	update = new StatusUpdate("Status_Lan.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("lan_proto", function(u) {
		setElementVisible("dhcp_1", u.lan_proto == "dhcp");
		setElementVisible("dhcp_2", u.lan_proto == "dhcp");
	});
	update.onUpdate("dhcp_start", function(u) {
		setElementContent("dhcp_start_ip", u.lan_ip_prefix + u.dhcp_start);
		setElementContent("dhcp_end_ip", u.lan_ip_prefix + (parseInt(u.dhcp_start) + parseInt(u.dhcp_num) - 1));
	});
	update.onUpdate("dhcp_leases", function(u) {
		eval('setDHCPTable(' + u.dhcp_leases + ')');
	});
	update.onUpdate("arp_table", function(u) {
		eval('setARPTable(' + u.arp_table + ')');
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
		
		//]]>
		</script>
	 </head>

	 <body class="gui">
	 	<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Status_Router.asp","Status_Lan.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="DHCPTable" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" value="delete" />
							<input type="hidden" name="next_page" value="Status_Lan.asp" />
							
							<input type="hidden" name="ip_del" />
							<input type="hidden" name="mac_del" />
							
							<h2><% tran("status_lan.h2"); %></h2>
							<fieldset>
								<legend><% tran("status_lan.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"lan_mac\" style=\"cursor:pointer; text-decoration:underline;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("lan_hwaddr"); %>')\" >");
									document.write("<% nvram_get("lan_hwaddr"); %>");
									document.write("</span>");
									//]]>
									</script>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.ip"); %></div>
									<span id="lan_ip"><% nvram_get("lan_ipaddr"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.subnet"); %></div>
									<span id="lan_netmask"><% nvram_get("lan_netmask"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.gateway"); %></div>
									<span id="lan_gateway"><% nvram_get("lan_gateway"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.localdns"); %></div>
									<span id="lan_dns"><% nvram_get("sv_localdns"); %></span>&nbsp;
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("status_lan.legend4"); %></legend>
								<table class="table center" cellspacing="5" id="active_clients_table" summary="active clients in arp table">
									<tr>
										<th width="25%"><% tran("share.hostname"); %></th>
										<th width="20%"><% tran("share.ip"); %></th>
										<th width="20%"><% tran("share.mac"); %></th>
										<th width="15%"><% tran("status_lan.concount"); %></th>
										<th width="20%"><% tran("status_lan.conratio"); %> [<% nvram_get("ip_conntrack_max"); %>]</th>
									</tr>
								</table>
								<script type="text/javascript">
								//<![CDATA[
								var t = new SortableTable(document.getElementById('active_clients_table'), 1000);
								//]]>
								</script>
							</fieldset><br />
							
							<h2><% tran("status_lan.h22"); %></h2>
							<fieldset>
								<legend><% tran("status_lan.legend2"); %></legend>
								<div class="setting">
									<div class="label"><% tran("service.dhcp_legend2"); %></div>
									<% nvram_match("lan_proto", "dhcp", "<script type="text/javascript">Capture(share.enabled)</script>"); %><% nvram_match("lan_proto", "static", "<script type="text/javascript">Capture(share.disabled)</script>"); %>&nbsp;
								</div>
								<div id="dhcp_1" style="display:none">
									<div class="setting">
										<div class="label"><% tran("service.dhcp_srv"); %></div>
										<span id="dhcp_daemon"><% nvram_else_match("dhcp_dnsmasq", "1", "DNSMasq", "uDHCPd"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_start"); %></div>
										<span id="dhcp_start_ip"><% prefix_ip_get("lan_ipaddr", "1"); %><% nvram_get("dhcp_start"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_end"); %></div>
										<span id="dhcp_end_ip"></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_lease"); %></div>
										<span id="dhcp_lease_time"><% nvram_get("dhcp_lease"); %></span> <% tran("share.minutes"); %>&nbsp;
									</div>
								</div>
							</fieldset><br />
							
							<div id="dhcp_2" style="display:none">
								<fieldset>
									<legend><% tran("status_lan.legend3"); %></legend>
									<table class="table center" cellspacing="6" id="dhcp_leases_table" summary="dhcp leases table">
										<tr>
											<th width="25%"><% tran("share.hostname"); %></th>
											<th width="25%"><% tran("share.ip"); %></th>
											<th width="25%"><% tran("share.mac"); %></th>
											<th width="25%"><script type="text/javascript">Capture(<% nvram_else_match("dhcp_dnsmasq","1","idx.dhcp_lease","share.expires"); %>)</script></th>
											<th><% tran("share.del"); %></th>
										</tr>
									</table>
									<script type="text/javascript">
									//<![CDATA[
									var t = new SortableTable(document.getElementById('dhcp_leases_table'), 1000);
									//]]>
									</script>
								</fieldset><br />
							</div>

							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("share.mac"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right2"); %></dd>
							<dt class="term"><% tran("share.ip"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right4"); %></dd>
							<dt class="term"><% tran("share.subnet"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right6"); %></dd>
							<dt class="term"><% tran("idx.dhcp_srv"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right8"); %></dd>
							<dt class="term"><% tran("share.oui"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right10"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HStatusLan.asp');"><% tran("share.more"); %></a>
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