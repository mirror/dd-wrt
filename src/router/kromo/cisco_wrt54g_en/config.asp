<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Backup &amp; Restore</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + config.titl;

function to_submit(F) {
	if (F.file.value == "")	{
		alert(errmsg.err42);
		return false;
	}
	F.save_button.value = sbutton.saving;
	apply(F);
}

		//]]>
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Management.asp","config.asp"); %>
				</div>
				    <div id="main">
					<div id="contents">
						<form name="nvramrestore" action="nvram.cgi" method="post" enctype="multipart/form-data">
							<h2><% tran("config.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("config.legend"); %></legend>
								<div class="setting">
									<% tran("config.mess1"); %>
								</div>
							</fieldset><br />
							
							<h2><% tran("config.h22"); %></h2>
							<fieldset>
								<legend><% tran("config.legend2"); %></legend>
								<div class="setting">
									<div class="label"><% tran("config.mess2"); %></div>
									<input type="file" name="file" size="40" />
								</div>
							</fieldset><br />
							
							<div class="warning">
								<p><b><% tran("config.mess3"); %></b></p>
								<p><% tran("config.mess4"); %></p>
							</div><br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"B1\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/nvrambak.bin'\" />");
								document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.restore + "\" onclick=\"to_submit(this.form);\" />");
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
							<dt class="term"><% tran("config.legend"); %>:</dt>
							<dd class="definition"><% tran("hconfig.right2"); %></dd>
							<dt class="term"><% tran("config.legend2"); %>:</dt>
							<dd class="definition"><% tran("hconfig.right4"); %></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HBackup.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>