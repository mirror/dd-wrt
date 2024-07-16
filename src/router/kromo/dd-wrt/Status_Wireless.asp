<% do_pagehead_nopwc("status_wireless.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function setWirelessTable() {
	var table = document.getElementById("wireless_table");
	var val = arguments;
	if (table) {
		cleanTable(table);
  }
	if(!val.length) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 11;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " -";
		return;
	}
	for(var i = 0; i < val.length; i = i + 21) {
		var row = table.insertRow(-1);
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		if (val[i + 11] == 0) {
			cellmac.title = share.oui;
		} else {
			cellmac.title = status_wireless.chaininfo + " [" + val[i + 11];
			if (val[i + 12] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 12];
			}
			if (val[i + 13] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 13];
			}
			if (val[i + 14] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 14];
			}
			if (val[i + 15] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 15];
			}
			if (val[i + 16] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 16];
			}
			if (val[i + 17] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 17];
			}
			if (val[i + 18] != 0) {
				cellmac.title = cellmac.title + "," + val[i + 18];
			}
			cellmac.title = cellmac.title + "]";
		}
		cellmac.classList.add("link");
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;
		row.insertCell(-1).innerHTML = val[i + 1];
		var ifn = val[i + 2];
		var label = val[i + 19];
		var iface = row.insertCell(-1);
		iface.classList.add("link");
		iface.title = status_band.titl;
		iface.style.cursor = "pointer";
		iface.style.textDecoration = "none";
			eval("addEvent(iface, 'click', function() { openBW('" + val[i + 20] + "') })");
		if (label.length == 0) {
			iface.innerHTML = ifn;
		} else {
			iface.innerHTML = label;
		}
		row.insertCell(-1).innerHTML = val[i + 3];
		row.insertCell(-1).innerHTML = val[i + 4];
		row.insertCell(-1).innerHTML = val[i + 5];
		row.insertCell(-1).innerHTML = val[i + 6];
		row.insertCell(-1).innerHTML = val[i + 7];
		row.insertCell(-1).innerHTML = val[i + 8];
		row.insertCell(-1).innerHTML = val[i + 9];
		setMeterBar(row.insertCell(-1), (val[i + 10] == "0" ? 0 : parseInt(val[i + 10]) * 0.1), "");
	}
}

function setWDSTable() {
	var table = document.getElementById("wds_table");
	var val = arguments;
	cleanTable(table);
	if(!val.length) {
		setElementVisible("wds", false);
		return;
	}
	for(var i = 0; i < val.length; i = i + 6) {
		var row = table.insertRow(-1);
		
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		cellmac.classList.add("link");
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;

		var ifn = val[i + 1];
		var iface = row.insertCell(-1);
		iface.classList.add("link");
		iface.title = status_band.titl;
		iface.style.cursor = "pointer";
		iface.style.textDecoration = "none";
		eval("addEvent(iface, 'click', function() { openBW('" + ifn + "') })");
		iface.innerHTML = ifn;
			
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		row.insertCell(-1).innerHTML = val[i + 4];
		row.insertCell(-1).innerHTML = val[i + 5];
		setMeterBar(row.insertCell(-1), (val[i + 3] == "0" ? 0 : parseInt(val[i + 3]) * 1.24 + 116), "");
	}
	setElementVisible("wds", true);
}

function setPacketInfo(val) {
	var packet = val.replace(/[A-Za-z=]/g, "").split(";");
	setMeterBar("packet_rx", 
		(parseInt(packet[1]) == 0 ? 100 : parseInt(packet[0]) / (parseInt(packet[0]) + parseInt(packet[1])) * 100),
		packet[0] + " OK, " + (packet[1] > 0 ? packet[1] + " " + share.errs : share.none2 + " " + share.err)
	);
	setMeterBar("packet_tx",
		(parseInt(packet[3]) == 0 ? 100 : parseInt(packet[2]) / (parseInt(packet[2]) + parseInt(packet[3])) * 100),
		packet[2] + " OK, " + (packet[3] > 0 ? packet[3] + " " + share.errs : share.none2 + " " + share.err)
	);
}

function OpenSiteSurvey () {
	if( "<% radio_on(); %>" == "1" ) {
		openWindow('Site_Survey.asp', 1024, 700);
	}
	else {
		alert(errmsg.err59);
	};
}

function OpenChannelSurvey () {
	if( "<% radio_on(); %>" == "1" ) {
		openWindow('Channel_Survey.asp', 980, 700);
	}
	else {
		alert(errmsg.err59);
	};
}

function OpenSpectral () {
	if( "<% radio_on(); %>" == "1" ) {
		openWindow('spectral_scan.html', 1024, 700);
	}
	else {
		alert(errmsg.err59);
	};
}

function OpenWiwizSurvey () {
	if( "<% radio_on(); %>" == "1" ) {
		openWindow('Wiviz_Survey.asp', 1024, 700);
	}
	else {
		alert(errmsg.err59);
	};
}	

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	setWirelessTable(<% active_wireless(0); %>);
	setWDSTable(<% active_wds(0); %>);
	setPacketInfo("<% wl_packet_get(); %>");

	update = new StatusUpdate("Status_Wireless.live.asp", <% nvg("refresh_time"); %>);
	update.onUpdate("active_wireless", function(u) {
		eval('setWirelessTable(' + u.active_wireless + ')');
	});
	update.onUpdate("active_wds", function(u) {
		eval('setWDSTable(' + u.active_wds + ')');
	});
	update.onUpdate("packet_info", function(u) {
		setPacketInfo(u.packet_info);
	});
	update.start();
});

function refresh(F) {
	F.submit();
}

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
					<% do_menu("Status_Router.asp","Status_Wireless.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="Status_Wireless" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Status_Wireless" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" value="refresh" />
							<h2><% tran("status_wireless.h2"); %></h2>

							<fieldset>
								<legend><% tran("status_wireless.legend"); %></legend>
								<% show_wifiselect(); %>
								<div class="setting">
									<div class="label"><% tran("share.mac"); %></div>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<span id=\"wl_mac\" class=\"link\" style=\"cursor: pointer;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% show_wl_mac(); %>')\" >");
										document.write("<% show_wl_mac(); %>");
										document.write("</span>");
										//]]>
										</script>&nbsp;
								</div>
								<% getchipset(); %>
								<div class="setting">
									<div class="label"><% tran("wl_basic.radio"); %></div>
									<span id="wl_radio"><% get_radio_statejs(); %></span>&nbsp;
								</div>								
								<div class="setting">
									<div class="label"><% tran("share.mode"); %></div>
									<% getwirelessmode(); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_wireless.net"); %></div>
									<% getwirelessnetmode(); %>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.ssid"); %></div>
									<span id="wl_ssid"><% getwirelessssid(); %></span>&nbsp;
								</div>
								<% get_status_curchannel(); %>
								<% show_busy(); %>
								<div class="setting">
									<div class="label"><% tran("wl_basic.TXpower"); %></div>
									<span id="wl_xmit"><% get_txpower(); %></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("share.rates"); %></div>
									<span id="wl_rate"><% get_currate(); %></span>&nbsp;
								</div>
								<% show_acktiming(); %>
								<% getencryptionstatus(); %>
								<div class="setting">
									<div class="label"><% tran("status_wireless.assoc_count"); %></div>
									<span id="assoc_count"><% assoc_count(); %></span>&nbsp;
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("status_wireless.legend2"); %></legend>
								<div class="setting">
									<div class="label"><% tran("status_wireless.rx"); %></div>
									<span id="packet_rx"></span>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("status_wireless.tx"); %></div>
									<span id="packet_tx"></span>&nbsp;
								</div>
							</fieldset><br />
							
							<h2><% tran("status_wireless.h22"); %></h2>
							<fieldset>
								<legend><% getwirelessstatus(); %></legend>
								<table class="table" cellspacing="5" id="wireless_table" summary="wireless clients table">
								<tbody>
									<tr>
										<th sortdir="up" width="16%"><% tran("share.mac"); %></th>
										<th sortdir="up" width="10%"><% tran("share.radioname"); %></th>
										<th sortdir="up" width="7%"><% tran("share.iftbl"); %></th>
										<th sortdir="up" width="8%"><% tran("status_router.sys_up"); %></th>
										<th sortdir="up" width="8%"><% tran("share.txrate"); %></th>
										<th sortdir="up" width="8%"><% tran("share.rxrate"); %></th>
										<th sortdir="up" width="11%"><% tran("share.infotbl"); %></th>
										<th sortdir="up" width="8%"><% tran("share.signal"); %></th>
										<th sortdir="up" width="7%"><% tran("share.noise"); %></th>
										<th sortdir="up" width="5%">SNR</th>
										<th sortdir="up" width="12%"><% tran("status_wireless.signal_qual"); %></th>
									</tr>
								</tbody>
								</table>
								<script type="text/javascript">
								//<![CDATA[
								var t = new SortableTable(document.getElementById('wireless_table'), 1000);
								//]]>
								</script>
							</fieldset><br />
							<div id="wds" style="display: none">
								<fieldset>
									<legend><% tran("status_wireless.wds"); %></legend>
									<table class="table" cellspacing="5" id="wds_table" summary="wds clients table">
									<tbody>
										<tr>
											<th sortdir="up" width="16%"><% tran("share.mac"); %></th>
											<th sortdir="up" width="10%"><% tran("share.iftbl"); %></th>
											<th sortdir="up" width="26%"><% tran("share.descr"); %></th>
											<th sortdir="up" width="8%"><% tran("share.signal"); %></th>
											<th sortdir="up" width="8%"><% tran("share.noise"); %></th>
											<th sortdir="up" width="8%">SNR</th>
											<th sortdir="up" width="24%"><% tran("status_wireless.signal_qual"); %></th>
										</tr>
									</tbody>
									</table>
									<script type="text/javascript">
									//<![CDATA[
									var t = new SortableTable(document.getElementById('wds_table'), 1000);
									//]]>
									</script>
								</fieldset><br />
							</div>
							<!--<div class="center">
								<script type="text/javascript">
								//<![CDATA[
								//]]>
								</script>
							</div><br /> -->
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								<% spectral_scan(); %>
								document.write("<input class=\"button\" type=\"button\" name=\"site_survey\" value=\"" + sbutton.survey + "\" onclick=\"OpenSiteSurvey()\" />");
								<% channel_survey(); %>
								<% ifndef("WIVIZ","/"); %><% ifndef("WIVIZ","*"); %>document.write("<input class=\"button\" type=\"button\" name=\"wiviz_survey\" value=\"" + sbutton.wsurvey + "\" onclick=\"OpenWiwizSurvey()\" />");
								<% ifndef("WIVIZ","*"); %><% ifndef("WIVIZ","/"); %>
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
							<dd class="definition"><% tran("hstatus_wireless.right2"); %></dd>
							<dt class="term"><% tran("status_wireless.net"); %>:</dt>
							<dd class="definition"><% tran("hstatus_wireless.right4"); %></dd>
							<dt class="term"><% tran("share.oui"); %>:</dt>
							<dd class="definition"><% tran("hstatus_lan.right10"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HStatusWireless.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
