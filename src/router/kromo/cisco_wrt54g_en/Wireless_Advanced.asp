<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
		<title><% nvram_get("router_name"); %> - Advanced Wireless Settings</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]> <link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /> <![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "Wireless_Advanced";
	F.save_button.value = "Saved";
	F.action.value = "Apply";
	apply(F);
}

function setWMM(val) {
	setElementsActive("wl_wme_no_ack", "_wl_wme_sta_vo5", val == "on");
}

addEvent(window, "load", function() {
	setWMM("<% nvram_get("wl_wme"); %>" == "on");
});
		</script>
	</head>

	<body class="gui"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li class="current"><span>Wireless</span>
								  <div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Wireless_Basic.asp">Basic Settings</a></li>
											<li><a href="Wireless_radauth.asp">Radius</a></li>
											<li><a href="WL_WPATable.asp">Wireless Security</a></li>
											<li><a href="Wireless_MAC.asp">MAC Filter</a></li>
											<li><span>Advanced Settings</span></li>
											<li><a href="Wireless_WDS.asp">WDS</a></li>
										</ul>
									</div>
								</li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href="Filters.asp">Access Restrictions</a></li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form action="apply.cgi" method="<% get_http_method(); %>" name="wireless" id="wireless">
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="submit_button" value="Wireless_Advanced" />
							<input type="hidden" name="commit" value="1" />
							<h2>Advanced Wireless Settings</h2>
							<fieldset>
								<legend>Advanced Settings</legend>
								<div class="setting">
									<div class="label">Authentication Type</div>
									<input type="radio" name="wl_auth" value="0" <% nvram_checked("wl_auth", "0"); %> />Auto&nbsp;
									<input type="radio" name="wl_auth" value="1" <% nvram_checked("wl_auth", "1"); %> />Shared Key&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">Basic Rate</div>
									<select name="wl_rateset">
										<option value="12" <% nvram_selected("wl_rateset", "12"); %> />1-2 Mbps</option>
										<option value="default" <% nvram_selected("wl_rateset", "default"); %> />Default</option>
										<option value="all" <% nvram_selected("wl_rateset", "all"); %> />All</option>
									</select>&nbsp;
									<span class="default">(Default: Default)</span>
								</div>
								<div class="setting">
									<div class="label">Transmission Rate</div>
									<select name="wl_rate">
										<option value="1000000" <% nvram_selected("wl_rate", "1000000"); %>>1 Mbps</option>
										<option value="2000000" <% nvram_selected("wl_rate", "2000000"); %>>2 Mbps</option>
										<option value="5500000" <% nvram_selected("wl_rate", "5500000"); %>>5.5 Mbps</option>
										<option value="6000000" <% nvram_selected("wl_rate", "6000000"); %>>6 Mbps</option>
										<option value="9000000" <% nvram_selected("wl_rate", "9000000"); %>>9 Mbps</option>
										<option value="11000000" <% nvram_selected("wl_rate", "11000000"); %>>11 Mbps</option>
										<option value="12000000" <% nvram_selected("wl_rate", "12000000"); %>>12 Mbps</option>
										<option value="18000000" <% nvram_selected("wl_rate", "18000000"); %>>18 Mbps</option>
										<option value="24000000" <% nvram_selected("wl_rate", "24000000"); %>>24 Mbps</option>
										<option value="36000000" <% nvram_selected("wl_rate", "36000000"); %>>36 Mbps</option>
										<option value="48000000" <% nvram_selected("wl_rate", "48000000"); %>>48 Mbps</option>
										<option value="54000000" <% nvram_selected("wl_rate", "54000000"); %>>54 Mbps</option>
										<option value="0" <% nvram_selected("wl_rate", "0"); %>>Auto</option>
									</select>&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">CTS Protection Mode</div>
									<input type="radio" name="wl_gmode_protection" value="auto" <% nvram_checked("wl_gmode_protection", "auto"); %> />Auto&nbsp;
									<input type="radio" name="wl_gmode_protection" value="off" <% nvram_checked("wl_gmode_protection", "off"); %> />Disable&nbsp;
									<span class="default">(Default: Disable)</span>
								</div>
								<div class="setting">
									<div class="label">Frame Burst</div>
									<input type="radio" name="wl_frameburst" value="on" <% nvram_checked("wl_frameburst", "on"); %> />Enable&nbsp;
									<input type="radio" name="wl_frameburst" value="off" <% nvram_checked("wl_frameburst", "off"); %> />Disable&nbsp;
									<span class="default">(Default: Disable)</span>
								</div><br />
								<div class="setting">
									<div class="label">Beacon Interval</div>
									<input class="num" name="wl_bcn" size="6" maxlength="5" onblur="valid_range(this,1,65535,'Beacon Interval')" value='<% nvram_selget("wl_bcn"); %>' />&nbsp;
									<span class="default">(Default: 100ms, Range: 1 - 65535)</span>
								</div>
								<div class="setting">
									<div class="label">DTIM Interval</div>
									<input class="num" name="wl_dtim" size="6" maxlength="3" onblur="valid_range(this,1,255,'DTIM Interval')" value='<% nvram_selget("wl_dtim"); %>' />&nbsp;
									<span class="default">(Default: <% get_wl_value("default_dtim"); %>, Range: 1 - 255)</span>
								</div>
								<div class="setting">
									<div class="label">Fragmentation Threshold</div>
									<input class="num" name="wl_frag" size="6" maxlength="4" onblur="valid_range(this,256,2346,'Fragmentation Threshold')" value='<% nvram_selget("wl_frag"); %>' />&nbsp;
									<span class="default">(Default: 2346, Range: 256 - 2346)</span>
								</div>
								<div class="setting">
									<div class="label">RTS Threshold</div>
									<input class="num" name="wl_rts" size="6" maxlength="4" onblur="valid_range(this,0,2347,'RTS Threshold')" value='<% nvram_selget("wl_rts"); %>' />&nbsp;
									<span class="default">(Default: 2347, Range: 0 - 2347)</span>
								</div>
								<div class="setting">
									<div class="label">Max Associated Clients</div>
									<input class="num" name="wl_maxassoc" size="6" maxlength="4" onblur="valid_range(this,1,256,'Maximum Associated Clients')" value='<% nvram_selget("wl_maxassoc"); %>' />&nbsp;
									<span class="default">(Default: 128, Range: 1 - 256)</span>
							 	</div><br />
								<div class="setting">
									<div class="label">AP Isolation</div>
									<input type="radio" name="wl_ap_isolate" value="1" <% nvram_checked("wl_ap_isolate", "1"); %> />Enable&nbsp;
									<input type="radio" name="wl_ap_isolate" value="0" <% nvram_checked("wl_ap_isolate", "0"); %> />Disable&nbsp;
									<span class="default">(Default: Disable)</span>
								</div>
								<div class="setting">
									<div class="label">TX Antenna</div>
									<select name="txant">
										<option value="0" <% nvram_selected("txant", "0"); %>>Right</option>
										<option value="1" <% nvram_selected("txant", "1"); %>>Left</option>
										<option value="3" <% nvram_selected("txant", "3"); %>>Auto</option>
									</select>&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">RX Antenna</div>
									<select name="wl_antdiv">
										<option value="0" <% nvram_selected("wl_antdiv", "0"); %>>Right</option>
										<option value="1" <% nvram_selected("wl_antdiv", "1"); %>>Left</option>
										<option value="3" <% nvram_selected("wl_antdiv", "3"); %>>Auto</option>
									</select>&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">Preamble</div>
									<select name="wl_plcphdr">
										<option value="long" <% nvram_selected("wl_plcphdr", "long"); %>>Long</option>
										<option value="short" <% nvram_selected("wl_plcphdr", "short"); %>>Short</option>
										<option value="auto" <% nvram_selected("wl_plcphdr", "auto"); %>>Auto</option>
									</select>&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">Xmit Power</div>
									<input class="num" name="txpwr" size="6" maxlength="3" onblur="valid_range(this,0,251,'TX Power')" value='<% nvram_selget("txpwr"); %>' />&nbsp;
									<span class="default">(Default: 28, Range: 0 - 251mW)</span>
								</div>
								<div class="setting">
									<div class="label">Afterburner</div>
									<select name="wl_afterburner">
										<option value="off" <% nvram_selected("wl_afterburner", "off"); %>>Disable</option>
										<option value="on" <% nvram_selected("wl_afterburner", "on"); %>>Enable</option>
										<option value="auto" <% nvram_selected("wl_afterburner", "auto"); %>>Auto</option>
									</select>&nbsp;
									<span class="default">(Default: Auto)</span>
								</div>
								<div class="setting">
									<div class="label">Wireless GUI Access</div>
									<input type="radio" name="web_wl_filter" value="0" <% nvram_checked("web_wl_filter", "0"); %> />Enable&nbsp;
									<input type="radio" name="web_wl_filter" value="1" <% nvram_checked("web_wl_filter", "1"); %> />Disable&nbsp;
									<span class="default">(Default: Enable)</span>
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Wireless Multimedia Support Settings</legend>
								<div class="setting">
									<div class="label">WMM Support</div>
									<input type="radio" name="wl_wme" value="on" <% nvram_checked("wl_wme", "on"); %>  onclick="setWMM(this.value)" />Enable&nbsp;
									<input type="radio" name="wl_wme" value="off" <% nvram_checked("wl_wme", "off"); %>  onclick="setWMM(this.value)" />Disable&nbsp;
									<span class="default">(Default: Disable)</span>
								</div>
								<div class="setting">
									<div class="label">No-Acknowledgement</div>
									<input type="radio" name="wl_wme_no_ack" value="on" <% nvram_checked("wl_wme_no_ack", "on"); %>  />Enable&nbsp;
									<input type="radio" name="wl_wme_no_ack" value="off" <% nvram_checked("wl_wme_no_ack", "off"); %> />Disable&nbsp;
									<span class="default">(Default: Disable)</span>
								</div>
								<table class="table center" cellspacing="5">
									<tr>
										<th colspan="7">EDCA AP Parameters</td>
									</tr>
									<tr>
										<td>&nbsp;</td>
										<td align="center">CWmin</td>
										<td align="center">CWmax</td>
										<td align="center">AIFSN</td>
										<td align="center">TXOP(b)</td>
										<td align="center">TXOP(a/g)</td>
										<td align="center">Admin Forced</td>
									</tr>
									<tr>
										<td>AC_BK<input type="hidden" name="wl_wme_ap_bk" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_bk0" value="<% nvram_list("wl_wme_ap_bk", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_bk1" value="<% nvram_list("wl_wme_ap_bk", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_bk2" value="<% nvram_list("wl_wme_ap_bk", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_bk3" value="<% nvram_list("wl_wme_ap_bk", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_bk4" value="<% nvram_list("wl_wme_ap_bk", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_ap_bk5" value="<% nvram_list("wl_wme_ap_bk", 5); %>" /><input type="checkbox" name="_wl_wme_ap_bk5" <% wme_match_op("wl_wme_ap_bk", "on", "checked='checked'"); %> onchange="this.form.wl_wme_ap_bk5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_BE<input type="hidden" name="wl_wme_ap_be" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_be0" value="<% nvram_list("wl_wme_ap_be", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_be1" value="<% nvram_list("wl_wme_ap_be", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_be2" value="<% nvram_list("wl_wme_ap_be", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_be3" value="<% nvram_list("wl_wme_ap_be", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_be4" value="<% nvram_list("wl_wme_ap_be", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_ap_be5" value="<% nvram_list("wl_wme_ap_be", 5); %>" /><input type="checkbox" name="_wl_wme_ap_be5" <% wme_match_op("wl_wme_ap_be", "on", "checked='checked'"); %> onchange="this.form.wl_wme_ap_be5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_VI<input type="hidden" name="wl_wme_ap_vi" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vi0" value="<% nvram_list("wl_wme_ap_vi", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vi1" value="<% nvram_list("wl_wme_ap_vi", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vi2" value="<% nvram_list("wl_wme_ap_vi", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vi3" value="<% nvram_list("wl_wme_ap_vi", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vi4" value="<% nvram_list("wl_wme_ap_vi", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_ap_vi5" value="<% nvram_list("wl_wme_ap_vi", 5); %>" /><input type="checkbox" name="_wl_wme_ap_vi5" <% wme_match_op("wl_wme_ap_vi", "on", "checked='checked'"); %> onchange="this.form.wl_wme_ap_vi5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_VO<input type="hidden" name="wl_wme_ap_vo" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vo0" value="<% nvram_list("wl_wme_ap_vo", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vo1" value="<% nvram_list("wl_wme_ap_vo", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vo2" value="<% nvram_list("wl_wme_ap_vo", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vo3" value="<% nvram_list("wl_wme_ap_vo", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_ap_vo4" value="<% nvram_list("wl_wme_ap_vo", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_ap_vo5" value="<% nvram_list("wl_wme_ap_vo", 5); %>" /><input type="checkbox" name="_wl_wme_ap_vo5" <% wme_match_op("wl_wme_ap_vo", "on", "checked='checked'"); %> onchange="this.form.wl_wme_ap_vo5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
								</table>
								<table cellspacing="5">
									<tr>
										<th colspan="7">EDCA STA Parameters</td>
									</tr>
									<tr>
										<td>&nbsp;</td>
										<td align="center">CWmin</td>
										<td align="center">CWmax</td>
										<td align="center">AIFSN</td>
										<td align="center">TXOP(b)</td>
										<td align="center">TXOP(a/g)</td>
										<td align="center">Admin Forced</td>
									</tr>
									<tr>
										<td>AC_BK<input type="hidden" name="wl_wme_sta_bk" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_bk0" value="<% nvram_list("wl_wme_sta_bk", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_bk1" value="<% nvram_list("wl_wme_sta_bk", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_bk2" value="<% nvram_list("wl_wme_sta_bk", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_bk3" value="<% nvram_list("wl_wme_sta_bk", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_bk4" value="<% nvram_list("wl_wme_sta_bk", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_sta_bk5" value="<% nvram_list("wl_wme_sta_bk", 5); %>" /><input type="checkbox" name="_wl_wme_sta_bk5" <% wme_match_op("wl_wme_sta_bk", "on", "checked='checked'"); %> onchange="this.form.wl_wme_sta_bk5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_BE<input type="hidden" name="wl_wme_sta_be" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_be0" value="<% nvram_list("wl_wme_sta_be", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_be1" value="<% nvram_list("wl_wme_sta_be", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_be2" value="<% nvram_list("wl_wme_sta_be", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_be3" value="<% nvram_list("wl_wme_sta_be", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_be4" value="<% nvram_list("wl_wme_sta_be", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_sta_be5" value="<% nvram_list("wl_wme_sta_be", 5); %>" /><input type="checkbox" name="_wl_wme_sta_be5" <% wme_match_op("wl_wme_sta_be", "on", "checked='checked'"); %> onchange="this.form.wl_wme_sta_be5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_VI<input type="hidden" name="wl_wme_sta_vi" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vi0" value="<% nvram_list("wl_wme_sta_vi", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vi1" value="<% nvram_list("wl_wme_sta_vi", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vi2" value="<% nvram_list("wl_wme_sta_vi", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vi3" value="<% nvram_list("wl_wme_sta_vi", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vi4" value="<% nvram_list("wl_wme_sta_vi", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_sta_vi5" value="<% nvram_list("wl_wme_sta_vi", 5); %>" /><input type="checkbox" name="_wl_wme_sta_vi5" <% wme_match_op("wl_wme_sta_vi", "on", "checked='checked'"); %> onchange="this.form.wl_wme_sta_vi5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
									<tr>
										<td>AC_VO<input type="hidden" name="wl_wme_sta_vo" value="5" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vo0" value="<% nvram_list("wl_wme_sta_vo", 0); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmin')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vo1" value="<% nvram_list("wl_wme_sta_vo", 1); %>" size="5" maxlength="6" onblur="valid_range(this,0,32767,'AC CWmax')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vo2" value="<% nvram_list("wl_wme_sta_vo", 2); %>" size="5" maxlength="6" onblur="valid_range(this,1,15,'AC AIFSN')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vo3" value="<% nvram_list("wl_wme_sta_vo", 3); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(b)')" /></td>
										<td align="center"><input class="num" name="wl_wme_sta_vo4" value="<% nvram_list("wl_wme_sta_vo", 4); %>" size="5" maxlength="6" onblur="valid_range(this,0,65504,'AC TXOP(a/g)')" /></td>
										<td align="center"><input type="hidden" name="wl_wme_sta_vo5" value="<% nvram_list("wl_wme_sta_vo", 5); %>" /><input type="checkbox" name="_wl_wme_sta_vo5" <% wme_match_op("wl_wme_sta_vo", "on", "checked='checked'"); %> onchange="this.form.wl_wme_sta_vo5.value = (this.checked ? 'on' : 'off');" /></td>
									</tr>
								</table>
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="reset" value="Cancel Changes" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Authentication Type: </dt>
							<dd class="definition">You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also support Shared Key authentication.</dd>
						</dl><br />
						<a href="javascript:openHelpWindow('HWirelessAdvanced.asp');">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <a href="javascript:openAboutWindow()"><% get_firmware_version(); %></a></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>