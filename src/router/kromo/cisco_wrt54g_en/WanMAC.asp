<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - WanMAC</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "WanMAC";
	F.save_button.value = "Saved";
	F.action.value = "Apply";
	apply(F);
}

function CloneMAC(F) {
	F.submit_button.value = "WanMAC";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "clone_mac";
	F.action.value = "Apply";
	F.submit();
}

function SelMac(num,F) {
	mac_enable_disable(F,num);
}

function mac_enable_disable(F,I) {
	EN_DIS3 = I;
	if ( I == "0" ){
		choose_disable(F.def_hwaddr);
		choose_disable(F.def_hwaddr_0);
		choose_disable(F.def_hwaddr_1);
		choose_disable(F.def_hwaddr_2);
		choose_disable(F.def_hwaddr_3);
		choose_disable(F.def_hwaddr_4);
		choose_disable(F.def_hwaddr_5);
		choose_disable(F.def_whwaddr);
		choose_disable(F.def_whwaddr_0);
		choose_disable(F.def_whwaddr_1);
		choose_disable(F.def_whwaddr_2);
		choose_disable(F.def_whwaddr_3);
		choose_disable(F.def_whwaddr_4);
		choose_disable(F.def_whwaddr_5);
		choose_disable(F.clone_b);
	} else {
		choose_enable(F.def_hwaddr);
		choose_enable(F.def_hwaddr_0);
		choose_enable(F.def_hwaddr_1);
		choose_enable(F.def_hwaddr_2);
		choose_enable(F.def_hwaddr_3);
		choose_enable(F.def_hwaddr_4);
		choose_enable(F.def_hwaddr_5);
		choose_enable(F.def_whwaddr);
		choose_enable(F.def_whwaddr_0);
		choose_enable(F.def_whwaddr_1);
		choose_enable(F.def_whwaddr_2);
		choose_enable(F.def_whwaddr_3);
		choose_enable(F.def_whwaddr_4);
		choose_enable(F.def_whwaddr_5);
		choose_enable(F.clone_b);
	}
}

function init() {
	mac_enable_disable(document.mac,'<% nvram_get("mac_clone_enable"); %>');
	<% onload("MACClone", "document.mac.mac_clone_enable[0].checked = true; mac_enable_disable(document.mac,1);"); %>
}
    </script>
   </head>
   
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
                        <li class="current"><span>Setup</span>
                          <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="index.asp">Basic Setup</a></li>
                                 <li><a href="DDNS.asp">DDNS</a></li>
                                 <li><span>MAC Address Clone</span></li>
                                 <li><a href="Routing.asp">Advanced Routing</a></li><% support_invmatch("HSIAB_SUPPORT", "1", "<!--"); %>
                                 <li><a href="HotSpot_Admin.asp">Hot Spot</a></li><% support_invmatch("HSIAB_SUPPORT", "1", "-->"); %>
                                 <li><a href="Vlan.asp">VLANs</a></li>
                              </ul>
                           </div>
                        </li>
                        <li><a href="Wireless_Basic.asp">Wireless</a></li>
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
                  <form name="mac" action="apply.cgi" method="<% get_http_method(); %>">
                    <input type="hidden" name="submit_button" />
                    <input type="hidden" name="change_action" />
                    <input type="hidden" name="submit_type" />
                    <input type="hidden" name="action" />
                    <h2>MAC Address Clone</h2>
                    <fieldset>
                      <legend>MAC Clone</legend>
                      <div class="setting">
                        <input type="radio" value="1" name="mac_clone_enable" onclick="SelMac(1,this.form)" <% nvram_match("mac_clone_enable", "1", "checked"); %>>Enable</input>
                        <input type="radio" value="0" name="mac_clone_enable" onclick="SelMac(0,this.form)" <% nvram_match("mac_clone_enable", "0", "checked"); %>>Disable</input>
                      </div>
                      <div class="setting">
                        <div class="label">Clone WAN MAC</div>
                        <input type="hidden" name="def_hwaddr" value="6" />
                        <input class="num" size="2" maxlength="2" name="def_hwaddr_0" onblur="valid_mac(this,0)" value='<% get_clone_mac("0"); %>' />:<input class="num" size="2" maxlength="2" name="def_hwaddr_1" onblur="valid_mac(this,1)" value='<% get_clone_mac("1"); %>' />:<input class="num" size="2" maxlength="2" name="def_hwaddr_2" onblur="valid_mac(this,1)" value='<% get_clone_mac("2"); %>' />:<input class="num" size="2" maxlength="2" name="def_hwaddr_3" onblur="valid_mac(this,1)" value='<% get_clone_mac("3"); %>' />:<input class="num" size="2" maxlength="2" name="def_hwaddr_4" onblur="valid_mac(this,1)" value='<% get_clone_mac("4"); %>' />:<input class="num" size="2" maxlength="2" name="def_hwaddr_5" onblur="valid_mac(this,1)" value='<% get_clone_mac("5"); %>' />
                      </div>
                      <div class="setting">
                        <input type="button" name="clone_b" onclick="CloneMAC(this.form)" value="Get Current PC MAC Address" />
                      </div>
                      <hr width="90%">
                      <br/>
                      <div class="setting">
                        <div class="label">Clone Wireless MAC</div>
                        <input type="hidden" name="def_whwaddr" value="6" />
                        <input class="num" size="2" maxlength="2" name="def_whwaddr_0" onblur="valid_mac(this,0)" value='<% get_clone_wmac("0"); %>' />:<input class="num" size="2" maxlength="2" name="def_whwaddr_1" onblur="valid_mac(this,1)" value='<% get_clone_wmac("1"); %>' />:<input class="num" size="2" maxlength="2" name="def_whwaddr_2" onblur="valid_mac(this,1)" value='<% get_clone_wmac("2"); %>' />:<input class="num" size="2" maxlength="2" name="def_whwaddr_3" onblur="valid_mac(this,1)" value='<% get_clone_wmac("3"); %>' />:<input class="num" size="2" maxlength="2" name="def_whwaddr_4" onblur="valid_mac(this,1)" value='<% get_clone_wmac("4"); %>' />:<input class="num" size="2" maxlength="2" name="def_whwaddr_5" onblur="valid_mac(this,1)" value='<% get_clone_wmac("5"); %>' />
                      </div>
                    </fieldset>
                    <br/>
                    <div class="submitFooter">
                      <input type="button" name="save_button" value="Save Settings" onClick="to_submit(this.form)" />
                      <input type="reset" value="Cancel Changes" />
                    </div>
                  </form>
               </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">MAC Address Clone: </dt>
                     <dd class="definition">Some ISP will require you to register your MAC address. If you do not wish to re-register your MAC address, you can have the
                        router clone the MAC address that is registered with your ISP.
                     </dd>
                  </dl><br />
                  <a href="javascript:openHelpWindow('HMAC.asp')">More...</a>
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>