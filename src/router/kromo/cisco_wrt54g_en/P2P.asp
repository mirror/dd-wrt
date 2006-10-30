<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Peer to Peer Apps</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + p2p.titl;


function to_submit(F) {
	F.submit_button.value = "Forward";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
	show_layer_ext(document.p2p.ctorrent_enable, 'idctorrent', <% nvram_else_match("ctorrent_enable", "1", "1", "0"); %> == 1);

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
				<% do_menu("Forward.asp","P2P.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="p2p" action="apply.cgi" method="<% get_http_method(); %>" >
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><% tran("p2p.h2"); %></h2>
							
							<% show_modules(".p2pwebconfig"); %>
						 	
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\">");
								//]]>
								</script>
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\">");
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
				<!--
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("p2p.h2"); %>:</dt>
							<dd class="definition"><% tran("hp2p.right2"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HP2P.asp')"><% tran("share.more"); %></a>
					</div>
					-->
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>