<% do_pagehead("upgrad.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

function process_aborted(F) {
	bar1.togglePause();
	alert(fail.mess2);
	window.location.replace("Upgrade.asp");
	return false;
}

function upgrade(F,id) {
	var upgrade_file = '';
	if (F.upgrade_files) {
		if( !F.upgrade_files.value ) {
			for(i = 0; i < F.upgrade_files.length; i++) {
				if( F.upgrade_files[i].checked == true) {
					upgrade_file = F.upgrade_files[i].value;
				}
			}
		} else if( F.upgrade_files.checked ) {
			upgrade_file = F.upgrade_files.value;
		}
	}
	if(upgrade_file) {
		F.upgrade_file.value = upgrade_file;
		//$(F.id).setAttribute('action', 'olupgrade.cgi');
		//$(F.id).setAttribute('enctype', '');
		F.action = 'olupgrade.cgi';
		F.removeAttribute('enctype');
		$('submit_button').remove();
	} else if (F.file.value == "") {
		alert(errmsg.err60);
		return false;
	}

	choose_disable(F.Upgrade_b);
	bar1.togglePause();
	change_style(id,'textblink');
	F.Upgrade_b.value = sbutton.upgrading;
	applyupdate(F, errmsg.err102,share.secondcharacter, <% nvg("upgrade_delay"); %>);
	return true;
}

function getUpgrades(F) {
	//$(F.id).setAttribute('action', 'apply.cgi');
	//$(F.id).setAttribute('enctype', '');
	F.action = 'apply.cgi';
	F.removeAttribute('enctype');
	//F.action.value = 'Apply';
	$('submit_action').value = 'Apply';
	$('submit_action').name = 'action';
	F.change_action.value = 'gozila_cgi';
	F.submit_button.value = 'Upgrade';
	F.submit_type.value = 'get_upgrades';
	F.submit();
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
					<% do_menu("Management.asp","Upgrade.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="firmware" id="firmware" method="post" action="upgrade.cgi" enctype="multipart/form-data">
							<input type="hidden" name="submit_button" id="submit_button" />
							<input type="hidden" name="_action" id="submit_action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />

							<h2><% tran("upgrad.h2"); %></h2>
							<fieldset>
								<legend><% tran("upgrad.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("upgrad.info1"); %></div>
									<select name="erase">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" >" + upgrad.resetOff + "</option>");
										document.write("<option value=\"1\" >" + upgrad.resetOn + "</option>");
										//]]>
										</script>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("upgrad.file"); %></div>
									<input type="file" name="file" size="40"/>
								</div>
							</fieldset><br />
							<% show_upgrade_options(); %>
							<div class="warning">
								<div id="warning_text"><p><b><% tran("upgrad.warning"); %></b></p></div>
								<p><% tran("upgrad.mess1"); %></p><br/>
								<div class="center"><script type="text/javascript">
								//<![CDATA[
								var bar1 = createBar(500,15,100,15,200,"process_aborted(this.form)");
								bar1.togglePause();
								//]]>
								</script></div><br />
							</div><br />
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button green_btn\" type=\"button\" aria-label=\"" + sbutton.upgrade + "\" name=\"Upgrade_b\" value=\"" + sbutton.upgrade + "\" onclick=\"upgrade(this.form,'warning_text');\" />");
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
							<dt class="term"><% tran("upgrad.legend"); %>: </dt>
							<dd class="definition"><% tran("hupgrad.right2"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HUpgrade.asp');"><% tran("share.more"); %></a>
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
