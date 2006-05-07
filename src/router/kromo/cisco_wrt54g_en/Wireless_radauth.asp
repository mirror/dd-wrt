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
document.title = "<% nvram_get("router_name"); %>" + radius.titl;

function to_submit(F) {
	if (F._radius_override.checked == false){
	    F.radius_override.value = 0;
	}else{
	    F.radius_override.value = 1;
	}

	F.submit_button.value = "Wireless_radauth";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	
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
//		alert("Radius is only available in AP mode.");
		alert(errmsg.err49);
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
                        <li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setup)</script></a></li>
                        <li class="current"><span><script type="text/javascript">Capture(bmenu.wireless)</script></span>
                          <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wirelessBasic)</script></a></li>
                                 <li><span><script type="text/javascript">Capture(bmenu.wirelessRadius)</script></span></li>
                                 <li><a href="WL_WPATable.asp"><script type="text/javascript">Capture(bmenu.wirelessSecurity)</script></a></li>
                                 <li><a href="Wireless_MAC.asp"><script type="text/javascript">Capture(bmenu.wirelessMac)</script></a></li>
                                 <li><a href="Wireless_Advanced.asp"><script type="text/javascript">Capture(bmenu.wirelessAdvanced)</script></a></li>
                                 <li><a href="Wireless_WDS.asp"><script type="text/javascript">Capture(bmenu.wirelessWds)</script></a></li>
                              </ul>
                           </div>
                        </li>
                        <% nvram_invmatch("sipgate","1","<!--"); %>
                        <li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
                        <li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
                        <li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
                        <li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
                        <li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
                        <li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
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
                    <h2><script type="text/javascript">Capture(radius.h2)</script></h2>
                    
                    <fieldset>
                      <legend><script type="text/javascript">Capture(radius.legend)</script></legend>
                     <div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label)</script></div>
                           <input class="spaceradio" type="radio" name="wl_radauth" value="1" <% nvram_checked("wl_radauth","1"); %> onclick="setRad(this.value)" ><script type="text/javascript">Capture(share.enable)</script></input>&nbsp;
                           <input class="spaceradio" type="radio" name="wl_radauth" value="0" <% nvram_checked("wl_radauth","0"); %> onclick="setRad(this.value)" ><script type="text/javascript">Capture(share.disable)</script></input>
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label2)</script></div>
                           <select name="wl_radmactype">
                              <option value="0" <% nvram_selected("wl_radmactype","0"); %>>aabbcc-ddeeff</option>
                              <option value="1" <% nvram_selected("wl_radmactype","1"); %>>aabbccddeeff</option>
                              <option value="2" <% nvram_selected("wl_radmactype","2"); %>>aa-bb-cc-dd-ee-ff</option>
                           </select>
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label3)</script></div>
                           <input type="hidden" name="wl_radius_ipaddr" value="4" />
                           <input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("wl_radius_ipaddr","0"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("wl_radius_ipaddr","1"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("wl_radius_ipaddr","2"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("wl_radius_ipaddr","3"); %>" />
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label4)</script></div>
                           <input class="num" size="5" maxlength="5" name="wl_radius_port" onblur="valid_range(this,1,65535,'Port')" value="<% nvram_get("wl_radius_port"); %>" />
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label5)</script></div>
                           <input class="num" size="5" maxlength="5" name="max_unauth_users" value="<% nvram_get("max_unauth_users"); %>" />
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label6)</script></div>
                           <input class="spaceradio" type="radio" name="wl_radmacpassword" value="1" <% nvram_checked("wl_radmacpassword","1"); %> ><script type="text/javascript">Capture(share.share_key)</script></input>&nbsp;
                           <input class="spaceradio" type="radio" name="wl_radmacpassword" value="0" <% nvram_checked("wl_radmacpassword","0"); %> ><script type="text/javascript">Capture(radius.mac)</script></input>
                        </div>
                        <div class="setting">
                           <div class="label"><script type="text/javascript">Capture(radius.label7)</script></div>
                           <input size="20" maxlength="32" name="wl_radius_key" value="<% nvram_get("wl_radius_key"); %>" />
                        </div>
                        <div class="setting">
                        	<div class="label"><script type="text/javascript">Capture(radius.label8)</script></div>
                        	<input type="checkbox" name="_radius_override" value="1" <% nvram_checked("radius_override", "1"); %> />
                        </div>
                      </div>
                    </fieldset><br/>
                    
                    <div class="submitFooter">
                    	<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onClick=\"to_submit(this.form)\" />")</script>
                    	<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
                    </div>
                  </form>
               </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2><script type="text/javascript">Capture(share.help)</script></h2>
                  </div><br />
                  <a href="javascript:openHelpWindow('Hradauth.asp')"><script type="text/javascript">Capture(share.more)</script></a>
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>