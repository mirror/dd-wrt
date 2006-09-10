<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless Security</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + wpa.titl;

function SelMode(varname,num,F)	{
	F.submit_button.value = "WL_WPATable";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "security";
	F.security_varname.value = varname;
	F.submit();
}

function to_submit(F) {
	if (valid_value(F)) {
		F.submit_button.value = "WL_WPATable";
		F.change_action.value = "gozila_cgi";
		F.submit_type.value = "save";
		F.save_button.value = sbutton.saving;

		F.action.value = "Apply";
       	apply(F);
	}
}

function valid_value(F) {
	return true;
	//if (wl0_security_mode)
	//if(F.security_mode.value == "disabled")
	//	return true;

	//if(!valid_wpa_psk(F) || !valid_wep(F) || !valid_radius(F)) {
	//	return false;
	//} else  {
	//	return true;
	//}
}

function valid_radius(F) {
	if(F.security_mode.value == "radius" || F.security_mode.value == "wpa" || F.security_mode.value == "wpa2" || F.security_mode.value == "wpa wpa2"){
		if(F.wl_radius_key.value == "") {
			alert(errmsg.err38);
			F.wl_radius_key.focus();
			return false;
		}
	}

	return true;
}

function valid_wpa_psk(F) {
	if(F.security_mode.value == "psk" || F.security_mode.value == "psk2" || F.security_mode.value == "psk psk2"){
		if(F.wl_wpa_psk.value.length == 64){
			if(!isxdigit(F.wl_wpa_psk, F.wl_wpa_psk.value)) {
				return false;
			}
		} else if(F.wl_wpa_psk.value.length >=8 && F.wl_wpa_psk.value.length <= 63 ){
			if(!isascii(F.wl_wpa_psk,F.wl_wpa_psk.value)) {
				return false;
			}
		} else{
			alert(errmsg.err39);
			return false;
		}
	}

	return true;
}

function valid_wep(F) {
	if(F.security_mode.value == "psk" || F.security_mode.value == "wpa" || F.security_mode.value == "psk2" || F.security_mode.value == "wpa2" || F.security_mode.value == "psk psk2" || F.security_mode.value == "wpa wpa2")
		return true;

	if (ValidateKey(F.wl_key1, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value,1) == false)
		return false;

  	if (ValidateKey(F.wl_key2, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value,2) == false)
		return false;

	if (ValidateKey(F.wl_key3, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value,3) == false)
		return false;

	if (ValidateKey(F.wl_key4, F.wl_wep_bit.options[F.wl_wep_bit.selectedIndex].value,4) == false)
		return false;

	for (var i=1; i <= 4; i++) {
		if(F.wl_key[i-1].checked){
			aaa = eval("F.wl_key"+i).value;
			//if(aaa == "" && F.security_mode.value == "wep"){
			if(aaa == ""){
				alert(errmsg.err40 + i);
				return false;
			}
			break;
		}
	}

    return true;
}

function ValidateKey(key, bit, index) {
	if(bit == 64) {
		switch(key.value.length){
			case 0:
				break;
			case 10:
				if(!isxdigit(key,key.value)) {
					return false;
				}
				break;
			default:
				alert(errmsg.err41 + key.value);
				return false;
		}
	} else {
		switch(key.value.length){
			case 0:
				break;
			case 26:
				if(!isxdigit(key,key.value)) {
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
function keyMode(F) {
F.submit_button.value = "WL_WPATable";
F.change_action.value = "gozila_cgi";
F.submit_type.value = "keysize";
F.submit();
}
function generateKey(F,PREFIX) {

	F.submit_button.value = "WL_WPATable";
	F.change_action.value = "gozila_cgi";
	F.security_varname.value = PREFIX;
	if(F.wl_wep_bit.value == 64) {
		F.submit_type.value = "key_64";
	} else {
		F.submit_type.value = "key_128";
	}

	F.submit();
}
function generateKey64(F,PREFIX) {

	F.submit_button.value = "WL_WPATable";
	F.change_action.value = "gozila_cgi";
	F.security_varname.value = PREFIX;
	F.submit_type.value = "key_64";
	F.submit();
}

function enable_idpeap()
{
show_layer_ext(this, 'idtls', false)
show_layer_ext(this, 'idpeap', true)
}
function enable_idtls()
{
show_layer_ext(this, 'idtls', true)
show_layer_ext(this, 'idpeap', false)
}
function generateKey128(F,PREFIX) {

	F.submit_button.value = "WL_WPATable";
	F.change_action.value = "gozila_cgi";
	F.security_varname.value = PREFIX;
	F.submit_type.value = "key_128";

	F.submit();
}

addEvent(window, "load", function() {
	var F = document.forms[0];
	if(F.security_mode && F.wl_wep_bit)
		if(F.security_mode.value == "wep" || F.security_mode.value == "radius") {
			keyMode(F.wl_wep_bit.value, F);
		}
show_layer_ext(document.wpa.ath0_8021xtype, 'idpeap', <% nvram_else_match("ath0_8021xtype", "peap", "1", "0"); %> == 1);
show_layer_ext(document.wpa.ath0_8021xtype, 'idtls', <% nvram_else_match("ath0_8021xtype", "tls", "1", "0"); %> == 1);

});

		</script>
	</head>


   <body class="gui">
   	<% showad(); %>
      <div id="wrapper">
         <div id="content">
            <div id="header">
               <div id="logo">
                  <h1><% show_control(); %></h1>
               </div>
			   <% do_menu("Wireless_Basic.asp","WL_WPATable.asp"); %>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="wpa" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" />
                  	<input type="hidden" name="submit_type" />
                  	<input type="hidden" name="change_action" />
                  	<input type="hidden" name="action" />
                  	<input type="hidden" name="security_varname" />
                  	<input type="hidden" name="security_mode_last" />
                  	<input type="hidden" name="wl_wep_last" />
                  	<input type="hidden" name="filter_mac_value" />
                  	<h2><% tran("wpa.h2"); %></h2>
                  		<% show_security(); %><br />
					
					<div class="submitFooter">
                  		<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\"/><input type=\"reset\" value=\"" + sbutton.cancel + "\"/>")</script>
                  	</div>
				</form>
					</div>
				</div>
				<div id="helpContainer">
               <div id="help">
                  <div><h2><% tran("share.help"); %></h2></div>
                  <dl>
                     <dt class="term"><% tran("wpa.secmode"); %>:</dt>
                     <dd class="definition"><% tran("hwpa.right2"); %></dd>
                  </dl><br />
                  <a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWPA.asp')"><% tran("share.more"); %></a>
               </div>
            </div>
			<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>
