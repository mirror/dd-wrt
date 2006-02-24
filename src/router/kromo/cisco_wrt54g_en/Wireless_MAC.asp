<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Wireless MAC Filter</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script type="text/javascript" language="JavaScript">
var win_options = 'alwaysRaised,resizable,scrollbars,width=660,height=460' ;

var wl_filter_win = null;
var EN_DIS =  '<% nvram_get("wl_macmode"); %>'
function closeWin(win_var) {
        if ( ((win_var != null) && (win_var.close)) || ((win_var != null) && (win_var.closed==false)) )
                win_var.close();
	
}

function ViewFilter() {
	wl_filter_win = self.open('WL_FilterTable.asp','FilterTable','alwaysRaised,resizable,scrollbars,width=600,height=530');
	wl_filter_win.focus();
}

function to_submit(F) {
	F.submit_button.value = "Wireless_MAC";
	F.change_action.value = "apply_cgi";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function SelMac(num,F) {
	F.submit_button.value = "Wireless_MAC";
	F.change_action.value = "gozila_cgi";
	F.wl_macmode1.value = F.wl_macmode1.value;
	F.submit();
}

function exit() {
	closeWin(wl_filter_win);
}
</script></head>
   <body class="gui" onunload=exit();> <% showad(); %>
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
                        <li class="current"><a class="current" href="Wireless_Basic.asp">Wireless</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Wireless_Basic.asp">Basic Settings</a></li>
                                 <li><a href="Wireless_radauth.asp">Radius</a></li>
                                 <li><a href="WL_WPATable.asp">Wireless Security</a></li>
                                 <li><span>MAC Filter</span></li>
                                 <li><a href="Wireless_Advanced.asp">Advanced Settings</a></li>
                                 <li><a href="Wireless_WDS.asp">WDS</a></li>
                              </ul>
                           </div>
                        </li>
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
                  <form name="wireless" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="change_action" /><input type="hidden" name="action" /><h2>Wireless MAC Filter</h2>
                     <div>
                        <div class="setting">
			<input type="radio" value="other" name="wl_macmode1" <% nvram_match("wl_macmode1","other","checked"); %>>Enable</input>
			<input type="radio" value="disabled" name="wl_macmode1" <% nvram_match("wl_macmode1","disabled","checked"); %>>Disable</input>
			</div><% nvram_match("wl_macmode1","disabled","<!--"); %>
                        <div class="setting">
                           <div class="label">Prevent</div><input type="radio" value="deny" name="wl_macmode"  <% nvram_invmatch("wl_macmode","allow","checked"); %>>Prevent PCs listed from accessing the wireless network</input></div>
                        <div class="setting">
                           <div class="label">Permit only</div>
			   <input type="radio" value="allow" name="wl_macmode"  <% nvram_match("wl_macmode","allow","checked"); %>>Permit only PCs listed to access the wireless network</input></div><input type="hidden" value="0" name="login_status" /><div class="setting">
			   <div class="label"> </div><input type="button" name="mac_filter_button" value="Edit MAC Filter List" onclick=ViewFilter() /></div><% nvram_match("wl_macmode1","disabled","-->"); %>
                     </div><br />
		     <div class="submitFooter">
		     <input type=button name="save_button" value="Save Settings" onClick=to_submit(this.form) />
		     <input type=button value="Cancel Changes" name=cancel onclick=window.location.reload() />
		     </div>
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
                  </div><br /><a target="_blank" href="help/HWirelessMAC.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>