<% do_pagehead("ddns.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function ddns_check(F,T) {
	if(F.ddns_enable.value == 0) {
		return true;
	} else if(F.ddns_enable.value == 1) {
		username = eval("F.ddns_username");
		passwd = eval("F.ddns_passwd");
		hostname = eval("F.ddns_hostname");
		dyndnstype = eval("F.ddns_dyndnstype");
		wildcard = eval("F.ddns_wildcard");
	} else if(F.ddns_enable.value == 2) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 3) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 4) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 6) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
		dyndnstype = eval("F.ddns_dyndnstype_"+F.ddns_enable.value);
		wildcard = eval("F.ddns_wildcard_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 7) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
		wildcard = eval("F.ddns_wildcard_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 8) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 5) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	} else if(F.ddns_enable.value == 9) {
		username = eval("F.ddns_username_"+F.ddns_enable.value);
		passwd = eval("F.ddns_passwd_"+F.ddns_enable.value);
		hostname = eval("F.ddns_hostname_"+F.ddns_enable.value);
	}
	if(username.value == "") {
		alert(errmsg.err0);
		username.focus();
		return false;
	}
	if(passwd.value == "") {
		alert(errmsg.err6);
		passwd.focus();
		return false;
	}
	if(hostname.value == "") {
		alert(errmsg.err7);
		hostname.focus();
		return false;
	}

	return true;
}

function submitcheck(F) {
	if(!ddns_check(F,"save"))
		return;
	
	F.change_action.value = "";
	F.save_button.value = sbutton.saving;
	update.stop();
}
function to_submit(F) {
    submitcheck(F);
    apply(F);
}
function to_apply(F) {
    submitcheck(F);
    applytake(F);
}

function SelDDNS(num,F) {
	F.change_action.value="gozila_cgi";
	F.ddns_enable.value=F.ddns_enable.options[num].value;
	update.stop();
	F.submit();
}

var update;

addEvent(window, "load", function() {
	update = new StatusUpdate("DDNS.live.asp", <% nvram_get("refresh_time"); %>);
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});

		//]]>
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","DDNS.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="ddns" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="DDNS" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("ddns.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("ddns.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("ddns.srv"); %></div>
									<select name="ddns_enable" onchange="SelDDNS(this.form.ddns_enable.selectedIndex,this.form)">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selmatch("ddns_enable", "0", "selected"); %> >" + share.disable + "</option>");
										//]]>
										</script>
										<option value="1" <% nvram_selmatch("ddns_enable", "1", "selected"); %> >DynDNS.org</option>
										<option value="2" <% nvram_selmatch("ddns_enable", "2", "selected"); %> >freedns.afraid.org</option>
										<option value="3" <% nvram_selmatch("ddns_enable", "3", "selected"); %> >ZoneEdit.com</option>
										<option value="4" <% nvram_selmatch("ddns_enable", "4", "selected"); %> >No-IP.com</option>
										<option value="6" <% nvram_selmatch("ddns_enable", "6", "selected"); %> >3322.org</option>
										<option value="7" <% nvram_selmatch("ddns_enable", "7", "selected"); %> >easyDNS.com</option>
										<option value="8" <% nvram_selmatch("ddns_enable", "8", "selected"); %> >TZO.com</option>
										<option value="9" <% nvram_selmatch("ddns_enable", "9", "selected"); %> >DynSIP.org</option>
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"5\" <% nvram_selmatch("ddns_enable", "5", "selected"); %> >" + ddns.custom + "</option>");
										//]]>
										</script>
									</select>
								</div>


					<% nvram_selmatch("ddns_enable","0","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.typ"); %></div>
									<select name="ddns_dyndnstype">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"1\" <% nvram_selmatch("ddns_dyndnstype", "1", "selected"); %> >" + ddns.dynamic + "</option>");
										document.write("<option value=\"2\" <% nvram_selmatch("ddns_dyndnstype", "2", "selected"); %> >" + share.sttic + "</option>");
										document.write("<option value=\"3\" <% nvram_selmatch("ddns_dyndnstype", "3", "selected"); %> >" + ddns.custom + "</option>");
										//]]>
										</script>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.wildcard"); %></div>
									<input type="checkbox" value="1" name="ddns_wildcard" <% nvram_checked("ddns_wildcard", "1"); %> />
								</div>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_2" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_2"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_2" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_2","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_2" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_2"); %>" />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
							<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_3" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_3"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_3" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_3","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_3" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_3"); %>" />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_4" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_4"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_4" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_4","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_4" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_4"); %>" />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_6" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_6"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_6" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_6","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_6" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_6"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.typ"); %></div>
									<select name="ddns_dyndnstype_6">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"1\" <% nvram_selmatch("ddns_dyndnstype_6", "1", "selected"); %> >" + ddns.dynamic + "</option>");
										//]]>
										</script>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.wildcard"); %></div>
									<input type="checkbox" value="1" name="ddns_wildcard_6" <% nvram_checked("ddns_wildcard_6", "1"); %> />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_7" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_7"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_7" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_7","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_7" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_7"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.wildcard"); %></div>
									<input type="checkbox" value="1" name="ddns_wildcard_7" <% nvram_checked("ddns_wildcard_7", "1"); %> />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_8" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_8"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_8" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_8","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_8" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_8"); %>" />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>
					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","5","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_9" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_9"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_9" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_9","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_9" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_9"); %>" />
								</div>
					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","5","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>


					<% nvram_selmatch("ddns_enable","1","<!--"); %>
					<% nvram_selmatch("ddns_enable","2","<!--"); %>
					<% nvram_selmatch("ddns_enable","3","<!--"); %>
					<% nvram_selmatch("ddns_enable","4","<!--"); %>
					<% nvram_selmatch("ddns_enable","6","<!--"); %>
					<% nvram_selmatch("ddns_enable","7","<!--"); %>
					<% nvram_selmatch("ddns_enable","8","<!--"); %>
					<% nvram_selmatch("ddns_enable","9","<!--"); %>
								<div class="setting">
									<div class="label"><% tran("ddns.system"); %></div>
									<input name="ddns_custom_5" size="30" maxlength="32" value="<% nvram_get("ddns_custom_5"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.usrname"); %></div>
									<input name="ddns_username_5" size="30" maxlength="32" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ddns_username_5"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.passwd"); %></div>
									<input type="password" name="ddns_passwd_5" size="30" maxlength="32" onblur="valid_name(this,share.passwd)" value="<% nvram_invmatch("ddns_passwd_5","","d6nw5v1x2pc7st9m"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.hostname"); %></div>
									<input name="ddns_hostname_5" size="42" onblur="valid_name(this,share.hostname)" value="<% nvram_get("ddns_hostname_5"); %>" />
								</div>
								<div class="setting">
									<div class="label">URL</div>
									<textarea cols="60" rows="2" id="ddns_url" name="ddns_url"></textarea>
									<script type="text/javascript">
									//<![CDATA[
										var ddns_url = fix_cr( '<% nvram_get("ddns_url"); %>' );
										document.getElementById("ddns_url").value = ddns_url;						
									//]]>
									</script>
								</div>
								<div class="setting">
									<div class="label"><% tran("ddns.options"); %></div>
									<textarea cols="60" rows="4" id="ddns_conf" name="ddns_conf"></textarea>
									<script type="text/javascript">
									//<![CDATA[
										var ddns_conf = fix_cr( '<% nvram_get("ddns_conf"); %>' );
										document.getElementById("ddns_conf").value = ddns_conf;
									//]]>
									</script>
								</div>

					<% nvram_selmatch("ddns_enable","1","-->"); %>
					<% nvram_selmatch("ddns_enable","2","-->"); %>
					<% nvram_selmatch("ddns_enable","3","-->"); %>
					<% nvram_selmatch("ddns_enable","4","-->"); %>
					<% nvram_selmatch("ddns_enable","6","-->"); %>
					<% nvram_selmatch("ddns_enable","7","-->"); %>
					<% nvram_selmatch("ddns_enable","8","-->"); %>
					<% nvram_selmatch("ddns_enable","9","-->"); %>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("share.option"); %></legend>
								<div class="setting">
									<div class="label"><% tran("ddns.forceupd"); %></div>
									<input type="text" size="4" class="num" name="ddns_force" value="<% nvram_get("ddns_force"); %>" />
									<span class="default">
									<script type="text/javascript">
									//<![CDATA[
									document.write("(" + share.deflt + ": 10 " + share.days_genetive + ")");
									//]]>
									</script></span>
								</div>
							</fieldset><br />

							<fieldset>
								<legend><% tran("ddns.statu"); %></legend>
								<div class="setting">
									<span id="ddns_status"><% show_ddns_status(); %></span>&nbsp;
								</div>
								<% nvram_selmatch("ddns_enable","0","-->"); %>
							</fieldset><br />
							
							<div class="submitFooter">
							 <script type="text/javascript">
							 //<![CDATA[
							 var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
							 submitFooterButton(1,1,0,autoref);
							 //]]>
							 </script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("ddns.srv"); %>:</dt>
							<dd class="definition"><% tran("hddns.right2"); %></dd>
							<dt class="term"><% tran("ddns.forceupd"); %>:</dt>
							<dd class="definition"><% tran("hddns.right4"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDDNS.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
					//]]>
					</script>
				</div>
				<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
				<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>