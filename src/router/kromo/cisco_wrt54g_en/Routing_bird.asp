<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Routing</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
var route_win = null;
function ViewRoute() {
	route_win = self.open('RouteTable.asp', 'Route', 'alwaysRaised,resizable,scrollbars,width=720,height=600');
	route_win.focus();
}
function DeleteEntry(F) {
	if(confirm("Delete the Entry?")){
		F.submit_button.value = "Routing";
		F.change_action.value = "gozila_cgi";
		F.submit_type.value = 'del';
		F.submit();
	}
}
function to_submit(F) {
	if(valid_value(F)){
		F.submit_button.value = "Routing";
		F.action.value = "Apply";
		F.save_button.value = "Saved";
		F.save_button.disabled = true;
		F.submit();
	}
}
function valid_value(F) {
	if(F.wk_mode.value != "ospf") {
		if(!valid_ip(F,"F.route_ipaddr","IP",0))
			return false;
		if(!valid_mask(F,"F.route_netmask",ZERO_OK))
			return false;
		if(!valid_ip(F,"F.route_gateway","Gateway",MASK_NO))
			return false;
	//	if(F.route_ifname.selectedIndex == 0 &&
	//		!valid_ip_gw(F,"F.route_ipaddr","F.route_netmask","F.route_gateway"))
	//		return false;
	}

	return true;
}
function SelRoute(num,F) {
	F.submit_button.value = "Routing";
	F.change_action.value = "gozila_cgi";
	F.route_page.value=F.route_page.options[num].value;
	F.submit();
}
function SelMode(num,F) {
	F.submit_button.value = "Routing";
	F.change_action.value = "gozila_cgi";
	F.wk_mode.value=F.wk_mode.options[num].value;
	F.submit();
}

function exit() {
	closeWin(route_win);
}
</script></head>
   <body class="gui" onunload="exit()"> <% showad(); %>
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
                        <li><a href="Firewall.asp">Security</a></li>
                        <li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
                        <li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
                        <li><a href="Management.asp">Administration</a></li>
                        <li><a href="Status_Router.asp">Status</a></li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="static" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="submit_type" /><input type="hidden" name="change_action" /><input type="hidden" name="action" /><input type="hidden" name="static_route" /><fieldset>
                        <legend>Operating Mode</legend>
                        <div class="setting"><select name="wk_mode" onChange=SelMode(this.form.wk_mode.selectedIndex,this.form)>
                              <option value="gateway" <% nvram_selmatch("wk_mode", "gateway", "selected"); %>>Gateway</option>
                              <option value="router" <% nvram_selmatch("wk_mode", "router", "selected"); %>>RIP2 Router</option>
                              <option value="ospf" <% nvram_selmatch("wk_mode", "ospf", "selected"); %>>OSPF Router</option></select></div>
                     </fieldset><br /><% nvram_selmatch("wk_mode","gateway","<!--"); %><% nvram_selmatch("wk_mode","ospf","<!--"); %>
                     <h2>Dynamic Routing</h2>
                     <div>
                        <div class="setting">
                           <name>RIP</name><select size="1" name="dr_setting">
                              <option value="0" <% nvram_match("dr_setting", "0", "selected"); %>>Disabled</option>
                              <option value="1" <% nvram_match("dr_setting", "1", "selected"); %>>LAN & Wireless</option>
                              <option value="2" <% nvram_match("dr_setting", "2", "selected"); %>>WAN (Internet)</option>
                              <option value="3" <% nvram_match("dr_setting", "3", "selected"); %>>Both</option></select></div>
                     </div><br /><% nvram_selmatch("wk_mode","gateway","-->"); %>
                     <h2>Static Routing</h2>
                     <div>
                        <div class="setting">
                           <div class="label">Select set number</div><select name="route_page" size="1" onChange=SelRoute(this.form.route_page.selectedIndex,this.form)><% static_route_table("select"); %></select><input type="button" value="Delete This Entry" onclick="DeleteEntry(this.form)" /></div>
                        <div class="setting">
                           <div class="label">Enter Route Name</div><input name="route_name" size="25" maxlength="25" onBlur="valid_name(this,'Route Name')" value='<% static_route_setting("name",""); %>' /></div>
                        <div class="setting">
                           <div class="label">Destination LAN IP</div><input type="hidden" name="route_ipaddr" value="4" /><input name="route_ipaddr_0" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("ipaddr","0"); %>' />.<input name="route_ipaddr_1" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("ipaddr","1"); %>' />.<input name="route_ipaddr_2" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("ipaddr","2"); %>' />.<input name="route_ipaddr_3" size="3" maxlength="3" onBlur="valid_range(this,1,254,'IP')" class="num" value='<% static_route_setting("ipaddr","3"); %>' /></div>
                        <div class="setting">
                           <div class="label">Subnet Mask</div><input type="hidden" name="route_netmask" value="4" /><input name="route_netmask_0" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("netmask","0"); %>' />.<input name="route_netmask_1" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("netmask","1"); %>' />.<input name="route_netmask_2" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("netmask","2"); %>' />.<input name="route_netmask_3" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("netmask","3"); %>' /></div>
                        <div class="setting">
                           <div class="label">Default Gateway</div><input type="hidden" name="route_gateway" value="4" /><input name="route_gateway_0" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("gateway","0"); %>' />.<input name="route_gateway_1" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("gateway","1"); %>' />.<input name="route_gateway_2" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" class="num" value='<% static_route_setting("gateway","2"); %>' />.<input name="route_gateway_3" size="3" maxlength="3" onBlur="valid_range(this,1,254,'IP')" class="num" value='<% static_route_setting("gateway","3"); %>' /></div>
                        <div class="setting">
                           <name>Interface</name><select name="route_ifname">
                              <option value="lan" <% static_route_setting("lan","0"); %>>LAN & Wireless</option>
                              <option value="wan" <% static_route_setting("wan","0"); %>>WAN (Internet)</option></select></div>
                     </div><br /><hr />
                     <div class="setting"><input onclick="ViewRoute()" type="button" value="Show Routing Table" name="button2" /></div><% nvram_selmatch("wk_mode","ospf","-->"); %><% nvram_selmatch("wk_mode","gateway","<!--"); %><% nvram_selmatch("wk_mode","router","<!--"); %>
                     <h2>Dynamic Routing</h2>
                     <fieldset>
                        <legend>OSPF</legend>
                        <div class="setting">
                           <div class="label">OSPF Configuration</div><script>
							var bird_ospf = fix_cr('<% nvram_get("bird_ospf"); %>');
							document.getElementById("bird_ospf").value = bird_ospf;
						</script></div>
                     </fieldset><br /><% nvram_selmatch("wk_mode","gateway","-->"); %><% nvram_selmatch("wk_mode","router","-->"); %>
                     <div class="submitFooter"><input type="hidden" value="0" name="Route_reload" /><input type="button" name="save_button" value="Save Settings" onClick="to_submit(this.form)" /><input type="reset" value="Cancel Changes" /></div>
                  </form>
               </div>
            </div>
            <div id="statusInfo">
               <div class="info">Firmware: <% get_firmware_version(); %></div>
               <div class="info">Time: <% get_uptime(); %></div>
			   <% nvram_match("wan_proto","disabled","<!--"); %>
			   <div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
			   <% nvram_match("wan_proto","disabled","-->"); %>
               <div class="info"><% nvram_match("wan_proto","disabled","WAN disabled"); %></div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div><br /><a target="_blank" href="help/HRouting.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>