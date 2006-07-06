<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Radius</title>
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
	show_layer_ext('idradius', val == 1);
	setElementsActive("wl_radmactype", "_radius_override", val == "1");
}

addEvent(window, "load", function() {
	setRad("<% nvram_get("wl_radauth"); %>");
	if ("ap" != "<% nvram_get("wl_mode"); %>"){
		setElementsActive("wl_radauth", "_radius_override", false);
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
                        <li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
                        <li class="current"><span><% tran("bmenu.wireless"); %></span>
                          <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Wireless_Basic.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
                                 <li><span><% tran("bmenu.wirelessRadius"); %></span></li>
                                 <li><a href="WL_WPATable.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
                                 <li><a href="Wireless_MAC.asp"><% tran("bmenu.wirelessMac"); %></a></li>
                                 <li><a href="Wireless_Advanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
                                 <li><a href="Wireless_WDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
                              </ul>
                           </div>
                        </li>
                        <% nvram_invmatch("sipgate","1","<!--"); %>
                        <li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
                        <li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
                        <li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
                        <li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
                        <li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
                        <li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
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
                    <h2><% tran("radius.h2"); %></h2>
                    
                    <fieldset>
                      <legend><% tran("radius.legend"); %></legend>
                     <div>
                        <div class="setting">
                           <div class="label"><% tran("radius.label"); %></div>
                           <input class="spaceradio" type="radio" name="wl_radauth" value="1" <% nvram_checked("wl_radauth","1"); %> onclick="setRad(this.value)" ><% tran("share.enable"); %></input>&nbsp;
                           <input class="spaceradio" type="radio" name="wl_radauth" value="0" <% nvram_checked("wl_radauth","0"); %> onclick="setRad(this.value)" ><% tran("share.disable"); %></input>
                        </div>
                        <div id="idradius">
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label2"); %></div>
	                           <select name="wl_radmactype">
	                              <option value="0" <% nvram_selected("wl_radmactype","0"); %>>aabbcc-ddeeff</option>
	                              <option value="1" <% nvram_selected("wl_radmactype","1"); %>>aabbccddeeff</option>
	                              <option value="2" <% nvram_selected("wl_radmactype","2"); %>>aa:bb:cc:dd:ee:ff</option>
	                              <option value="3" <% nvram_selected("wl_radmactype","3"); %>>aa-bb-cc-dd-ee-ff</option>
	                           </select>
	                        </div>
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label3"); %></div>
	                           <input type="hidden" name="wl_radius_ipaddr" value="4" />
	                           <input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_0" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","0"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_1" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","1"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_2" onblur="valid_range(this,0,255,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","2"); %>" />.<input class="num" size="3" maxlength="3" name="wl_radius_ipaddr_3" onblur="valid_range(this,1,254,radius.label3)" value="<% get_single_ip("wl_radius_ipaddr","3"); %>" />
	                        </div>
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label4"); %></div>
	                           <input class="num" size="5" maxlength="5" name="wl_radius_port" onblur="valid_range(this,1,65535,radius.label4)" value="<% nvram_get("wl_radius_port"); %>" />
	                        </div>
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label5"); %></div>
	                           <input class="num" size="5" maxlength="5" name="max_unauth_users" value="<% nvram_get("max_unauth_users"); %>" />
	                        </div>
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label6"); %></div>
	                           <input class="spaceradio" type="radio" name="wl_radmacpassword" value="1" <% nvram_checked("wl_radmacpassword","1"); %> ><% tran("share.share_key"); %></input>&nbsp;
	                           <input class="spaceradio" type="radio" name="wl_radmacpassword" value="0" <% nvram_checked("wl_radmacpassword","0"); %> ><% tran("share.mac"); %></input>
	                        </div>
	                        <div class="setting">
	                           <div class="label"><% tran("radius.label7"); %></div>
	                           <input size="20" maxlength="32" name="wl_radius_key" value="<% nvram_get("wl_radius_key"); %>" />
	                        </div>
	                        <div class="setting">
	                        	<div class="label"><% tran("radius.label8"); %></div>
	                        	<input type="checkbox" name="_radius_override" value="1" <% nvram_checked("radius_override", "1"); %> />
	                        </div>
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
                     <h2><% tran("share.help"); %></h2>
                  </div><br />
                  <a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('Hradauth.asp')"><% tran("share.more"); %></a>
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>