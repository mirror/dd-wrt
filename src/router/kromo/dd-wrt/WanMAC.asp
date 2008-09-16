<% do_pagehead("wanmac.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.submit_type.value = "";
	F.change_action.value = "";	
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.submit_type.value = "";
	F.change_action.value = "";	
	F.save_button.value = sbutton.saving;
	applytake(F);
}

function CloneMAC(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "clone_mac";
	F.submit();
}

function SelMac(val) {
	show_layer_ext(document.mac.mac_clone_enable,'idmac', val == "1");
	setElementsActive("def_hwaddr", "def_whwaddr_5", val == "1");
}

var update;

addEvent(window, "load", function() {
	SelMac("<% nvram_get("mac_clone_enable"); %>");
	<% onload("MACClone", "document.mac.mac_clone_enable[0].checked = true; SelMac(1);"); %>
	
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
					<% do_menu("index.asp","WanMAC.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="mac" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="WanMAC" />
							<input type="hidden" name="action" value="Apply"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("wanmac.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("wanmac.legend"); %></legend>
								<div class="setting">
									<input class="spaceradio" type="radio" value="1" name="mac_clone_enable" onclick="SelMac(this.value)" <% nvram_checked("mac_clone_enable", "1"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="mac_clone_enable" onclick="SelMac(this.value)" <% nvram_checked("mac_clone_enable", "0"); %> /><% tran("share.disable"); %>
								</div>
								<div id="idmac">
									<div class="setting">
										<div class="label"><% tran("wanmac.wan"); %></div>
										<input type="hidden" name="def_hwaddr" value="6" />
										<input class="num" size="2" maxlength="2" name="def_hwaddr_0" onblur="valid_mac(this,0)" value="<% get_clone_mac("0"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_1" onblur="valid_mac(this,1)" value="<% get_clone_mac("1"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_2" onblur="valid_mac(this,1)" value="<% get_clone_mac("2"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_3" onblur="valid_mac(this,1)" value="<% get_clone_mac("3"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_4" onblur="valid_mac(this,1)" value="<% get_clone_mac("4"); %>" />:<input class="num" size="2" maxlength="2" name="def_hwaddr_5" onblur="valid_mac(this,1)" value="<% get_clone_mac("5"); %>" />
									</div>
									<div class="setting">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" type=\"button\" name=\"clone_b\" value=\"" + sbutton.wanmac + "\" onclick=\"CloneMAC(this.form);\" />");
										//]]>
										</script>
									</div>
									
									<hr width="90%" /><br />
									
									<div class="setting">
										<div class="label"><% tran("wanmac.wlan"); %></div>
										<input type="hidden" name="def_whwaddr" value="6" />
										<input class="num" size="2" maxlength="2" name="def_whwaddr_0" onblur="valid_mac(this,0)" value="<% get_clone_wmac("0"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_1" onblur="valid_mac(this,1)" value="<% get_clone_wmac("1"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_2" onblur="valid_mac(this,1)" value="<% get_clone_wmac("2"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_3" onblur="valid_mac(this,1)" value="<% get_clone_wmac("3"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_4" onblur="valid_mac(this,1)" value="<% get_clone_wmac("4"); %>" />:<input class="num" size="2" maxlength="2" name="def_whwaddr_5" onblur="valid_mac(this,1)" value="<% get_clone_wmac("5"); %>" />
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
					<dl>
						<dt class="term"><% tran("wanmac.legend"); %>:</dt>
						<dd class="definition"><% tran("hwanmac.right2"); %></dd>
					</dl><br />
					<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWanMAC.asp');"><% tran("share.more"); %></a>
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
