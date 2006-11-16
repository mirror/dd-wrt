<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Firewall</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + firewall.titl;

function to_submit(F) {
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
	apply(F);
}

function setFirewall(val) {
	setElementsActive("_block_proxy", "_block_ident", val == "on");
}

addEvent(window, "load", function() {
	setFirewall("<% nvram_get("filter"); %>");
	//show_layer_ext(document.firewall.log_enable, 'idfilter', <% nvram_else_match("filter", "on", "on", "off"); %> == 'on');
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
					<form name="firewall" action="apply.cgi" method="<% get_http_method(); %>" >
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
										
										<% support_invmatch("MULTICAST_SUPPORT", "1", "<!--"); %>
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_multicast" <% nvram_checked("block_multicast", "1"); %> /><% tran("firewall.muticast"); %>
										</div>
										<% support_invmatch("MULTICAST_SUPPORT", "1", "-->"); %>
										
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_loopback" <% nvram_checked("block_loopback", "1"); %> /><% tran("filter.nat"); %>
										</div>
										<div class="setting">
											<input class="spaceradio" type="checkbox" value="1" name="_block_ident" <% nvram_checked("block_ident", "1"); %> /><% tran("filter.port113"); %>
										</div>
									</fieldset><br />
								</div>
								
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HFirewall.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
			<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>