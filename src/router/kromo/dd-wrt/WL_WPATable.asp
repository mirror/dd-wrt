<% do_pagehead("wpa.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function SelMode(prefix, varname, num, F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "security";
	F.security_varname.value = varname;
	F.ifname.value = prefix;
	F.submit();
}

function keyMode(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "keysize";
	F.submit();
}

function generateKey(F, PREFIX) {
	F.change_action.value = "gozila_cgi";
	F.security_varname.value = PREFIX;
	F.submit_type.value = "wep_key_generate";
	F.submit();
}

var wpa_psk_error = 0;
var wep_error = 0;

function check_form(F) {
	var security_mode = 'disabled';
	var element = null;

	// parse through security modes
	for (var i = 0; i < F.elements.length; i++) {
		element = F.elements[i];
		if (element.name) {
			if (element.name.substr(element.name.length - 14, 14) == "_security_mode") {
				if (element.options[element.selectedIndex].value.substr(0, 3) == "psk" ||
					element.options[element.selectedIndex].value.substr(0, 3) == "wpa") {
					var iface = element.name.substr(0, element.name.length - 14).replace("X", ".");
					var wpa_psk_input = document.getElementById(iface + "_wpa_psk");
					if (wpa_psk_input) {
						result = valid_wpa_psk(wpa_psk_input, false);
						wpa_psk_error = 0;
						if (result == false) return result;
					}
				} else {
					if (element.options[element.selectedIndex].value.substr(0, 3) == "wep") {
						var iface = element.name.substr(0, element.name.length - 14).replace("X", ".");
						for (var j = 1; j <= 4; j++) {
							if (F.elements[iface + '_key'][j - 1].checked) {
								aaa = eval(F.elements[iface + "_key" + j]).value;
								if (aaa == "") {
									alert(errmsg.err40 + j);
									return false;
								} else {
									result = valid_wep(F.elements[iface + "_key" + j]);
									wep_error = 0;
									if (result == false) return result;
								}
								//break;
							} else {
								result = valid_wep(F.elements[iface + "_key" + j]);
								wep_error = 0;
								if (result == false) return result;
							}
						}
					}
				}
			}
		}
	}
	if (wpa_psk_error) return false;
	return true;
}

function to_submit(F) {
	if (!check_form(F)) return false;
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	if (!check_form(F)) return false;
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	applytake(F);
}

function valid_radius(F) {
	if (F.security_mode.value == "radius" || F.security_mode.value == "wpa" || F.security_mode.value == "wpa2" || F.security_mode.value == "wpa wpa2") {
		if (F.wl_radius_key.value == "") {
			alert(errmsg.err38);
			F.wl_radius_key.focus();
			return false;
		}
	}

	return true;
}

function valid_wpa_psk(F, blur) {
	if (F.nodeName == 'INPUT') {
		var value = F.value;
		if (F.value.length == 64) {
			if (!isxdigit(F, F.value)) {
				wpa_psk_error = 1;
				setTimeout("wpa_psk_error=0", 1000);
				return false;
			}
		} else {
			if (F.value.length >= 8 && F.value.length <= 63) {
				if (!isascii(F, F.value)) {
					F.value = value;
					wpa_psk_error = 1;
					setTimeout("wpa_psk_error=0", 1000);
					return false;
				}
			} else {
				if (blur) {
					if (!isascii(F, F.value)) {
						F.value = value;
						wpa_psk_error = 1;
						setTimeout("wpa_psk_error=0", 1000);
						return false;
					}
				} else {
					if (!wpa_psk_error) {
						alert(errmsg.err39);
						wpa_psk_error = 1;
						setTimeout("wpa_psk_error=0", 1000);
					}
					return false;
				}
			}
		}
	} else {
		if (F.security_mode.value == "psk" || F.security_mode.value == "psk2" || F.security_mode.value == "psk psk2") {
			if (F.wl_wpa_psk.value.length == 64) {
				if (!isxdigit(F.wl_wpa_psk, F.wl_wpa_psk.value)) {
					return false;
				}
			} else {
				if (F.wl_wpa_psk.value.length >= 8 && F.wl_wpa_psk.value.length <= 63) {
					if (!isascii(F.wl_wpa_psk, F.wl_wpa_psk.value)) {
						return false;
					}
				} else {
					alert(errmsg.err39);
					return false;
				}
			}
		}
	}

	wpa_psk_error = 0;
	return true;
}

function valid_wep(F, blur) {
	if (F.nodeName == 'INPUT') {
		var iface = F.name.substr(0, F.name.length - 5).replace(".", "X");
		if (document.forms[0].elements[iface + '_security_mode'].value != "wep")
			return true;

		if (wep_error) {
			wep_error = 0;
			return false;
		}

		var value = F.value;
		var bitselect = document.forms[0].elements[iface.replace("X", ".") + '_wep_bit'];
		if (ValidateKey(F, bitselect.options[bitselect.selectedIndex].value, 1) == false) {
			wep_error = 1;
			setTimeout("wep_error=0", 1000);
			return false;
		}
	} else {
		if (F.security_mode.value == "psk" || F.security_mode.value == "wpa" || F.security_mode.value == "psk2" || F.security_mode.value == "wpa2" || F.security_mode.value == "psk psk2" || F.security_mode.value == "wpa wpa2")
			return true;

		if (ValidateKey(F.wl_key1, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value, 1) == false)
			return false;

		if (ValidateKey(F.wl_key2, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value, 2) == false)
			return false;

		if (ValidateKey(F.wl_key3, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value, 3) == false)
			return false;

		if (ValidateKey(F.wl_key4, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value, 4) == false)
			return false;

		for (var i = 1; i <= 4; i++) {
			if (F.wl_key[i - 1].checked) {
				aaa = eval("F.wl_key" + i).value;
				if (aaa == "") {
					alert(errmsg.err40 + i);
					return false;
				}
				break;
			}
		}
	}

	wep_error = 0;
	return true;
}

function ValidateKey(key, bit, index) {
	var value = key.value;
	if (!isxdigit(key, key.value)) {
		key.value = value;
		return false;
	}
	if (bit == 64) {
		switch (key.value.length) {
		case 0:
			break;
		case 10:
			break;
		default:
			alert(errmsg.err41 + key.value);
			return false;
		}
	} else {
		switch (key.value.length) {
		case 0:
			break;
		case 26:
			if (!isxdigit(key, key.value)) {
				return false;
			}
			break;
		default:
			alert(errmsg.err41 + key.value);
			return false;
		}
	}

	return true;
}

function enable_idttls(ifname) {
	show_layer_ext(this, 'idttls' + ifname, true);
	show_layer_ext(this, 'idtls' + ifname, false);
	show_layer_ext(this, 'idpeap' + ifname, false);
	show_layer_ext(this, 'idleap' + ifname, false);
}

function enable_idpeap(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false);
	show_layer_ext(this, 'idtls' + ifname, false);
	show_layer_ext(this, 'idpeap' + ifname, true);
	show_layer_ext(this, 'idleap' + ifname, false);
}

function enable_idleap(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false);
	show_layer_ext(this, 'idtls' + ifname, false);
	show_layer_ext(this, 'idpeap' + ifname, false);
	show_layer_ext(this, 'idleap' + ifname, true);
}

function enable_idtls(ifname) {
	show_layer_ext(this, 'idttls' + ifname, false);
	show_layer_ext(this, 'idtls' + ifname, true);
	show_layer_ext(this, 'idpeap' + ifname, false);
	show_layer_ext(this, 'idleap' + ifname, false);
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);

	<% init_80211x_layers(); %>
	var F = document.forms[0];
	if (F.security_mode && F.wl_wep_bit) {
		if (F.security_mode.value == "wep" || F.security_mode.value == "radius") {
			keyMode(F.wl_wep_bit.value, F);
		}
	}

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
			<div id="logo"><h1><% show_control(); %></h1></div>
			<% do_menu("Wireless_Basic.asp","WL_WPATable.asp"); %>
		</div>
		<div id="main">
			<div id="contents">
			<form name="wpa" action="apply.cgi" method="post" autocomplete="new-password">
				<input type="hidden" name="submit_button" value="WL_WPATable" />
				<input type="hidden" name="action" value="Apply" />
				<input type="hidden" name="change_action" value="gozila_cgi" />
				<input type="hidden" name="submit_type" value="save" />

				<input type="hidden" name="security_varname" />
				<input type="hidden" name="ifname" />
				<input type="hidden" name="security_mode_last" />
				<input type="hidden" name="wl_wep_last" />
				<input type="hidden" name="filter_mac_value" />

				<% show_security(); %>

				<div id="footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(1);
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
				 		<dt class="term"><% tran("wpa.secmode"); %>:</dt>
				 		<dd class="definition"><% tran("hwpa.right2"); %></dd>
			   </dl><br />
			   <a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWPA.asp')"><% tran("share.more"); %></a>
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
