<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Factory Defaults</title>
		<script type="text/javascript"><![CDATA[
document.title = "<% nvram_get("router_name"); %>" + factdef.titl;

function to_submit(F) {
	if( F.FactoryDefaults[0].checked == 1 ) {
		if(!confirm(factdef.mess1)) {
			return false;
		}
		F.submit_button.value = "Factory_Defaults";
//		F.save_button.value = "Saved";
		F.save_button.value = sbutton.saving;
		
		F.action.value="Restore";
		apply(F);
	}
}
		]]></script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Management.asp","Factory_Defaults.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="default" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<h2><% tran("factdef.h2"); %></h2>
							
							<fieldset>
							<legend><% tran("factdef.legend"); %></legend>
							<div class="setting">
								<div class="label"><% tran("factdef.restore"); %></div>
								<input class="spaceradio" type="radio" name="FactoryDefaults" value="1" /><% tran("share.yes"); %>&nbsp;
								<input class="spaceradio" type="radio" name="FactoryDefaults" value="0" checked="checked" /><% tran("share.no"); %>
							</div>
							</fieldset><br/>
							
							<div class="submitFooter">
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")
]]></script>
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")
]]></script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dd class="definition"><% tran("hfactdef.right1"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HDefault.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>