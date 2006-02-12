<html>
<head>
<title>SKYMAX 254-B</title>
<script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">

function dhcp_enable_disable(F,T) {
	var start = '';
	var end = '';
 	var total = F.elements.length;
	for(i=0 ; i < total ; i++){
		if(F.elements[i].name == "dhcp_start")
			start = i;
		if(F.elements[i].name == "wan_wins_3")
			end = i;
	}
	if(start == '' || end == '')
		return true;

	if( T == "static" ) {
		EN_DIS = 0;
		for(i = start; i<=end ;i++) {
			choose_disable(F.elements[i]);
		}
	} else {
		EN_DIS = 1;
		for(i = start; i<=end ;i++) {
			choose_enable(F.elements[i]);
		}
	}
}


function SelDHCP(T,F) {
	dhcp_enable_disable(F,T);
}
 
function valid_password(F) {
	if (F.http_passwd.value != F.http_passwdConfirm.value) {
		alert("Confirmed password did not match Entered Password.  Please re-enter password");
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}

	return true;
}


function to_submit(F) {
	        if (valid_password(F) == false)return false;

 
		F.submit_button.value = "setupindex";
		F.action.value = "Apply";
		F.submit();
	
}



</script>
</head>

<body bgcolor="#333333">


                  <form name="setup" action="applyuser.cgi" method="<% get_http_method(); %>">
		  <input type="hidden" name="submit_button" />
		  <input type="hidden" name="change_action" />
		  <input type="hidden" name="submit_type" />
		  <input type="hidden" name="action" />
		  <input type="hidden" name="lan_ipaddr" value="4" />



<table width="800" cellspacing="0" cellpadding="0" align="left">
	<tr>
		<td colspan="5" bgcolor="#333333" align="right"><img src="style/skytron/logo.gif" vspace="10"></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="bottom" height="60" align="left" width="200"><font face="Arial" color="#666666"><b>&nbsp;&nbsp;&nbsp;SKYMAX 254-B</b></font></td>
		<td bgcolor="#B2B2B2" width="10"></td>
		<td colspan="3" bgcolor="#FFFFFF" valign="center" align="left">&nbsp;</td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="#C8C8C8" height="1"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2"><b>Router-IP</b></font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF" width="10"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left">&nbsp;</td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Lokale IP-Adresse</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><% prefix_ip_get("lan_ipaddr",2); %><input class="num" maxlength="3" size="3" onblur="valid_range(this,0,255,&#34;IP&#34;)" name="lan_ipaddr_2" value='<% get_single_ip("lan_ipaddr","2"); %>' />.<input class="num" maxlength="3" size="3" onblur="valid_range(this,1,254,&#34;IP&#34;)" name="lan_ipaddr_3" value='<% get_single_ip("lan_ipaddr","3"); %>' /></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Subnetzmaske</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><% prefix_ip_get("lan_netmask",2); %><input class="num" maxLength="3" size="3" name="lan_netmask_2" onBlur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("lan_netmask","2"); %>' />.<input class="num" maxLength="3" size="3" name="lan_netmask_3" onBlur="valid_range(this,0,255,'Netmask')" value='<% get_single_ip("lan_netmask","3"); %>' /></font></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="#C8C8C8" height="1"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2"><b>DHCP-Einstellungen</b></font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left">&nbsp;</td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">DHCP-Server</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input type="radio" name="lan_proto" value="dhcp" onclick=SelDHCP('dhcp',this.form) <% nvram_selmatch("lan_proto", "dhcp", "checked"); %>>An</input><input type="radio" name="lan_proto" value="static" <% nvram_selmatch("lan_proto", "static", "checked"); %>>Aus</input></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Anfang des IP-Adressbereichs</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><% prefix_ip_get("lan_ipaddr",1); %><input class="num" name="dhcp_start" size="3" maxlength="3" onblur=valid_range(this,1,254,&#34;DHCP%20starting%20IP&#34;) value='<% nvram_get("dhcp_start"); %>' /></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Maximale DHCP-Benutzer</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input class="num" name="dhcp_num" size="3" maxlength="3" onblur="valid_range(this,1,253,'Number of DHCP users')" value='<% nvram_get("dhcp_num"); %>' /></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">DHCP-G&uuml;ltigkeitsdauer</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input class="num" name="dhcp_lease" size="4" maxlength="4" onblur="valid_range(this,0,9999,'DHCP Lease Time')" value='<% nvram_get("dhcp_lease"); %>'> Minuten</input></font></td>
	</tr>
		<% nvram_selmatch("wan_proto","static","<!--"); %>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Static DNS 1</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input type="hidden" name="wan_dns" value="4" /><input class="num" name="wan_dns0_0" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","0","0"); %>' />.<input class="num" name="wan_dns0_1" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","0","1"); %>' />.<input class="num" name="wan_dns0_2" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","0","2"); %>' />.<input class="num" name="wan_dns0_3" size="3" maxlength="3" onblur=valid_range(this,0,254,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","0","3"); %>' /></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Static DNS 2</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input class="num" name="wan_dns1_0" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","1","0"); %>' />.<input class="num" name="wan_dns1_1" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","1","1"); %>' />.<input class="num" name="wan_dns1_2" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","1","2"); %>' />.<input class="num" name="wan_dns1_3" size="3" maxlength="3" onblur=valid_range(this,0,254,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","1","3"); %>' /></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Static DNS 3</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input class="num" name="wan_dns2_0" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","2","0"); %>' />.<input class="num" name="wan_dns2_1" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","2","1"); %>' />.<input class="num" name="wan_dns2_2" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","2","2"); %>' />.<input class="num" name="wan_dns2_3" size="3" maxlength="3" onblur=valid_range(this,0,254,&#34;DNS&#34;) value='<% get_dns_ip("wan_dns","2","3"); %>' /></font></td>
	</tr>
	<% nvram_selmatch("wan_proto","static","-->"); %>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">WINS-Server</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><font face="Arial" color="#000000" size="2"><input type="hidden" name="wan_wins" value="4" /><input class="num" name="wan_wins_0" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;WINS&#34;) value='<% get_single_ip("wan_wins","0"); %>' />.<input class="num" name="wan_wins_1" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;WINS&#34;) value='<% get_single_ip("wan_wins","1"); %>' />.<input class="num" name="wan_wins_2" size="3" maxlength="3" onblur=valid_range(this,0,255,&#34;WINS&#34;) value='<% get_single_ip("wan_wins","2"); %>' />.<input class="num" name="wan_wins_3" size="3" maxlength="3" onblur=valid_range(this,0,254,&#34;WINS&#34;) value='<% get_single_ip("wan_wins","3"); %>' /></font></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="#C8C8C8" height="1"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2"><b>Router Passwort</font></b></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left">&nbsp;</td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Neues Kennwort</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><input type="password" maxlength="63" size="20" value="d6nw5v1x2pc7st9m" name="http_passwd" onblur="valid_name(this,'Password',SPACE_NO)" /></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" align="right" width="200" height="25"><font face="Arial" color="#000000" size="2">Kennwort best&auml;tigen</font></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#FFFFFF"></td>
		<td colspan="2" bgcolor="#FFFFFF" valign="center" align="left"><input type="password" maxlength="63" size="20" value="d6nw5v1x2pc7st9m" name="http_passwdConfirm" onblur="valid_name(this,'Password',SPACE_NO)" /></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="#C8C8C8" height="1"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<% active_wireless2(); %>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#FFFFFF"></td>
	</tr>
	<tr>
		<td colspan="5" bgcolor="#C8C8C8" height="1"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="B2B2B2" height="10"></td>
		<td colspan="3" bgcolor="#F4F4F4"></td>
	</tr>
	<tr>
		<td colspan="2" bgcolor="#B2B2B2" valign="center" align="right" width="200"></td>
		<td colspan="3" bgcolor="#F4F4F4" valign="center" align="left"></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="center" height="40" align="right" width="200"><input type="button" value="Speichern" onClick="to_submit(this.form)" /></td>		
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#F4F4F4"></td>
		<td bgcolor="#F4F4F4" valign="center" align="right" width="200"><font face="Arial" color="#666666" size="2">Firmware: &nbsp;&nbsp;&nbsp;</font></td>
		<td bgcolor="#F4F4F4" valign="center" align="left"><font face="Arial" color="#666666" size="2"><% get_firmware_version(); %></font></td>
	</tr>
	<tr>
		<td bgcolor="#B2B2B2" valign="top" height="40" align="right" width="200"><input type="reset" value="Abbrechen" /></td>
		<td bgcolor="#B2B2B2"></td>
		<td bgcolor="#F4F4F4"></td>
		<td bgcolor="#F4F4F4" valign="top" align="right" width="200"><font face="Arial" color="#666666" size="2">Uptime: &nbsp;&nbsp;&nbsp;</font></td>
		<td bgcolor="#F4F4F4" valign="top" align="left"><font face="Arial" color="#666666" size="2"><% get_uptime(); %></font></td>
	</tr>
</table>

</form>
</body>
</html>