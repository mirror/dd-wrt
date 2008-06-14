<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - User Password Change</title>
		<script type="text/javascript">//
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %> - User Password Change";

function valid_password(F) {
	if (F.http_passwd.value != F.http_passwdConfirm.value) {
		alert(errmsg.err10);
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}

	return true;
}


function to_submit(F) {
if (valid_password(F))
    {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "index";
	F.submit_type.value = "changepass";
	F.changepassword.value = "Changing Password";
	F.action.value = "Apply";
	apply(F);
    }
}


//]]>
</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><% tran("bmenu.setup"); %></span></li>
								<li><span><% tran("bmenu.wireless"); %></span></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><span><% tran("bmenu.sipath"); %></span></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><span><% tran("bmenu.security"); %></span></li>
								<li><span><% tran("bmenu.accrestriction"); %></span></li>
								<li><span><% tran("bmenu.applications"); %></span></li>
								<li><span><% tran("bmenu.admin"); %></span></li>
								<li><span><% tran("bmenu.statu"); %></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
                    		<form name="changepassword" action="apply.cgi" method="<% get_http_method(); %>">
                  			<input type="hidden" name="submit_button" value="index" />
                  			<input type="hidden" name="submit_type" />
                  			<input type="hidden" name="change_action" />
					<input type="hidden" name="action" value="Apply" />

					<div id="contentsInfo">
			<dl>
				<dd>Your Router is currently not protected and uses a unsafe default username and password combination, please change it using the following dialog</dd>
			</dl>
	<fieldset>
		<legend><% tran("management.psswd_legend"); %></legend>
		<div class="setting">
			<div class="label"><% tran("management.psswd_user"); %></div>
			<input type="password" maxlength="63" size="20" value="d6nw5v1x2pc7st9m" name="http_username" onblur="valid_name(this,management.psswd_user,SPACE_NO)" />
		</div>
		<div class="setting">
			<div class="label"><% tran("management.psswd_pass"); %></div>
			<input type="password" maxlength="63" size="20" value="d6nw5v1x2pc7st9m" name="http_passwd" onblur="valid_name(this,management.psswd_pass,SPACE_NO)" />
		</div>
		<div class="setting">
			<div class="label"><% tran("management.pass_conf"); %></div>
			<input type="password" maxlength="63" size="20" value="d6nw5v1x2pc7st9m" name="http_passwdConfirm" onblur="valid_name(this,management.pass_conf,SPACE_NO)" />
		</div>
	</fieldset><br />
						<div class="submitFooter">
							<script type="text/javascript">
							//<![CDATA[
							document.write("<input type=\"button\" name=\"changepassword\" value=\"Change Password\" onclick=\"to_submit(this.form)\" />");
							//]]>
							</script>
						</div>
					</div>
				</form>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>