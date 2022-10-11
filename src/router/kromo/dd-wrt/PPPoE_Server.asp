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
	F.apply_button.value = sbutton.applied;
	checked(F);
	applytake(F);
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	
	show_layer_ext(document.setup.pppoeradius_enabled, 'idpppoelocal', <% nvem("pppoeradius_enabled", "0", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pppoeradius_enabled, 'idpppoerad', <% nvem("pppoeradius_enabled", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pppoeserver_clip, 'idpppoeiploc', <% nvem("pppoeserver_clip", "local", "1", "0"); %> == 1);
	show_layer_ext(document.setup.pppoeserver_enabled, 'idpppoesrv', <% nvem("pppoeserver_enabled", "1", "1", "0"); %> == 1);

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
										<input class="spaceradio" type="radio" name="pppoeserver_enabled" value="1" <% nvc("pppoeserver_enabled", "1"); %> onclick="show_layer_ext(this, 'idpppoesrv', true)" /><% tran("share.enable"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pppoeserver_enabled" value="0" <% nvc("pppoeserver_enabled", "0"); %> onclick="show_layer_ext(this, 'idpppoesrv', false)" /><% tran("share.disable"); %>
									</div>
								</fieldset>
								<div id="idpppoesrv"><br />
								<fieldset>
									<legend><% tran("service.pppoesrv_srvopt"); %></legend>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_interface"); %></div>
										<% show_ifselect("pppoeserver_interface"); %>
									</div>
<!--			<div id="idpppoerad">
				<div class="setting">
					<div class="label"><% tran("share.ip"); %></div>
					<input class="spaceradio" type="radio" name="pppoeserver_clip" value="radius" <% nvc("pppoeserver_clip", "radius"); %> onclick="show_layer_ext(this, 'idpppoeiploc', false)" /><% tran("radius.legend"); %>
					<input class="spaceradio" type="radio" name="pppoeserver_clip" value="local" <% nvc("pppoeserver_clip", "local"); %> onclick="show_layer_ext(this, 'idpppoeiploc', true)" /><% tran("share.localip"); %>
				</div>
			</div>
			<div id="idpppoeiploc">		-->	
									<div class="setting">
										<div class="label"><% tran("filterIP.ip_range"); %></div>
										<input size="15" maxlength="20" class="num" name="pppoeserver_pool" value="<% nvg("pppoeserver_pool"); %>" />
									</div>			
									<div class="setting">
										<div class="label"><% tran("wl_adv.label10"); %></div>
										<input size="5" maxlength="4" class="num" name="pppoeserver_clcount" value="<% nvg("pppoeserver_clcount"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 64)");
										//]]>
										</script></span>
									</div>
<!--			</div> -->
									<div class="setting">
										<div class="label">Deflate <% tran("service.pppoesrv_compr"); %></div>
										<input type="checkbox" value="1" name="_pppoeserver_deflate" <% nvc("pppoeserver_deflate", "1"); %> />
									</div>
									<div class="setting">
										<div class="label">BSD <% tran("service.pppoesrv_compr"); %></div>
										<input type="checkbox" value="1" name="_pppoeserver_bsdcomp" <% nvc("pppoeserver_bsdcomp", "1"); %> />
									</div>
									<div class="setting">
										<div class="label">LZS Stac <% tran("service.pppoesrv_compr"); %></div>
										<input type="checkbox" value="1" name="_pppoeserver_lzs" <% nvc("pppoeserver_lzs", "1"); %> />
									</div>
									<div class="setting">
										<div class="label">MPPC <% tran("service.pppoesrv_compr"); %></div>
										<input type="checkbox" value="1" name="_pppoeserver_mppc" <% nvc("pppoeserver_mppc", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_encry"); %></div>
										<input type="checkbox" value="1" name="_pppoeserver_encryption" <% nvc("pppoeserver_encryption", "1"); %> />
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_limit"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_sessionlimit" value="<% nvg("pppoeserver_sessionlimit"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 0)");
										//]]>
										</script></span>
									</div>	
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_lcpei"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_lcpechoint" value="<% nvg("pppoeserver_lcpechoint"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 5)");
										//]]>
										</script></span>
									</div>	
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_lcpef"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_lcpechofail" value="<% nvg("pppoeserver_lcpechofail"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 12)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_idlet"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_idle" value="<% nvg("pppoeserver_idle"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 0 = Disable)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mtu"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_mtu" value="<% nvg("pppoeserver_mtu"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1492)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pptpd_mru"); %></div>
										<input size="5" maxlength="5" class="num" name="pppoeserver_mru" value="<% nvg("pppoeserver_mru"); %>" />
										<span class="default"><script type="text/javascript">
										//<![CDATA[
										document.write("(" + share.deflt + ": 1492)");
										//]]>
										</script></span>
									</div>
									<div class="setting">
										<div class="label"><% tran("service.pppoesrv_auth"); %></div>
										<input class="spaceradio" type="radio" name="pppoeradius_enabled" value="1" <% nvc("pppoeradius_enabled", "1"); %> onclick="toggle_layer_ext(this, 'idpppoerad', 'idpppoelocal', true)" /><% tran("radius.legend"); %>&nbsp;
										<input class="spaceradio" type="radio" name="pppoeradius_enabled" value="0" <% nvc("pppoeradius_enabled", "0"); %> onclick="toggle_layer_ext(this, 'idpppoerad', 'idpppoelocal', false)" /><% tran("service.pppoesrv_chaps"); %>
									</div>
								</fieldset><br />
								<div id="idpppoerad">
									<fieldset>
										<legend><% tran("service.pppoesrv_radauth"); %></legend>
											<div class="setting">
												<div class="label"><% tran("service.pppoesrv_radip"); %></div>
												<input maxlength="15" size="20" name="pppoeserver_authserverip" onblur="valid_ip_str(this, share.ip)" value="<% nvg("pppoeserver_authserverip"); %>" />
											</div>
											<div class="setting">
												<div class="label"><% tran("service.pppoesrv_radauthport"); %></div>
												<input size="5" maxlength="5" class="num" name="pppoeserver_authserverport" value="<% nvg("pppoeserver_authserverport"); %>" />
												<span class="default"><script type="text/javascript">
												//<![CDATA[
												document.write("(" + share.deflt + ": 1812)");
												//]]>
												</script></span>
											</div>			
											<div class="setting">
												<div class="label"><% tran("service.pppoesrv_radaccport"); %></div>
												<input size="5" maxlength="5" class="num" name="pppoeserver_acctserverport" value="<% nvg("pppoeserver_acctserverport"); %>" />
												<span class="default"><script type="text/javascript">
												//<![CDATA[
												document.write("(" + share.deflt + ": 1813)");
												//]]>
												</script></span>
											</div>
											<div class="setting">
												<div class="label"><% tran("service.pppoesrv_radkey"); %></div>
												<input size="20" maxlength="63" type="password" autocomplete="new-password" name="pppoeserver_sharedkey" value="d6nw5v1x2pc7st9m" />
											</div>
											<div class="setting">
												<div class="label"><% tran("radius.label23"); %></div>
												<input maxlength="15" size="20" name="pppoeserver_authserverip_backup" value="<% nvg("pppoeserver_authserverip_backup"); %>" />
											</div>
											<div class="setting">
												<div class="label"><% tran("radius.label24"); %></div>
												<input size="5" maxlength="5" class="num" name="pppoeserver_authserverport_backup" value="<% nvg("pppoeserver_authserverport_backup"); %>" />
												<span class="default"><script type="text/javascript">
												//<![CDATA[
												document.write("(" + share.deflt + ": 1812)");
												//]]>
												</script></span>
											</div>			
											<div class="setting">
												<div class="label"><% tran("radius.label14"); %></div>
												<input size="5" maxlength="5" class="num" name="pppoeserver_acctserverport_backup" value="<% nvg("pppoeserver_acctserverport_backup"); %>" />
												<span class="default"><script type="text/javascript">
												//<![CDATA[
												document.write("(" + share.deflt + ": 1813)");
												//]]>
												</script></span>
											</div>
											<div class="setting">
												<div class="label"><% tran("radius.label27"); %></div>
												<input size="20" maxlength="63" type="password" autocomplete="new-password" name="pppoeserver_sharedkey_backup" value="<% nvg("pppoeserver_sharedkey_backup"); %>" />
											</div>
									</fieldset>
								</div>
								<div id="idpppoelocal">
									<fieldset>
										<legend><% tran("service.pppoesrv_chaps"); %></legend>
											<table class="table" summary="chap secrets table">
											<tr>
												<th width="30%"><% tran("share.user"); %></th>
												<th width="30%"><% tran("share.passwd"); %></th>
												<th width="30%"><% tran("share.ip"); %></th>
												<th width="10%" class="center"><% tran("share.enable"); %></th>
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
							</div><br/>
							<div id="footer" class="submitFooter">
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
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term"><% tran("wl_adv.label10"); %>:</dt>
							<dd class="definition"><% tran("hpppoesrv.right3"); %></dd>
							<dt class="term"><% tran("service.pppoesrv_chaps"); %>:</dt>
							<dd class="definition"><% tran("hpppoesrv.right2"); %></dd>
						</dl><br />
							<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HPPPoE_Server.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
