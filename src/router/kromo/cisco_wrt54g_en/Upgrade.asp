<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Firmware Upgrade</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + upgrad.titl;

function process_aborted(F) {
	bar1.togglePause();
	alert(fail.mess2);
	window.location.replace("Upgrade.asp")
	return false;
}

function upgrade(F,id) {
	if (F.file.value == "")	{
		alert(errmsg.err60);
		return false;
	}
	var len = F.file.value.length;
	var ext = new Array('.','b','i','n');
	var IMAGE = F.file.value.toLowerCase();
	for (i=0; i < 4; i++)	{
		if (ext[i] != IMAGE.charAt(len-4+i)){
			alert(errmsg.err61);
			return false;
		}
	}

	choose_disable(F.Upgrade_b);
	bar1.togglePause();
	change_style(id,'textblink');
	F.Upgrade_b.value = sbutton.upgrading;
	
	F.submit_button.value = "Upgrade";
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
					<% do_menu("Management.asp","Upgrade.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="firmware" method="post" action="upgrade.cgi" enctype="multipart/form-data">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<h2><% tran("upgrad.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("upgrad.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("upgrad.info1"); %></div>
									<select name="erase">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selmatch("erase", "0", "selected"); %> >" + upgrad.resetOff + "</option>");
										document.write("<option value=\"1\" <% nvram_selmatch("erase", "1", "selected"); %> >" + upgrad.resetOn + "</option>");
										//]]>
										</script>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("upgrad.file"); %></div>
									<input type="file" name="file" size="40"/>
								</div>
							</fieldset><br />
							
							<div class="warning">
								<div id="warning_text"><p><b><% tran("upgrad.warning"); %></b></p></div>
								<p><% tran("upgrad.mess1"); %></p><br/>
								<div align="center"><script type="text/javascript">
								//<![CDATA[
								var bar1 = createBar(500,15,100,15,50,"process_aborted(this.form)");
								bar1.togglePause();
								//]]>
								</script></div><br />
							</div><br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" name=\"Upgrade_b\" value=\"" + sbutton.upgrade + "\" onclick=\"upgrade(this.form,'warning_text');\" />");
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
							<dt class="term"><% tran("upgrad.legend"); %>: </dt>
							<dd class="definition"><% tran("hupgrad.right2"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HUpgrade.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>
