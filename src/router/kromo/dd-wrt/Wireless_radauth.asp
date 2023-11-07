<% do_pagehead("radius.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function to_submit(F) {
	if (F._radius_override.checked == false){
		F.wl0_radius_override.value = 0;
	} else {
		F.wl0_radius_override.value = 1;
	}

	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	if (F._radius_override.checked == false){
		F.wl0_radius_override.value = 0;
	} else {
			F.wl0_radius_override.value = 1;
	}

	F.apply_button.value = sbutton.applied;
	applytake(F);
}

function setRad(val) {
	show_layer_ext(document.wireless.wl_radauth, 'idradius', val == "1");
	setElementsActive("wl_radmactype", "_radius_override", val == "1");
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	setRad("<% nvg("wl_radauth"); %>");
	if ("ap" != "<% nvg("wl_mode"); %>"){
		setElementsActive("wl_radauth", "_radius_override", false);
		alert(errmsg.err49);
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
				<% do_menu("Wireless_Basic.asp","Wireless_radauth.asp"); %>
		</div>
		<div id="main">
			<div id="contents">
				<form name="wireless" action="apply.cgi" method="post">
				<input type="hidden" name="submit_button" value="Wireless_radauth" />
				<input type="hidden" name="action" value="Apply" />
				<input type="hidden" name="change_action" />
				<input type="hidden" name="submit_type" />
				<input type="hidden" name="commit" value="1" />
				<input type="hidden" name="wl0_radius_override" />
				<h2><% tran("radius.h2"); %></h2>
				<fieldset>
				<legend><% tran("radius.legend"); %></legend>
				<div class="setting">
					<div class="label"><% tran("radius.label"); %></div>
					<input class="spaceradio" type="radio" name="wl_radauth" value="1" <% nvc("wl_radauth","1"); %> onclick="setRad(this.value)" /><% tran("share.enable"); %>&nbsp;
					<input class="spaceradio" type="radio" name="wl_radauth" value="0" <% nvc("wl_radauth","0"); %> onclick="setRad(this.value)" /><% tran("share.disable"); %>
				</div>
				<div id="idradius">
					<div class="setting">
						<div class="label"><% tran("radius.label2"); %></div>
						<select name="wl_radmactype">
							<option value="0" <% nvs("wl_radmactype","0"); %>>aabbcc-ddeeff</option>
							<option value="1" <% nvs("wl_radmactype","1"); %>>aabbccddeeff</option>
							<option value="2" <% nvs("wl_radmactype","2"); %>>aa:bb:cc:dd:ee:ff</option>
							<option value="3" <% nvs("wl_radmactype","3"); %>>aa-bb-cc-dd-ee-ff</option>
						</select>
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label3"); %></div>
						<input type="hidden" name="wl_radius_ipaddr" value="4" />
						<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_0" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","0"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_1" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","1"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_2" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","2"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_3" onblur="valid_range(this,1,254,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","3"); %>" />
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label4"); %></div>
						<input class="num" size="5" maxlength="5" name="wl_radius_port" onblur="valid_range(this,1,65535,radius.label4)" value="<% nvg("wl_radius_port"); %>" />
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label5"); %></div>
						<input class="num" size="5" maxlength="5" name="wl0_max_unauth_users" value="<% nvg("wl0_max_unauth_users"); %>" />
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label6"); %></div>
						<input class="spaceradio" type="radio" name="wl0_radmacpassword" value="1" <% nvc("wl0_radmacpassword","1"); %> /><% tran("share.share_key"); %>&nbsp;
						<input class="spaceradio" type="radio" name="wl0_radmacpassword" value="0" <% nvc("wl0_radmacpassword","0"); %> /><% tran("share.mac"); %>
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label7"); %></div>
						<input type="password" autocomplete="new-password" id="wl_radius_key" name="wl_radius_key" size="20" maxlength="79" value="<% nvg("wl_radius_key"); %>" />&nbsp;&nbsp;&nbsp;
						<input type="checkbox" name="_wl_radius_unmask" value="0" onclick="setElementMask('wl_radius_key', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
					</div>
					<div class="setting">
						<div class="label"><% tran("radius.label8"); %></div>
						<input type="checkbox" name="_radius_override" value="1" <% nvc("wl0_radius_override", "1"); %> />
					</div>
					</div>
				</fieldset><br/>
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
				<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('Hradauth.asp')"><% tran("share.more"); %></a>
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
