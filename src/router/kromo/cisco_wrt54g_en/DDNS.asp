<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
   <META http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />

      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Dynamic DNS</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
function ddns_check(F,T) {
	if(F.ddns_enable.value == 0) {
		return true;
	} else if(F.ddns_enable.value == 1) {
		username = eval("F.ddns_username");
		passwd = eval("F.ddns_username");
		hostname = eval("F.ddns_hostname");
		dyndnstype = eval("F.ddns_dyndnstype");
		wildcard = eval("F.ddns_wildcard");
	} else if(F.ddns_enable.value == 2) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_username_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 3) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_username_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	}

	if(username.value == ""){
		alert("You must input a username!");
		username.focus();
		return false;
	}
	if(passwd.value == ""){
		alert("You must input a passwd!");
		passwd.focus();
		return false;
	}
	if(hostname.value == ""){
		alert("You must input a hostname!");
		hostname.focus();
		return false;
	}

	return true;
}
function to_save(F) {
	if(ddns_check(F,"update") == true){
		F.change_action.value = "gozila_cgi";
		F.submit_button.value = "DDNS";
		F.submit_type.value = "save";
		F.action.value = "Apply";
		F.submit();
	}
}
function to_submit(F) {
	if(ddns_check(F,"save") == true) {
		F.submit_button.value = "DDNS";
		F.action.value = "Apply";
		F.save_button.value = "Saved";
		F.save_button.disabled = true;

		F.submit();
	}
}
function SelDDNS(num,F) {
	F.submit_button.value = "DDNS";
	F.change_action.value = "gozila_cgi";
	F.ddns_enable.value=F.ddns_enable.options[num].value;
	F.submit();
}
</script></head>
   <body class="gui"> <% showad(); %>
      <div id="wrapper">
         <div id="content">
            <div id="header">
               <div id="logo">
	    	  <h1><% show_control(); %></h1>
               </div>
               <div id="menu">
                  <div id="menuMain">
                     <ul id="menuMainList">
                        <li class="current"><a class="current" href="index.asp">Setup</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="index.asp">Basic Setup</a></li>
                                 <li><span>DDNS</span></li>
                                 <li><a href="WanMAC.asp">MAC Address Clone</a></li>
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
                  <form name="ddns" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="action" /><input type="hidden" name="change_action" /><input type="hidden" name="submit_type" /><fieldset>
                        <legend>DDNS</legend>
                        <div class="setting">
                           <div class="label">DDNS Service</div>
			   <select name="ddns_enable" onChange=SelDDNS(this.form.ddns_enable.selectedIndex,this.form)>
                              <option value="0" <% nvram_selmatch("ddns_enable", "0", "selected"); %>>Disable</option>
                              <option value="1" <% nvram_selmatch("ddns_enable", "1", "selected"); %>>DynDNS.org</option>
                              <option value="2" <% nvram_selmatch("ddns_enable", "2", "selected"); %>>TZO.com</option>
                              <option value="3" <% nvram_selmatch("ddns_enable", "3", "selected"); %>>ZoneEdit.com</option></select></div><% nvram_selmatch("ddns_enable","1","<!--"); %><% nvram_selmatch("ddns_enable","2","<!--"); %><% nvram_selmatch("ddns_enable","3","<!--"); %><% nvram_selmatch("ddns_enable","1","-->"); %><% nvram_selmatch("ddns_enable","2","-->"); %><% nvram_selmatch("ddns_enable","3","-->"); %><% nvram_selmatch("ddns_enable","0","<!--"); %><% nvram_selmatch("ddns_enable","2","<!--"); %><% nvram_selmatch("ddns_enable","3","<!--"); %>
                        <div class="setting">
                           <div class="label">User Name</div><input name="ddns_username" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'User Name')" value='<% nvram_get("ddns_username"); %>' /></div>
                        <div class="setting">
                           <div class="label">Password</div><input type="password" name="ddns_passwd" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd","","d6nw5v1x2pc7st9m"); %>' /></div>
                        <div class="setting">
                           <div class="label">Host Name</div><input name="ddns_hostname" size="42" maxlength="48" onFocus="check_action(this,0)" onBlur="valid_name(this,'Host Name')" value='<% nvram_get("ddns_hostname"); %>' /></div><% nvram_selmatch("ddns_enable","2","-->"); %><% nvram_selmatch("ddns_enable","3","-->"); %><% nvram_selmatch("ddns_enable","1","<!--"); %><% nvram_selmatch("ddns_enable","3","<!--"); %>
                        <div class="setting">
                           <div class="label">E-mail Address</div><input name="ddns_username_2" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'E-mail Address')" value='<% nvram_get("ddns_username_2"); %>' /></div>
                        <div class="setting">
                           <div class="label">Password</div><input type="password" name="ddns_passwd_2" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd_2","","d6nw5v1x2pc7st9m"); %>' /></div>
                        <div class="setting">
                           <div class="label">Domain Name</div><input name="ddns_hostname_2" size="42" maxlength="48" onFocus="check_action(this,0)" onBlur="valid_name(this,'Domain%20Name')" value='<% nvram_get("ddns_hostname_2"); %>' /></div><% nvram_selmatch("ddns_enable","1","-->"); %><% nvram_selmatch("ddns_enable","3","-->"); %><% nvram_selmatch("ddns_enable","1","<!--"); %><% nvram_selmatch("ddns_enable","2","<!--"); %>
                        <div class="setting">
                           <div class="label">User Name</div><input name="ddns_username_3" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'User Name')" value='<% nvram_get("ddns_username_3"); %>' /></div>
                        <div class="setting">
                           <div class="label">Password</div><input type="password" name="ddns_passwd_3" size="30" maxlength="32" onFocus="check_action(this,0)" onBlur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd_3","","d6nw5v1x2pc7st9m"); %>' /></div>
                        <div class="setting">
                           <div class="label">Host Name</div><input name="ddns_hostname_3" size="42" maxlength="48" onFocus="check_action(this,0)" onBlur="valid_name(this,'Host Name')" value='<% nvram_get("ddns_hostname_3"); %>' /></div><% nvram_selmatch("ddns_enable","1","-->"); %><% nvram_selmatch("ddns_enable","2","-->"); %>
                        <div class="setting">
                           <div class="label">Internet IP Address</div><% show_ddns_ip(); %>
                        </div>
                        <div class="setting">
                           <div class="label">Status</div><% show_ddns_status(); %>
                        </div>
                        <div class="setting">
                           <div class="label">Type</div><select name="ddns_dyndnstype">
                              <option value="1" <% nvram_selmatch("ddns_dyndnstype", "1", "selected"); %>>Dynamic</option>
                              <option value="2" <% nvram_selmatch("ddns_dyndnstype", "2", "selected"); %>>Static</option>
                              <option value="3" <% nvram_selmatch("ddns_dyndnstype", "3", "selected"); %>>Custom</option></select></div>
                        <div class="setting">
                           <div class="label">Wildcard</div><input type="checkbox" value="1" name="ddns_wildcard" <% nvram_selmatch("ddns_wildcard", "1", "checked"); %> /></div><% nvram_selmatch("ddns_enable","0","-->"); %>
                     </fieldset><br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick=to_submit(this.form) /><input type="reset" value="Cancel Changes" /></div>
                  </form>
               </div>
            </div>
            <div id="statusInfo">
               <div class="info">Firmware: <% get_firmware_version(); %></div>
               <div class="info">Time: <% get_uptime(); %></div>
               <div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div>
                  <dl>
                     <dt class="term">DDNS Service: </dt>
                     <dd class="definition">DDNS allows you to access your network using domain names instead of IP addresses. The service manages changing IP address
                        and updates your domain information dynamically. You must sign up for service through TZO.com or DynDNS.org or ZoneEdit.com
                     </dd>
                  </dl><br /><a target="_blank" href="help/HDDNS.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>