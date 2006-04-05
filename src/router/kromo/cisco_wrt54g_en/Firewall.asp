<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Firewall</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function wan_enable_disable(F) {
	if(F._block_wan.checked == true)
		choose_enable(F._ident_pass);
	else {
		choose_disable(F._ident_pass);
	}
}

function to_submit(F) {
	F.submit_button.value = "Firewall";
	F.block_wan.value = (F._block_wan.checked == true) ? 1 : 0;
	if(F._block_loopback){
		F.block_loopback.value = (F._block_loopback.checked == true) ? 1 : 0;
	}
	if(F._block_cookie){
		F.block_cookie.value = (F._block_cookie.checked == true) ? 1 : 0;
	}
	if(F._block_java){
		F.block_java.value = (F._block_java.checked == true) ? 1 : 0;
	}
	if(F._block_proxy){
		F.block_proxy.value = (F._block_proxy.checked == true) ? 1 : 0;
	}
	if(F._block_activex){
		F.block_activex.value = (F._block_activex.checked == true) ? 1 : 0;
	}
	if(F._ident_pass){
		F.ident_pass.value = (F._ident_pass.checked == true) ? 0 : 1;
	}
	if(F._block_multicast) {
		F.multicast_pass.value = (F._block_multicast.checked == true) ? 0 : 1;
	}
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

	F.action.value = "Apply";
	apply(F);
}
function init() {
	if(document.firewall._block_wan.checked == true) {
		choose_enable(document.firewall._ident_pass);
	} else {
		choose_disable(document.firewall._ident_pass);
	}
}
</script></head>
   <body class="gui" onload="init()"> <% showad(); %>
      <div id="wrapper">
         <div id="content">
            <div id="header">
               <div id="logo">
                  <h1><% show_control(); %></h1>
               </div>
               <div id="menu">
                  <div id="menuMain">
                     <ul id="menuMainList">
                        <li><a href="index.asp">Setup</a></li>
                        <li><a href="Wireless_Basic.asp">Wireless</a></li>
			<% nvram_invmatch("sipgate","1","<!--"); %>
			<li><a href="Sipath.asp">SIPatH</a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
                        <li class="current"><span>Security</span><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><span>Firewall</span></li>
                                 <li><a href="VPN.asp">VPN</a></li>
                              </ul>
                           </div>
                        </li>
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
                  <form name="firewall" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="change_action" /><input type="hidden" name="action" /><input type="hidden" name="block_wan" /><input type="hidden" name="block_loopback" /><input type="hidden" name="multicast_pass" value="0" /><input type="hidden" name="ident_pass" /><input type="hidden" name="block_cookie" value="0" /><input type="hidden" name="block_java" value="0" /><input type="hidden" name="block_proxy" value="0" /><input type="hidden" name="block_activex" value="0" /><h2>Security</h2>
                     <fieldset>
                        <legend>Firewall Protection</legend>
                        <div class="setting">
			<input type="radio" value="on" name="filter" <% nvram_match("filter","on","checked"); %>>Enable</input>
			<input type="radio" value="off" name="filter" <% nvram_match("filter","off","checked"); %>>Disable</input>
			</div>
                     </fieldset><br /><fieldset>
                        <legend>Additional Filters</legend>
                        <div class="setting"><input type="checkbox" value="1" name="_block_proxy" <% nvram_match("block_proxy","1","checked"); %>>Filter Proxy</input></div>
                        <div class="setting"><input type="checkbox" value="1" name="_block_cookie" <% nvram_match("block_cookie","1","checked"); %>>Filter Cookies</input></div>
                        <div class="setting"><input type="checkbox" value="1" name="_block_java" <% nvram_match("block_java","1","checked"); %>>Filter Java Applets</input></div>
                        <div class="setting"><input type="checkbox" value="1" name="_block_activex" <% nvram_match("block_activex","1","checked"); %>>Filter ActiveX</input></div>
                     </fieldset><br /><fieldset>
                        <legend>Block WAN Requests</legend>
                        <div class="setting"><input type="checkbox" value="1" name="_block_wan" onclick=wan_enable_disable(this.form) <% nvram_match("block_wan","1","checked"); %>>Block Anonymous Internet Requests</input></div><% support_invmatch("MULTICAST_SUPPORT", "1", "<!--"); %>
                        <div class="setting"><input type="checkbox" value="0" name="_block_multicast" <% nvram_match("multicast_pass","0","checked"); %>>Filter Multicast</input></div><% support_invmatch("MULTICAST_SUPPORT", "1", "-->"); %>
                        <div class="setting"><input type="checkbox" value="0" name="_block_loopback" <% nvram_match("block_loopback","1","checked"); %>>Filter Internet NAT Redirection</input></div>
                        <div class="setting"><input type="checkbox" value="1" name="_ident_pass" <% nvram_match("ident_pass","0","checked"); %>>Filter IDENT (Port 113)</input></div>
                     </fieldset><br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick=to_submit(this.form) /><input type="reset" value="Cancel Changes" /></div>
                  </form>
               </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">Firewall Protection: </dt>
                     <dd class="definition">Enable or disable the SPI firewall.</dd>
                  </dl><br />
                  <a href="javascript:openHelpWindow('HFirewall.asp')">More...</a>
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