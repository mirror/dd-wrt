<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - LAN Status</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + status_lan.titl;

function deleteLease(val, val2) {
	document.forms[0].ip_del.value = val;
	document.forms[0].mac_del.value = val2;
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
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];

		var cellbail = row.insertCell(-1);
		cellbail.align = "center";
		cellbail.innerHTML = val[i+3];
		
		var cell = row.insertCell(-1);
		cell.className = "bin";
		cell.title = errmsg.err58;
		eval("addEvent(cell, 'click', function() { deleteLease('" + val[i+1] + "','" + val[i+2] + "') })");
	}
}

function setARPTable() {
	var table = document.getElementById("arp_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 3;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 3) {
	
		var row = table.insertRow(-1);
		row.style.height = "15px";
		
		row.insertCell(-1).innerHTML = val[i];
		
		row.insertCell(-1).innerHTML = val[i+1];
		
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];
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
						<form action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="DHCPTable" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" value="delete" />
							<input type="hidden" name="ip_del" />
							<input type="hidden" name="mac_del" />
							<input type="hidden" name="next_page" value="Status_Lan.asp" />
							<h2><% tran("status_lan.h2"); %></h2>
							<fieldset>
								<legend><% tran("status_lan.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"lan_mac\" style=\"cursor:pointer\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvram_get("lan_hwaddr"); %>')\" >");
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
											<th width="25%"><% tran("share.expires"); %></th>
											<th>Delete</th>
										</tr>
									</table>
								</fieldset><br />
							</div>
							
							<fieldset>
								<legend><% tran("status_lan.legend4"); %></legend>
								<table class="table center" cellspacing="4" id="arp_table" summary="active clients in arp table">
									<tr>
										<th width="25%"><% tran("share.hostname"); %></th>
										<th width="25%"><% tran("share.ip"); %></th>
										<th width="50%"><% tran("share.mac"); %></th>
									</tr>
								</table>
							</fieldset><br />

							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HStatusLan.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>