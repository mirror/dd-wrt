<% do_pagehead("status_wireless.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function setWirelessTable() {
	var table = document.getElementById("wireless_table");
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
		
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		cellmac.style.textDecoration = "underline";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;

		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		row.insertCell(-1).innerHTML = val[i + 4];
		row.insertCell(-1).innerHTML = val[i + 5];
		row.insertCell(-1).innerHTML = val[i + 6];
		row.insertCell(-1).innerHTML = val[i + 7];
		setMeterBar(row.insertCell(-1), (val[i + 5] == "0" ? 0 : parseInt(val[i + 5]) * 1.24 + 116), "");
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
	for(var i = 0; i < val.length; i = i + 5) {
		var row = table.insertRow(-1);
		
		var mac = val[i];
		var cellmac = row.insertCell(-1);
		cellmac.title = share.oui;
		cellmac.style.cursor = "pointer";
		cellmac.style.textDecoration = "underline";
		eval("addEvent(cellmac, 'click', function() { getOUIFromMAC('" + mac + "') })");
		cellmac.innerHTML = mac;
		
		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
		row.insertCell(-1).innerHTML = val[i + 3];
		row.insertCell(-1).innerHTML = val[i + 4];
		setMeterBar(row.insertCell(-1), (val[i + 2] == "0" ? 0 : parseInt(val[i + 2]) * 1.24 + 116), "");
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
		openWindow('Site_Survey.asp', 760, 700);
	}
	else {
		openWindow('Site_Survey.asp', 760, 700);
		alert(errmsg.err59);
	};
}

function OpenWiwizSurvey () {
	if( "<% radio_on(); %>" == "1" ) {
		openWindow('Wiviz_Survey.asp', 760, 700);
	}
	else {
		alert(errmsg.err59);
	};
}	

var update;

addEvent(window, "load", function() {
	setWirelessTable(<% active_wireless(0); %>);
	setWDSTable(<% active_wds(0); %>);
	setPacketInfo("<% wl_packet_get(); %>");

	update = new StatusUpdate("Status_Wireless.live.asp", <% nvram_get("refresh_time"); %>);
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

function refresh(F)
{
F.submit();
}

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
					<% do_menu("Status_Router.asp","Status_Wireless.asp"); %>
				</div>

				<div id="main">
					<div id="contents">
						<form name="Status_Wireless" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Status_Wireless" />
							<input type="hidden" name="next_page" value="Status_Wireless.asp" />
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
									document.write("<span id=\"wl_mac\" style=\"cursor:pointer; text-decoration:underline;\" title=\"" + share.oui + "\" onclick=\"getOUIFromMAC('<% show_wl_mac(); %>')\" >");
									document.write("<% show_wl_mac(); %>");
									document.write("</span>");
									//]]>
									</script>&nbsp;
								</div>
								<div class="setting">
									<div class="label"><% tran("wl_basic.radio"); %></div>
									<span id="wl_radio"><% get_radio_state(); %></span>&nbsp;
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
								<div class="setting">
									<div class="label"><% tran("share.channel"); %></div>
									<span id="wl_channel"><% get_curchannel(); %></span>&nbsp;
								</div>
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
									<div class="label"><% tran("status_wireless.pptp"); %></div>
									<% nvram_else_match("pptpd_connected", "1", "<script type="text/javascript">Capture(share.connected)</script>", "<script type="text/javascript">Capture(share.disconnected)</script>"); %>&nbsp;
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
								<table class="table center" cellspacing="5" id="wireless_table" summary="wireless clients table">
									<tr>
										<th width="14%"><% tran("share.mac"); %></th>
										<th width="8%"><% tran("share.intrface"); %></th>
										<th width="10%"><% tran("status_router.sys_up"); %></th>
										<th width="10%"><% tran("share.txrate"); %></th>
										<th width="11%"><% tran("share.rxrate"); %></th>
										<th width="8%"><% tran("share.signal"); %></th>
										<th width="8%"><% tran("share.noise"); %></th>
										<th width="7%">SNR</th>
										<th width="24%"><% tran("status_wireless.signal_qual"); %></th>
									</tr>
								</table>
							</fieldset><br />
							
							<div id="wds" style="display:none">
								<fieldset>
									<legend><% tran("status_wireless.wds"); %></legend>
									<table class="table center" cellspacing="5" id="wds_table" summary="wds clients table">
										<tr>
											<th width="28%"><% tran("share.mac"); %></th>
											<th width="24%"><% tran("share.descr"); %></th>
											<th width="8%"><% tran("share.signal"); %></th>
											<th width="8%"><% tran("share.noise"); %></th>
											<th width="8%">SNR</th>
											<th width="24%"><% tran("status_wireless.signal_qual"); %></th>
										</tr>
									</table>
								</fieldset><br />
								
							</div>
							<div class="center">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"site_survey\" value=\"" + sbutton.survey + "\" onclick=\"OpenSiteSurvey()\" />");
								<% ifndef("WIVIZ","/"); %><% ifndef("WIVIZ","/"); %>document.write("<input class=\"button\" type=\"button\" name=\"wiviz_survey\" value=\"Wiviz survey\" onclick=\"OpenWiwizSurvey()\" />");
								//]]>
								</script>
							</div><br />
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
