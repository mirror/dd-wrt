<% do_pagehead_nopwc("status_lan.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function deleteLease(val, val2) {
	document.forms[0].del_ip.value = val;
	document.forms[0].del_mac.value = val2;
	document.forms[0].submit_type.value = "delete";
	document.forms[0].change_action.value="gozila_cgi";
	document.forms[0].submit();
}

function staticLease(hostname, ip, mac) {
	document.forms[0].add_ip.value = ip;
	document.forms[0].add_mac.value = mac;
	document.forms[0].hostname.value = hostname;
	document.forms[0].submit_type.value = "static";
	document.forms[0].change_action.value="gozila_cgi";
	document.forms[0].submit();
}

function deletepptp(val) {
	document.forms[0].del_if.value = val;
	document.forms[0].submit_type.value = "deletepptp";
	document.forms[0].change_action.value="gozila_cgi";
	document.forms[0].submit();
}

function setPPTPTable() {
	var val = arguments;
	var table = document.getElementById("pptp_table");
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

		row.insertCell(-1).innerHTML = val[i]; // interface
		row.insertCell(-1).innerHTML = val[i+1]; // peer name
		row.insertCell(-1).innerHTML = val[i+2]; // local ip
		row.insertCell(-1).innerHTML = val[i+3]; // remote ip

		var cell = row.insertCell(-1);
		cell.title = errmsg.err581;
		cell.align = "center";
		cell.innerHTML = "<input class=\"remove\" type=\"button\" aria-label=\"" + sbutton.del + "\" onclick=\"deletepptp('" + val[i + 4] + "')\" />";
	}
}

function setPPPOETable() {
	var val = arguments;
	var table = document.getElementById("pppoe_table");
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

		row.insertCell(-1).innerHTML = val[i]; // interface
		row.insertCell(-1).innerHTML = val[i+1]; // peer name
		row.insertCell(-1).innerHTML = val[i+2]; // local ip

		var cell = row.insertCell(-1);
		cell.title = errmsg.err581;
		cell.align = "center";
		cell.innerHTML = "<input class=\"remove\" type=\"button\" aria-label=\"" + sbutton.del + "\" onclick=\"deletepptp('" + val[i + 3] + "')\" />";
	}
}

function setDHCPTable() {
	var val = arguments;
	var table = document.getElementById("dhcp_leases_table");
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 6;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 7) {
		var row = table.insertRow(-1);
		row.style.height = "15px";
		row.insertCell(-1).innerHTML = val[i];

		var cellip = row.insertCell(-1);
		cellip.title = val[i+1];
		cellip.innerHTML = val[i+1];

		var cellmac = row.insertCell(-1);
		cellmac.classList.add("link");
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];

		var cellbail = row.insertCell(-1);
		cellbail.innerHTML = val[i+3];
		
		var cell = row.insertCell(-1);
		cell.title = errmsg.err58;
		cell.align = "right";
		cell.innerHTML = "<input class=\"remove\" type=\"button\" aria-label=\"" + sbutton.del + "\" onclick=\"deleteLease('" + val[i+1] + "', '" + val[i+2] + "')\" />";
		var cellbutton = row.insertCell(-1);
		cellbutton.title = errmsg.err582;
		cellbutton.align = "left";
		cellbutton.innerHTML = "<input class=\"add\" type=\"button\" aria-label=\"" + share.add + "\" onclick=\"staticLease('" + val[i] + "', '" + val[i+1] + "', '" + val[i+2] + "')\" />";
	}
}

function getSize(size) {
	var prefix=new Array("","k","M","G","T","P","E","Z"); var base = 1000;
	var pos=0;
	while (size>base) {
		size/=base; pos++;
	}
	if (pos > 2) {
		precision = 100; 
	} else {
		precision = 1;
	}
	return (Math.round(size*precision)/precision)+prefix[pos];
}

function setARPTable() {
	var table = document.getElementById("active_clients_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 9;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 8) {
		var row = table.insertRow(-1);
		row.style.height = "20px";
		row.insertCell(-1).innerHTML = val[i];

		var cellip = row.insertCell(-1);
		cellip.title = val[i+1];
		cellip.innerHTML = val[i+1];

		var cellmac = row.insertCell(-1);
		cellmac.classList.add("link");
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + val[i+2] + "') })");
		cellmac.innerHTML = val[i+2];

		var cellif = row.insertCell(-1);
		cellif.style.textAlign = 'center';
		cellif.innerHTML = val[i+4];

		var cellcount = row.insertCell(-1);
		cellcount.style.textAlign = 'center';
		cellcount.innerHTML = getSize(val[i+5]);

		var cellcount = row.insertCell(-1);
		cellcount.style.textAlign = 'center';
		cellcount.innerHTML = getSize(val[i+6]);

		var cellcount = row.insertCell(-1);
		cellcount.style.textAlign = 'center';
		cellcount.innerHTML = getSize(val[i+7]);

		var cellcount = row.insertCell(-1);
		cellcount.style.textAlign = 'center';
		cellcount.innerHTML = val[i+3];

		setMeterBar(row.insertCell(-1), parseInt(val[i+3])/<% nvg("ip_conntrack_max"); %>*100, "");
	}
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	setElementContent("dhcp_end_ip", "<% calcendip(); %>");
	setDHCPTable(<% dumpleases(0); %>);
<% ifndef("PPTPD", "/*"); %>
	setPPTPTable(<% dumppptp(); %>);
	setElementVisible("pptp", "<% nvg("pptpd_enable"); %>" == "1");
<% ifndef("PPTPD", "*/"); %>
<% ifndef("PPPOESERVER", "/*"); %>
	setPPPOETable(<% dumppppoe(); %>);
	setElementVisible("pppoe", "<% nvg("pppoeserver_enabled"); %>" == "1");
<% ifndef("PPPOESERVER", "*/"); %>
	setARPTable(<% dumparptable(0); %>);
	setElementVisible("dhcp_1", "<% dhcpenabled("dhcp","static"); %>" == "dhcp");
	setElementVisible("dhcp_2", "<% dhcpenabled("dhcp","static"); %>" == "dhcp");

	update = new StatusUpdate("Status_Lan.live.asp", <% nvg("refresh_time"); %>);
	update.onUpdate("lan_proto", function(u) {
		setElementVisible("dhcp_1", u.lan_proto == "dhcp");
		setElementVisible("dhcp_2", u.lan_proto == "dhcp");
	});
	update.onUpdate("dhcp_start", function(u) {
		setElementContent("dhcp_start_ip", u.dhcp_start);
	});
	update.onUpdate("dhcp_end", function(u) {
		setElementContent("dhcp_end_ip", u.dhcp_end);
	});
	update.onUpdate("dhcp_leases", function(u) {
		eval('setDHCPTable(' + u.dhcp_leases + ')');
	});
<% ifndef("PPTPD", "/*"); %>
	update.onUpdate("pptp_leases", function(u) {
		eval('setPPTPTable(' + u.pptp_leases + ')');
	});
<% ifndef("PPTPD", "*/"); %>
<% ifndef("PPPOESERVER", "/*"); %>
	update.onUpdate("pppoe_leases", function(u) {
		eval('setPPPOETable(' + u.pppoe_leases + ')');
	});
<% ifndef("PPPOESERVER", "*/"); %>
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
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Status_Router.asp","Status_Lan.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Status_Lan" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" value="delete" />

							<input type="hidden" name="del_if" />
							<input type="hidden" name="del_ip" />
							<input type="hidden" name="del_mac" />
							<input type="hidden" name="add_ip" />
							<input type="hidden" name="add_mac" />
							<input type="hidden" name="hostname" />
							<input type="hidden" name="ip" />
							<input type="hidden" name="mac" />

							<h2><% tran("status_lan.h2"); %></h2>
							<fieldset>
								<legend><% tran("status_lan.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<span id=\"lan_mac\" class=\"link\" style=\"cursor:pointer;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% nvg("lan_hwaddr"); %>')\" >");
									document.write("<% nvg("lan_hwaddr"); %>");
									document.write("</span>");
									//]]>
									</script>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.ip"); %></div>
									<span id="lan_ip"><% nvg("lan_ipaddr"); %>/<% get_cidr_mask("lan_netmask"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.gateway"); %></div>
									<span id="lan_gateway"><% nvg("lan_gateway"); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.localdns"); %></div>
									<span id="lan_dns"><% nvg("sv_localdns"); %></span>&nbsp;
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("status_lan.legend4"); %></legend>
								<table class="table" cellspacing="5" id="active_clients_table" summary="active clients in arp table">
								<tbody>
									<tr>
										<th sortdir="up" width="15%"><% tran("share.hostname"); %></th>
										<th sortdir="up" width="20%"><% tran("share.ip"); %></th>
										<th sortdir="up" width="16%"><% tran("share.mac"); %></th>
										<th sortdir="up" width="5%"><% tran("share.iftbl"); %></th>
										<th sortdir="up" width="5%"><% tran("share.input"); %></th>
										<th sortdir="up" width="5%"><% tran("share.output"); %></th>
										<th sortdir="up" width="5%"><% tran("share.total"); %></th>
										<th sortdir="up" width="13%"><% tran("status_lan.concount"); %></th>
										<th sortdir="up" width="15%"><% tran("status_lan.conratio"); %> [<% nvg("ip_conntrack_max"); %>]</th>
									</tr>
								</tbody>
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
									<% dhcpenabled("<script type="text/javascript">Capture(share.enabled)</script>","<script type="text/javascript">Capture(share.disabled)</script>"); %>&nbsp;
								</div>
								<div id="dhcp_1" style="display:none">
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_start"); %></div>
										<span id="dhcp_start_ip"><% nvg("dhcp_start"); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_end"); %></div>
										<span id="dhcp_end_ip"><% calcendip(); %></span>&nbsp;
									</div>
									<div class="setting">
										<div class="label"><% tran("idx.dhcp_lease"); %></div>
										<span id="dhcp_lease_time"><% nvg("dhcp_lease"); %></span> <% tran("share.minutes"); %>&nbsp;
									</div>
								</div>
							</fieldset><br />
							<div id="dhcp_2" style="display:none">
								<fieldset>
									<legend><% tran("status_lan.legend3"); %></legend>
									<table class="table" cellspacing="6" id="dhcp_leases_table" summary="dhcp leases table">
									<tbody>
										<tr>
											<th sortdir="up" width="20%"><% tran("share.hostname"); %></th>
											<th sortdir="up" width="17%"><% tran("share.ip"); %></th>
											<th sortdir="up" width="16%"><% tran("share.mac"); %></th>
											<th sortdir="up" width="18%"><% tran("idx.dhcp_lease"); %></th>
											<th sortdir="up" width="12%" class="center" colspan="2"><% tran("share.static_lease"); %></th>
										</tr>
									</tbody>
									</table>
									<script type="text/javascript">
									//<![CDATA[
									var t = new SortableTable(document.getElementById('dhcp_leases_table'), 1000);
									//]]>
									</script>
								</fieldset><br />
							</div>
							<% ifndef("PPTPD", "<!--"); %>
							<div id="pptp" style="display:none">
								<fieldset>
									<legend><% tran("status_lan.legend5"); %></legend>
									<table class="table" cellspacing="6" id="pptp_table" summary="pptp table">
									<tbody>
										<tr>
											<th sortdir="up" width="15%"><% tran("share.iftbl"); %></th>
											<th sortdir="up" width="51%"><% tran("share.usrname"); %></th>
											<th sortdir="up" width="17%"><% tran("share.localip"); %></th>
											<th sortdir="up" width="17%"><% tran("share.remoteip"); %></th>
											<th sortdir="up" width="10%"><% tran("share.del"); %></th>
										</tr>
									</tbody>
									</table>
									<script type="text/javascript">
									//<![CDATA[
									var t = new SortableTable(document.getElementById('pptp_table'), 1000);
									//]]>
									</script>
								</fieldset><br />
							</div>
							<% ifndef("PPTPD", "-->"); %>
							<% ifndef("PPPOESERVER", "<!--"); %>
							<div id="pppoe" style="display:none">
								<fieldset>
									<legend><% tran("status_lan.legend6"); %></legend>
									<table class="table" cellspacing="6" id="pppoe_table" summary="pppoe table">
									<tbody>
										<tr>
											<th sortdir="up" width="15%"><% tran("share.iftbl"); %></th>
											<th sortdir="up" width="68%"><% tran("share.usrname"); %></th>
											<th sortdir="up" width="17%"><% tran("share.localip"); %></th>
											<th sortdir="up" width="10%"><% tran("share.del"); %></th>
										</tr>
									</tbody>
									</table>
									<script type="text/javascript">
									//<![CDATA[
									var t = new SortableTable(document.getElementById('pppoe_table'), 1000);
									//]]>
									</script>
								</fieldset><br />
							</div>
							<% ifndef("PPPOESERVER", "-->"); %>
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
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
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"<% get_firmware_version_href(); %>\"><% get_firmware_version(); %></a>");
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
