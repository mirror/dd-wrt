<% do_pagehead("wl_basic.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

var wl0_channel = '<% nvg("wl0_channel"); %>';
var wl0_nctrlsb = '<% nvg("wl0_nctrlsb"); %>';
var wl0_nbw = '<% nvg("wl0_nbw"); %>';
var wl0_phytype = '<% nvg("wl0_phytype"); %>';
var wl0_40m_disable = '<% nvg("wl0_40m_disable"); %>';
var wl1_channel = '<% nvg("wl1_channel"); %>';
var wl1_nctrlsb = '<% nvg("wl1_nctrlsb"); %>';
var wl1_nbw = '<% nvg("wl1_nbw"); %>';
var wl1_phytype = '<% nvg("wl1_phytype"); %>';
var wl1_40m_disable = '<% nvg("wl1_40m_disable"); %>';
var wl2_channel = '<% nvg("wl2_channel"); %>';
var wl2_nctrlsb = '<% nvg("wl2_nctrlsb"); %>';
var wl2_nbw = '<% nvg("wl2_nbw"); %>';
var wl2_phytype = '<% nvg("wl2_phytype"); %>';
var wl2_40m_disable = '<% nvg("wl2_40m_disable"); %>';

function create_wchannel0_auto(F) {
	F.wl0_wchannel.length = 1;
	F.wl0_wchannel[0] = new Option(share.auto);
	F.wl0_wchannel[0].value = "0";
}

function create_wchannel1_auto(F) {
	F.wl1_wchannel.length = 1;
	F.wl1_wchannel[0] = new Option(share.auto);
	F.wl1_wchannel[0].value = "0";
}

function create_wchannel2_auto(F) {
	F.wl2_wchannel.length = 1;
	F.wl2_wchannel[0] = new Option(share.auto);
	F.wl2_wchannel[0].value = "0";
}

function create_wchannel0(F) {
	var max_channel = '14';
	var wch;

	if (wl0_nctrlsb == "lower") {
		wch = parseInt(F.wl0_channel.value) + 2;
	} else {
		wch = parseInt(F.wl0_channel.value) - 2;
	}
	F.wl0_wchannel.length = parseInt(max_channel) - 4;

	for (ch = 3; ch <= (parseInt(max_channel) - 2); ch++) {
		F.wl0_wchannel[ch - 3] = new Option(ch);
		F.wl0_wchannel[ch - 3].value = ch;
	}
	if (wch < 3 || wch > max_channel - 2 || wch == "0")
		F.wl0_wchannel[0].selected = true;
	else
		F.wl0_wchannel[wch - 3].selected = true;
}

function create_wchannel1(F) {
	var max_channel = '14';
	var wch;

	if (wl1_nctrlsb == "lower") {
		wch = parseInt(F.wl1_channel.value) + 2;
	} else {
		wch = parseInt(F.wl1_channel.value) - 2;
	}
	F.wl1_wchannel.length = parseInt(max_channel) - 4;

	for (ch = 3; ch <= (parseInt(max_channel) - 2); ch++) {
		F.wl1_wchannel[ch - 3] = new Option(ch);
		F.wl1_wchannel[ch - 3].value = ch;
	}
	if (wch < 3 || wch > max_channel - 2 || wch == "0")
		F.wl1_wchannel[0].selected = true;
	else
		F.wl1_wchannel[wch - 3].selected = true;
}

function create_wchannel2(F) {
	var max_channel = '14';
	var wch;

	if (wl2_nctrlsb == "lower") {
		wch = parseInt(F.wl2_channel.value) + 2;
	} else {
		wch = parseInt(F.wl2_channel.value) - 2;
	}
	F.wl2_wchannel.length = parseInt(max_channel) - 4;

	for (ch = 3; ch <= (parseInt(max_channel) - 2); ch++) {
		F.wl2_wchannel[ch - 3] = new Option(ch);
		F.wl2_wchannel[ch - 3].value = ch;
	}
	if (wch < 3 || wch > max_channel - 2 || wch == "0")
		F.wl2_wchannel[0].selected = true;
	else
		F.wl2_wchannel[wch - 3].selected = true;
}

function InitBW0(num, F) {
	if (wl0_channel == "0") {
		if (F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);

		choose_enable(F.wl0_schannel);
		if (F.wl0_wchannel)
			create_wchannel0_auto(F)
	} else {
			SelBW0(num, F);
	}
}

function InitBW1(num, F) {
	if (wl1_channel == "0") {
		if (F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);

		choose_enable(F.wl1_schannel);
		if (F.wl1_wchannel)
			create_wchannel1_auto(F)
	} else {
			SelBW1(num, F);
	}
}

function InitBW2(num, F) {
	if (wl2_channel == "0") {
		if (F.wl2_wchannel)
			choose_enable(F.wl2_wchannel);

		choose_enable(F.wl2_schannel);
		if (F.wl2_wchannel) create_wchannel2_auto(F)
	} else {
			SelBW2(num, F);
	}
}

function SelBW0(num, F) {
	if (num == 0) { // Auto
		if (F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);

		choose_enable(F.wl0_channel);
		if (F.wl0_wchannel)
			create_wchannel0_auto(F);
	} else if (num == 10 || num == 20) {
		if (F.wl0_wchannel)
			choose_disable(F.wl0_wchannel);

		choose_enable(F.wl0_schannel);
		if (F.wl0_wchannel)
			create_wchannel0(F);
	} else {
		if (F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);

		choose_enable(F.wl0_schannel);
		if (F.wl0_wchannel)
			create_wchannel0(F);
	}
}

function SelBW1(num, F) {
	if (num == 0) { // Auto
		if (F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);

		choose_enable(F.wl1_channel);
		if (F.wl1_wchannel)
			create_wchannel1_auto(F);
	} else if (num == 10 || num == 20) {
		if (F.wl1_wchannel)
			choose_disable(F.wl1_wchannel);

		choose_enable(F.wl1_schannel);
		if (F.wl1_wchannel)
			create_wchannel1(F);
	} else {
		if (F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);

		choose_enable(F.wl1_schannel);
		if (F.wl1_wchannel)
			create_wchannel1(F);
	}
}

function SelBW2(num, F) {
	if (num == 0) { // Auto
		if (F.wl2_wchannel)
			choose_enable(F.wl2_wchannel);

		choose_enable(F.wl2_channel);
		if (F.wl2_wchannel)
			create_wchannel2_auto(F);
	} else if (num == 10 || num == 20) {
		if (F.wl2_wchannel)
			choose_disable(F.wl2_wchannel);

		choose_enable(F.wl2_schannel);
		if (F.wl2_wchannel)
			create_wchannel2(F);
	} else {
		if (F.wl2_wchannel)
			choose_enable(F.wl2_wchannel);

		choose_enable(F.wl2_schannel);
		if (F.wl2_wchannel)
			create_wchannel2(F);
	}
}

function vifs_add_submit(F, I) {
	F.iface.value = I;
	F.submit_type.value = "add_vifs";
	F.submit();
}

function vifs_remove_submit(F, I, VAP) {
	F.iface.value = I;
	F.vap.value = VAP;
	F.submit_type.value = "remove_vifs";
	F.submit();
}

function copy_submit(F, I) {
	F.iface.value = I;
	F.submit_type.value = "copy_if";
	F.submit();
}

function paste_submit(F, I) {
	F.iface.value = I;
	F.submit_type.value = "paste_if";
	F.submit();
}

function toggle_layer(checkbox, label) {
	if (checkbox.checked) {
		show_layer_ext(this, label, true);
	} else {
		show_layer_ext(this, label, false);
	}
}

function submitcheck(F) {
	if (F.wl0_ssid)
		if (F.wl0_ssid.value == "") {
			alert(errmsg.err50);
			F.wl0_ssid.focus();
			return false;
		}
	if (F.wl1_ssid)
		if (F.wl1_ssid.value == "") {
			alert(errmsg.err50);
			F.wl1_ssid.focus();
			return false;
		}
	if (F.wl2_ssid)
		if (F.wl2_ssid.value == "") {
			alert(errmsg.err50);
			F.wl2_ssid.focus();
			return false;
		}
	if (F.wl0_nbw) {
		if (F.wl0_nbw.value == 0) { // Auto
			F.wl0_channel.value = 0;
		} else if (F.wl0_nbw.value == 10) { // 10MHz
			F.wl0_nctrlsb.value = "none";
			F.wl0_nbw.value = 10;
		} else if (F.wl0_nbw.value == 5) { // 5MHz
			F.wl0_nctrlsb.value = "none";
			F.wl0_nbw.value = 5;
		} else if (F.wl0_nbw.value == 20) { // 20MHz
			F.wl0_nctrlsb.value = "none";
			F.wl0_nbw.value = 20;
		} else if (F.wl0_nbw.value == 80) { // 80MHz
			F.wl0_nbw.value = 80;
		} else if (F.wl0_nbw.value == 160) { // 160MHz
			F.wl0_nbw.value = 160;
		} else if (F.wl0_nbw.value == "80+80") { // 80+80 MHz
			F.wl0_nbw.value = "80+80";
		} else { // 40MHz
			F.wl0_nbw.value = 40;
		}
	}
	if (F.wl1_nbw) {
		if (F.wl1_nbw.value == 0) { // Auto
			F.wl1_channel.value = 0;
		} else if (F.wl1_nbw.value == 5) { // 5MHz
			F.wl1_nctrlsb.value = "none";
			F.wl1_nbw.value = 5;
		} else if (F.wl1_nbw.value == 10) { // 10MHz
			F.wl1_nctrlsb.value = "none";
			F.wl1_nbw.value = 10;
		} else if (F.wl1_nbw.value == 20) { // 20MHz
			F.wl1_nctrlsb.value = "none";
			F.wl1_nbw.value = 20;
		} else if (F.wl1_nbw.value == 80) { // 80MHz
			F.wl1_nbw.value = 80;
		} else if (F.wl1_nbw.value == 160) { // 160MHz
			F.wl1_nbw.value = 160;
		} else if (F.wl1_nbw.value == "80+80") { // 80+80 MHz
			F.wl1_nbw.value = "80+80";
		} else { // 40MHz
			F.wl1_nbw.value = 40;
		}
	}
	if (F.wl2_nbw) {
		if (F.wl2_nbw.value == 0) { // Auto
			F.wl2_channel.value = 0;
		} else if (F.wl2_nbw.value == 5) { // 5MHz
			F.wl2_nctrlsb.value = "none";
			F.wl2_nbw.value = 5;
		} else if (F.wl2_nbw.value == 10) { // 10MHz
			F.wl2_nctrlsb.value = "none";
			F.wl2_nbw.value = 10;
		} else if (F.wl2_nbw.value == 20) { // 20MHz
			F.wl2_nctrlsb.value = "none";
			F.wl2_nbw.value = 20;
		} else if (F.wl2_nbw.value == 80) { // 80MHz
			F.wl2_nbw.value = 80;
		} else if (F.wl2_nbw.value == 160) { // 160MHz
			F.wl2_nbw.value = 160;
		} else if (F.wl2_nbw.value == "80+80") { // 80+80 MHz
			F.wl2_nbw.value = "80+80";
		} else { // 40MHz
			F.wl2_nbw.value = 40;
		}
	}
	<% gen_timer_compute(); %>
	return true;
}

function to_submit(F) {
	if (submitcheck(F)) {
		F.submit_type.value = "save";
		F.save_button.value = sbutton.saving;
		apply(F);
	}
}

function to_apply(F) {
	if (submitcheck(F)) {
		F.submit_type.value = "save";
		F.apply_button.value = sbutton.applied;
		applytake(F);
	}
}

function initWlTimer(radio_on_time, radio) {
	var color_red = '#CD0000';
	var color_green = '#228B22';

	for (var i = 0; i < radio_on_time.length; i++) {
		if (radio_on_time.charAt(i) == 1) {
			bgcolor = color_green;
			val = 1;
		} else {
			bgcolor = color_red;
			val = 0;
		}
		if (ie4 || op) {
			eval("document.all.td" + radio + "_" + i + ".style.backgroundColor = '" + bgcolor + "'");
			eval("document.all.td" + radio + "_" + i + ".value = '" + val + "'");
		}
		if (ns4) {
			eval("document.td" + radio + "_" + i + ".backgroundColor = '" + bgcolor + "'");
			eval("document.td" + radio + "_" + i + ".value = '" + val + "'");
		}
		if (ns6) {
			eval("document.getElementById('td" + radio + "_" + i + "').style.backgroundColor = '" + bgcolor + "'");
			eval("document.getElementById('td" + radio + "_" + i + "').value = '" + val + "'");
		}
	}
}

function setWlTimer(id, state, radio) {
	var color_red = '#CD0000';
	var color_green = '#228B22';

	if (id == 'all') {
		if (state) {
			bgcolor = color_green;
			val = 1;
		} else {
			bgcolor = color_red;
			val = 0;
		}

		for (var i = 0; i < 24; i++) {
			if (ie4 || op) {
				eval("document.all.td" + radio + "_" + i + ".style.backgroundColor = '" + bgcolor + "'");
				eval("document.all.td" + radio + "_" + i + ".value = '" + val + "'");
			}
			if (ns4) {
				eval("document.td" + radio + "_" + i + ".backgroundColor = '" + bgcolor + "'");
				eval("document.td" + radio + "_" + i + ".value = '" + val + "'");
			}
			if (ns6) {
				eval("document.getElementById('td" + radio + "_" + i + "').style.backgroundColor = '" + bgcolor + "'");
				eval("document.getElementById('td" + radio + "_" + i + "').value = '" + val + "'");
			}
		}
	} else {
		if (ie4 || op) {
			if (eval("document.all." + id + ".value") == '1') {
				eval("document.all." + id + ".style.backgroundColor = '" + color_red + "'");
				eval("document.all." + id + ".value = '0'");
			} else {
				eval("document.all." + id + ".style.backgroundColor = '" + color_green + "'");
				eval("document.all." + id + ".value = '1'");
			}
		}
		if (ns4) {
			if (eval("document." + id + ".value") == '1') {
				eval("document." + id + ".backgroundColor = '" + color_red + "'");
				eval("document." + id + ".value = '0'");
			} else {
				eval("document." + id + ".backgroundColor = '" + color_green + "'");
				eval("document." + id + ".value = '1'");
			}
		}
		if (ns6) {
			if (eval("document.getElementById('" + id + "').value") == '1') {
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_red + "'");
				eval("document.getElementById('" + id + "').value = '0'");
			} else {
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_green + "'");
				eval("document.getElementById('" + id + "').value = '1'");
			}
		}
	}
}

function computeWlTimer(radio) {
	var radio_on_time = '';

	for (var i = 0; i < 24; i++) {
		if (ie4 || op) {
			radio_on_time += eval("document.all.td" + radio + "_" + i + ".value");
		}
		if (ns4) {
			radio_on_time += eval("document.td" + radio + "_" + i + ".value");
		}
		if (ns6) {
			radio_on_time += eval("document.getElementById('td" + radio + "_" + i + "').value");
		}
	}
	return radio_on_time;
}

function setRadioTable(radio) {
	var table = document.getElementById("radio" + radio + "_table");
	cleanTable(table);

	var row1 = table.insertRow(-1);
	var row2 = table.insertRow(-1);
	row2.style.cursor = "pointer";

	for (var i = 0; i < 24; i++) {
		var cell_label = row1.insertCell(-1);
		cell_label.innerHTML = i;

		var cell_timer = row2.insertCell(-1);
		cell_timer.style.width = "4%";
		cell_timer.id = "td" + radio + "_" + i;
		cell_timer.title = i + "h - " + eval(i + 1) + "h";
		cell_timer.innerHTML = "&nbsp;";
		cell_timer.onclick = function () {
			setWlTimer(this.id, true, radio);
		};
	}
}

var update;

addEvent(window, "load", function () {
	stickControl(<% nvg("sticky_footer"); %>);

	<% gen_init_timer(); %>
	<% ifdef("HAVE_ATH9K", "initChannelProperties();"); %>
	var wl0_mode = "<% nvg("wl0_mode"); %>";
	if (wl0_mode == "ap" || wl0_mode == "infra") {
		if (wl0_phytype == 'n' || wl0_phytype == 'h' || wl0_phytype == 'v' || wl0_phytype == 's')
			InitBW0('<% nvg("wl0_nbw"); %>', document.wireless);
	}
	var wl1_mode = "<% nvg("wl1_mode"); %>";
	if (wl1_mode == "ap" || wl1_mode == "infra") {
		if (wl1_phytype == 'n' || wl1_phytype == 'h' || wl1_phytype == 'v' || wl1_phytype == 's')
			InitBW1('<% nvg("wl1_nbw"); %>', document.wireless);
	}
	var wl2_mode = "<% nvg("wl2_mode"); %>";
	if (wl2_mode == "ap" || wl2_mode == "infra") {
		if (wl2_phytype == 'n' || wl2_phytype == 'h' || wl2_phytype == 'v' || wl2_phytype == 's')
			InitBW2('<% nvg("wl2_nbw"); %>', document.wireless);
	}

	var thisTitle = idx.h22;
	document.getElementsByName('wireless')[0].setAttribute("title", thisTitle);

	update = new StatusbarUpdate();
	update.start();
});

addEvent(window, "unload", function () {
	update.stop();
});

function setChannelProperties(channels) {
	index = channels.selectedIndex;
	properties = eval('(' + channels[index].getAttribute('rel') + ')');

	// get the interface label
	var iflabel = channels.name.substr(0, channels.name.length - 8);
	// check for HT40 upper / lower channel
	var nctrlsb = document.forms[0][iflabel + '_nctrlsb'];
	var F = document.forms[0][iflabel + '_channelbw'];
	if (!F) {
		F = document.forms[0][iflabel + '_nbw'];
	}
	if (nctrlsb) {
		var selected = 0;
		if (nctrlsb.length) {
			selected = nctrlsb.options[nctrlsb.selectedIndex].value;
		}
		// remove unneeded the entries
		while (nctrlsb.length) {
			nctrlsb.remove(0);
		}
		// auto channel
		if (channels[index].value == '0') {
			nctrlsb.options[nctrlsb.length] = new Option('Auto', wl_basic.ch_pos_auto);
		} else {
			if (F.value == 40 || F.value == 2040) {
				// HT40 minus
				if (properties.luu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_lwr, 'luu');
				}
				if (properties.ull == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_upr, 'ull');
				}
			}
			if (F.value == "80+80" || F.value == 80) {
				// HT40 minus
				if (properties.lul == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_ll, 'lul');
				}
				if (properties.luu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_lu, 'luu');
				}
				if (properties.ull == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_ul, 'ull');
				}
				if (properties.ulu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_uu, 'ulu');
				}
			}
			if (F.value == 160) {
				// HT40 minus
				if (properties.lll == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_lll, 'lll');
				}
				if (properties.llu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_llu, 'llu');
				}
				if (properties.lul == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_lul, 'lul');
				}
				if (properties.luu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_luu, 'luu');
				}
				if (properties.ull == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_ull, 'ull');
				}
				if (properties.ulu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_ulu, 'ulu');
				}
				if (properties.uul == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_uul, 'uul');
				}
				if (properties.uuu == 1) {
					nctrlsb.options[nctrlsb.length] = new Option(wl_basic.ch_pos_uuu, 'uuu');
				}
			}
		}
		// adjust selected index
		nctrlsb.selectedIndex = 0;
		for (i = 0; i < nctrlsb.length; i++) {
			if (nctrlsb.options[i].value == selected) {
				nctrlsb.selectedIndex = i;
			}
		}
	}
}

function show_airtime_policy(F, prefix, idname, vifs) {
	var elem = F.elements[prefix + "_at_policy"];
	var ifs = vifs.split(" ");
	if (elem.value == 0) {
		show_layer_ext(F, idname + "_idairtimelimit", false);
		show_layer_ext(F, idname + "_idairtimeweight", false);
		if (vifs != '') {
			for (i = 0; i < ifs.length; i++) {
				show_layer_ext(F, ifs[i] + "_idairtimelimit", false);
				show_layer_ext(F, ifs[i] + "_idairtimeweight", false);
			}
		}
	}
	if (elem.value == 1) {
		show_layer_ext(F, idname + "_idairtimelimit", false);
		show_layer_ext(F, idname + "_idairtimeweight", true);
		if (vifs != '') {
			for (i = 0; i < ifs.length; i++) {
				show_layer_ext(F, ifs[i] + "_idairtimelimit", false);
				show_layer_ext(F, ifs[i] + "_idairtimeweight", true);
			}
		}
	}
	if (elem.value == 2) {
		show_layer_ext(F, idname + "_idairtimelimit", true);
		show_layer_ext(F, idname + "_idairtimeweight", true);
		if (vifs != '') {
			for (i = 0; i < ifs.length; i++) {
				show_layer_ext(F, ifs[i] + "_idairtimelimit", true);
				show_layer_ext(F, ifs[i] + "_idairtimeweight", true);
			}
		}
	}
}

function initChannelProperties() {
	for (j = 0; j < document.forms[0].elements.length; j++) {
		element = document.forms[0].elements[j];
		if (element.name) {
			if (element.name.substr(element.name.length - 8, 8) == '_channel' && element.getAttribute('rel') == 'mac80211') {
				setChannelProperties(element);
			}
		}
	}
}
	//]]>
	</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Wireless_Basic.asp","Wireless_Basic.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form title="" name="wireless" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Wireless_Basic" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" value="gozila_cgi" />
							<input type="hidden" name="submit_type" value="save" />
							<% gen_timer_fields(); %>
							<input type="hidden" name="wl0_nctrlsb" />
							<input type="hidden" name="wl1_nctrlsb" /> 
							<input type="hidden" name="wl2_nctrlsb" /> 
							<input type="hidden" name="iface" />
							<input type="hidden" name="vap" />
							<% show_wireless(); %>
							<% show_wireless_advanced(); %>
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
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dd class="definition"><% tran("hwl_basic.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWireless.asp')"><% tran("share.more"); %></a>
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
