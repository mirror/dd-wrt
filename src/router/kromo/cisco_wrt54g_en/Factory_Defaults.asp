<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Factory Defaults</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
function to_submit(F) {
	if( F.FactoryDefaults[0].checked == 1 ) {
		if(!confirm('Warning! If you click OK, the device will reset to factory default and all previous settings will be erased.')) {
			return false;
		}
			
		F.action.value='Restore';
		F.submit_button.value = "Factory_Defaults";
		F.submit();

       	return true;
	}
	
	F.action.value='Apply';
	F.submit_button.value = "Factory_Defaults";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

	F.submit();

	return true;
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
                        <li><a href="index.asp">Setup</a></li>
                        <li><a href="Wireless_Basic.asp">Wireless</a></li>
			<% nvram_invmatch("sipgate","1","<!--"); %>
			<li><a href="Sipath.asp">SIPatH</a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
                        <li><a href="Firewall.asp">Security</a></li>
                        <li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
                        <li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
                        <li class="current"><a class="current" href="Management.asp">Administration</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Management.asp">Management</a></li>
				 <li><a href="Hotspot.asp">Hotspot</a></li>
				 <li><a href="Services.asp">Services</a></li>
				 <li><a href="Alive.asp">Keep Alive</a></li>
                                 <li><a href="Log.asp">Log</a></li>
                                 <li><a href="Diagnostics.asp">Diagnostics</a></li>
                                 <li><span>Factory Defaults</span></li>
				 <li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Firmware Upgrade</a>","<a href=Upgrade.asp >Firmware Upgrade</a>"); %></li> 
				 <li><% support_elsematch("HTTPS","1","<a onClick=alert('Not&nbsp;available!&nbsp;Please&nbsp;use&nbsp;HTTP&nbsp;mode.')>Backup</a>","<a href=config.asp >Backup</a>"); %></li>
                              </ul>
                           </div>
                        </li>
                        <li><a href="Status_Router.asp">Status</a></li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="default" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="action" /><input type="hidden" name="change_action" /><h2>Factory Defaults</h2>
                     <div class="setting">
                        <name>Restore Factory Defaults</name><input type="radio" name="FactoryDefaults" value="1" /> Yes<input type="radio" name="FactoryDefaults" value="0" checked="checked" /> No
                     </div>
                     <div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick=to_submit(this.form) /><input type="reset" value="Cancel Changes" /></div>
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
                     <dd class="definition">This will reset all settings back to factory defaults. All of your settings will be erased.</dd>
                  </dl><br /><a target="_blank" href="help/HDefault.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>