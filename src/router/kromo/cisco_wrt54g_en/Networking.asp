<% do_pagehead("bmenu.networking"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save_networking";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function vlan_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_vlan";
	F.submit();
}
function bridge_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_bridge";
	F.submit();
}
function bridgeif_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_bridgeif";
	F.submit();
}
function bond_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_bond";
	F.submit();
}

function vlan_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_vlan";
	F.del_value.value=I;
	F.submit();
}
function bridge_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_bridge";
	F.del_value.value=I;
	F.submit();
}
function bridgeif_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_bridgeif";
	F.del_value.value=I;
	F.submit();
}
function bond_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_bond";
	F.del_value.value=I;
	F.submit();
}

function init() {

}

function setBRCTLTable() {
	var table = document.getElementById("Bridging_table");
	var val = arguments;
	cleanTable(table);
	for(var i = 0; i < val.length; i = i + 3) {
		
		var row = table.insertRow(-1);
		row.insertCell(-1).innerHTML = val[i];
		row.insertCell(-1).innerHTML = val[i + 1];
		row.insertCell(-1).innerHTML = val[i + 2];
	}
}

var update;

addEvent(window, "load", function() {
	setBRCTLTable(<% show_bridgetable(); %>);
	
	update = new StatusUpdate("Networking.live.asp", <% nvram_get("refresh_time"); %>);
	
	update.onUpdate("bridges_table", function(u) {
		eval('setBRCTLTable(' + u.bridges_table + ')');
	});

	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
		
		//]]>
		</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","Networking.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="vlan" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Networking" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="del_value" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />

							<h2><% tran("networking.h2"); %></h2>
							<fieldset>
							   <legend><% tran("networking.legend"); %></legend>
							   <% show_vlantagging(); %>
							</fieldset><br />

							<h2><% tran("networking.h22"); %></h2>
							<fieldset>
							   <legend><% tran("networking.legend2"); %></legend>
							   <% show_bridgenames(); %>
							</fieldset>
							<fieldset>
							   <legend><% tran("networking.legend3"); %></legend>
							   <% show_bridgeifnames(); %>
							</fieldset>
							<fieldset>
							   <legend><% tran("networking.legend4"); %></legend>
							   	<table class="table center" cellspacing="5" id="Bridging_table" summary="current bridging table">
								<tr>
								<th width="15%"><% tran("networking.brname"); %></th>
								<th width="15%"><% tran("networking.stp"); %></th>
								<th width="70%"><% tran("networking.iface"); %></th>
								</tr>
								</table><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload();\" />");
									//]]>
									</script>
								</div>
							</fieldset>
							<br />
							<% show_bondings(); %>
							<% portsetup(); %>
		
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HNetworking.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
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