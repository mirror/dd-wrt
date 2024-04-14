<% do_pagehead("config.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function to_submit(F) {
	if (F.file.value == "")	{
		alert(errmsg.err42);
		return false;
	}
	F.save_button.value = sbutton.restored;
	applyupdate(F, errmsg.err101,share.secondcharacter);
	return true;
}

var update;

addEvent(window, "load", function() {
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
									<div class="label"><% tran("config.force"); %></div>
									<input type="checkbox" value="1" name="force" />
								</div>
								<div class="setting">
									<div class="label"><% tran("config.mess2"); %></div>
									<input type="file" name="file" size="40" />
								</div>
							</fieldset><br />
							<div class="warning">
								<p><b><% tran("config.mess3"); %></b></p>
								<p><% tran("config.mess4"); %></p>
							</div><br />
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"backup_button\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/<% get_backup_name(); %>';\" />");
								document.write("<input class=\"button\" type=\"button\" name=\"save_button\" value=\"" + sbutton.restore + "\" onclick=\"to_submit(this.form);\" />");
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
						<dl>
							<dt class="term"><% tran("config.legend"); %>:</dt>
							<dd class="definition"><% tran("hconfig.right2"); %></dd>
							<dt class="term"><% tran("config.legend2"); %>:</dt>
							<dd class="definition"><% tran("hconfig.right4"); %></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HBackup.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
