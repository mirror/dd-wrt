<% do_pagehead("service.pptp_legend"); %>
		<script type="text/javascript">
		//<![CDATA[



function to_submit(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.change_action.value = "";
	F.submit_type.value = "";
	F.save_button.value = sbutton.saving;
	applytake(F);
}

var update;

addEvent(window, "load", function() {

		show_layer_ext(document.setup.pptpd_enable, 'idpptp', <% nvram_else_match("pptpd_enable", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.pptpd_radius, 'idradius', <% nvram_else_match("pptpd_radius", "1", "1", "0"); %> == 1);
		show_layer_ext(document.setup.pptpd_client_enable, 'idpptpcli', <% nvram_else_match("pptpd_client_enable", "1", "1", "0"); %> == 1);
		
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
					<% do_menu("Services.asp","PPTP.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="applyuser.cgi" method="post">
							<input type="hidden" name="submit_button" value="PPTP" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							
<h2><% tran("service.pptp_srv"); %></h2>

<fieldset>
	<legend><% tran("service.pptp_srv"); %></legend>
	<div class="setting">
		<div class="label"><% tran("service.pptp_srv"); %></div>
		<input class="spaceradio" type="radio" name="pptpd_enable" value="1" <% nvram_checked("pptpd_enable", "1"); %> onclick="show_layer_ext(this, 'idpptp', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" name="pptpd_enable" value="0" <% nvram_checked("pptpd_enable", "0"); %> onclick="show_layer_ext(this, 'idpptp', false)" /><% tran("share.disable"); %>
	</div>
	<div id="idpptp">
		<div class="setting">
			<div class="label"><% tran("share.broadcast"); %></div>
			<input class="spaceradio" type="radio" name="pptpd_bcrelay" value="1" <% nvram_checked("pptpd_bcrelay", "1"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" name="pptpd_bcrelay" value="0" <% nvram_checked("pptpd_bcrelay", "0"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("share.srvip"); %></div>
			<input size="25" name="pptpd_lip" value="<% nvram_get("pptpd_lip"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptp_client"); %></div>
			<input size="60" name="pptpd_rip" value="<% nvram_get("pptpd_rip"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptp_chap"); %></div>
			<textarea id="pptpd_auth" name="pptpd_auth" cols="60" rows="4"></textarea>
			<script type="text/javascript">
			//<![CDATA[
				var var_pptpd_auth = fix_cr( '<% nvram_get("pptpd_auth"); %>' );
				document.getElementById("pptpd_auth").value = var_pptpd_auth;
			//]]>
			</script>
		</div>
		<div class="setting">
			<div class="label"><% tran("radius.legend"); %></div>
			<input class="spaceradio" type="radio" name="pptpd_radius" value="1" <% nvram_checked("pptpd_radius", "1"); %> onclick="show_layer_ext(this, 'idradius', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" name="pptpd_radius" value="0" <% nvram_checked("pptpd_radius", "0"); %> onclick="show_layer_ext(this, 'idradius', false)" /><% tran("share.disable"); %>
		</div>
		<div id="idradius">
	<fieldset>
		<legend>Radius <% tran("service.pppoesrv_auth"); %></legend>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radip"); %></div>
				<input maxlength="15" size="24" name="pptpd_radserver" onblur="valid_ip_str(this, share.ip)" value="<% nvram_get("pptpd_radserver"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radauthport"); %></div>
				<input size="5" maxlength="5" class="num" name="pptpd_radport" value="<% nvram_get("pptpd_radport"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 1812)");
				//]]>
				</script></span>
			</div>			
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radaccport"); %></div>
				<input size="5" maxlength="5" class="num" name="pptpd_acctport" value="<% nvram_get("pptpd_acctport"); %>" />
				<span class="default"><script type="text/javascript">
				//<![CDATA[
				document.write("(" + share.deflt + ": 1813)");
				//]]>
				</script></span>
			</div>
			<div class="setting">
				<div class="label"><% tran("service.pppoesrv_radkey"); %></div>
				<input size="20" maxlength="63" type="password" name="pptpd_radpass" value="d6nw5v1x2pc7st9m" />
			</div>
	</fieldset>
		</div>
	</div>
</fieldset><br/>


<h2><% tran("service.pptpd_legend"); %></h2>
<fieldset>
	<legend><% tran("service.pptpd_legend"); %></legend>
	<div class="setting">
		<div class="label"><% tran("service.pptpd_option"); %></div>
		<input class="spaceradio" type="radio" name="pptpd_client_enable" value="1" <% nvram_checked("pptpd_client_enable", "1"); %> onclick="show_layer_ext(this, 'idpptpcli', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" name="pptpd_client_enable" value="0" <% nvram_checked("pptpd_client_enable", "0"); %> onclick="show_layer_ext(this, 'idpptpcli', false)" /><% tran("share.disable"); %>
	</div>
	<div id="idpptpcli">
		<div class="setting">
			<div class="label"><% tran("service.pptpd_ipdns"); %></div>
			<input size="27" name="pptpd_client_srvip" value="<% nvram_get("pptpd_client_srvip"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_subnet"); %></div>
			<input type="hidden" name="pptpd_client_srvsub" value="0.0.0.0" />
			<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_0" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","0"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_1" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","1"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_2" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","2"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsub_3" onblur="valid_range(this,0,255,service.pptpd_subnet)" value="<% get_single_ip("pptpd_client_srvsub","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_subnetmask"); %></div>
			<input type="hidden" name="pptpd_client_srvsubmsk" value="0.0.0.0" />
			<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_0" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","0"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_1" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","1"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_2" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","2"); %>" />.<input size="3" maxlength="3" class="num" name="pptpd_client_srvsubmsk_3" onblur="valid_range(this,0,255,service.pptpd_subnetmask)" value="<% get_single_ip("pptpd_client_srvsubmsk","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_encry"); %></div>
			<input size="27" name="pptpd_client_srvsec" value="<% nvram_get("pptpd_client_srvsec"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_mtu"); %></div>
			<input class="num" maxlength="4" size="5" name="pptpd_client_srvmtu" onblur="valid_range(this,0,1500,service.pptpd_mtu)" value="<% nvram_get("pptpd_client_srvmtu"); %>" />&nbsp;
			<span class="default"><script type="text/javascript">
			//<![CDATA[
			document.write("(" + share.deflt + ": 1450)");
			//]]>
			</script></span>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_mru"); %></div>
			<input class="num" maxlength="4" size="5" name="pptpd_client_srvmru" onblur="valid_range(this,0,1500,service.pptpd_mru)" value="<% nvram_get("pptpd_client_srvmru"); %>" />&nbsp;
			<span class="default"><script type="text/javascript">
			//<![CDATA[
			document.write("(" + share.deflt + ": 1450)");
			//]]>
			</script></span>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.pptpd_nat"); %></div>
			<input class="spaceradio" type="radio" name="pptpd_client_nat" value="1" <% nvram_checked("pptpd_client_nat", "1"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" name="pptpd_client_nat" value="0" <% nvram_checked("pptpd_client_nat", "0"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("share.usrname"); %></div>
			<input size="27" name="pptpd_client_srvuser" value="<% nvram_get("pptpd_client_srvuser"); %>" />
		</div>		
		<div class="setting">
			<div class="label"><% tran("share.passwd"); %></div>
			<input size="27" id="pptpd_client_srvpass" name="pptpd_client_srvpass" type="password" value="<% nvram_get("pptpd_client_srvpass"); %>" />
			<input type="checkbox" name="_wl_unmask" value="0" onclick="setElementMask('pptpd_client_srvpass', this.checked)" />&nbsp;<% tran("share.unmask"); %>
		</div>
	</div>
</fieldset><br />
							
							
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
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HPPTP.asp');"><% tran("share.more"); %></a>
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