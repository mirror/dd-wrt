
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

<html>
<head>
<% no_cache(); %>
<META http-equiv=Content-Type content="text/html; charset=iso-8859-1">
<title>Log info</title>
</head>
<script src="common.js"></script>
<SCRIPT language=JavaScript>
function to_submit(F)
{
	F.submit_button.value = "Log_settings";
	F.action.value = "Apply";
        F.submit();
}

</SCRIPT>
<body>
<form method=get action=apply.cgi>
<input type=hidden name=submit_button>
<input type=hidden name=action value="Apply">
<input type=hidden name=log_settings>
<input type=hidden name=log_show_all value=0>

<table border="1" width="43%" height="88">
<script language=javascript>
var table = new Array(
<% dump_log_settings(""); %>
);
var i=0;
for(;;){
	if(table[i]){
	var none_check = "";
	var log_check = "";
	var debug_check = "";
	if(table[i+2] == "2") debug_check = " checked";	
	else if(table[i+2] == "1") log_check = " checked";	
	else none_check = " checked";	

  document.write("<tr>");
  document.write("<td width=40% height=17>"+table[i+1]+"</td>");
  document.write("<td width=14% height=17><input type=radio name=LOG_"+table[i]+" value=0"+none_check+">None</td>");
  document.write("<td width=14% height=17><input type=radio name=LOG_"+table[i]+" value=1"+log_check+">LOG</td>");
  document.write("<td width=14% height=17><input type=radio name=LOG_"+table[i]+" value=2"+debug_check+">DEBUG</td>");
  document.write("</tr>");
  i = i + 3;
  
  }
  else{
  	break;
  }
 }
</script>
</table>
<input type=checkbox name=_log_show_all value=1 <% nvram_match("log_show_all","1","checked"); %>>&nbsp;Show all messages
  <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 
  <input type=button value="Save Settings" onClick=to_submit(this.form)></p>
</form>

</body>

</html>
