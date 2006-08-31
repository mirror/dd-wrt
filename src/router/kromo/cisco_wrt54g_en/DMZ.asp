<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - DMZ</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + dmz.titl;

function to_submit(F) {
	F.submit_button.value = "DMZ";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
		
	F.action.value = "Apply";
	apply(F);
}

function setDMZ(val) {
	setElementActive("dmz_ipaddr", val == "1");
}

addEvent(window, "load", function() {
	setDMZ("<% nvram_get("dmz_enable"); %>");
});
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Forward.asp","DMZ.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="dmz" action="apply.cgi" method="<% get_http_method(); %>" >
							<input type="hidden" name="submit_button" value="DMZ" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" value="Apply" />
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
	                				<input class="num" maxLength="3" size="3" name="dmz_ipaddr" value="<% nvram_get("dmz_ipaddr"); %>" onblur="valid_range(this,1,254,dmz.host)" />
	                			</div>
	                		</fieldset><br />
	                		
	                		<div class="submitFooter">
	                			<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
	                			<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
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
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HDMZ.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>