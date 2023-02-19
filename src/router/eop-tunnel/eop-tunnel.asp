<% do_pagehead("eoip.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	F.submit_type.value = "save";
	F.change_action.value="gozila_cgi";
	apply(F);
}

function to_apply(F) {
	F.apply_button.value = sbutton.applied;
	F.submit_type.value = "save";
	F.change_action.value="gozila_cgi";
	applytake(F);
}

function gen_wg_key(F,keyindex) {
	F.keyindex.value = keyindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gen_wg_key";
	apply(F);
}

function gen_wg_client(F,keyindex,peerindex) {
	F.keyindex.value = keyindex;
	F.peerindex.value = peerindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gen_wg_client";
	apply(F);
}

function del_wg_client(F,keyindex,peerindex) {
	F.keyindex.value = keyindex;
	F.peerindex.value = peerindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_wg_client";
	apply(F);
}

function add_peer(F,keyindex) {
	F.keyindex.value = keyindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_peer";
	apply(F);
}

//egc
function import_tunnel(F,myid,keyindex) {
	//this is triggered by on change event of filepicker
	//alert("getwgfile: F:" + F.name + "; key-tun: " + keyindex + "; myid: " + myid);
	var wgfileid = document.getElementById(myid).files[0];
	//console.log("wgfileid.name: " + wgfileid.name + "; F: " + F + "; key-tun: " + keyindex + "; myid: " + myid);
	if (wgfileid.size > 3000) {
		alert("filesize of: " + wgfileid.size +"B exceeds limit, is this the right file?");
	} else {
		var wgreader = new FileReader();
		wgreader.readAsText(wgfileid, "UTF-8");
		wgreader.onload = function () {
			var wgfile = wgreader.result;
			//alert("wgconffile: " + wgfile);
			F.keyindex.value = keyindex;
			F.wg_conf_file.value = wgfile;
			F.change_action.value="gozila_cgi";
			F.submit_type.value = "import_tunnel";
			apply(F);
		}
	}
}

function del_peer(F, keyindex, peerindex) {
	F.keyindex.value = keyindex;
	F.peerindex.value = peerindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_peer";
	apply(F);
}

function add_tunnel(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_tunnel";
	apply(F);
}

function del_tunnel(F,tunnelindex) {
	F.keyindex.value = tunnelindex;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_tunnel";
	apply(F);
}

function gen_wg_psk(F,keyindex,peer) {
	F.keyindex.value = keyindex;
	F.peerindex.value = peer;
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "gen_wg_psk";
	apply(F);
}

function changespbr(F, index, value) {
	//alert(" F:" + F.name + "; tun: " + index + "; value: " + value);
	if (value == 1 || value == 2) {
		show_layer_ext(F, "idoet" + index + "_spbr", true);
	} else {
		show_layer_ext(F, "idoet" + index + "_spbr", false);
	}
}

function changedpbr(F, index, value) {
	//alert(" F:" + F.name + "; tun: " + index + "; value: " + value);
	if (value == 1 || value == 2) {
		show_layer_ext(F, "idoet" + index + "_dpbr", true);
	} else {
		show_layer_ext(F, "idoet" + index + "_dpbr", false);
	}
}

function changedns46(F, index) {
	//alert(" F:" + F.name + "; tun: " + index + "; value: " + oetdnspbr);
	if ( F.checked == true) {
		//console.log(" F:" + F.name + " Checked");
		show_layer_ext(F, "idoet" + index + "_dns46", true);
	} else {
		//console.log(" F:" + F.name + " Not Checked");
		show_layer_ext(F, "idoet" + index + "_dns46", false);
	}
}

function changeproto(F, index, value, brvalue) {
	if (value == 1) {
		show_layer_ext(F, "idmtik" + index, true);
	} else {
		show_layer_ext(F, "idmtik" + index, false);
	}

	if (value == 2) {
		show_layer_ext(F, "idwireguard" + index, true);
		show_layer_ext(F, "idl2support" + index, false);
		show_layer_ext(F, "idwginput" + index, true);
		show_layer_ext(F, "idlocalip" + index, false);
		show_layer_ext(F, "idbridged" + index, false);
	} else {
		show_layer_ext(F, "idwireguard" + index, false);
		show_layer_ext(F, "idl2support" + index, true);
		show_layer_ext(F, "idwginput" + index, false);
		show_layer_ext(F, "idlocalip" + index, true);
		if (brvalue == 1) {
			show_layer_ext(F, "idbridged" + index, false);
		} else {
			show_layer_ext(F, "idbridged" + index, true);
		}
	}
}

function failover_show(F, index, value) {
	if (value == 1) {
		show_layer_ext(F, "idoet" + index + "_tunnelstate", true);
		show_layer_ext(F, "idoet" + index + "_wdog", true);
		show_layer_ext(F, "idoet" + index + "_wdog2", false);
	} else {
		show_layer_ext(F, "idoet" + index + "_tunnelstate", false);
		show_layer_ext(F, "idoet" + index + "_wdog", false);
		show_layer_ext(F, "idoet" + index + "_wdog2", true);
	}
}

function wdog_show(F, index, value) {
	if (value == 1) {
		show_layer_ext(F, "idoet" + index + "_failgrp", false);
		show_layer_ext(F, "idoet" + index + "_wdog", true);
	} else {
		show_layer_ext(F, "idoet" + index + "_failgrp", true);
		show_layer_ext(F, "idoet" + index + "_wdog", false);
	}
}
var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	update = new StatusbarUpdate();
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
					<% do_menu("index.asp","eop-tunnel.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="eop" action="apply.cgi" method="post" spellcheck="false">
						//<form name="eop" action="apply.cgi" method="post" enctype="multipart/form-data">
							<input type="hidden" name="submit_button" value="eop-tunnel" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="keyindex" />
							<input type="hidden" name="peerindex" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="wg_conf_file" />

							<h2><% tran("eoip.legend"); %></h2>
								<% show_eop_tunnels(); %>
								<div id="footer" class="submitFooter">
									<script type="text/javascript">
									//<![CDATA[
									submitFooterButton(1,1,0,0,1,0);
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
								<dt class="term"><% tran("eoip.wireguard_lanac"); %></dt>
								<dd class="definition"><% tran("hstatus_vpn.right4"); %></dd>
							</dl><br />
							<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HEoIP.asp');"><% tran("share.more"); %></a>
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
