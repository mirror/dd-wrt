<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Radius</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	if (F._radius_override.checked == false){
	    F.radius_override.value = 0;
	}else{
	    F.radius_override.value = 1;
	}

	F.submit_button.value = "Wireless_radauth";
	F.save_button.value = "Saved";
	F.action.value = "Apply";
	apply(F);
}

function setRad(val) {
	setElementsActive("wl_radmactype", "_radius_override", val == "1");
}

addEvent(window, "load", function() {
	setElementsActive("wl_radmactype", "_radius_override", "<% nvram_get("wl_radauth"); %>" == "1");
	if ("ap" != "<% nvram_get("wl_mode"); %>"){
		setElementsActive("wl_radauth", "_radius_override", false);
		alert("Radius is only available in AP mode.");
	}
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
               <div id="menu">
                  <div id="menuMain">
                     <ul id="menuMainList">
                        <li><a href="index.asp">Setup</a></li>
                        <li class="current"><span>Wireless</span>
                          <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Wireless_Basic.asp">Basic Settings</a></li>
                                 <li><span>Radius</span></li>
                                 <li><a href="WL_WPATable.asp">Wireless Security</a></li>
                                 <li><a href="Wireless_MAC.asp">MAC Filter</a></li>
                                 <li><a href="Wireless_Advanced.asp">Advanced Settings</a></li>
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
                  <form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
                    <input type="hidden" name="submit_button" value="Wireless_radauth" />
                    <input type="hidden" name="change_action" />
                    <input type="hidden" name="radius_override" />
                    <input type="hidden" name="action" value="Apply" />
                    <input type="hidden" name="commit" value="1" />
                    <h2>Remote Authentication Dial-In User Service</h2>
                    <fieldset>
                      <legend>Radius</legend>
                     <div>
                        <div class="setting">
                           <div class="label">MAC Radius Client</div>
                           <input type="radio" name="wl_radauth" value="1" <% nvram_match("wl_radauth","1","checked"); %> onclick="setRad(this.value)" >Enable</input>
                           <input type="radio" name="wl_radauth" value="0" <% nvram_match("wl_radauth","0","checked"); %> onclick="setRad(this.value)" >Disable</input>
                        </div>
                        <div class="setting">
                           <div class="label">MAC Format</div>
                           <select name="wl_radmactype">
                              <option value="0" <% nvram_match("wl_radmactype","0","selected"); %>>aabbcc-ddeeff</option>
                              <option value="1" <% nvram_match("wl_radmactype","1","selected"); %>>aabbccddeeff</option>
                              <option value="2" <% nvram_match("wl_radmactype","2","selected"); %>>aa-bb-cc-dd-ee-ff</option>
                           </select>
                        </div>
                        <div class="setting">
                           <div class="label">Radius Server IP</div>
                           <input type="hidden" name="wl_radius_ipaddr" value="4" />
                           <input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_0" onBlur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wl_radius_ipaddr","0"); %>' />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_1" onBlur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wl_radius_ipaddr","1"); %>' />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_2" onBlur="valid_range(this,0,255,'IP')" value='<% get_single_ip("wl_radius_ipaddr","2"); %>' />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_3" onBlur="valid_range(this,1,254,'IP')" value='<% get_single_ip("wl_radius_ipaddr","3"); %>' />
                        </div>
                        <div class="setting">
                           <div class="label">Radius Server Port</div>
                           <input size="5" maxLength="32" name="wl_radius_port" value='<% nvram_get("wl_radius_port"); %>' />
                        </div>
                        <div class="setting">
                           <div class="label">Maximum Unauthenticated Users</div>
                           <input size="5" maxLength="32" name="max_unauth_users" value='<% nvram_get("max_unauth_users"); %>' />
                        </div>
                        <div class="setting">
                           <div class="label">Password Format</div>
                           <input type="radio" name="wl_radmacpassword" value="1" <% nvram_match("wl_radmacpassword","1","checked"); %> >Shared Key</input>
                           <input type="radio" name="wl_radmacpassword" value="0" <% nvram_match("wl_radmacpassword","0","checked"); %> >MAC</input>
                        </div>
                        <div class="setting">
                           <div class="label">RADIUS Shared Secret</div>
                           <input size="20" maxLength="32" name="wl_radius_key" value='<% nvram_get("wl_radius_key"); %>' />
                        </div>
                        <div class="setting">
                        	<div class="label">Override Radius if Server is unavailable</div>
                        	<input type="checkbox" name="_radius_override" value="1" <% nvram_match("radius_override", "1", "checked"); %>/>
                        </div>
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
                  </div><br />
                  <a href="javascript:openHelpWindow('Hradauth.asp')">More...</a>
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>