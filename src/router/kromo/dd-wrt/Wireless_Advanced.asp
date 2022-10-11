<%% do_pagehead("wl_adv.titl"); %%>
	<script type="text/javascript">
	//<![CDATA[

var wl_net_mode = '<%% nvg("%s_net_mode"); %%>';
var wl_phytype = '<%% nvg("%s_phytype"); %%>';

function initWlTimer(radio_on_time) {
	var color_red = '#cd0000';
	var color_green = '#228b22';
	
	for(var i = 0; i < radio_on_time.length; i++){
		if(radio_on_time.charAt(i) == 1){
			bgcolor = color_green;
			val = 1;
		} else {
			bgcolor = color_red;
			val = 0;
		}
		if(ie4 || op){
			eval("document.all.td_" + i + ".style.backgroundColor = '" + bgcolor + "'");
			eval("document.all.td_" + i + ".value = '" + val + "'");
		}
		if(ns4) {
			eval("document.td_" + i + ".backgroundColor = '" + bgcolor + "'");
			eval("document.td_" + i + ".value = '" + val + "'");
		}
		if(ns6) {
			eval("document.getElementById('td_" + i + "').style.backgroundColor = '" + bgcolor + "'");
			eval("document.getElementById('td_" + i + "').value = '" + val + "'");
		}
	}
}

function setWlTimer(id, state) {
	var color_red = '#cd0000';
	var color_green = '#228b22';

	if(id=='all') {
		if(state) {
			bgcolor = color_green;
			val = 1;
		} else {
			bgcolor = color_red;
			val = 0;
		}

		for(var i = 0; i < 24; i++) {
			if(ie4 || op) {
				eval("document.all.td_" + i + ".style.backgroundColor = '" + bgcolor + "'");
				eval("document.all.td_" + i + ".value = '" + val + "'");
			}
			if(ns4) {
				eval("document.td_" + i + ".backgroundColor = '" + bgcolor + "'");
				eval("document.td_" + i + ".value = '" + val + "'");
			}
			if(ns6) {
				eval("document.getElementById('td_" + i + "').style.backgroundColor = '" + bgcolor + "'");
				eval("document.getElementById('td_" + i + "').value = '" + val + "'");
			}
		}
	} else {
		if(ie4 || op) {
			if(eval("document.all." + id + ".value") == '1') {
				eval("document.all." + id + ".style.backgroundColor = '" + color_red + "'");
				eval("document.all." + id + ".value = '0'");
			} else {
				eval("document.all." + id + ".style.backgroundColor = '" + color_green + "'");
				eval("document.all." + id + ".value = '1'");
			}
		}
		if(ns4) {
			if(eval("document." + id + ".value") == '1') {
				eval("document." + id + ".backgroundColor = '" + color_red + "'");
				eval("document." + id + ".value = '0'");
			} else {
				eval("document." + id + ".backgroundColor = '" + color_green + "'");
				eval("document." + id + ".value = '1'");
			}
		}
		if(ns6) {
			if(eval("document.getElementById('" + id + "').value") == '1') {
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_red + "'");
				eval("document.getElementById('" + id + "').value = '0'");
			} else {
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_green + "'");
				eval("document.getElementById('" + id + "').value = '1'");
			}
		}
	}
}

function computeWlTimer() {
	var radio_on_time='';
	
	for(var i = 0; i < 24; i++) {
		if(ie4 || op) {
			radio_on_time += eval("document.all.td_" + i + ".value");
		}
		if(ns4) {
			radio_on_time += eval("document.td_" + i + ".value");
		}
		if(ns6) {
			radio_on_time += eval("document.getElementById('td_" + i + "').value");
		}
	}

	return radio_on_time;
}

function setRadioTable() {
	var table = document.getElementById("radio_table");
	cleanTable(table);
	
	var row1 = table.insertRow(-1);
	var row2 = table.insertRow(-1);
	row2.style.cursor = "pointer";

	for(var i = 0; i < 24; i++) {

		var cell_label = row1.insertCell(-1);
		cell_label.innerHTML = i;
				
		var cell_timer = row2.insertCell(-1);
		cell_timer.style.width = "4%%";
		cell_timer.id = "td_" + i;
		cell_timer.title = i + "h - " + eval(i+1) + "h";
		cell_timer.innerHTML = "&nbsp;";
		cell_timer.onclick = function(){setWlTimer(this.id, true);};
	}
}

function create_nrate(num,F) {
	var bw20_1 = new Array("6.5 (7.2)", "13 (14.4)", "19.5 (21.7)", "26 (28.9)", "39 (43.3)", "52 (57.8)", "58.5 (65)", "65 (72.2)");
	var bw20_2 = new Array("13 (14.4)", "26 (28.9)", "39 (43.3)", "52 (57.8)", "78 (86.6)", "104 (115.6)", "117 (130)", "130 (144)");
	var bw20_3 = new Array("19.5 (21.7)", "39 (43.3)", "58.5 (65)", "78 (86.7)", "117 (130.7)", "156 (173.3)", "175.5 (195)", "195 (216.7)");
	var bw40_1 = new Array("13.5 (15)", "27 (30)", "40.5 (45)", "54 (60)", "81 (90)", "108 (120)", "121.5 (135)", "135 (150)");
	var bw40_2 = new Array("27 (30)", "54 (60)", "81 (90)", "108 (120)", "162 (180)", "216 (240)", "243 (270)", "270 (300)");
	var bw40_3 = new Array("40.5 (45)", "81 (90)", "121.5 (135)", "162 (180)", "243 (270)", "324 (360)", "364.5 (405)", "405 (450)");
	var index = '<%% nvg("%s_nmcsidx"); %%>';

	F.%s_nmcsidx[0] = new Option(share.auto);
	F.%s_nmcsidx[0].value = "-1";

	if(num == 0 || num == 20) {
		for(i=0;i<8;i++) {
			F.%s_nmcsidx[i+1] = new Option(i+" - "+bw20_1[i]+" "+wl_adv.mbps);
			F.%s_nmcsidx[i+1].value = i;
		}
		if(wl_phytype == "n" || wl_phytype == "h") {
			for(i=8;i<16;i++) {
				F.%s_nmcsidx[i+1] = new Option(i+" - "+bw20_2[i-8]+" "+wl_adv.mbps);
				F.%s_nmcsidx[i+1].value = i;
			}
		}
		if(wl_phytype == "h") {
			for(i=16;i<24;i++) {
				F.%s_nmcsidx[i+1] = new Option(i+" - "+bw20_3[i-16]+" "+wl_adv.mbps);
				F.%s_nmcsidx[i+1].value = i;
			}
		}
	} else {
		for(i=0;i<8;i++) {
			F.%s_nmcsidx[i+1] = new Option(i+" - "+bw40_1[i]+" "+wl_adv.mbps);
			F.%s_nmcsidx[i+1].value = i;
		}
		if(wl_phytype == "n" || wl_phytype == "h") {
			for(i=8;i<16;i++) {
				F.%s_nmcsidx[i+1] = new Option(i+" - "+bw40_2[i-8]+" "+wl_adv.mbps);
				F.%s_nmcsidx[i+1].value = i;
			}
		}
		if(wl_phytype == "h") {
			for(i=16;i<24;i++) {
				F.%s_nmcsidx[i+1] = new Option(i+" - "+bw40_3[i-16]+" "+wl_adv.mbps);
				F.%s_nmcsidx[i+1].value = i;
			}
		}
	}

	if(index == "-2" && (wl_net_mode == "b-only" || wl_net_mode == "g-only" || wl_net_mode == "bg-mixed" || wl_net_mode == "a-only")) {
		F.%s_nmcsidx[0].selected = true;
		choose_disable(F.%s_nmcsidx);
	}
	else
		F.%s_nmcsidx[parseInt(index)+1].selected = true;
}

function to_submit(F) {
	F.%s_nmode_protection.value = F.%s_gmode_protection.value;
	F.save_button.value = sbutton.saving;
	F.radio%d_on_time.value = computeWlTimer();
	apply(F);
}

function to_apply(F) {
	F.%s_nmode_protection.value = F.%s_gmode_protection.value;
	F.apply_button.value = sbutton.applied;
	F.radio%d_on_time.value = computeWlTimer();
	applytake(F);
}

function setWMM(val) {
	setElementsActive("%s_wme_no_ack", "wl_wme_txp_vo4", val == "on");
}

var update;

addEvent(window, "load", function() {
	stickControl(<%% nvg("sticky_footer"); %%>);
	setRadioTable();
	setWMM("<%% nvg("%s_wme"); %%>");
	show_layer_ext(document.wireless.%s_wme, 'idwl_wme', <%% nvem("%s_wme", "on", "1", "0"); %%> == 1);
	show_layer_ext(document.wireless.%s_txpwrusr, 'idtxpwr', <%% nvem("%s_txpwrusr", "0", "1", "0"); %%> == 1);
	show_layer_ext(document.wireless.radio%d_timer_enable, 'radio', <%% nvem("radio%d_timer_enable", "1", "1", "0"); %%> == 1);
	initWlTimer('<%% nvg("radio%d_on_time"); %%>');
	show_layer_ext(document.wireless.%s_nmcsidx, 'id%s_nmcsidx', <%% nvem("%s_phytype", "n", "1", "0"); %%> == 1 || <%% nvem("%s_phytype", "s", "1", "0"); %%> == 1 || <%% nvram_else_match("%s_phytype", "h", "1", "0"); %%> == 1);
	setElementActive( "document.wireless.wl_rate", !(wl_net_mode=="n-only") );

	if(wl_phytype == "s" || wl_phytype == "n" || wl_phytype== "h")
		create_nrate('<%% nvg("%s_nbw"); %%>',document.wireless);

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
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><%% show_control(); %%></h1></div>
					<%% do_menu("Wireless_Basic.asp","Wireless_Advanced-%s.asp"); %%>
				</div>
				<div id="main">
					<div id="contents">
						<form id="wireless" name="wireless" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Wireless_Advanced-%s" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="interface" value="%s" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" value="save" />
							<input type="hidden" name="commit" value="1" />

							<input type="hidden" name="radio%d_on_time" />
							<input type="hidden" name="%s_nmode_protection" />
							<h2><%% tran("wl_adv.h2"); %%></h2>
							<fieldset>
								<legend><%% tran("wl_adv.legend"); %%></legend>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label"); %%></div>
									<input class="spaceradio" type="radio" name="%s_auth" value="0" <%% nvc("%s_auth", "0"); %%> /><%% tran("share.auto"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_auth" value="1" <%% nvc("%s_auth", "1"); %%> /><%% tran("share.share_key"); %%>
									<span class="default"><script type="text/javascript">
									//<![CDATA[
									document.write("(" + share.deflt + ": " + share.auto + ")");
									//]]>
									</script></span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label2"); %%></div>
									<select name="%s_rateset">
										<option value="12" <%% nvs("%s_rateset", "12"); %%>>1-2 <%% tran("wl_adv.mbps"); %%></option>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"default\" <%% nvsjs("%s_rateset", "default"); %%>>" + share.deflt + "</option>");
										document.write("<option value=\"all\" <%% nvsjs("%s_rateset", "all"); %%>>" + share.all + "</option>");
										//]]>
										</script>
									</select>
									<span class="default"><script type="text/javascript">
									//<![CDATA[
									document.write("(" + share.deflt + ": " + share.deflt + ")");
									//]]>
									</script></span>
								</div>
								<div id="id%s_nmcsidx" class="setting">
									<div class="label">MIMO - <%% tran("wl_adv.label3"); %%></div>
									<select name="%s_nmcsidx">
									</select>
									<span class="default"><script type="text/javascript">
									//<![CDATA[
									document.write("(" + share.deflt + ": " + share.auto + ")");
									//]]>
									</script></span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label3"); %%></div>
									<select name="%s_rate">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <%% nvsjs("%s_rate", "0"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
										<option value="1000000" <%% nvs("%s_rate", "1000000"); %%>>1 <%% tran("wl_adv.mbps"); %%></option>
										<option value="2000000" <%% nvs("%s_rate", "2000000"); %%>>2 <%% tran("wl_adv.mbps"); %%></option>
										<option value="5500000" <%% nvs("%s_rate", "5500000"); %%>>5.5 <%% tran("wl_adv.mbps"); %%></option>
										<option value="6000000" <%% nvs("%s_rate", "6000000"); %%>>6 <%% tran("wl_adv.mbps"); %%></option>
										<option value="9000000" <%% nvs("%s_rate", "9000000"); %%>>9 <%% tran("wl_adv.mbps"); %%></option>
										<option value="11000000" <%% nvs("%s_rate", "11000000"); %%>>11 <%% tran("wl_adv.mbps"); %%></option>
										<option value="12000000" <%% nvs("%s_rate", "12000000"); %%>>12 <%% tran("wl_adv.mbps"); %%></option>
										<option value="18000000" <%% nvs("%s_rate", "18000000"); %%>>18 <%% tran("wl_adv.mbps"); %%></option>
										<option value="24000000" <%% nvs("%s_rate", "24000000"); %%>>24 <%% tran("wl_adv.mbps"); %%></option>
										<option value="36000000" <%% nvs("%s_rate", "36000000"); %%>>36 <%% tran("wl_adv.mbps"); %%></option>
										<option value="48000000" <%% nvs("%s_rate", "48000000"); %%>>48 <%% tran("wl_adv.mbps"); %%></option>
										<option value="54000000" <%% nvs("%s_rate", "54000000"); %%>>54 <%% tran("wl_adv.mbps"); %%></option>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label4"); %%></div>
									<input class="spaceradio" type="radio" name="%s_gmode_protection" value="auto" <%% nvc("%s_gmode_protection", "auto"); %%> /><%% tran("share.auto"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_gmode_protection" value="off" <%% nvc("%s_gmode_protection", "off"); %%> /><%% tran("share.disable"); %%>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label5"); %%></div>
									<input class="spaceradio" type="radio" name="%s_frameburst" value="on" <%% nvc("%s_frameburst", "on"); %%> /><%% tran("share.enable"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_frameburst" value="off" <%% nvc("%s_frameburst", "off"); %%> /><%% tran("share.disable"); %%>
								</div><br />
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label6"); %%></div>
									<input class="num" name="%s_bcn" size="6" maxlength="5" onblur="valid_range(this,10,65535,wl_adv.label6)" value="<%% nvram_selget("%s_bcn"); %%>" />&nbsp;
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 100&nbsp;ms, " + share.range + ": 10 - 65535)");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label7"); %%></div>
									<input class="num" name="%s_dtim" size="6" maxlength="3" onblur="valid_range(this,1,255,wl_adv.label7)" value="<%% nvram_selget("%s_dtim"); %%>" />&nbsp;
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": ");
										//]]>
										</script>
										<%% get_wl_value("default_dtim"); %%>
										<script type="text/javascript">
										//<![CDATA[
										document.write(", " + share.range + ": 1 - 255)");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label8"); %%></div>
									<input class="num" name="%s_frag" size="6" maxlength="4" onblur="valid_range(this,256,2346,wl_adv.label8)" value="<%% nvram_selget("%s_frag"); %%>" />&nbsp;
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 2346, " + share.range + ": 256 - 2346)");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label9"); %%></div>
									<input class="num" name="%s_rts" size="6" maxlength="4" onblur="valid_range(this,0,2347,wl_adv.label9)" value="<%% nvram_selget("%s_rts"); %%>" />&nbsp;
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 2347, " + share.range + ": 0 - 2347)");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label10"); %%></div>
									<input class="num" name="%s_maxassoc" size="6" maxlength="4" onblur="valid_range(this,1,256,wl_adv.label10)" value="<%% nvram_selget("%s_maxassoc"); %%>" />&nbsp;
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 128, " + share.range + ": 1 - 256)");
										//]]>
										</script>
									</span>
							 	</div><br />
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label11"); %%></div>
									<input class="spaceradio" type="radio" name="%s_ap_isolate" value="1" <%% nvc("%s_ap_isolate", "1"); %%> /><%% tran("share.enable"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_ap_isolate" value="0" <%% nvc("%s_ap_isolate", "0"); %%> /><%% tran("share.disable"); %%>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.disable + ")");
										//]]>
										</script>
									</span>
								</div>
								<%% ifdef("80211AC", "<!--"); %%>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label12"); %%></div>
									<select name="%s_txant">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <%% nvsjs("%s_txant", "0"); %%>>" + share.right + "</option>");
										document.write("<option value=\"1\" <%% nvsjs("%s_txant", "1"); %%>>" + share.left + "</option>");
										document.write("<option value=\"3\" <%% nvsjs("%s_txant", "3"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label13"); %%></div>
									<select name="%s_antdiv">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <%% nvsjs("%s_antdiv", "0"); %%>>" + share.right + "</option>");
										document.write("<option value=\"1\" <%% nvsjs("%s_antdiv", "1"); %%>>" + share.left + "</option>");
										document.write("<option value=\"3\" <%% nvsjs("%s_antdiv", "3"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<%% ifdef("80211AC", "-->"); %%>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label14"); %%></div>
									<select name="%s_plcphdr">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"long\" <%% nvsjs("%s_plcphdr", "long"); %%>>" + wl_adv.lng + "</option>");
										document.write("<option value=\"short\" <%% nvsjs("%s_plcphdr", "short"); %%>>" + wl_adv.shrt + "</option>");
										document.write("<option value=\"auto\" <%% nvsjs("%s_plcphdr", "auto"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + wl_adv.lng + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label20"); %%></div>
									<select name="%s_shortslot">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"long\" <%% nvsjs("%s_shortslot", "long"); %%>>" + wl_adv.lng + "</option>");
										document.write("<option value=\"short\" <%% nvsjs("%s_shortslot", "short"); %%>>" + wl_adv.shrt + "</option>");
										document.write("<option value=\"auto\" <%% nvsjs("%s_shortslot", "auto"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting">
									<div class="label"><%% tran("wl_basic.TXpower"); %%></div>
									<input class="spaceradio" type="radio" name="%s_txpwrusr" value="1" <%% nvc("%s_txpwrusr", "1"); %%> onclick="show_layer_ext(this, 'idtxpwr', false)" /><%% tran("share.auto"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_txpwrusr" value="0" <%% nvc("%s_txpwrusr", "0"); %%> onclick="show_layer_ext(this, 'idtxpwr', true)"/><%% tran("share.manual"); %%>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.auto + ")");
										//]]>
										</script>
									</span>
								</div>
								<div class="setting" id="idtxpwr">
									<div class="label">&nbsp;</div>
									<input class="num" name="%s_txpwr" size="6" maxlength="3" onblur="valid_range(this,1,1000,wl_basic.TXpower)" value="<%% nvram_selget("%s_txpwr"); %%>" />
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.range + ": 1 - 1000mW)");
										//]]>
										</script>
									</span>
								</div>
								<%% ifndef("AFTERBURNER_%s", "<!--"); %%>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label16"); %%></div>
									<select name="%s_afterburner">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"off\" <%% nvsjs("%s_afterburner", "off"); %%>>" + share.disable + "</option>");
										document.write("<option value=\"on\" <%% nvsjs("%s_afterburner", "on"); %%>>" + share.enable + "</option>");
										document.write("<option value=\"auto\" <%% nvsjs("%s_afterburner", "auto"); %%>>" + share.auto + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.disable + ")");
										//]]>
										</script>
									</span>
								</div>
								<%% ifndef("AFTERBURNER_%s", "-->"); %%>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label22"); %%></div>
									<select name="%s_btc_mode">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <%% nvsjs("%s_btc_mode", "0"); %%>>" + share.disable + "</option>");
										document.write("<option value=\"1\" <%% nvsjs("%s_btc_mode", "1"); %%>>" + share.enable + "</option>");
										document.write("<option value=\"2\" <%% nvsjs("%s_btc_mode", "2"); %%>>" + share.preempt + "</option>");
										//]]>
										</script>
									</select>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.disable + ")");
										//]]>
										</script>
									</span>
								</div>
							</fieldset><br />
							<fieldset>
								<legend><%% tran("wl_basic.legend2"); %%></legend>
								<div class="setting">
									<div class="label"><%% tran("wl_basic.radiotimer"); %%></div>
									<input class="spaceradio" type="radio" value="1" name="radio%d_timer_enable" <%% nvc("radio%d_timer_enable", "1"); %%> onclick="show_layer_ext(this, 'radio', true)" /><%% tran("share.enable"); %%>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="radio%d_timer_enable" <%% nvc("radio%d_timer_enable", "0"); %%> onclick="show_layer_ext(this, 'radio', false)" /><%% tran("share.disable"); %%>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.disable + ")");
										//]]>
										</script>
									</span>
								</div>
								<div id="radio">
									<table id="radio_table"></table>
									<br />
									<div class="center">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.allways_on + "\" onclick=\"setWlTimer('all',true);\" />");
										document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.allways_off + "\" onclick=\"setWlTimer('all',false);\" />");
										//]]>
										</script>
									</div>
								</div>
							</fieldset><br/>
							<fieldset>
								<legend><%% tran("wl_adv.legend2"); %%></legend>
								<div class="setting">
									<div class="label"><%% tran("wl_adv.label18"); %%></div>
									<input class="spaceradio" type="radio" name="%s_wme" value="on" <%% nvc("%s_wme", "on"); %%>  onclick="show_layer_ext(this, 'idwl_wme', true);setWMM(this.value)" /><%% tran("share.enable"); %%>&nbsp;
									<input class="spaceradio" type="radio" name="%s_wme" value="off" <%% nvc("%s_wme", "off"); %%>  onclick="show_layer_ext(this, 'idwl_wme', false);setWMM(this.value)" /><%% tran("share.disable"); %%>
									<span class="default">
										<script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": " + share.enable + ")");
										//]]>
										</script>
									</span>
								</div>
								<div id="idwl_wme">
									<div class="setting">
										<div class="label"><%% tran("wl_adv.label19"); %%></div>
										<input class="spaceradio" type="radio" name="%s_wme_no_ack" value="on" <%% nvc("%s_wme_no_ack", "on"); %%> /><%% tran("share.enable"); %%>&nbsp;
										<input class="spaceradio" type="radio" name="%s_wme_no_ack" value="off" <%% nvc("%s_wme_no_ack", "off"); %%> /><%% tran("share.disable"); %%>
										<span class="default">
											<script type="text/javascript">
											//<![CDATA[
											document.write("(" + share.deflt + ": " + share.disable + ")");
											//]]>
											</script>
										</span>
									</div>
									<table class="table" cellspacing="5" summary="edca ap parameters">
										<thead>
										<tr>
											<th colspan="7"><%% tran("wl_adv.table1"); %%></th>
										</tr>
									</thead>
									<tbody>
										<tr>
											<td>&nbsp;</td>
											<td class="center"><%% tran("wl_adv.col1"); %%></td>
											<td class="center"><%% tran("wl_adv.col2"); %%></td>
											<td class="center"><%% tran("wl_adv.col3"); %%></td>
											<td class="center"><%% tran("wl_adv.col4"); %%></td>
											<td class="center"><%% tran("wl_adv.col5"); %%></td>
											<td class="center"><%% tran("wl_adv.col6"); %%></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row1"); %%><input type="hidden" name="%s_wme_ap_bk" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_bk0" value="<%% nvram_list("%s_wme_ap_bk", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_bk1" value="<%% nvram_list("%s_wme_ap_bk", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_bk2" value="<%% nvram_list("%s_wme_ap_bk", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_bk3" value="<%% nvram_list("%s_wme_ap_bk", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_bk4" value="<%% nvram_list("%s_wme_ap_bk", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_ap_bk5" value="<%% nvram_list("%s_wme_ap_bk", 5); %%>" /><input type="checkbox" name="_%s_wme_ap_bk5" <%% wme_match_op("%s_wme_ap_bk", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_ap_bk5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row2"); %%><input type="hidden" name="%s_wme_ap_be" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_be0" value="<%% nvram_list("%s_wme_ap_be", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_be1" value="<%% nvram_list("%s_wme_ap_be", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_be2" value="<%% nvram_list("%s_wme_ap_be", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_be3" value="<%% nvram_list("%s_wme_ap_be", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_be4" value="<%% nvram_list("%s_wme_ap_be", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_ap_be5" value="<%% nvram_list("%s_wme_ap_be", 5); %%>" /><input type="checkbox" name="_%s_wme_ap_be5" <%% wme_match_op("%s_wme_ap_be", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_ap_be5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row3"); %%><input type="hidden" name="%s_wme_ap_vi" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vi0" value="<%% nvram_list("%s_wme_ap_vi", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vi1" value="<%% nvram_list("%s_wme_ap_vi", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vi2" value="<%% nvram_list("%s_wme_ap_vi", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vi3" value="<%% nvram_list("%s_wme_ap_vi", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vi4" value="<%% nvram_list("%s_wme_ap_vi", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_ap_vi5" value="<%% nvram_list("%s_wme_ap_vi", 5); %%>" /><input type="checkbox" name="_%s_wme_ap_vi5" <%% wme_match_op("%s_wme_ap_vi", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_ap_vi5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row4"); %%><input type="hidden" name="%s_wme_ap_vo" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vo0" value="<%% nvram_list("%s_wme_ap_vo", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vo1" value="<%% nvram_list("%s_wme_ap_vo", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vo2" value="<%% nvram_list("%s_wme_ap_vo", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vo3" value="<%% nvram_list("%s_wme_ap_vo", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_ap_vo4" value="<%% nvram_list("%s_wme_ap_vo", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_ap_vo5" value="<%% nvram_list("%s_wme_ap_vo", 5); %%>" /><input type="checkbox" name="_%s_wme_ap_vo5" <%% wme_match_op("%s_wme_ap_vo", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_ap_vo5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
									</tbody>
									</table>
									<table cellspacing="5" summary="edca sta parameters">
										<thead>
										<tr>
											<th colspan="7"><%% tran("wl_adv.table2"); %%></th>
										</tr>
									</thead>
									<tbody>
										<tr>
											<td>&nbsp;</td>
											<td class="center"><%% tran("wl_adv.col1"); %%></td>
											<td class="center"><%% tran("wl_adv.col2"); %%></td>
											<td class="center"><%% tran("wl_adv.col3"); %%></td>
											<td class="center"><%% tran("wl_adv.col4"); %%></td>
											<td class="center"><%% tran("wl_adv.col5"); %%></td>
											<td class="center"><%% tran("wl_adv.col6"); %%></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row1"); %%><input type="hidden" name="%s_wme_sta_bk" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_bk0" value="<%% nvram_list("%s_wme_sta_bk", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_bk1" value="<%% nvram_list("%s_wme_sta_bk", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_bk2" value="<%% nvram_list("%s_wme_sta_bk", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_bk3" value="<%% nvram_list("%s_wme_sta_bk", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_bk4" value="<%% nvram_list("%s_wme_sta_bk", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_sta_bk5" value="<%% nvram_list("%s_wme_sta_bk", 5); %%>" /><input type="checkbox" name="_%s_wme_sta_bk5" <%% wme_match_op("%s_wme_sta_bk", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_sta_bk5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row2"); %%><input type="hidden" name="%s_wme_sta_be" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_be0" value="<%% nvram_list("%s_wme_sta_be", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_be1" value="<%% nvram_list("%s_wme_sta_be", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_be2" value="<%% nvram_list("%s_wme_sta_be", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_be3" value="<%% nvram_list("%s_wme_sta_be", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_be4" value="<%% nvram_list("%s_wme_sta_be", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_sta_be5" value="<%% nvram_list("%s_wme_sta_be", 5); %%>" /><input type="checkbox" name="_%s_wme_sta_be5" <%% wme_match_op("%s_wme_sta_be", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_sta_be5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row3"); %%><input type="hidden" name="%s_wme_sta_vi" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vi0" value="<%% nvram_list("%s_wme_sta_vi", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vi1" value="<%% nvram_list("%s_wme_sta_vi", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vi2" value="<%% nvram_list("%s_wme_sta_vi", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vi3" value="<%% nvram_list("%s_wme_sta_vi", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vi4" value="<%% nvram_list("%s_wme_sta_vi", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_sta_vi5" value="<%% nvram_list("%s_wme_sta_vi", 5); %%>" /><input type="checkbox" name="_%s_wme_sta_vi5" <%% wme_match_op("%s_wme_sta_vi", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_sta_vi5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row4"); %%><input type="hidden" name="%s_wme_sta_vo" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vo0" value="<%% nvram_list("%s_wme_sta_vo", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col1)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vo1" value="<%% nvram_list("%s_wme_sta_vo", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,0,32767,wl_adv.col2)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vo2" value="<%% nvram_list("%s_wme_sta_vo", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.col3)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vo3" value="<%% nvram_list("%s_wme_sta_vo", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col4)" /></td>
											<td class="center"><input class="num" name="%s_wme_sta_vo4" value="<%% nvram_list("%s_wme_sta_vo", 4); %%>" size="5" maxlength="6" onblur="valid_range(this,0,65504,wl_adv.col5)" /></td>
											<td class="center"><input type="hidden" name="%s_wme_sta_vo5" value="<%% nvram_list("%s_wme_sta_vo", 5); %%>" /><input type="checkbox" name="_%s_wme_sta_vo5" <%% wme_match_op("%s_wme_sta_vo", "on", "checked='checked'"); %%> onchange="this.form.%s_wme_sta_vo5.value = (this.checked ? 'on' : 'off');" /></td>
										</tr>
									</tbody>
									</table>
									<table class="table" cellspacing="5" summary="WMM Tx retry limits">
										<thead>
										<tr>
											<th colspan="7"><%% tran("wl_adv.table3"); %%></th>
										</tr>
									</thead>
									<tbody>
										<tr>
											<td>&nbsp;</td>
											<td class="center"><%% tran("wl_adv.txpcol1"); %%></td>
											<td class="center"><%% tran("wl_adv.txpcol2"); %%></td>
											<td class="center"><%% tran("wl_adv.txpcol3"); %%></td>
											<td class="center"><%% tran("wl_adv.txpcol4"); %%></td>
											<td class="center"><%% tran("wl_adv.txpcol5"); %%></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row1"); %%><input type="hidden" name="%s_wme_txp_bk" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_bk0" value="<%% nvram_list("%s_wme_txp_bk", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol1)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_bk1" value="<%% nvram_list("%s_wme_txp_bk", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol2)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_bk2" value="<%% nvram_list("%s_wme_txp_bk", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol3)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_bk3" value="<%% nvram_list("%s_wme_txp_bk", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol4)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_bk4" value="<%% nvram_list("%s_wme_txp_bk", 4); %%>" size="5" maxlength="6" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row2"); %%><input type="hidden" name="%s_wme_txp_be" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_be0" value="<%% nvram_list("%s_wme_txp_be", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol1)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_be1" value="<%% nvram_list("%s_wme_txp_be", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol2)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_be2" value="<%% nvram_list("%s_wme_txp_be", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol3)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_be3" value="<%% nvram_list("%s_wme_txp_be", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol4)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_be4" value="<%% nvram_list("%s_wme_txp_be", 4); %%>" size="5" maxlength="6" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row3"); %%><input type="hidden" name="%s_wme_txp_vi" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vi0" value="<%% nvram_list("%s_wme_txp_vi", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol1)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vi1" value="<%% nvram_list("%s_wme_txp_vi", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol2)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vi2" value="<%% nvram_list("%s_wme_txp_vi", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol3)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vi3" value="<%% nvram_list("%s_wme_txp_vi", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol4)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vi4" value="<%% nvram_list("%s_wme_txp_vi", 4); %%>" size="5" maxlength="6" /></td>
										</tr>
										<tr>
											<td><%% tran("wl_adv.row4"); %%><input type="hidden" name="%s_wme_txp_vo" value="5" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vo0" value="<%% nvram_list("%s_wme_txp_vo", 0); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol1)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vo1" value="<%% nvram_list("%s_wme_txp_vo", 1); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol2)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vo2" value="<%% nvram_list("%s_wme_txp_vo", 2); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol3)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vo3" value="<%% nvram_list("%s_wme_txp_vo", 3); %%>" size="5" maxlength="6" onblur="valid_range(this,1,15,wl_adv.txpcol4)" /></td>
											<td class="center"><input class="num" name="%s_wme_txp_vo4" value="<%% nvram_list("%s_wme_txp_vo", 4); %%>" size="5" maxlength="6" /></td>
										</tr>
									</tbody>
									</table>
								</div>
							</fieldset><br />
							<div id="footer" class="submitFooter">
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
						<h2><%% tran("share.help"); %%></h2>
						<dl>
							<dt class="term"><%% tran("wl_adv.label"); %%>:</dt>
							<dd class="definition"><%% tran("hwl_adv.right2"); %%></dd>
							<dt class="term"><%% tran("wl_basic.legend2"); %%>:</dt>
							<dd class="definition"><%% tran("hwl_basic.right6"); %%></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<%% ifdef("EXTHELP","Ext"); %%>('HWirelessAdvanced.asp');"><%% tran("share.more"); %%></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><%% tran("share.firmware"); %%>:&nbsp;
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><%% get_firmware_version(); %%></a>");
					//]]>
					</script>
				</div>
				<div class="info"><%% tran("share.time"); %%>:  <span id="uptime"><%% get_uptime(); %%></span></div>
				<div class="info">WAN<span id="ipinfo"><%% show_wanipinfo(); %%></span></div>
				</div>
			</div>
		</div>
	</body>
</html>
