<% do_pagehead("firewall.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function submitcheck(F) {
	if(F._block_proxy){
		F.block_proxy.value = F._block_proxy.checked ? 1 : 0;
	}
	if(F._block_cookie){
		F.block_cookie.value = F._block_cookie.checked ? 1 : 0;
	}
	if(F._block_java){
		F.block_java.value = F._block_java.checked ? 1 : 0;
	}
		if(F._block_activex){
		F.block_activex.value = F._block_activex.checked ? 1 : 0;
	}
	
	if (F._block_wan){
		F.block_wan.value = F._block_wan.checked ? 1 : 0;
	}
	if(F._block_multicast) {
		F.block_multicast.value = F._block_multicast.checked ? 1 : 0;
	}
	if(F._block_loopback){
		F.block_loopback.value = F._block_loopback.checked ? 1 : 0;
	}
	if(F._block_ident){
		F.block_ident.value = F._block_ident.checked ? 1 : 0;
	}

	F.save_button.value = sbutton.saving;
	
}

function to_submit(F)
{
    submitcheck(F);
    apply(F);
}
function to_apply(F)
{
    submitcheck(F);
    applytake(F);
}

function setFirewall(val) {
<% ifdef("MICRO", "/"); %><% ifdef("MICRO", "/"); %>if (val != "on") document.firewall.log_enable[1].click();
<% ifdef("MICRO", "/"); %><% ifdef("MICRO", "/"); %>setElementsActive("_block_proxy", "log_level", val == "on");
<% ifdef("MICRO", "/"); %><% ifdef("MICRO", "/"); %>setElementsActive("_block_proxy", "_block_ident", val == "on");
}

var update;

addEvent(window, "load", function() {
	setFirewall("<% nvram_get("filter"); %>");
	show_layer_ext(document.firewall.log_enable, 'idlog1', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.firewall.log_enable, 'idlog2', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
	
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
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<% do_menu("Firewall.asp","Firewall.asp"); %>
				</div>
				<div id="main">
				<div id="contents">
					<form name="firewall" action="apply.cgi" method="post" >
						<input type="hidden" name="submit_button" value="Firewall" />
						<input type="hidden" name="action" value="Apply" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="submit_type" />
						
						<input type="hidden" name="block_wan" />
						<input type="hidden" name="block_loopback" />
						<input type="hidden" name="block_multicast" />
						<input type="hidden" name="block_ident" />
						<input type="hidden" name="block_cookie" />
						<input type="hidden" name="block_java" />
						<input type="hidden" name="block_proxy" />
						<input type="hidden" name="block_activex" />
						<h2><% tran("firewall.h2"); %></h2>
						
						<fieldset>
							<legend><% tran("firewall.legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("firewall.firewall"); %></div>
								<input class="spaceradio" type="radio" value="on" name="filter" <% nvram_checked("filter", "on"); %> onclick="setFirewall(this.value);" /><% tran("share.enable"); %>&nbsp;
								<input class="spaceradio" type="radio" value="off" name="filter" <% nvram_checked("filter", "off"); %> onclick="setFirewall(this.value);" /><% tran("share.disable"); %>
							</div>
						</fieldset><br />
						
						<div id="idfilter">
							<fieldset>
								<legend><% tran("firewall.legend2"); %></legend>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_proxy" <% nvram_checked("block_proxy", "1"); %> /><% tran("firewall.proxy"); %>
									</div>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_cookie" <% nvram_checked("block_cookie", "1"); %> /><% tran("firewall.cookies"); %>
									</div>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_java" <% nvram_checked("block_java", "1"); %> /><% tran("firewall.applet"); %>
									</div>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_activex" <% nvram_checked("block_activex", "1"); %> /><% tran("firewall.activex"); %>
									</div>
								</fieldset><br />
								
								<fieldset>
									<legend><% tran("firewall.legend3"); %></legend>
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_wan" <% nvram_checked("block_wan", "1"); %> /><% tran("firewall.ping"); %>
										</div>
										
										<% ifndef("MULTICAST", "<!--"); %>
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_multicast" <% nvram_checked("block_multicast", "1"); %> /><% tran("firewall.muticast"); %>
										</div>
										<% ifndef("MULTICAST", "-->"); %>
										
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_loopback" <% nvram_checked("block_loopback", "1"); %> /><% tran("filter.nat"); %>
										</div>
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_ident" <% nvram_checked("block_ident", "1"); %> /><% tran("filter.port113"); %>
										</div>
									</fieldset><br />
								</div>
								<% ifdef("MICRO", "<!--"); %>
								<h2><% tran("log.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("log.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("log.legend"); %></div>
									<input class="spaceradio" type="radio" value="1" name="log_enable" <% nvram_checked("log_enable", "1"); %> onclick="show_layer_ext(this, 'idlog1', true);show_layer_ext(this,'idlog2', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="log_enable" <% nvram_checked("log_enable", "0"); %> onclick="show_layer_ext(this, 'idlog1', false);show_layer_ext(this,'idlog2', false)" /><% tran("share.disable"); %>
								</div>
							<div id="idlog1">
								<div class="setting">
									<div class="label"><% tran("log.lvl"); %></div>
									<select name="log_level">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected_js("log_level", "0"); %> >" + share.low + "</option>");
										document.write("<option value=\"1\" <% nvram_selected_js("log_level", "1"); %> >" + share.medium + "</option>");
										document.write("<option value=\"2\" <% nvram_selected_js("log_level", "2"); %> >" + share.high + "</option>");
										//]]>
										</script>
									</select>
								</div>
							</div>
							</fieldset><br />
							
							<div id="idlog2">
								<fieldset>
									<legend><% tran("share.option"); %></legend>
									<div class="setting">
										<div class="label"><% tran("log.drop"); %></div>
										<select name="log_dropped">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected_js("log_dropped", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvram_selected_js("log_dropped", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.reject"); %></div>
										<select name="log_rejected">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected_js("log_rejected", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvram_selected_js("log_rejected", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.accept"); %></div>
										<select name="log_accepted">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected_js("log_accepted", "0"); %> >" + share.disable + "</option>");
										document.write("<option value=\"1\" <% nvram_selected_js("log_accepted", "1"); %> >" + share.enable + "</option>");
										//]]>
										</script>
										</select>
									</div>
								</fieldset><br />

								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"log_incoming\" value=\"" + sbutton.log_in + "\" onclick=\"openWindow('Log_incoming.asp', 580, 600);\" />");
									document.write("<input class=\"button\" type=\"button\" name=\"log_outgoing\" value=\"" + sbutton.log_out + "\" onclick=\"openWindow('Log_outgoing.asp', 760, 600);\" />");
									//]]>
									</script>
								</div><br />
							</div>
								<% ifdef("MICRO", "-->"); %>								
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
							<dt class="term"><% tran("firewall.legend"); %>:</dt>
							<dd class="definition"><% tran("hfirewall.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HFirewall.asp');"><% tran("share.more"); %></a>
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