<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Networking</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %> - Networking";

function to_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save_networking";
	F.save_button.value = sbutton.saving;
	apply(F);
}

function vlan_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_vlan";
	F.submit();
}
function bridge_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_bridge";
	F.submit();
}
function bridgeif_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_bridgeif";
	F.submit();
}

function vlan_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_vlan";
	F.del_value.value=I;
	F.submit();
}
function bridge_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_bridge";
	F.del_value.value=I;
	F.submit();
}
function bridgeif_del_submit(F,I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "del_bridgeif";
	F.del_value.value=I;
	F.submit();
}

function init() {

}
		
		//]]>
		</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","Networking.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="vlan" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Networking" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="del_value" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />

							<h2>VLAN Tagging</h2>
							<fieldset>
							   <legend>Tagging</legend>
							   <% show_vlantagging(); %>
							</fieldset>
							<h2>Bridging</h2>
							<fieldset>
							   <legend>Create Bridge</legend>
							   <% show_bridgenames(); %>
							</fieldset>
							<fieldset>
							   <legend>Assign to Bridge</legend>
							   <% show_bridgeifnames(); %>
							</fieldset>
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
						<br />
						<!-- <a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HNetworking.asp');"><% tran("share.more"); %></a> -->
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>