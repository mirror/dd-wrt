<% do_pagehead("dmz.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function submitcheck(F) {
	F.save_button.value = sbutton.saving;
}

function to_submit(F) {
    submitcheck(F);
    apply(F);
}
function to_apply(F) {
    submitcheck(F);
    applytake(F);
}

function setDMZ(val) {
	setElementActive("dmz_ipaddr", val == "1");
}

var update;

addEvent(window, "load", function() {
	setDMZ("<% nvram_get("dmz_enable"); %>");
	
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
				<% do_menu("ForwardSpec.asp","DMZ.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="dmz" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="DMZ" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<h2><% tran("dmz.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("dmz.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("dmz.serv"); %></div>
									<input class="spaceradio" type="radio" value="1" name="dmz_enable" onclick="setDMZ(this.value)" <% nvram_checked("dmz_enable", "1"); %> /><% tran("share.enable"); %>&nbsp;
					<input class="spaceradio" type="radio" value="0" name="dmz_enable" onclick="setDMZ(this.value)" <% nvram_checked("dmz_enable", "0"); %> /><% tran("share.disable"); %>
				</div>
				<div class="setting">
					<div class="label"><% tran("dmz.host"); %></div>
					<% prefix_ip_get("lan_ipaddr",1); %>
					<input class="num" maxlength="3" size="3" name="dmz_ipaddr" value="<% nvram_get("dmz_ipaddr"); %>" onblur="valid_range(this,1,254,dmz.host)" />
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
							<dt class="term"><% tran("dmz.legend"); %>:</dt>
							<dd class="definition"><% tran("hdmz.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HDMZ.asp');"><% tran("share.more"); %></a>
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