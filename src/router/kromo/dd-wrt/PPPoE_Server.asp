<% do_pagehead("service.pppoesrv_legend"); %>
		<script type="text/javascript">
		//<![CDATA[


function checked(F) {
	if (F._pppoeserver_deflate) {
		(F._pppoeserver_deflate.checked == true) ? F.pppoeserver_deflate.value = 1 : F.pppoeserver_deflate.value = 0;
	}
	
	if (F._pppoeserver_bsdcomp) {
		(F._pppoeserver_bsdcomp.checked == true) ? F.pppoeserver_bsdcomp.value = 1 : F.pppoeserver_bsdcomp.value = 0;
	}
	if (F._pppoeserver_lzs) {
		(F._pppoeserver_lzs.checked == true) ? F.pppoeserver_lzs.value = 1 : F.pppoeserver_lzs.value = 0;
	}
	if (F._pppoeserver_mppc) {
		(F._pppoeserver_mppc.checked == true) ? F.pppoeserver_mppc.value = 1 : F.pppoeserver_mppc.value = 0;
	}
	
	if (F._pppoeserver_encryption) {
		(F._pppoeserver_encryption.checked == true) ? F.pppoeserver_encryption.value = 1 : F.pppoeserver_encryption.value = 0;
	}
}

function chap_user_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_chap_user";
	checked(F);
	F.submit();
}

function chap_user_remove_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "remove_chap_user";
	checked(F);
	F.submit();
}

function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	checked(F);
	apply(F);
}
function to_apply(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	checked(F);
	applytake(F);
}

var update;

addEvent(window, "load", function() {

		toggle_layer_ext(document.setup.pppoeradius_enabled, 'idpppoerad', 'idpppoelocal', <% nvram_else_match("pppoeradius_enabled", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.pppoeserver_enabled, 'idpppoesrv', <% nvram_else_match("pppoeserver_enabled", "1", "1", "0"); %> == 1);
		
		update = new StatusbarUpdate();
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
					<% do_menu("Services.asp","PPPoE_Server.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="post">
							<input type="hidden" name="submit_button" value="PPPoE_Server" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<input type="hidden" name="pppoeserver_deflate" />
							<input type="hidden" name="pppoeserver_bsdcomp" />
							<input type="hidden" name="pppoeserver_lzs" />
							<input type="hidden" name="pppoeserver_mppc" />
							<input type="hidden" name="pppoeserver_encryption" />
							<input type="hidden" name="pppoeserver_chaps" />
							
							<h2><% tran("service.pppoesrv_legend"); %></h2>

	<fieldset>
		<legend><% tran("service.pppoesrv_legend"); %></legend>
			<div class="setting">
			<div class="label"><% tran("service.pppoesrv_srv"); %></div>
				<input class="spaceradio" type="radio" name="pppoeserver_enabled" value="1" <% nvram_checked("pppoeserver_enabled", "1"); %> onclick="show_layer_ext(this, 'idpppoesrv', true)" /><% tran("share.enable"); %>&nbsp;
				<input class="spaceradio" type="radio" name="pppoeserver_enabled" value="0" <% nvram_checked("pppoeserver_enabled", "0"); %> onclick="show_layer_ext(this, 'idpppoesrv', false)" /><% tran("share.disable"); %>
			</div>
	</fieldset><br />
	
<div id="idpppoesrv">

	<fieldset>
		<legend><% tran("service.pppoesrv_srvopt"); %></legend>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_remoteaddr"); %></div>
				<input maxlength="15" size="24" name="pppoeserver_remoteaddr" onblur="valid_ip_str(this, share.ip)" value="<% nvram_get("pppoeserver_remoteaddr"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_remotenet"); %></div>
				<input maxlength="15" size="24" name="pppoeserver_remotenet" onblur="valid_ip_str(this, share.ip)" value="<% nvram_get("pppoeserver_remotenet"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_remotemask"); %></div>
				<input maxlength="15" size="24" name="pppoeserver_remotemask" onblur="valid_ip_str(this, share.ip)" value="<% nvram_get("pppoeserver_remotemask"); %>" />
			</div>
			<div class="setting">
				<div class="label">Deflate <% tran("service.pppoesrv_compr"); %></div>
				<input type="checkbox" value="1" name="_pppoeserver_deflate" <% nvram_checked("pppoeserver_deflate", "1"); %> />
			</div>
			<div class="setting">
				<div class="label">BSD <% tran("service.pppoesrv_compr"); %></div>
				<input type="checkbox" value="1" name="_pppoeserver_bsdcomp" <% nvram_checked("pppoeserver_bsdcomp", "1"); %> />
			</div>
			<div class="setting">
				<div class="label">LZS Stac <% tran("service.pppoesrv_compr"); %></div>
				<input type="checkbox" value="1" name="_pppoeserver_lzs" <% nvram_checked("pppoeserver_lzs", "1"); %> />
			</div>
			<div class="setting">
				<div class="label">MPPC <% tran("service.pppoesrv_compr"); %></div>
				<input type="checkbox" value="1" name="_pppoeserver_mppc" <% nvram_checked("pppoeserver_mppc", "1"); %> />
			</div>
			<div class="setting">
				<div class="label">MPPE PPPoE <% tran("share.encrypt"); %></div>
				<input type="checkbox" value="1" name="_pppoeserver_encryption" <% nvram_checked("pppoeserver_encryption", "1"); %> />
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_lcpei"); %></div>
				<input size="5" maxlength="5" class="num" name="pppoeserver_lcpechoint" value="<% nvram_get("pppoeserver_lcpechoint"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 60)");
				//]]>
				</script></span>
			</div>	
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_lcpef"); %></div>
				<input size="5" maxlength="5" class="num" name="pppoeserver_lcpechofail" value="<% nvram_get("pppoeserver_lcpechofail"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 5)");
				//]]>
				</script></span>
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_idlet"); %></div>
				<input size="5" maxlength="5" class="num" name="pppoeserver_idle" value="<% nvram_get("pppoeserver_idle"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 600)");
				//]]>
				</script></span>
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_auth"); %></div>
				<input class="spaceradio" type="radio" name="pppoeradius_enabled" value="1" <% nvram_checked("pppoeradius_enabled", "1"); %> onclick="toggle_layer_ext(this, 'idpppoerad', 'idpppoelocal', true)" />Radius&nbsp;
				<input class="spaceradio" type="radio" name="pppoeradius_enabled" value="0" <% nvram_checked("pppoeradius_enabled", "0"); %> onclick="toggle_layer_ext(this, 'idpppoerad', 'idpppoelocal', false)" /><% tran("service.pppoesrv_chaps"); %>
			</div>
	</fieldset><br />

<div id="idpppoerad">
	<fieldset>
		<legend>Radius <% tran("service.pppoesrv_auth"); %></legend>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radip"); %></div>
				<input maxlength="15" size="24" name="pppoeserver_authserverip" onblur="valid_ip_str(this, share.ip)" value="<% nvram_get("pppoeserver_authserverip"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radauthport"); %></div>
				<input size="5" maxlength="5" class="num" name="pppoeserver_authserverport" value="<% nvram_get("pppoeserver_authserverport"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 1812)");
				//]]>
				</script></span>
			</div>			
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radaccport"); %></div>
				<input size="5" maxlength="5" class="num" name="pppoeserver_acctserverport" value="<% nvram_get("pppoeserver_acctserverport"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 1813)");
				//]]>
				</script></span>
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radkey"); %></div>
				<input size="20" maxlength="63" type="password" name="pppoeserver_sharedkey" value="d6nw5v1x2pc7st9m" />
			</div>
	</fieldset>
</div>

<div id="idpppoelocal">
	<fieldset>
		<legend><% tran("service.pppoesrv_chaps"); %></legend>
			<table class="table center" summary="chap secrets table">
			<tr>
				<th width="30%"><% tran("share.user"); %></th>
				<th width="30%"><% tran("share.passwd"); %></th>
				<th width="30%"><% tran("share.ip"); %></th>
				<th><% tran("share.enable"); %></th>
			</tr>
			<% show_chaps(); %>
			</table><br />
			<div class="center">
				<script type="text/javascript">
				//<![CDATA[
				document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"chap_user_add_submit(this.form);\" />");
				document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"chap_user_remove_submit(this.form);\" />");
				//]]>
				</script>
			</div>
	</fieldset>
</div>

</div>
<br/>

							
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<br/>
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HPPPoE_Server.asp');"><% tran("share.more"); %></a>
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