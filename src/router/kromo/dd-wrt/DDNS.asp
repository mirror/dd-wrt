<% do_pagehead("ddns.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function ddns_check(F,T) {
	if(F.ddns_enable.value == 0) {
		return true;
	}
	enable = eval("F.ddns_enable");
	username = eval("F.ddns_username");
	passwd = eval("F.ddns_passwd");
	hostname = eval("F.ddns_hostname");

	if (enable.value != "28" && enable.value != "11") {
		if(username.value == "") {
			alert(errmsg.err0);
			username.focus();
			return false;
		}
	}
	if (enable.value != "27" && enable.value != "32") {
		if(passwd.value == "") {
			alert(errmsg.err6);
			passwd.focus();
			return false;
		}
	}
	if(hostname && hostname.value == "") {
		alert(errmsg.err7);
		hostname.focus();
		return false;
	}

	return true;
}

function submitcheck(F) {
	if(!ddns_check(F,"save"))
		return;
	F.change_action.value = "";
	update.stop();
}
function to_submit(F) {
	submitcheck(F);
	F.save_button.value = sbutton.saving;
	apply(F);
}

function to_apply(F) {
	submitcheck(F);
	F.apply_button.value = sbutton.applied;
	applytake(F);
}

function SelDDNS(num,F) {
	F.change_action.value="gozila_cgi";
	F.ddns_enable.value=F.ddns_enable.options[num].value;
	update.stop();
	F.submit();
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	update = new StatusUpdate("DDNS.live.asp", <% nvg("refresh_time"); %>);
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
					<% do_menu("index.asp","DDNS.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="dyndns" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="DDNS" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type" />
							<h2><% tran("ddns.h2"); %></h2>
							<fieldset>
								<legend><% tran("ddns.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("ddns.srv"); %></div>
									<select name="ddns_enable" onchange="SelDDNS(this.form.ddns_enable.selectedIndex,this.form)">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvsm("ddns_enable", "0", "selected"); %> >" + share.disable + "</option>");
										//]]>
										</script>
										<option value="6" <% nvsm("ddns_enable", "6", "selected"); %> >3322.org</option>
										<option value="47" <% nvsm("ddns_enable", "47", "selected"); %> >All-Inkl.com</option>
										<option value="18" <% nvsm("ddns_enable", "18", "selected"); %> >ChangeIP</option>
										<option value="21" <% nvsm("ddns_enable", "21", "selected"); %> >Cloudflare</option>
										<option value="22" <% nvsm("ddns_enable", "22", "selected"); %> >CloudXNS</option>
										<option value="48" <% nvsm("ddns_enable", "48", "selected"); %> >Core-Networks</option>
										<option value="23" <% nvsm("ddns_enable", "23", "selected"); %> >Ddnss.de</option>
										<option value="36" <% nvsm("ddns_enable", "36", "selected"); %> >Desec.io</option>
										<option value="24" <% nvsm("ddns_enable", "24", "selected"); %> >DHIS.org</option>
										<option value="8" <% nvsm("ddns_enable", "8", "selected"); %> >DNS-O-Matic</option>
										<option value="25" <% nvsm("ddns_enable", "25", "selected"); %> >DNSExit.com</option>
										<option value="49" <% nvsm("ddns_enable", "49", "selected"); %> >DNSEver.com</option>
										<option value="50" <% nvsm("ddns_enable", "50", "selected"); %> >DNSHome.de</option>
										<option value="51" <% nvsm("ddns_enable", "51", "selected"); %> >DNSMadeeasy.com</option>
										<option value="52" <% nvsm("ddns_enable", "52", "selected"); %> >DNSMax.com</option>
										<option value="26" <% nvsm("ddns_enable", "26", "selected"); %> >DNSPod</option>
										<option value="38" <% nvsm("ddns_enable", "38", "selected"); %> >Domain-Offensive (do.de)</option>
										<option value="35" <% nvsm("ddns_enable", "35", "selected"); %> >Domaindiscount24</option>
										<option value="39" <% nvsm("ddns_enable", "39", "selected"); %> >Domopoli.de</option>
										<option value="27" <% nvsm("ddns_enable", "27", "selected"); %> >Duck DNS</option>
										<option value="11" <% nvsm("ddns_enable", "11", "selected"); %> >DUIAdns.net</option>
										<option value="37" <% nvsm("ddns_enable", "37", "selected"); %> >DY.fi</option>
										<option value="54" <% nvsm("ddns_enable", "54", "selected"); %> >DynDNS.it</option>
										<option value="1" <% nvsm("ddns_enable", "1", "selected"); %> >DynDNS.org</option>
										<option value="17" <% nvsm("ddns_enable", "17", "selected"); %> >Dynu Systems</option>
										<option value="32" <% nvsm("ddns_enable", "32", "selected"); %> >Dynv6</option>
										<option value="7" <% nvsm("ddns_enable", "7", "selected"); %> >EasyDNS.com</option>
										<option value="2" <% nvsm("ddns_enable", "2", "selected"); %> >FreeDNS</option>
										<option value="28" <% nvsm("ddns_enable", "28", "selected"); %> >FreeMyIP</option>
										<option value="29" <% nvsm("ddns_enable", "29", "selected"); %> >Gira</option>
										<option value="33" <% nvsm("ddns_enable", "33", "selected"); %> >Goip.de</option>
										<option value="16" <% nvsm("ddns_enable", "16", "selected"); %> >Google Domains</option>
										<option value="10" <% nvsm("ddns_enable", "10", "selected"); %> >he.net</option>
										<option value="55" <% nvsm("ddns_enable", "55", "selected"); %> >Infomaniak.com</option>
										<option value="63" <% nvsm("ddns_enable", "63", "selected"); %> >IPv64.net</option>
										<option value="41" <% nvsm("ddns_enable", "41", "selected"); %> >It's DNS (itsdns.de)</option>
										<option value="40" <% nvsm("ddns_enable", "40", "selected"); %> >Inwx.com</option>
										<option value="42" <% nvsm("ddns_enable", "42", "selected"); %> >Joker.com</option>
										<option value="15" <% nvsm("ddns_enable", "15", "selected"); %> >Loopia</option>
										<option value="57" <% nvsm("ddns_enable", "57", "selected"); %> >Mydns.jp</option>
										<option value="56" <% nvsm("ddns_enable", "56", "selected"); %> >Moniker.com</option>
										<option value="34" <% nvsm("ddns_enable", "34", "selected"); %> >Myonlineportal.net</option>
										<option value="4" <% nvsm("ddns_enable", "4", "selected"); %> >No-IP.com</option>
										<option value="58" <% nvsm("ddns_enable", "58", "selected"); %> >Namecheap.com</option>
										<option value="14" <% nvsm("ddns_enable", "14", "selected"); %> >NSupdate.info</option>
										<option value="43" <% nvsm("ddns_enable", "43", "selected"); %> >OpenDNS</option>
										<option value="59" <% nvsm("ddns_enable", "59", "selected"); %> >Oray.com</option>
										<option value="19" <% nvsm("ddns_enable", "19", "selected"); %> >OVH</option>
										<option value="64" <% nvsm("ddns_enable", "64", "selected"); %> >Porkbun.com</option>
										<option value="60" <% nvsm("ddns_enable", "60", "selected"); %> >Regfish.de</option>
										<option value="44" <% nvsm("ddns_enable", "44", "selected"); %> >Schokokeks.org</option>
										<option value="9" <% nvsm("ddns_enable", "9", "selected"); %> >SelfHOST</option>
										<option value="61" <% nvsm("ddns_enable", "61", "selected"); %> >Simply.com</option>
										<option value="30" <% nvsm("ddns_enable", "30", "selected"); %> >Sitelutions</option>
										<option value="13" <% nvsm("ddns_enable", "13", "selected"); %> >spDYN</option>
										<option value="20" <% nvsm("ddns_enable", "20", "selected"); %> >Strato</option>
										<option value="53" <% nvsm("ddns_enable", "53", "selected"); %> >Thatip.com</option>
										<option value="12" <% nvsm("ddns_enable", "12", "selected"); %> >Tunnelbroker.net</option>
										<option value="62" <% nvsm("ddns_enable", "62", "selected"); %> >TwoDNS.de</option>
										<option value="46" <% nvsm("ddns_enable", "46", "selected"); %> >Udmedia.de</option>
										<option value="45" <% nvsm("ddns_enable", "45", "selected"); %> >Variomedia.de</option>
										<option value="31" <% nvsm("ddns_enable", "31", "selected"); %> >Yandex</option>
										<option value="3" <% nvsm("ddns_enable", "3", "selected"); %> >ZoneEdit.com</option>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"5\" <% nvsm("ddns_enable", "5", "selected"); %> >" + ddns.custom + "</option>");
										//]]>
										</script>
									</select>
								<% nvlsm("ddns_enable","0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 61 62 63 64","<!--"); %>
								<div class="setting">
									<div class="label"><script type="text/javascript">Capture(share.token);</script></div>
									<input name="ddns_username" size="35" maxlength="64" onblur="valid_name(this,share.token)" value="<% get_ddns_value("ddns_username"); %>" />
								</div>
								<% nvlsm("ddns_enable","0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 61 62 63 64","-->"); %>
								</div>
								<% nvlsm("ddns_enable","0 5 28 32 11 60","<!--"); %>
								<div class="setting">
									<div class="label"><script type="text/javascript"><% nvesm("ddns_enable","27","Capture(share.token);","Capture(share.usrname);"); %></script></div>
									<input name="ddns_username" size="35" maxlength="64" onblur="valid_name(this,<% nvesm("ddns_enable","27","share.token","share.usrname"); %>)" value="<% get_ddns_value("ddns_username"); %>" />
								</div>
								<% nvlsm("ddns_enable","0 5 28 32 11 60","-->"); %>
								<% nvlsm("ddns_enable","0 27 32 5 60","<!--"); %>
								<div class="setting">
		  							<div class="label"><script type="text/javascript"><% nvelsm("ddns_enable","28","Capture(share.token);","Capture(share.passwd);"); %></script></div>
									<input type="password" autocomplete="new-password" id="ddns_passwd" name="ddns_passwd" size="35" maxlength="64" onblur="valid_name(this,<% nvelsm("ddns_enable","28","share.token","share.passwd"); %>)" value="<% get_ddns_value("ddns_passwd"); %>" />&nbsp;&nbsp;&nbsp;
									<input type="checkbox" name="ddns_passwd_unmask" value="0" onclick="setElementMask('ddns_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
								</div>			
								<% nvlsm("ddns_enable","0 27 32 5 60","-->"); %>
								<% nvlsm("ddns_enable","0 27 28 5 60","<!--"); %>
								<div class="setting">
									<div class="label"><script type="text/javascript"><% nvesm("ddns_enable","32","Capture(share.token);","Capture(share.usrname);"); %></script></div>
									<input name="ddns_username" size="35" maxlength="64" onblur="valid_name(this,<% nvesm("ddns_enable","32","share.token","share.usrname"); %>)" value="<% get_ddns_value("ddns_username"); %>" />
								</div>
								<% nvlsm("ddns_enable","0 27 28 5 60","-->"); %>
								<% nvlsm("ddns_enable","0 5","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname" size="35" maxlength="255" onblur="valid_hostname(this,share.hostname)" value="<% get_ddns_value("ddns_hostname"); %>" />
								</div>
								<% nvlsm("ddns_enable","0 5","-->"); %>
								<% nvlsm("ddns_enable","0 2 4 5 11 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 35 36 47 48 49 50 51 52 53 56 57 58 60","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("ddns.wildcard"); %></div>
									<input type="checkbox" value="1" name="ddns_wildcard" <% ddns_checked("ddns_wildcard", "1"); %> />
								</div>
								<% nvlsm("ddns_enable","0 2 4 5 11 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 35 36 47 48 49 50 51 52 53 56 57 58 60","-->"); %>
								<% nvlsmn6("ddns_enable","0 1 5 15 18 22 23 28 30 31 35 42 49 51 52 53 54 56 58 59 62 63 64","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("management.ipv6_srv"); %></div>
									<input class="spaceradio" type="radio" value="1" name="ddns_ipv6" <% nvc("ddns_ipv6", "1"); %> onclick="show_layer_ext(this, 'idipv6', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="ddns_ipv6" <% nvc("ddns_ipv6", "0"); %> onclick="show_layer_ext(this, 'idipv6', false)" /><% tran("share.disable"); %>
								</div>
								<div id="idipv6">
									<div class="setting">
										<div class="label"><% tran("ddns.ipv6_only"); %></div>
										<input type="checkbox" value="1" name="ddns_ipv6_only" <% nvc("ddns_ipv6_only", "1"); %> />
									</div>
								</div>
								<script type="text/javascript">
								//<![CDATA[
								show_layer_ext(document.dyndns.ddns_ipv6,'idipv6', <% nvem("ddns_ipv6", "1", "1", "0"); %> == 1);
								//]]>
								</script>
								<% nvlsmn6("ddns_enable","0 1 5 15 18 22 23 28 30 31 35 42 49 51 52 53 54 56 58 59 62 63 64","-->"); %>
								<% nvlsm("ddns_enable","0 1 2 3 4 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("ddns.system"); %></div>
									<input name="ddns_custom" size="35" maxlength="32" value="<% get_ddns_value("ddns_custom"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("service.samba3_share_path"); %></div>
									<input name="ddns_path" size="35" maxlength="255" value="<% get_ddns_value("ddns_path"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username" size="35" maxlength="64" onblur="valid_name(this,share.usrname)" value="<% get_ddns_value("ddns_username"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" autocomplete="new-password" id="ddns_passwd" name="ddns_passwd" size="35" maxlength="64" onblur="valid_name(this,share.passwd)" value="<% get_ddns_value("ddns_passwd"); %>" />&nbsp;&nbsp;&nbsp;
									<input type="checkbox" name="ddns_passwd_unmask" value="0" onclick="setElementMask('ddns_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname" size="35" onblur="valid_hostname(this,share.hostname)" value="<% get_ddns_value("ddns_hostname"); %>" />
								</div>
								<% nvlsm("ddns_enable","0 1 2 3 4 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 32 28 29 30 31 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64","-->"); %>
								<% nvsm("ddns_enable","0","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("ddns.wanip"); %></div>
									<input class="spaceradio" type="radio" value="0" name="ddns_wan_ip" <% nvc("ddns_wan_ip","0"); %> /><% tran("share.yes"); %>&nbsp;
									<input class="spaceradio" type="radio" value="1" name="ddns_wan_ip" <% nvc("ddns_wan_ip","1"); %> /><% tran("share.no"); %>
								</div>
								<% nvsm("ddns_enable","0","-->"); %>
								<% nvsm("ddns_enable","0","<!--"); %>
								<% ifndef("OPENSSL","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("ddns.ssl"); %></div>
									<input class="spaceradio" type="radio" value="1" name="ddns_ssl" <% nvc("ddns_ssl","1"); %> /><% tran("share.yes"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="ddns_ssl" <% nvc("ddns_ssl","0"); %> /><% tran("share.no"); %>
								</div>
								<% ifndef("OPENSSL","-->"); %>
								<% nvsm("ddns_enable","0","-->"); %>
							</fieldset>
							<% nvsm("ddns_enable","0","<!--"); %>
							<br /><fieldset>
								<legend><% tran("share.option"); %></legend>
								<div class="setting">
									<div class="label"><% tran("ddns.forceupd"); %></div>
									<input type="text" size="4" class="num" name="ddns_force" onblur="valid_range(this,1,60,ddns.forceupd)" value="<% nvg("ddns_force"); %>" />
									<span class="default">
									<script type="text/javascript">
									//<![CDATA[
									document.write("(" + share.deflt + ": 10 " + share.days_genetive + ", " + share.range + ": 1 - 60)");
									//]]>
									</script></span>
								</div>
							</fieldset><br />
							<% nvsm("ddns_enable","0","-->"); %>
							<% nvsm("ddns_enable","0","<!--"); %>
							<fieldset class="dark_fs_bg">
								<legend><% tran("ddns.statu"); %></legend>
								<div class="setting">
									<span id="ddns_status"><% show_ddns_status(); %></span>
								</div>
							<% nvsm("ddns_enable","0","-->"); %>
							</fieldset><br />
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
							<dt class="term"><% tran("ddns.hlp"); %>:</dt>
							<dd class="definition"><% tran("hddns.right2"); %></dd>
							<dt class="term"><% tran("ddns.forceupd"); %>:</dt>
							<dd class="definition"><% tran("hddns.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDDNS.asp');"><% tran("share.more"); %></a>
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
