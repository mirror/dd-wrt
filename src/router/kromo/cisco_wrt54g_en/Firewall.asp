<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Firewall</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + firewall.titl;

function to_submit(F) {
	F.submit_button.value = "Firewall";
	F.block_wan.value = (F._block_wan.checked == true) ? 1 : 0;
	if(F._block_loopback){
		F.block_loopback.value = (F._block_loopback.checked == true) ? 1 : 0;
	}
	if(F._block_cookie){
		F.block_cookie.value = (F._block_cookie.checked == true) ? 1 : 0;
	}
	if(F._block_java){
		F.block_java.value = (F._block_java.checked == true) ? 1 : 0;
	}
	if(F._block_proxy){
		F.block_proxy.value = (F._block_proxy.checked == true) ? 1 : 0;
	}
	if(F._block_activex){
		F.block_activex.value = (F._block_activex.checked == true) ? 1 : 0;
	}
	if(F._ident_pass){
		F.ident_pass.value = (F._ident_pass.checked == true) ? 0 : 1;
	}
	if(F._block_multicast) {
		F.multicast_pass.value = (F._block_multicast.checked == true) ? 0 : 1;
	}
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

function setFirewall(val) {
	setElementsActive("_block_proxy", "_ident_pass", val == "on");
}

addEvent(window, "load", function() {
	setFirewall("<% nvram_get("filter"); %>");
});

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
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li class="current"><span><% tran("bmenu.security"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><span><% tran("bmenu.firwall"); %></span></li>
											<li><a href="VPN.asp"><% tran("bmenu.vpn"); %></a></li>
										</ul>
									</div>
								</li>
								<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
								<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
								<li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
								<li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
				<div id="contents">
					<form name="firewall" action="apply.cgi" method="<% get_http_method(); %>" >
						<input type="hidden" name="submit_button" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="action" />
						<input type="hidden" name="block_wan" />
						<input type="hidden" name="block_loopback" />
						<input type="hidden" name="multicast_pass" value="0" />
						<input type="hidden" name="ident_pass" />
						<input type="hidden" name="block_cookie" value="0" />
						<input type="hidden" name="block_java" value="0" />
						<input type="hidden" name="block_proxy" value="0" />
						<input type="hidden" name="block_activex" value="0" />
						<h2><% tran("firewall.h2"); %></h2>
						
						<fieldset>
							<legend><% tran("firewall.legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("firewall.firewall"); %></div>
								<input class="spaceradio" type="radio" value="on" name="filter" <% nvram_checked("filter", "on"); %> onclick="setFirewall(this.value)" ><% tran("share.enable"); %></input>&nbsp;
								<input class="spaceradio" type="radio" value="off" name="filter" <% nvram_checked("filter", "off"); %> onclick="setFirewall(this.value)" ><% tran("share.disable"); %></input>
							</div>
						</fieldset><br />
						
						<fieldset>
							<legend><% tran("firewall.legend2"); %></legend>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_proxy" <% nvram_checked("block_proxy", "1"); %> ><% tran("firewall.proxy"); %></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_cookie" <% nvram_checked("block_cookie", "1"); %> ><% tran("firewall.cookies"); %></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_java" <% nvram_checked("block_java", "1"); %> ><% tran("firewall.applet"); %></input>
								</div>
								<div class="setting">
									<input class="spaceradio" type="checkbox" value="1" name="_block_activex" <% nvram_checked("block_activex", "1"); %> ><% tran("firewall.activex"); %></input>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("firewall.legend3"); %></legend>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_block_wan" <% nvram_checked("block_wan", "1"); %> ><% tran("firewall.ping"); %></input>
									</div>
									<% support_invmatch("MULTICAST_SUPPORT", "1", "<!--"); %>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="0" name="_block_multicast" <% nvram_checked("multicast_pass", "0"); %> ><% tran("firewall.muticast"); %></input>
									</div>
									<% support_invmatch("MULTICAST_SUPPORT", "1", "-->"); %>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="0" name="_block_loopback" <% nvram_checked("block_loopback", "1"); %> ><% tran("filter.nat"); %></input>
									</div>
									<div class="setting">
										<input class="spaceradio" type="checkbox" value="1" name="_ident_pass" <% nvram_checked("ident_pass", "0"); %> ><% tran("filter.port113"); %></input>
									</div>
								</fieldset><br />
								<div class="submitFooter">
									<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
									<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"setFirewall('<% nvram_get("filter"); %>')\" />")</script>
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