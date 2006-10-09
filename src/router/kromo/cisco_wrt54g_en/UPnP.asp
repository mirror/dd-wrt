<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - UPnP</title>

<!--
		UPnP
		11.2005	tofu10		init
		Intergated to DD-WRT by LawnMowerGuy1
-->
		<script type="text/javascript">
//<![CDATA[


document.title = "<% nvram_get("router_name"); %>" + upnp.titl;

function to_submit(F) {
	F.submit_button.value = "UPnP";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	update.stop();
	apply(F);
}

var data = new Array();

function parseForwards(upnpForwards) {
	// wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc
	data = [];
	for (var i = 0; i < upnpForwards.length; ++i) {
		if (upnpForwards[i] == '' || !upnpForwards[i].match(/^(\d+-\d+)>(.*?):(\d+-\d+),(.*?),(.*?),(.*)/)) continue;
		var e = {};
		e.index = i;
		e.wanPorts = RegExp.$1;
		e.lanIP = RegExp.$2;
		e.lanPorts = RegExp.$3;
		e.proto = RegExp.$4.toUpperCase();
		e.enabled = (RegExp.$5 == 'on');
		e.desc = RegExp.$6;

		if ((e.wanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.wanPorts = RegExp.$1;
			else e.wanPorts = RegExp.$1 + "&nbsp;-&nbsp;" + RegExp.$2;
		if ((e.lanPorts.match(/^(\d+)-(\d+)$/)) && (RegExp.$1 == RegExp.$2)) e.lanPorts = RegExp.$1;
			else e.lanPorts = RegExp.$1 + "&nbsp;-&nbsp;" + RegExp.$2;
		data.push(e);
	}
}

function setUPnPTable(forwards) {
	parseForwards(forwards);
	var table = document.getElementById("upnp_table");
	for(var i = table.rows.length - 1; i > 0 ; i--) {
		table.deleteRow(i);
	}
	if(data.length == 0) {
		var cell = table.insertRow(-1).insertCell(-1);
		cell.colSpan = 6;
		cell.align = "center";
		cell.innerHTML = "- " + share.none + " - ";
		return;
	}
	for(var i = 0; i < data.length; i++) {
		var row = table.insertRow(-1);
		row.style.height = "15px";
		row.className = (data[i].enabled ? '' : 'disabled');
		row.insertCell(-1).innerHTML = data[i].desc;
		row.insertCell(-1).innerHTML = data[i].wanPorts;
		row.insertCell(-1).innerHTML = data[i].lanPorts;
		row.insertCell(-1).innerHTML = data[i].lanIP;
		row.insertCell(-1).innerHTML = data[i].proto;
		var cell = row.insertCell(-1);
		cell.className = "bin";
//		cell.title = upnp.msg1;
		cell.title = "";
		cell.innerHTML = " ";
		eval("addEvent(cell, 'click', function() { deleteForward(" + i + ") })");
	}
}

function deleteForward(x) {
	if (x != 'all') {
		var e = data[x];
		if (!confirm(share.del + " " + e.desc + "? [" + e.wanPorts + "->" + e.lanPorts + " " + e.lanIP + " " + e.proto + "]")) return;
	}
	else {
		if (!confirm(upnp.msg2)) return;
	}
	var fupnp = document.getElementById("fupnp");
	fupnp.submit_button.value = "UPnP";
	fupnp.action.value = "Apply";
	fupnp.remove.value = (x == 'all' ? 'all' : e.index);
//	fupnp.delete_button.value = "Deleted";
	fupnp.delete_button.value = sbutton.deleted;
	fupnp.save_button.disabled = true;
	fupnp.delete_button.disabled = true;
	update.stop();
	fupnp.submit();
}

function setUPnP(val) {
	setElementsActive("upnpcas", "upnpmnp", val == "1");
	setElementActive("upnpmnp", val == "1");
}

var update;

addEvent(window, "load", function() {
	setUPnP("<% nvram_get("upnp_enable"); %>");
	setUPnPTable(new Array(<% tf_upnp(); %>));

	update = new StatusUpdate("UPnP.live.asp", <% nvram_get("refresh_time"); %>);
	update.onUpdate("upnp_forwards", function(u) {
		setUPnPTable(eval("new Array(" + u.upnp_forwards + ")"));
	});
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
		
//]]>
</script>
		<style type="text/css">
			tr.disabled td {text-decoration:line-through; color:#999;}
		</style>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Forward.asp","UPnP.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="fupnp" id="fupnp" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="action" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="remove" />
							<h2><% tran("upnp.h2"); %></h2>
							<fieldset>
								<legend><% tran("upnp.legend"); %></legend>
								<table class="table center" cellspacing="6" id="upnp_table" summary="UPnP table">
									<tr>
										<th width="40%"><% tran("share.descr"); %></th>
										<th width="15%"><% tran("share.from"); %>&nbsp;(WAN)</th>
										<th width="15%"><% tran("share.to"); %>&nbsp;(LAN)</th>
										<th width="20%"><% tran("share.ip"); %></th>
										<th width="10%"><% tran("share.proto"); %></th>
										<th><% tran("share.del"); %></th>
									</tr>
								</table><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input type=\"button\" name=\"delete_button\" value=\"" + sbutton.delall + "\" onclick=\"deleteForward('all')\">");
									//]]>
									</script>&nbsp;
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");
									//]]>
									</script>
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("upnp.legend2"); %></legend>
								<div class="setting">
									<div class="label"><% tran("upnp.serv"); %></div>
									<input class="spaceradio" type="radio" name="upnp_enable" value="1" <% nvram_checked("upnp_enable","1"); %> onclick="setUPnP(this.value)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="upnp_enable" value="0" <% nvram_checked("upnp_enable","0"); %> onclick="setUPnP(this.value)" /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("upnp.clear"); %></div>
									<input class="spaceradio" type="radio" name="upnpcas" value="1" <% nvram_checked("upnpcas","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="upnpcas" value="0" <% nvram_checked("upnpcas","0"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("upnp.url"); %></div>
									<input class="spaceradio" type="radio" name="upnpmnp" value="1" <% nvram_checked("upnpmnp","1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" name="upnpmnp" value="0" <% nvram_checked("upnpmnp","0"); %> /><% tran("share.disable"); %>
								</div>
							</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\">");
								//]]>
								</script>
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\">");
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
							<dt class="term"><% tran("upnp.legend"); %>:</dt>
							<dd class="definition"><% tran("hupnp.right2"); %></dd>
							<dt class="term"><% tran("upnp.serv"); %>:</dt>
							<dd class="definition"><% tran("hupnp.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HUPnP.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>
