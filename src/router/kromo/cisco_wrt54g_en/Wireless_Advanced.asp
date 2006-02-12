<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Wireless</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
var win_options = 'alwaysRaised,resizable,scrollbars,width=660,height=460' ;

var wl_filter_win = null;
var EN_DIS = '<% nvram_get("wl_macmode"); %>'

function to_submit(F) {
	F.submit_button.value = "Wireless_Advanced";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function SelWME(num,F)
{
	wme_enable_disable(F,num);
}
function wme_enable_disable(F,I)
{
	var start = '';
	var end = '';
	var total = F.elements.length;
	for(i=0 ; i < total ; i++){
                if(F.elements[i].name == "wl_wme_no_ack")  start = i;
                if(F.elements[i].name == "wl_wme_sta_vo5")  end = i;
        }
        if(start == '' || end == '')    return true;

	if( I == "0" || I == "off") {
		EN_DIS = 0;
		for(i = start; i<=end ;i++)
                        choose_disable(F.elements[i]);
	}
	else {
		EN_DIS = 1;
                for(i = start; i<=end ;i++)
                        choose_enable(F.elements[i]);
	}
}
function init()
{
	wme_enable_disable(document.wireless, '<% nvram_get("wl_wme"); %>');
}

</script></head>
   <body class="gui" onload=init()> <% showad(); %>
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
                                 <li><a href="Wireless_MAC.asp">MAC Filter</a></li>
                                 <li><span>Advanced Settings</span></li>
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
                  <form name="wireless" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="commit" value="1" /><input type="hidden" name="action" /><h2>Advanced Wireless</h2>
                     <div>
                        <div class="setting">
                           <div class="label">Authentication Type</div><select name="wl_auth">
                              <option value="0" <% nvram_selmatch("wl_auth", "0", "selected"); %>>Auto</option>
                              <option value="1" <% nvram_selmatch("wl_auth", "1", "selected"); %>>Shared Key</option></select> (Default: Auto)
                        </div>
                        <div class="setting">
                           <div class="label">Basic Rate</div><select name="wl_rateset">
                              <option value="12" <% nvram_selmatch("wl_rateset", "12", "selected"); %>>1-2 Mbps</option>
                              <option value="default" <% nvram_selmatch("wl_rateset", "default", "selected"); %>>Default</option>
                              <option value="all" <% nvram_selmatch("wl_rateset", "all", "selected"); %>>All</option></select> (Default: Default)
                        </div>
                        <div class="setting">
                           <div class="label">Transmission Rate</div><select name="wl_rate">
                              <option value="1000000" <% nvram_selmatch("wl_rate", "1000000", "selected"); %>>1 Mbps</option>
                              <option value="2000000" <% nvram_selmatch("wl_rate", "2000000", "selected"); %>>2 Mbps</option>
                              <option value="5500000" <% nvram_selmatch("wl_rate", "5500000", "selected"); %>>5.5 Mbps</option>
                              <option value="6000000" <% nvram_selmatch("wl_rate", "6000000", "selected"); %>>6 Mbps</option>
                              <option value="9000000" <% nvram_selmatch("wl_rate", "9000000", "selected"); %>>9 Mbps</option>
                              <option value="11000000" <% nvram_selmatch("wl_rate", "11000000", "selected"); %>>11 Mbps</option>
                              <option value="12000000" <% nvram_selmatch("wl_rate", "12000000", "selected"); %>>12 Mbps</option>
                              <option value="18000000" <% nvram_selmatch("wl_rate", "18000000", "selected"); %>>18 Mbps</option>
                              <option value="24000000" <% nvram_selmatch("wl_rate", "24000000", "selected"); %>>24 Mbps</option>
                              <option value="36000000" <% nvram_selmatch("wl_rate", "36000000", "selected"); %>>36 Mbps</option>
                              <option value="48000000" <% nvram_selmatch("wl_rate", "48000000", "selected"); %>>48 Mbps</option>
                              <option value="54000000" <% nvram_selmatch("wl_rate", "54000000", "selected"); %>>54 Mbps</option>
                              <option value="0" <% nvram_selmatch("wl_rate", "0", "selected"); %>>Auto</option></select> (Default: Auto)
                        </div>
                        <div class="setting">
                           <div class="label">CTS Protection Mode</div><select name="wl_gmode_protection">
                              <option value="off" <% nvram_selmatch("wl_gmode_protection", "off", "selected"); %>>Disable</option>
                              <option value="auto" <% nvram_selmatch("wl_gmode_protection", "auto", "selected"); %>>Auto</option></select> (Default: Disable)
                        </div>
                        <div class="setting">
                           <div class="label">Frame Burst</div><select name="wl_frameburst">
                              <option value="off" <% nvram_selmatch("wl_frameburst", "off", "selected"); %>>Disable</option>
                              <option value="on" <% nvram_selmatch("wl_frameburst", "on", "selected"); %>>Enable</option></select> (Default: Disable)
                        </div>
                        <div class="setting">
                           <div class="label">Beacon Interval</div><input class="num" name="wl_bcn" size="6" maxLength="5" onBlur="valid_range(this,1,65535,'Beacon Interval')" value='<% nvram_selget("wl_bcn"); %>' /> (Default: 100ms, Range: 1 - 65535)
                        </div>
                        <div class="setting">
                           <div class="label">DTIM Interval</div><input class="num" name="wl_dtim" size="6" maxLength="3" onBlur="valid_range(this,1,255,'DTIM Interval')" value='<% nvram_selget("wl_dtim"); %>' /> (Default: <% get_wl_value("default_dtim"); %>, Range: 1 - 255)
                        </div>
                        <div class="setting">
                           <div class="label">Fragmentation Threshold</div><input class="num" name="wl_frag" size="6" maxLength="4" onBlur="valid_range(this,256,2346,'Fragmentation Threshold')" value='<% nvram_selget("wl_frag"); %>' /> (Default: 2346, Range: 256 - 2346)
                        </div>
                        <div class="setting">
                           <div class="label">RTS Threshold</div><input class="num" name="wl_rts" size="6" maxLength="4" onBlur="valid_range(this,0,2347,'RTS Threshold')" value='<% nvram_selget("wl_rts"); %>' /> (Default: 2347, Range: 0 - 2347)
                        </div>
			<div class="setting">
                           <div class="label">Max Associated Clients</div><input class="num" name="wl_maxassoc" size="6" maxLength="4" onBlur="valid_range(this,1,256,'Maximum Associated Clients')" value='<% nvram_selget("wl_maxassoc"); %>' /> (Default: 128, Range: 1 - 256)
                        </div>
                        
                        <div class="setting">
                           <div class="label">AP Isolation</div><select name="wl_ap_isolate">
                              <option value="0" <% nvram_selmatch("wl_ap_isolate", "0", "selected"); %>>Off</option>
                              <option value="1" <% nvram_selmatch("wl_ap_isolate", "1", "selected"); %>>On</option></select> (Default: Off)
                        </div>
                        <div class="setting">
                           <div class="label">TX Antenna</div><select name="txant">
                              <option value="0" <% nvram_selmatch("txant", "0", "selected"); %>>Right</option>
                              <option value="1" <% nvram_selmatch("txant", "1", "selected"); %>>Left</option>
                              <option value="3" <% nvram_selmatch("txant", "3", "selected"); %>>Auto</option></select> (Default: Auto)
                        </div>
                        <div class="setting">
                           <div class="label">Preamble</div><select name="wl_plcphdr">
                              <option value="long" <% nvram_selmatch("wl_plcphdr", "long", "selected"); %>>Long</option>
                              <option value="short" <% nvram_selmatch("wl_plcphdr", "short", "selected"); %>>Short</option>
                              <option value="auto" <% nvram_selmatch("wl_plcphdr", "auto", "selected"); %>>Auto</option></select> (Default: Long)
                        </div>
                        <div class="setting">
                           <div class="label">RX Antenna</div><select name="wl_antdiv">
                              <option value="0" <% nvram_selmatch("wl_antdiv", "0", "selected"); %>>Right</option>
                              <option value="1" <% nvram_selmatch("wl_antdiv", "1", "selected"); %>>Left</option>
                              <option value="3" <% nvram_selmatch("wl_antdiv", "3", "selected"); %>>Auto</option></select> (Default: Auto)
                        </div>
                        <div class="setting">
                           <div class="label">Xmit Power</div><input class="num" name="txpwr" size="6" maxLength="3" onBlur="valid_range(this,0,251,'TX Power')" value='<% nvram_selget("txpwr"); %>' /> (Default: 28, Range: 0 - 251mW)
                        </div>
                        <div class="setting">
                           <div class="label">Afterburner</div><select name="wl_afterburner">
                              <option value="off" <% nvram_selmatch("wl_afterburner", "off", "selected"); %>>Off</option>
                              <option value="on" <% nvram_selmatch("wl_afterburner", "on", "selected"); %>>On</option>
                              <option value="auto" <% nvram_selmatch("wl_afterburner", "auto", "selected"); %>>Auto</option></select> (Default: Off)
                        </div>
                        <div class="setting">
                           <div class="label">Wireless GUI Access</div><select name="web_wl_filter">
                              <option value="1" <% nvram_selmatch("web_wl_filter", "1", "selected"); %>>Off</option>
                              <option value="0" <% nvram_selmatch("web_wl_filter", "0", "selected"); %>>On</option></select> (Default: On)
                        </div>
			
			<div class="setting">
			    <div class="label">WMM Support</div><select name="wl_wme" onChange=SelWME(this.form.wl_wme.selectedIndex,this.form)>
			    <option value="off" <% nvram_selmatch("wl_wme", "off", "selected"); %>>Off</option>
			    <option value="on" <% nvram_selmatch("wl_wme", "on", "selected"); %>>On</option></select> (Default: Off)
			</div>
			<div class="setting">
			    <div class="label">No-Acknowledgement</div>
			    <SELECT name="wl_wme_no_ack"> 
			    <option value="off" <% nvram_selmatch("wl_wme_no_ack", "off", "selected"); %>>Off</option>
			    <option value="on" <% nvram_selmatch("wl_wme_no_ack", "on", "selected"); %>>On</option></SELECT> (Default: Off)
			</div>
			<div class="setting">
			    <div class="label">EDCA AP Parameters</div>
			    CWmin&nbsp;&nbsp;CWmax&nbsp;&nbsp;&nbsp;AIFSN&nbsp;&nbsp;TXOP(b)&nbsp;&nbsp;TXOP(a/g)&nbsp;Admin Forced
			    <div class="setting">
				<div class="label">AC_BK</div>
				<input type="hidden" name="wl_wme_ap_bk" value="5">
				<input class=num name="wl_wme_ap_bk0" value="<% nvram_list("wl_wme_ap_bk", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_ap_bk1" value="<% nvram_list("wl_wme_ap_bk", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_ap_bk2" value="<% nvram_list("wl_wme_ap_bk", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_ap_bk3" value="<% nvram_list("wl_wme_ap_bk", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_ap_bk4" value="<% nvram_list("wl_wme_ap_bk", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_ap_bk5">
        			<option value="off" <% wme_match_op("wl_wme_ap_bk", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_ap_bk", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
			    	<div class="label">AC_BE</div>
				<input type="hidden" name="wl_wme_ap_be" value="5">
				<input class=num name="wl_wme_ap_be0" value="<% nvram_list("wl_wme_ap_be", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_ap_be1" value="<% nvram_list("wl_wme_ap_be", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_ap_be2" value="<% nvram_list("wl_wme_ap_be", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_ap_be3" value="<% nvram_list("wl_wme_ap_be", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_ap_be4" value="<% nvram_list("wl_wme_ap_be", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_ap_be5">
        			<option value="off" <% wme_match_op("wl_wme_ap_be", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_ap_be", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
				<div class="label">AC_VI</div>
				<input type="hidden" name="wl_wme_ap_vi" value="5">
				<input class=num name="wl_wme_ap_vi0" value="<% nvram_list("wl_wme_ap_vi", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_ap_vi1" value="<% nvram_list("wl_wme_ap_vi", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_ap_vi2" value="<% nvram_list("wl_wme_ap_vi", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_ap_vi3" value="<% nvram_list("wl_wme_ap_vi", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_ap_vi4" value="<% nvram_list("wl_wme_ap_vi", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_ap_vi5">
        			<option value="off" <% wme_match_op("wl_wme_ap_vi", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_ap_vi", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
				<div class="label">AC_VO</div>
				<input type="hidden" name="wl_wme_ap_vo" value="5">
				<input class=num name="wl_wme_ap_vo0" value="<% nvram_list("wl_wme_ap_vo", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_ap_vo1" value="<% nvram_list("wl_wme_ap_vo", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_ap_vo2" value="<% nvram_list("wl_wme_ap_vo", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_ap_vo3" value="<% nvram_list("wl_wme_ap_vo", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_ap_vo4" value="<% nvram_list("wl_wme_ap_vo", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_ap_vo5">
        			<option value="off" <% wme_match_op("wl_wme_ap_vo", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_ap_vo", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			</div>
			<div class="setting">
			    <div class="label">EDCA STA Parameters</div>
			    CWmin&nbsp;&nbsp;CWmax&nbsp;&nbsp;&nbsp;AIFSN&nbsp;&nbsp;TXOP(b)&nbsp;&nbsp;TXOP(a/g)&nbsp;Admin Forced
			    <div class="setting">
				<div class="label">AC_BK</div>
				<input type="hidden" name="wl_wme_sta_bk" value="5">
				<input class=num name="wl_wme_sta_bk0" value="<% nvram_list("wl_wme_sta_bk", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_sta_bk1" value="<% nvram_list("wl_wme_sta_bk", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_sta_bk2" value="<% nvram_list("wl_wme_sta_bk", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_sta_bk3" value="<% nvram_list("wl_wme_sta_bk", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_sta_bk4" value="<% nvram_list("wl_wme_sta_bk", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_sta_bk5">
        			<option value="off" <% wme_match_op("wl_wme_sta_bk", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_sta_bk", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
			    	<div class="label">AC_BE</div>
				<input type="hidden" name="wl_wme_sta_be" value="5">
				<input class=num name="wl_wme_sta_be0" value="<% nvram_list("wl_wme_sta_be", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_sta_be1" value="<% nvram_list("wl_wme_sta_be", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_sta_be2" value="<% nvram_list("wl_wme_sta_be", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_sta_be3" value="<% nvram_list("wl_wme_sta_be", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_sta_be4" value="<% nvram_list("wl_wme_sta_be", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_sta_be5">
        			<option value="off" <% wme_match_op("wl_wme_sta_be", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_sta_be", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
				<div class="label">AC_VI</div>
				<input type="hidden" name="wl_wme_sta_vi" value="5">
				<input class=num name="wl_wme_sta_vi0" value="<% nvram_list("wl_wme_sta_vi", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_sta_vi1" value="<% nvram_list("wl_wme_sta_vi", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_sta_vi2" value="<% nvram_list("wl_wme_sta_vi", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_sta_vi3" value="<% nvram_list("wl_wme_sta_vi", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_sta_vi4" value="<% nvram_list("wl_wme_sta_vi", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_sta_vi5">
        			<option value="off" <% wme_match_op("wl_wme_sta_vi", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_sta_vi", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			    <div class="setting">
				<div class="label">AC_VO</div>
				<input type="hidden" name="wl_wme_sta_vo" value="5">
				<input class=num name="wl_wme_sta_vo0" value="<% nvram_list("wl_wme_sta_vo", 0); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmin")>
				<input class=num name="wl_wme_sta_vo1" value="<% nvram_list("wl_wme_sta_vo", 1); %>" size="5" maxlength="6" onBlur=valid_range(this,0,32767,"AC%20CWmax")>
				<input class=num name="wl_wme_sta_vo2" value="<% nvram_list("wl_wme_sta_vo", 2); %>" size="5" maxlength="6" onBlur=valid_range(this,1,15,"AC%20AIFSN")>
				<input class=num name="wl_wme_sta_vo3" value="<% nvram_list("wl_wme_sta_vo", 3); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(b)")>
				<input class=num name="wl_wme_sta_vo4" value="<% nvram_list("wl_wme_sta_vo", 4); %>" size="5" maxlength="6" onBlur=valid_range(this,0,65504,"AC%20TXOP(a/g)")>
				<select class=num name="wl_wme_sta_vo5">
        			<option value="off" <% wme_match_op("wl_wme_sta_vo", "off", "selected"); %>>Off</option>
        			<option value="on" <% wme_match_op("wl_wme_sta_vo", "on", "selected"); %>>On</option>
        			</select>
			    </div>
			</div>
				
                     </div><br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick="to_submit(this.form)" /><input type="reset" value="Cancel Changes" /></div>
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
                     <dt class="term">Authentication Type: </dt>
                     <dd class="definition">You may choose from Auto or Shared Key. Shared key authentication is more secure, but all devices on your network must also
                        support Shared Key authentication.
                     </dd>
                  </dl><br /><a target="_blank" href="help/HWirelessAdvanced.asp">More...</a></div>
            </div>
         </div>
      </div>
   </body>
</html>