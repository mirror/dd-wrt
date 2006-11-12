<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<link rel="icon" href="images/favicon.ico" type="image/x-icon" />
		<link rel="shortcut icon" href="images/favicon.ico" type="image/x-icon" />
		<script type="text/javascript" src="../common.js"></script>
		<script type="text/javascript" src="../lang_pack/english.js"></script>
		<script type="text/javascript" src="../lang_pack/language.js"></script>

		<link type="text/css" rel="stylesheet" href="../style/elegant/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="../style/elegant/style_ie.css" /><![endif]-->



<script type="text/javascript" src="prototype.js"></script>
<script type="text/javascript" src="effects.js"></script>
<script type="text/javascript" src="window.js"></script>
<link href="themes/default.css" rel="stylesheet" type="text/css" >	 </link>
<link href="themes/ddwrt.css" rel="stylesheet" type="text/css" >	 </link>

		<title><% nvram_get("router_name"); %> - Log</title>
		<script type="text/javascript">
//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + log.titl;

function to_submit(F) {
	F.submit_button.value = "Log";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";



openInfoDialog();
window.location.reload();

	//apply(F);
}


var errmsg = new Object();
errmsg.err100="Processing...<br/> Please, wait.";

function openInfoDialog() {
	Dialog.info(errmsg.err100, {windowParameters: {className: "ddwrt",width:250, height:100}, showProgress: true});
	}
	



addEvent(window, "load", function() {
	show_layer_ext(document.log.log_enable, 'idlog1', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.log.log_enable, 'idlog2', <% nvram_else_match("log_enable", "1", "1", "0"); %> == 1);
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
					<% do_menu("Management.asp","Log.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="log" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="action" />
							<h2><% tran("log.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("log.legend"); %></legend>
								<div class="setting">
									<div class="label">Log</div>
									<input class="spaceradio" type="radio" value="1" name="log_enable" <% nvram_checked("log_enable", "1"); %> onclick="show_layer_ext(this, 'idlog1', true);show_layer_ext(this,'idlog2', true)" /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="log_enable" <% nvram_checked("log_enable", "0"); %> onclick="show_layer_ext(this, 'idlog1', false);show_layer_ext(this,'idlog2', false)" /><% tran("share.disable"); %>
								</div>
							<div id="idlog1">
								<div class="setting">
									<div class="label"><% tran("log.lvl"); %></div>
									<select name="log_level">
										<script type="text/javascript">
										//<![CDATA[
										document.write("<option value=\"0\" <% nvram_selected("log_level", "0", "js"); %> >" + share.low + "</option>");
										document.write("<option value=\"1\" <% nvram_selected("log_level", "1", "js"); %> >" + share.medium + "</option>");
										document.write("<option value=\"2\" <% nvram_selected("log_level", "2", "js"); %> >" + share.high + "</option>");
										//]]>
										</script>
									</select>
								</div>
							</div>
							</fieldset><br />
							
							<div id="idlog2">
								<fieldset>
									<legend><% tran("share.option"); %></legend>
									<div class="setting">
										<div class="label"><% tran("log.drop"); %></div>
										<select name="log_dropped">
											<option value="0" <% nvram_selected("log_dropped", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_dropped", "1"); %>>On</option>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.reject"); %></div>
										<select name="log_rejected">
											<option value="0" <% nvram_selected("log_rejected", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_rejected", "1"); %>>On</option>
										</select>
									</div>
									<div class="setting">
										<div class="label"><% tran("log.accept"); %></div>
										<select name="log_accepted">
											<option value="0" <% nvram_selected("log_accepted", "0"); %>>Off</option>
											<option value="1" <% nvram_selected("log_accepted", "1"); %>>On</option>
										</select>
									</div>
								</fieldset><br />

								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input type=\"button\" name=\"log_incoming\" value=\"" + sbutton.log_in + "\" onclick=\"openWindow('Log_incoming.asp', 580, 600)\" />");
									document.write("<input type=\"button\" name=\"log_outgoing\" value=\"" + sbutton.log_out + "\" onclick=\"openWindow('Log_outgoing.asp', 760, 600)\" />");
									//]]>
									</script>
								</div><br />
							</div>
							
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form);\" />");
								document.write("<input type=\"button\" name=\"reset_button\" value=\"" + sbutton.cancel + "\" onclick=\"window.location.reload();\" />");
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HLog.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>