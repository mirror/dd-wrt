<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
    <title><% nvram_get("router_name"); %> - Wireless</title>
    <link type="text/css" rel="stylesheet" href="style.css" />
    <script type="text/JavaScript" src="common.js">{}</script>
    <script language="JavaScript">
      
function SelSubnet(F,num) {
	if(num==1){
		choose_enable(F.wl_br1_ipaddr0);
		choose_enable(F.wl_br1_ipaddr1);
		choose_enable(F.wl_br1_ipaddr2);
		choose_enable(F.wl_br1_ipaddr3);
		choose_enable(F.wl_br1_netmask0);
		choose_enable(F.wl_br1_netmask1);
		choose_enable(F.wl_br1_netmask2);
		choose_enable(F.wl_br1_netmask3);
		choose_enable(F.wl_br1_nat);
	} else{
		choose_disable(F.wl_br1_ipaddr0);
		choose_disable(F.wl_br1_ipaddr1);
		choose_disable(F.wl_br1_ipaddr2);
		choose_disable(F.wl_br1_ipaddr3);
		choose_disable(F.wl_br1_netmask0);
		choose_disable(F.wl_br1_netmask1);
		choose_disable(F.wl_br1_netmask2);
		choose_disable(F.wl_br1_netmask3);
		choose_disable(F.wl_br1_nat);
	}
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_WDS";
	F.submit_type.value = "save";
 	F.action.value = "Apply";
	F.submit();
}

function SelWDS(index, num, F) {
	if(index==1){
		F.wl_wds1_enable.value=F.wl_wds1_enable.options[num].value;
	} else if(index==2) {
		F.wl_wds2_enable.value=F.wl_wds2_enable.options[num].value;
	} else if(index==3){
		F.wl_wds3_enable.value=F.wl_wds3_enable.options[num].value;
	} else if(index==4){
		F.wl_wds4_enable.value=F.wl_wds4_enable.options[num].value;
	} else if(index==5){
		F.wl_wds5_enable.value=F.wl_wds5_enable.options[num].value;
	} else if(index==6){
		F.wl_wds6_enable.value=F.wl_wds6_enable.options[num].value;
	} else if(index==7){
		F.wl_wds7_enable.value=F.wl_wds7_enable.options[num].value;
	} else if(index==8) {
		F.wl_wds8_enable.value=F.wl_wds8_enable.options[num].value;
	} else if(index==9){
		F.wl_wds9_enable.value=F.wl_wds9_enable.options[num].value;
	} else if(index==10){
		F.wl_wds10_enable.value=F.wl_wds10_enable.options[num].value;
	}
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_WDS";
	F.submit_type.value = "save";
	F.action.value = "Apply";
	F.submit();
}

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.submit_button.value = "Wireless_WDS";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

	F.action.value = "Apply";
	F.submit();
}

function init() {
	if("1" == <% nvram_get("wl_br1_enable"); %>){
		choose_enable(document.forms[0].wl_br1_ipaddr0);
		choose_enable(document.forms[0].wl_br1_ipaddr1);
		choose_enable(document.forms[0].wl_br1_ipaddr2);
		choose_enable(document.forms[0].wl_br1_ipaddr3);
		choose_enable(document.forms[0].wl_br1_netmask0);
		choose_enable(document.forms[0].wl_br1_netmask1);
		choose_enable(document.forms[0].wl_br1_netmask2);
		choose_enable(document.forms[0].wl_br1_netmask3);
		choose_enable(document.forms[0].wl_br1_nat);
	} else {
		choose_disable(document.forms[0].wl_br1_ipaddr0);
		choose_disable(document.forms[0].wl_br1_ipaddr1);
		choose_disable(document.forms[0].wl_br1_ipaddr2);
		choose_disable(document.forms[0].wl_br1_ipaddr3);
		choose_disable(document.forms[0].wl_br1_netmask0);
		choose_disable(document.forms[0].wl_br1_netmask1);
		choose_disable(document.forms[0].wl_br1_netmask2);
		choose_disable(document.forms[0].wl_br1_netmask3);
		choose_disable(document.forms[0].wl_br1_nat);
	}

	if ("ap" != "<% nvram_get("wl_mode"); %>"){
	  alert("WDS is only available in AP mode!");
	}
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
                        <li><a href="index.asp">Setup</a></li>
                        <li class="current"><span>Wireless</span>
                          <div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="Wireless_Basic.asp">Basic Settings</a></li>
                                 <li><a href="Wireless_radauth.asp">Radius</a></li>
                                 <li><a href="WL_WPATable.asp">Wireless Security</a></li>
                                 <li><a href="Wireless_MAC.asp">MAC Filter</a></li>
                                 <li><a href="Wireless_Advanced.asp">Advanced Settings</a></li>
                                 <li><span>WDS</span></li>
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
                  <form name="wds" action="apply.cgi" method="<% get_http_method(); %>">
		  <input type="hidden" name="submit_button" value="Wireless_WDS" />
		  <input type="hidden" name="change_action">
		  <input type="hidden" name="submit_type" />
		  <input type="hidden" name="commit" value="1" />
		  <input type="hidden" name="action" value="Apply" />
		  <h2>Wireless Distribution System</h2>
                     <div>
                      <fieldset>
                        <legend>WDS Settings</legend>
                        <div class="setting">
                           <div class="label">Wireless MAC</div><% nvram_get("wl0_hwaddr"); %>
                        </div>
                        
                        <div class="setting">
                          <select name="wl_wds1_enable" size="1" onChange="SelWDS(1,this.form.wl_wds1_enable.selectedIndex,this.form)">
                            <option value="0" <% nvram_selmatch("wl_wds1_enable", "0", "selected"); %>>Disable</option>
                            <option value="1" <% nvram_selmatch("wl_wds1_enable", "1", "selected"); %>>Point to Point</option>
                            <% show_wds_subnet(1); %>
                            <option value="3" <% nvram_selmatch("wl_wds1_enable", "3", "selected"); %>>LAN</option>
                          </select>
                          <input type="hidden" name="wl_wds1_hwaddr" value="6" />
                          <input class="num" name="wl_wds1_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("1", "0"); %>' />:
                          <input class="num" name="wl_wds1_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("1", "1"); %>' />:
                          <input class="num" name="wl_wds1_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("1", "2"); %>' />:
                          <input class="num" name="wl_wds1_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("1", "3"); %>' />:
                          <input class="num" name="wl_wds1_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("1", "4"); %>' />:
                          <input class="num" name="wl_wds1_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("1", "5"); %>' />
                          <input name="wl_wds1_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds1_desc"); %>' />
                          <% get_wdsp2p(1); %>
                        </div>
                        
                        <div class="setting">
                          <select name="wl_wds2_enable" size="1" onChange="SelWDS(2,this.form.wl_wds2_enable.selectedIndex,this.form)">
                            <option value="0" <% nvram_selmatch("wl_wds2_enable", "0", "selected"); %>>Disable</option>
                            <option value="1" <% nvram_selmatch("wl_wds2_enable", "1", "selected"); %>>Point to Point</option>
                            <% show_wds_subnet(2); %>
                            <option value="3" <% nvram_selmatch("wl_wds2_enable", "3", "selected"); %>>LAN</option>
                          </select>
                          <input type="hidden" name="wl_wds2_hwaddr" value="6" />
                          <input class="num" name="wl_wds2_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("2", "0"); %>' />:
                          <input class="num" name="wl_wds2_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("2", "1"); %>' />:
                          <input class="num" name="wl_wds2_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("2", "2"); %>' />:
                          <input class="num" name="wl_wds2_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("2", "3"); %>' />:
                          <input class="num" name="wl_wds2_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("2", "4"); %>' />:
                          <input class="num" name="wl_wds2_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("2", "5"); %>' />
                          <input name="wl_wds2_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds2_desc"); %>' />
                          <% get_wdsp2p(2); %>

                          </div>
                          <div class="setting">
                            <select name="wl_wds3_enable" size="1" onChange="SelWDS(3,this.form.wl_wds3_enable.selectedIndex,this.form)">
                              <option value="0" <% nvram_selmatch("wl_wds3_enable", "0", "selected"); %>>Disable</option>
                              <option value="1" <% nvram_selmatch("wl_wds3_enable", "1", "selected"); %>>Point to Point</option>
                              <% show_wds_subnet(3); %>
                              <option value="3" <% nvram_selmatch("wl_wds3_enable", "3", "selected"); %>>LAN</option>
                            </select>
                            <input type="hidden" name="wl_wds3_hwaddr" value="6" />
                            <input class="num" name="wl_wds3_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("3", "0"); %>' />:
                            <input class="num" name="wl_wds3_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("3", "1"); %>' />:
                            <input class="num" name="wl_wds3_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("3", "2"); %>' />:
                            <input class="num" name="wl_wds3_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("3", "3"); %>' />:
                            <input class="num" name="wl_wds3_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("3", "4"); %>' />:
                            <input class="num" name="wl_wds3_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("3", "5"); %>' />
                            <input name="wl_wds3_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds3_desc"); %>' />
                            <% get_wdsp2p(3); %>
                          </div>
                          
                          <div class="setting">
                            <select name="wl_wds4_enable" size="1" onChange="SelWDS(4,this.form.wl_wds4_enable.selectedIndex,this.form)">
                              <option value="0" <% nvram_selmatch("wl_wds4_enable", "0", "selected"); %>>Disable</option>
                              <option value="1" <% nvram_selmatch("wl_wds4_enable", "1", "selected"); %>>Point to Point</option>
                              <% show_wds_subnet(4); %>
                              <option value="3" <% nvram_selmatch("wl_wds4_enable", "3", "selected"); %>>LAN</option>
                            </select>
                            <input type="hidden" name="wl_wds4_hwaddr" value="6" />
                            <input class="num" name="wl_wds4_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("4", "0"); %>' />:
                            <input class="num" name="wl_wds4_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("4", "1"); %>' />:
                            <input class="num" name="wl_wds4_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("4", "2"); %>' />:
                            <input class="num" name="wl_wds4_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("4", "3"); %>' />:
                            <input class="num" name="wl_wds4_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("4", "4"); %>' />:
                            <input class="num" name="wl_wds4_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("4", "5"); %>' />
                            <input name="wl_wds4_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds4_desc"); %>' />
                            <% get_wdsp2p(4); %>
                            
                            </div>
                            <div class="setting">
                              <select name="wl_wds5_enable" size="1" onChange="SelWDS(5,this.form.wl_wds5_enable.selectedIndex,this.form)">
                                <option value="0" <% nvram_selmatch("wl_wds5_enable", "0", "selected"); %>>Disable</option>
                                <option value="1" <% nvram_selmatch("wl_wds5_enable", "1", "selected"); %>>Point to Point</option>
                                <% show_wds_subnet(5); %>
                                <option value="3" <% nvram_selmatch("wl_wds5_enable", "3", "selected"); %>>LAN</option>
                              </select>
                              <input type="hidden" name="wl_wds5_hwaddr" value="6" />
                              <input class="num" name="wl_wds5_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("5", "0"); %>' />:
                              <input class="num" name="wl_wds5_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("5", "1"); %>' />:
                              <input class="num" name="wl_wds5_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("5", "2"); %>' />:
                              <input class="num" name="wl_wds5_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("5", "3"); %>' />:
                              <input class="num" name="wl_wds5_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("5", "4"); %>' />:
                              <input class="num" name="wl_wds5_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("5", "5"); %>' />
                              <input name="wl_wds5_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds5_desc"); %>' />
                              <% get_wdsp2p(5); %>
                            </div>
                            
                            <div class="setting">
                              <select name="wl_wds6_enable" size="1" onChange="SelWDS(6,this.form.wl_wds6_enable.selectedIndex,this.form)">
                                <option value="0" <% nvram_selmatch("wl_wds6_enable", "0", "selected"); %>>Disable</option>
                                <option value="1" <% nvram_selmatch("wl_wds6_enable", "1", "selected"); %>>Point to Point</option>
                                <% show_wds_subnet(6); %>
                                <option value="3" <% nvram_selmatch("wl_wds6_enable", "3", "selected"); %>>LAN</option>
                              </select>
                              <input type="hidden" name="wl_wds6_hwaddr" value="6" />
                              <input class="num" name="wl_wds6_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("6", "0"); %>' />:
                              <input class="num" name="wl_wds6_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("6", "1"); %>' />:
                              <input class="num" name="wl_wds6_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("6", "2"); %>' />:
                              <input class="num" name="wl_wds6_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("6", "3"); %>' />:
                              <input class="num" name="wl_wds6_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("6", "4"); %>' />:
                              <input class="num" name="wl_wds6_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("6", "5"); %>' />
                              <input name="wl_wds6_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds6_desc"); %>' />
                              <% get_wdsp2p(6); %>
                            </div>
                            
                            <div class="setting">
                              <select name="wl_wds7_enable" size="1" onChange="SelWDS(7,this.form.wl_wds7_enable.selectedIndex,this.form)">
                                <option value="0" <% nvram_selmatch("wl_wds7_enable", "0", "selected"); %>>Disable</option>
                                <option value="1" <% nvram_selmatch("wl_wds7_enable", "1", "selected"); %>>Point to Point</option>
                                <% show_wds_subnet(7); %>
                                <option value="3" <% nvram_selmatch("wl_wds7_enable", "3", "selected"); %>>LAN</option>
                              </select>
                              <input type="hidden" name="wl_wds7_hwaddr" value="6" />
                   			      <input class="num" name="wl_wds7_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("7", "0"); %>' />:
                   			      <input class="num" name="wl_wds7_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("7", "1"); %>' />:
                   			      <input class="num" name="wl_wds7_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("7", "2"); %>' />:
                   			      <input class="num" name="wl_wds7_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("7", "3"); %>' />:
                   			      <input class="num" name="wl_wds7_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("7", "4"); %>' />:
                   			      <input class="num" name="wl_wds7_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("7", "5"); %>' />
                   			      <input name="wl_wds7_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds7_desc"); %>' />
                   			      <% get_wdsp2p(7); %>
                   			    </div>
                   			    
                   			    <div class="setting">
                   			      <select name="wl_wds8_enable" size="1" onChange="SelWDS(8,this.form.wl_wds8_enable.selectedIndex,this.form)">
                   			        <option value="0" <% nvram_selmatch("wl_wds8_enable", "0", "selected"); %>>Disable</option>
                   			        <option value="1" <% nvram_selmatch("wl_wds8_enable", "1", "selected"); %>>Point to Point</option>
                   			        <% show_wds_subnet(8); %>
                   			        <option value="3" <% nvram_selmatch("wl_wds8_enable", "3", "selected"); %>>LAN</option>
                   			      </select>
                              <input type="hidden" name="wl_wds8_hwaddr" value="6" />
                              <input class="num" name="wl_wds8_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("8", "0"); %>' />:
                              <input class="num" name="wl_wds8_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("8", "1"); %>' />:
                              <input class="num" name="wl_wds8_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("8", "2"); %>' />:
                              <input class="num" name="wl_wds8_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("8", "3"); %>' />:
                              <input class="num" name="wl_wds8_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("8", "4"); %>' />:
                              <input class="num" name="wl_wds8_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("8", "5"); %>' />
                              <input name="wl_wds8_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds8_desc"); %>' />
                              <% get_wdsp2p(8); %>
                            </div>
                            
                            <div class="setting">
                              <select name="wl_wds9_enable" size="1" onChange="SelWDS(9,this.form.wl_wds9_enable.selectedIndex,this.form)">
                                <option value="0" <% nvram_selmatch("wl_wds9_enable", "0", "selected"); %>>Disable</option>
                                <option value="1" <% nvram_selmatch("wl_wds9_enable", "1", "selected"); %>>Point to Point</option>
                                <% show_wds_subnet(9); %>
                                <option value="3" <% nvram_selmatch("wl_wds9_enable", "3", "selected"); %>>LAN</option>
                              </select>
			                        <input type="hidden" name="wl_wds9_hwaddr" value="6" />
			                        <input class="num" name="wl_wds9_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("9", "0"); %>' />:
			                        <input class="num" name="wl_wds9_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("9", "1"); %>' />:
			                        <input class="num" name="wl_wds9_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("9", "2"); %>' />:
			                        <input class="num" name="wl_wds9_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("9", "3"); %>' />:
			                        <input class="num" name="wl_wds9_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("9", "4"); %>' />:
			                        <input class="num" name="wl_wds9_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("9", "5"); %>' />
			                        <input name="wl_wds9_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds9_desc"); %>' />
			                        <% get_wdsp2p(9); %>
                         </div>
                         
                          <div class="setting">
                            <select name="wl_wds10_enable" size="1" onChange="SelWDS(10,this.form.wl_wds10_enable.selectedIndex,this.form)">
                              <option value="0" <% nvram_selmatch("wl_wds10_enable", "0", "selected"); %>>Disable</option>
                              <option value="1" <% nvram_selmatch("wl_wds10_enable", "1", "selected"); %>>Point to Point</option>
                              <% show_wds_subnet(10); %>
                              <option value="3" <% nvram_selmatch("wl_wds10_enable", "3", "selected"); %>>LAN</option>
                            </select>
			                      <input type="hidden" name="wl_wds10_hwaddr" value="6" />
			                      <input class="num" name="wl_wds10_hwaddr0" size="2" maxlength="2" onBlur="valid_mac(this,0)" value='<% get_wds_mac("10", "0"); %>' />:
			                      <input class="num" name="wl_wds10_hwaddr1" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("10", "1"); %>' />:
			                      <input class="num" name="wl_wds10_hwaddr2" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("10", "2"); %>' />:
			                      <input class="num" name="wl_wds10_hwaddr3" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("10", "3"); %>' />:
			                      <input class="num" name="wl_wds10_hwaddr4" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("10", "4"); %>' />:
			                      <input class="num" name="wl_wds10_hwaddr5" size="2" maxlength="2" onBlur="valid_mac(this,1)" value='<% get_wds_mac("10", "5"); %>' />
			                      <input name="wl_wds10_desc" size="40" maxlength="39" value='<% nvram_get("wl_wds10_desc"); %>' />
			                      <% get_wdsp2p(10); %>
                        </div>
                        
                        </fieldset>
                        <br/>
                        <fieldset>
                          <legend>Extra Options</legend>
                        <div class="setting">
                           <div class="label">Lazy WDS</div><input type="radio" name="wl_lazywds" value="1" <% nvram_match("wl_lazywds", "1", "checked"); %>>Enable</input><input type="radio" name="wl_lazywds" value="0" <% nvram_match("wl_lazywds", "0", "checked"); %>>Disable</input> (Default: Disable)
                        </div>
                        <div class="setting">
                           <div class="label">WDS Subnet</div><input type="radio" name="wl_br1_enable" value="1" OnClick="SelSubnet(this.form,1)" <% nvram_match("wl_br1_enable", "1", "checked"); %>>Enable</input><input type="radio" name="wl_br1_enable" value="0" OnClick="SelSubnet(this.form,0)" <% nvram_match("wl_br1_enable", "0", "checked"); %>>Disable</input></div>
                        <div class="setting">
                           <div class="label">NAT</div><select name="wl_br1_nat">
                              <option value='<% nvram_match("wl_br1_nat", "0", "selected"); %>'>Disable</option>
                              <option value='<% nvram_match("wl_br1_nat", "1", "selected"); %>'>wLAN->WDS</option>
                              <option value='<% nvram_match("wl_br1_nat", "2", "selected"); %>'>WDS->wLAN</option></select></div>
                        <div class="setting">
                           <div class="label">IP Address</div><input type="hidden" name="wl_br1_ipaddr" value="4" /><input class="num" name="wl_br1_ipaddr0" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_ip("0"); %>' />.<input class="num" name="wl_br1_ipaddr1" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_ip("1"); %>' />.<input class="num" name="wl_br1_ipaddr2" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_ip("2"); %>' />.<input class="num" name="wl_br1_ipaddr3" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_ip("3"); %>' /></div>
                        <div class="setting">
                           <div class="label">Subnet Mask</div><input type="hidden" name="wl_br1_netmask" value="4" /><input class="num" name="wl_br1_netmask0" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_netmask("0"); %>' />.<input class="num" name="wl_br1_netmask1" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_netmask("1"); %>' />.<input class="num" name="wl_br1_netmask2" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_netmask("2"); %>' />.<input class="num" name="wl_br1_netmask3" size="3" maxlength="3" onBlur="valid_range(this,0,255,'IP')" value='<% get_br1_netmask("3"); %>' /></div>
                     </fieldset>
                     </div>
                     <br/>
                     <div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick="to_submit(this.form)" /><input type="reset" value="Cancel Changes" /></div>
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
                  </div><br /><a target="_blank" href="help/HWDS.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>