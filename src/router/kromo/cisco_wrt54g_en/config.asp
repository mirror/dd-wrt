<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Backup &amp; Restore</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + config.titl;

function to_submit(F) {
	if (F.file.value == "")	{
//		alert("Please select a configuration file to restore.");
		alert(errmsg.err42);
		return false;
	}
	F.save_button.value = sbutton.saving;
	apply(F);
}

		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
						<div id="menu">
							<div id="menuMain">
								<ul id="menuMainList">
									<li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
									<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
									<% nvram_invmatch("sipgate","1","<!--"); %>
									<li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
									<% nvram_invmatch("sipgate","1","-->"); %>
									<li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
									<li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
									<li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
									<li class="current"><span><% tran("bmenu.admin"); %></span>
										<div id="menuSub">
											<ul id="menuSubList">
												<li><a href="Management.asp"><% tran("bmenu.adminManagement"); %></a></li>
												<li><a href="Hotspot.asp"><% tran("bmenu.adminHotspot"); %></a></li>
												<li><a href="Services.asp"><% tran("bmenu.adminServices"); %></a></li>
												<li><a href="Alive.asp"><% tran("bmenu.adminAlive"); %></a></li>
												<li><a href="Log.asp"><% tran("bmenu.adminLog"); %></a></li>
												<li><a href="Diagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
												<li><a href="Wol.asp"><% tran("bmenu.adminWol"); %></a></li>
												<li><a href="Factory_Defaults.asp"><% tran("bmenu.adminFactory"); %></a></li>
												<li><a href="Upgrade.asp"><% tran("bmenu.adminUpgrade"); %></a></li>
											    <li><span><% tran("bmenu.adminBackup"); %></span></li>
											</ul>
										</div>
									</li>
								    <li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
								</ul>
							</div>
						</div>
					</div>
				    <div id="main">
					<div id="contents">
						<form name="nvramrestore" action="nvram.cgi" method="POST" encType="multipart/form-data">
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
								<script type="text/javascript">document.write("<input type=\"button\" name=\"B1\" value=\"" + sbutton.backup + "\" onclick=\"window.location.href='/nvrambak.bin'\" />")</script>
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.restore + "\" onclick=\"to_submit(this.form)\" />")</script>
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