<!--
*********************************************************
*   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
*********************************************************

This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
the contents of this file may not be disclosed to third parties,
copied or duplicated in any form without the prior written
permission of CyberTAN Inc.

This software should be used as a reference only, and it not
intended for production use!


THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
-->

<HTML><HEAD><TITLE><% nvram_get("router_name"); %> - Management</TITLE>
<% no_cache(); %>
<META http-equiv=Content-Type content="text/html; charset=iso-8859-1">
<link rel="stylesheet" type="text/css" href="style.css">
<style fprolloverstyle>
A:hover {color: #00FFFF}
.small A:hover {color: #00FFFF}
</style>
<script src="common.js"></script>
<SCRIPT language=JavaScript>
var EN_DIS1 = '<% nvram_get("remote_management"); %>'
var wan_proto = '<% nvram_get("wan_proto"); %>'
function SelPort(num,F)
{
	if(num == 1 && F.PasswdModify.value == 1){
		 if(ChangePasswd(F) == true)
			port_enable_disable(F,num);
	}
	else
		port_enable_disable(F,num);
}
function port_enable_disable(F,I)
{
	EN_DIS2 = I;
	if ( I == "0" ){
		choose_disable(F.http_wanport);
	}
	else{
		choose_enable(F.http_wanport);
	}
}
function ChangePasswd(F)
{
	if((F.PasswdModify.value==1 && F.http_passwd.value == "d6nw5v1x2pc7st9m") || F.http_passwd.value == "admin")
	{
		if(confirm('The Router is currently set to its default password. As a security measure, you must change the password before the Remote Management feature can be enabled. Click the OK button to change your password.  Click the Cancel button to leave the Remote Management feature disabled.'))
		{
			//window.location.replace('Management.asp');
			F.remote_management[1].checked = true;
			return false;
		}
		else
		{
			F.remote_management[1].checked = true;
			return false;
		}
	}
	else
		return true;
}

function valid_password(F)
{
	if (F.http_passwd.value != F.http_passwdConfirm.value)
	{
		alert("Confirmed password did not match Entered Password.  Please re-enter password");
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}
	return true;
}
function to_reboot(F)
{
F.action.value='Reboot';
F.submit();
return true;
}
function to_submit(F)
{
	if( F.http_passwd.value != F.http_passwdConfirm.value )
		{
		alert('Password confirmation is not matched.');
		return false;
		}
	else
		F.action.value='Apply';

	valid_password(F);

	if(F.remote_management[0].checked == true){
		if(!ChangePasswd(F))
			return false;
	}

	if(F._http_enable){
		if(F._http_enable.checked == true) F.http_enable.value = 1;
		else 	 F.http_enable.value = 0;
	}
	if(F._https_enable){
		if(F._https_enable.checked == true) F.https_enable.value = 1;
		else 	 F.https_enable.value = 0;
	}
	
	
	F.smtp_redirect_destination.value = F.smtp_redirect_destination_0.value+'.'+F.smtp_redirect_destination_1.value+'.'+F.smtp_redirect_destination_2.value+'.'+F.smtp_redirect_destination_3.value;
	F.smtp_source_network.value = F.smtp_source_network_0.value+'.'+F.smtp_source_network_1.value+'.'+F.smtp_source_network_2.value+'.'+F.smtp_source_network_3.value;

	F.submit_button.value = "Management";
	F.submit();
	return true;
}
function init()
{
	port_enable_disable(document.password, '<% nvram_get("remote_management"); %>');
}

</SCRIPT>

</HEAD>
<BODY vLink=#b5b5e6 aLink=#ffffff link=#b5b5e6 onload=init()>
<DIV align=center>
<FORM name=password method=<% get_http_method(); %> action=apply.cgi>
<input type=hidden name=submit_button>
<input type=hidden name=reboot_button>
<input type=hidden name=change_action>
<input type=hidden name=action>
<input type=hidden name=commit value="1">
<INPUT type=hidden name=PasswdModify value='<% nvram_else_match("http_passwd", "admin", "1", "0"); %>'>
<input type=hidden name=http_enable>
<input type=hidden name=https_enable>
<TABLE cellSpacing=0 cellPadding=0 width=809 border=0>
  <TBODY>
  <TR>
    <TD width=95><IMG src="image/UI_Linksys.gif" border=0 width="165" height="57"></TD>
    <TD vAlign=bottom align=right width=714 bgColor=#6666cc><FONT
      style="FONT-SIZE: 7pt"><FONT face=Arial><font color=#AAAAAA><br>Firmware version:&nbsp;</font><FONT Color=#FFFFFF><% get_firmware_version(); %>&nbsp;&nbsp;&nbsp;</font></FONT></TD></TR>
  <TR>
    <TD width=808 bgColor=#6666cc colSpan=2><IMG height=11 src="image/UI_10.gif" width=809 border=0></TD></TR></TBODY></TABLE>
    
    
    
    
<TABLE height=77 cellSpacing=0 cellPadding=0 width=809 bgColor=black border=0>
  <TBODY>
  <TR>
    <TD style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" borderColor=#000000 align=middle width=163 height=49><H3 style="MARGIN-TOP: 1px; MARGIN-BOTTOM: 1px"><FONT style="FONT-SIZE: 15pt" face=Arial color=#ffffff>Administration</FONT></H3></TD>
    <TD style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" vAlign=center borderColor=#000000 width=646 bgColor=#000000 height=49><TABLE style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; BORDER-COLLAPSE: collapse; FONT-VARIANT: normal" height=33 cellSpacing=0 cellPadding=0 bgColor=#6666cc border=0>
       <TBODY>
        <TR>
          <TD style="FONT-WEIGHT: bolder; FONT-SIZE: 10pt" align=right bgColor=#6666cc height=33><FONT color=#ffffff>Wireless-G Broadband Router&nbsp;&nbsp;</FONT></TD>
          <TD borderColor=#000000 borderColorLight=#000000 align=middle width=109 bgColor=#000000 borderColorDark=#000000 height=12 rowSpan=2><FONT color=#ffffff><SPAN style="FONT-SIZE: 8pt"><B><% nvram_get("router_name"); %></B></SPAN></FONT></TD></TR>
        <TR>
          <TD style="FONT-WEIGHT: normal; FONT-SIZE: 1pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" width=537 bgColor=#000000 height=1></TD></TR>
        <TR>
          <TD width=646 bgColor=#000000 colSpan=2 height=32>
            <TABLE id=AutoNumber1 style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; BORDER-COLLAPSE: collapse; FONT-VARIANT: normal" height=6 cellSpacing=0 cellPadding=0 width=637 border=0>
            <TBODY>
              <TR style="BORDER-RIGHT: medium none; BORDER-TOP: medium none; FONT-WEIGHT: normal; FONT-SIZE: 1pt; BORDER-LEFT: medium none; COLOR: black; BORDER-BOTTOM: medium none; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" align=middle bgColor=#6666cc>
                <TD width=83 height=1><IMG height=10 src="image/UI_06.gif" width=83 border=0></TD>
                <TD width=73 height=1><IMG height=10 src="image/UI_06.gif" width=83 border=0></TD>
                <TD width=113 height=1><IMG height=10 src="image/UI_06.gif" width=83 border=0></TD>
                <TD width=103 height=1><IMG height=10 src="image/UI_06.gif" width=103 border=0></TD>
                <TD width=85 height=1><IMG height=10 src="image/UI_06.gif" width=100 border=0></TD>
                <TD width=115 height=1><IMG height=10 src="image/UI_07.gif" width=115 border=0></TD>
                <TD width=74 height=1><IMG height=10 src="image/UI_06.gif" width=79 border=0></TD></TR>
              <TR>
                <TD align=middle bgColor=#000000 height=20><FONT style="FONT-WEIGHT: 700" color=#ffffff><A style="TEXT-DECORATION: none" href="index.asp">Setup</A></FONT></TD>
                <TD align=middle bgColor=#000000 height=20><FONT style="FONT-WEIGHT: 700" color=#ffffff><a style="TEXT-DECORATION: none" href="Wireless_Basic.asp">Wireless</a></FONT></TD>
                <TD align=middle bgColor=#000000 height=20><FONT style="FONT-WEIGHT: 700" color=#ffffff><a style="TEXT-DECORATION: none" href="Firewall.asp">Security</a></FONT></TD>
                <TD align=middle bgColor=#000000 height=20><FONT style="FONT-WEIGHT: 700" color=#ffffff><A style="TEXT-DECORATION: none" href="<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>">Access Restrictions</A></FONT></TD>
                <TD align=middle bgColor=#000000 height=20><P style="MARGIN-BOTTOM: 4px"><FONT style="FONT-WEIGHT: 700" color=#ffffff><a style="TEXT-DECORATION: none" href="Forward.asp">Applications <BR>&amp; Gaming</a>&nbsp;&nbsp;&nbsp;&nbsp;</FONT></P></TD>
                <TD align=middle bgColor=#6666cc height=20><P style="MARGIN-BOTTOM: 4px"><FONT style="FONT-WEIGHT: 700" color=#ffffff>Administration&nbsp;&nbsp;&nbsp;&nbsp;</FONT></P></TD>
                <TD align=middle bgColor=#000000 height=20><P style="MARGIN-BOTTOM: 4px"><FONT style="FONT-WEIGHT: 700" color=#ffffff><a style="TEXT-DECORATION: none" href="Status_Router.asp">Status</a>&nbsp;&nbsp;&nbsp;&nbsp;</FONT></P></TD></TR>
              <TR>
                <TD width=643 bgColor=#6666cc colSpan=7 height=21>
                  <TABLE borderColor=black height=21 cellSpacing=0 cellPadding=0 width=643>
                    <TBODY>
                    <TR align=left>
                      <TD width=25></TD>
                      <TD width=80><FONT style="COLOR: white">Management</FONT></TD>
                      <TD width=1><P class=bar><font color='white'><b>|</b></font></P></TD>
                      <TD width=20></TD>
                      <TD class=small width=40><A href="Log.asp">Log</A></TD>
                      <TD width=1><P class=bar><font color='white'><b>|</b></font></P></TD>
                      <TD width=15></TD>
                      <TD class=small width=75><A href="Diagnostics.asp">Diagnostics</A></TD>
                      <TD width=1><P class=bar><font color='white'><b>|</b></font></P></TD>
                      <TD width=15></TD>
                      <TD class=small width=95><A href="Factory_Defaults.asp">Factory Defaults</A></TD>
                      <TD width=1><P class=bar><font color='white'><b>|</b></font></P></TD>
                      <TD width=15></TD>
                      <TD class=small width=105><A href="Upgrade.asp">Firmware Upgrade</A></TD>
		      <TD width=1><P class=bar><font color='white'><b>|</b></font></P></TD>
                      <TD width=15></TD>
                      <TD class=small><A href='config.asp'>Backup</A></TD>


</TR></TBODY></TABLE></TD></TR></TBODY></TABLE></TD></TR></TBODY></TABLE></TD></TR></TBODY></TABLE>

<TABLE height=5 cellSpacing=0 cellPadding=0 width=806 bgColor=black border=0>
  <TBODY>
  <TR bgColor=black>
    <TD 
    style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" 
    borderColor=#e7e7e7 width=163 bgColor=#e7e7e7 height=1><IMG height=15 
      src="image/UI_03.gif" width=164 border=0></TD>
    <TD 
    style="FONT-WEIGHT: normal; FONT-SIZE: 10pt; COLOR: black; FONT-STYLE: normal; FONT-FAMILY: Arial, Helvetica, sans-serif; FONT-VARIANT: normal" 
    width=646 height=1><IMG height=15 src="image/UI_02.gif" width=645 
      border=0></TD>
 </TR>
 </TBODY>
</TABLE>



<!-- <TABLE id=AutoNumber9 style="BORDER-COLLAPSE: collapse" borderColor=#111111 height=23 cellSpacing=0 cellPadding=0 width=809 border=0>
-->
<TABLE id=AutoNumber9 height=23 cellSpacing=0 cellPadding=0 width=809 border=0>
  <TBODY>
  <TR>
    <TD width=633>
    	<!-- <TABLE cellSpacing=0 cellPadding=0 border=0 height="2077" style="border-collapse: collapse; border-style: solid" bordercolor="#111111" width="633">
    	-->
	<TABLE cellSpacing=0 cellPadding=0 border=0 height="2077" width="633">    	
        <TBODY>
