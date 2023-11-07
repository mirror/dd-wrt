<% do_pagehead("aoss.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function startAOSS(F) {
	F.submit_type.value = "start";
	F.submit();
}

function toggleAOSS(button, show) {
	show_layer_ext(button, 'aoss_button', show);
	show_layer_ext(button, 'aoss_options', show);
}

function to_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
  apply(F);
}

function to_apply(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save";
	F.apply_button.value = sbutton.applied;
  applytake(F);
}

/* the following 3 functions are taken and ported from HostAPD to Javascript*/
function wps_pin_checksum(pin) {
	accum = 0;
	while (pin) {
		accum += parseInt(3 * (pin % 10));
		pin = parseInt(pin / 10);
		accum += parseInt(pin % 10);
		pin = parseInt(pin / 10);
	}

	return (10 - accum % 10) % 10;
}

function wps_pin_valid(pin) {
	return wps_pin_checksum(parseInt(pin / 10)) == (pin % 10);
}

function wps_generate_pin(field) {
	/* Generate seven random digits for the PIN */
	val = parseInt(Math.random()*(9999999+1));
	val %= 10000000;

	/* Append checksum digit */
	value = val * 10 + wps_pin_checksum(val);
	if (value<10000000)
		field.value = "0" + value;
	else
		field.value = value;
}

function to_register(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "wps_register";
  apply(F);
}

function to_forcerelease(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "wps_forcerelease";
  apply(F);
}

function to_configure(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "wps_configure";
  apply(F);
}

function to_ap_register(F) {
	if (!wps_pin_valid(F.wps_ap_pin.value)) {
		alert(aoss.pinnotvalid);
		return;
	}
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "wps_ap_register";
  apply(F);
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);

	update = new StatusUpdate("AOSS.live.asp", <% nvg("refresh_time"); %>);
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
			<% do_menu("Wireless_Basic.asp","AOSS.asp"); %>
		</div>
		<div id="main">
			<div id="contents">
			<form name="aoss" action="apply.cgi" method="post" spellcheck="false">
				<input type="hidden" name="submit_button" value="AOSS" />
				<input type="hidden" name="action" value="Apply" />
				<input type="hidden" name="change_action" value="gozila_cgi" />
				<input type="hidden" name="submit_type" value="save" />

				<input type="hidden" name="security_varname" />
				<input type="hidden" name="security_mode_last" />
				<input type="hidden" name="wl_wep_last" />
				<input type="hidden" name="filter_mac_value" />

				<!-- AOSS start -->
				<h2><% tran("aoss.aoss"); %></h2>
				<% ifaoss_possible("yes", "<!--"); %>
				<fieldset>
					<legend><% tran("aoss.service"); %></legend>
					<div class="setting">
						<div class="label"><% tran("aoss.enable"); %></div>
						<input class="spaceradio" type="radio" value="1" name="aoss_enable" disabled /><% tran("share.enable"); %>&nbsp;
						<input class="spaceradio" type="radio" value="0" name="aoss_enable" checked disabled /><% tran("share.disable"); %>
					</div>
				</fieldset>
				<br />
				<div class="warning">
					<p>
						<% tran("aoss.ap_mode_notice"); %>
					</p>
				</div>
				<br />
				<% ifaoss_possible("yes", "-->"); %>
				<% ifaoss_possible("no", "<!--"); %>
				<fieldset>
					<legend><% tran("aoss.service"); %></legend>
					<div class="setting">
						<div class="label"><% tran("aoss.enable"); %></div>
						<input class="spaceradio" type="radio" value="1" name="aoss_enable" <% nvc("aoss_enable", "1"); %> onClick="toggleAOSS(this, true);" /><% tran("share.enable"); %>&nbsp;
						<input class="spaceradio" type="radio" value="0" name="aoss_enable" <% nvc("aoss_enable", "0"); %> onClick="toggleAOSS(this, false);" /><% tran("share.disable"); %>
					</div>
					<div id="aoss_button" class="setting" style="<% visible_css("aoss_enable", "1"); %>">
						<div class="label"><% tran("aoss.start"); %></div>
						<input type="button" class="aoss_enable" value="" onclick="startAOSS(this.form)">
					</div>
				</fieldset><br />
				<div id="aoss_options" style="<% visible_css("aoss_enable", "1"); %>">
					<fieldset>
						<legend><% tran("aoss.securitymodes"); %></legend>
						<div class="setting">
							<div class="label"><% tran("aoss.wpaaes"); %></div>
							<input type="checkbox" name="aoss_aes" value="1"<% isChecked("aoss_aes", "1"); %>></input>
						</div>
						<div class="setting">
							<div class="label"><% tran("aoss.wpatkip"); %></div>
								<input type="checkbox" name="aoss_tkip" value="1"<% isChecked("aoss_tkip", "1"); %>></input>
						</div>
						<div class="setting">
							<div class="label"><% tran("aoss.wep"); %></div>
								<input type="checkbox" name="aoss_wep" value="1"<% isChecked("aoss_wep", "1"); %>></input>
						</div>
					</fieldset><br />
				</div>
				<% ifaoss_possible("no", "-->"); %>
<!--fieldset>
<legend><% tran("aoss.clients"); %></legend>
<table class="center table" cellspacing="5">
<tr>
<th width="25%"><% tran("aoss.client_name"); %></th>
<th width="20%"><% tran("share.mac"); %></th>
<th width="20%"><% tran("aoss.security"); %></th>
<th width="20%"><% tran("wl_basic.label"); %></th>
<th width="15%"><% tran("aoss.connectivity"); %></th>
</tr>

<script type="text/javascript">
//<![CDATA[

var table = new Array(
 "Nintendo DS","00:1c:4a:01:5f:63","WPA-PSK-TKIP","802.11b/g"
,"Sony PS3","00:1c:4a:42:19:3d","WPA-PSK-AES","802.11b/g"

);

if (table.length == 0) {
document.write("<tr><td colspan=\"11\" class=\"center\">" + share.none + "</td></tr>");
}
else {
for (var i = 0; i < table.length; i = i + 4) {
document.write("<tr>");
document.write("<td>"+table[i]+"</td>");
document.write("<td align=\"left\">"+table[i+1]+"</td>");
document.write("<td align=\"right\">"+table[i+2]+"</td>");
document.write("<td align=\"right\">"+table[i+3]+"</td>");
document.write("<td align=\"left\"><select name=\"\"><option value=\"allow\">Allow</option><option value=\"deny\">Deny</option><option value=\"delete\">Delete</option></select></td>");
document.write("<\/tr>");
}
}

//]]>
</script>

</table>
</fieldset>
<br /-->
<!-- AOSS end -->
				<% ifndef("HAVE_WPS", "<!--"); %>
				<h2>WPS</h2>
				<fieldset>
					<legend><% tran("aoss.wps"); %></legend>
					<div class="setting">
						<div class="label"><% tran("aoss.wpsenable"); %></div>
						<input class="spaceradio" type="radio" value="1" name="wps_enabled" <% nvc("wps_enabled", "1"); %> /><% tran("share.enable"); %>&nbsp;
						<input class="spaceradio" type="radio" value="0" name="wps_enabled" <% nvc("wps_enabled", "0"); %> /><% tran("share.disable"); %>
					</div>
					<div class="setting">
						<div class="label"><% tran("aoss.externalregistrar"); %></div>
						<input class="spaceradio" type="radio" value="1" name="wps_registrar" <% nvc("wps_registrar", "1"); %> /><% tran("share.enable"); %>&nbsp;
						<input class="spaceradio" type="radio" value="0" name="wps_registrar" <% nvc("wps_registrar", "0"); %> /><% tran("share.disable"); %>
					</div>
					<div class="setting">
						<div class="label"><% tran("aoss.wps_ap_pin"); %></div>
						<input class="num" name="wps_ap_pin" size="16" maxlength="16" value="<% nvg("pincode"); %>"/>&nbsp;
						<script type="text/javascript">
						//<![CDATA[
						document.write("<input class=\"button\" type=\"button\" value=\"" + aoss.wpsactivate + "\" onclick=\"to_ap_register(this.form);\" />");
						document.write("<input class=\"button\" type=\"button\" value=\"" + aoss.wpsgenerate + "\" onclick=\"wps_generate_pin(this.form.wps_ap_pin);\" />");
						//]]>
						</script>
					</div>
					<div class="setting">
						<div class="label"><% tran("aoss.wpspin"); %></div>
						<input class="num" name="wps_pin" size="16" maxlength="16" />&nbsp;
						<script type="text/javascript">
						//<![CDATA[
						document.write("<input class=\"button\" type=\"button\" value=\"" + aoss.wpsregister + "\" onclick=\"to_register(this.form);\" />");
						//]]>
						</script>
					</div>
					<div class="setting">
						<div class="label"><% tran("aoss.wpsstatus"); %></div>
						<span id="wpsstatus"><% get_wpsstatus(); %></span>
						<span id="wpsconfigure"><% get_wpsconfigure(); %></span>
					</div>
				</fieldset>
				<br />
				<% ifndef("HAVE_WPS", "-->"); %>
				<div id="footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
					submitFooterButton(1,1,0,autoref);
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
				 <dt class="term"><% tran("aoss.aoss"); %></dt>
				 <dd class="definition"><% tran("haoss.basic"); %></dd>
				 <dt class="term"><% tran("aoss.securitymodes"); %></dt>
				 <dd class="definition"><% tran("haoss.securitymodes"); %></dd>
				 <% ifndef("HAVE_WPS", "<!--"); %>
				 <dt class="term"><% tran("aoss.wps"); %></dt>
				 <dd class="definition"><% tran("haoss.wps"); %></dd>
				 <% ifndef("HAVE_WPS", "-->"); %>
			   </dl><br />
			   <!--a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWPA.asp')"><% tran("share.more"); %></a-->
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
