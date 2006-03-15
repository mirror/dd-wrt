<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
	<title>DDNS</title>
	<link type="text/css" rel="stylesheet" href="style.css"/>
	<script type="text/JavaScript" src="common.js">{}</script>
	<script language="Javascript" type="text/javascript" src="lang_pack/language.js"></SCRIPT>
	<script language="JavaScript">

document.title = '<% nvram_get("router_name"); %>'+ddns.titl;

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
//	alert("You must input a username !");
		alert(errmsg.err0);
		username.focus();
		return false;
	}
	if(passwd.value == ""){
//	alert("You must input a password !");
		alert(errmsg.err6);
		passwd.focus();
		return false;
	}
	if(hostname.value == ""){
//	alert("You must input a hostname !");
    alert(errmsg.err7);
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
		</script>
	</head>

	<body class="gui"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li class="current"><span><script>Capture(bmenu.setup)</script></span>
								  <div id="menuSub">
  									<ul id="menuSubList">
  										<li><a href="index.asp"><script>Capture(bmenu.setupbasic)</script></a></li>
  										<li><span><script>Capture(bmenu.setupddns)</script></span></li>
  										<li><a href="WanMAC.asp"><script>Capture(bmenu.setupmacclone)</script></a></li>
  										<li><a href="Routing.asp"><script>Capture(bmenu.setuprouting)</script></a></li>
  										<li><a href="Vlan.asp"><script>Capture(bmenu.setupvlan)</script></a></li>
  									</ul>
								  </div>
							 </li>
							 <li><a href="Wireless_Basic.asp"><script>Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script>Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script>Capture(bmenu.security)</script></a></li>
								<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'><script>Capture(bmenu.accrestriction)</script></a></li>
								<li><a href="Forward.asp"><script>Capture(bmenu.applications)</script></a></li>
								<li><a href="Management.asp"><script>Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script>Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ddns" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<h2><script>Capture(ddns.h2)</script></h2>
							<fieldset>
								<legend><script>Capture(ddns.legend)</script></legend>
								<div class="setting">
									<div class="label"><script>Capture(ddns.srv)</script></div>
									<select name="ddns_enable" onchange="SelDDNS(this.form.ddns_enable.selectedIndex,this.form)">
										<option value="0" <% nvram_selmatch("ddns_enable", "0", "selected"); %>><script>Capture(share.disable)</script></option>
										<option value="1" <% nvram_selmatch("ddns_enable", "1", "selected"); %>><script>Capture(ddns.dyndns)</script></option>
										<option value="2" <% nvram_selmatch("ddns_enable", "2", "selected"); %>><script>Capture(ddns.tzo)</script></option>
										<option value="3" <% nvram_selmatch("ddns_enable", "3", "selected"); %>><script>Capture(ddns.zone)</script></option>
									</select>
								</div>
								<% nvram_selmatch("ddns_enable","1","<!--"); %>
								<% nvram_selmatch("ddns_enable","2","<!--"); %>
								<% nvram_selmatch("ddns_enable","3","<!--"); %>
								<% nvram_selmatch("ddns_enable","1","-->"); %>
								<% nvram_selmatch("ddns_enable","2","-->"); %>
								<% nvram_selmatch("ddns_enable","3","-->"); %>
								<% nvram_selmatch("ddns_enable","0","<!--"); %>
								<% nvram_selmatch("ddns_enable","2","<!--"); %>
								<% nvram_selmatch("ddns_enable","3","<!--"); %>
								<div class="setting">
									<div class="label"><script>Capture(share.usrname)</script></div>
									<input name="ddns_username" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'User Name')" value='<% nvram_get("ddns_username"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.passwd)</script></div>
									<input type="password" name="ddns_passwd" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd","","d6nw5v1x2pc7st9m"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.hostname)</script></div>
									<input name="ddns_hostname" size="42" maxlength="48" onfocus="check_action(this,0)" onblur="valid_name(this,'Host Name')" value='<% nvram_get("ddns_hostname"); %>'/>
								</div>
								<% nvram_selmatch("ddns_enable","2","-->"); %>
								<% nvram_selmatch("ddns_enable","3","-->"); %>
								<% nvram_selmatch("ddns_enable","1","<!--"); %>
								<% nvram_selmatch("ddns_enable","3","<!--"); %>
								<div class="setting">
									<div class="label"><script>Capture(ddns.emailaddr)</script></div>
									<input name="ddns_username_2" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'E-mail Address')" value='<% nvram_get("ddns_username_2"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.passwd)</script></div>
									<input type="password" name="ddns_passwd_2" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd_2","","d6nw5v1x2pc7st9m"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.domainname)</script></div>
									<input name="ddns_hostname_2" size="42" maxlength="48" onfocus="check_action(this,0)" onblur="valid_name(this,'Domain%20Name')" value='<% nvram_get("ddns_hostname_2"); %>'/>
								</div>
								<% nvram_selmatch("ddns_enable","1","-->"); %>
								<% nvram_selmatch("ddns_enable","3","-->"); %>
								<% nvram_selmatch("ddns_enable","1","<!--"); %>
								<% nvram_selmatch("ddns_enable","2","<!--"); %>
								<div class="setting">
									<div class="label"><script>Capture(share.usrname)</script></div>
									<input name="ddns_username_3" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'User Name')" value='<% nvram_get("ddns_username_3"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.passwd)</script></div>
									<input type="password" name="ddns_passwd_3" size="30" maxlength="32" onfocus="check_action(this,0)" onblur="valid_name(this,'Password')" value='<% nvram_invmatch("ddns_passwd_3","","d6nw5v1x2pc7st9m"); %>'/>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.hostname)</script></div>
									<input name="ddns_hostname_3" size="42" maxlength="48" onfocus="check_action(this,0)" onblur="valid_name(this,'Host Name')" value='<% nvram_get("ddns_hostname_3"); %>'/>
								</div>
								<% nvram_selmatch("ddns_enable","1","-->"); %>
								<% nvram_selmatch("ddns_enable","2","-->"); %>
								<div class="setting">
									<div class="label"><script>Capture(share.interipaddr)</script></div>
									<% show_ddns_ip(); %>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(share.statu)</script></div>
									<% show_ddns_status(); %>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(ddns.typ)</script></div>
									<select name="ddns_dyndnstype">
										<option value="1" <% nvram_selmatch("ddns_dyndnstype", "1", "selected"); %>><script>Capture(ddns.dynamic)</script></option>
										<option value="2" <% nvram_selmatch("ddns_dyndnstype", "2", "selected"); %>><script>Capture(ddns.static)</script></option>
										<option value="3" <% nvram_selmatch("ddns_dyndnstype", "3", "selected"); %>><script>Capture(ddns.custom)</script></option>
									</select>
								</div>
								<div class="setting">
									<div class="label"><script>Capture(ddns.wildcard)</script></div>
									<input type="checkbox" value="1" name="ddns_wildcard" <% nvram_selmatch("ddns_wildcard", "1", "checked"); %>/>
								</div>
								<% nvram_selmatch("ddns_enable","0","-->"); %>
							</fieldset>
							<br/>
							<div class="submitFooter">
							 <script>document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\">");</script>
							 <script>document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\">");</script>
							</div>
						</form>
					</div>
				</div>
				<div id="statusInfo">
					<div class="info"><script>Capture(share.firmwarever)</script> : <% get_firmware_version(); %></div>
					<div class="info"><script>Capture(share.time)</script> : <% get_uptime(); %></div>
					<% nvram_match("wan_proto","disabled","<!--"); %>
					<div class="info"><script>Capture(share.interipaddr)</script> : <% nvram_status_get("wan_ipaddr"); %></div>
					<% nvram_match("wan_proto","disabled","-->"); %>
          <div class="info"><% nvram_match("wan_proto","disabled","WAN disabled"); %></div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2><script>Capture(share.help)</script></h2></div>
						<dl>
							<dt class="term"><script>Capture(hddns.right1)</script></dt>
							<dd class="definition"><script>Capture(hddns.right2)</script></dd>
						</dl>
						<br/>
						<a target="_blank" href="help/HDDNS.asp"><script>Capture(share.more)</script></a>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>