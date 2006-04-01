<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - MAC Address Filter List</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
var active_win = null;

function ViewActive() {
	active_win = self.open('WL_ActiveTable.asp','ActiveTable','alwaysRaised,resizable,scrollbars,width=650,height=450');
	active_win.focus();
}

function to_submit_mac(F) {
	F.submit_button.value = "WL_FilterTable";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function SelPage(num,F) {
	F.submit_button.value = "WL_FilterTable";
	F.change_action.value = "gozila_cgi";
	F.wl_filter_page.value=F.wl_filter_page.options[num].value;
	F.submit();
}

function valid_macs_all(I) {
	if(I.value == "") {
		return true;
	} else if(I.value.length == 12) {
		valid_macs_12(I);
	} else if(I.value.length == 17) {
		valid_macs_17(I);
	} else {
		alert("The MAC Address length is not correct !");
		I.value = I.defaultValue;
	}
}

function exit() {
	closeWin(active_win);
}
		</script>
	</head>
	
	<body onload="window.focus();" onunload="exit()">
		<form name="macfilter" action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="action" />
			<input type="hidden" name="wl_mac_list" />
			<input type="hidden" name="small_screen" />
			<div id="main">
				<div id="contentsInfo">
					<h2>MAC Address Filter List</h2>
					<table width="100%" >
						<tr>
							<TD align="left">Enter MAC Address in this format&nbsp;:&nbsp;&nbsp;&nbsp;xx:xx:xx:xx:xx:xx</TD>
							<TD align="right" ><input type="button" id="button4" name="button5" value="Wireless Client MAC List" onclick="ViewActive()"/></TD>
						</tr>
					</table><br/>
					
					<% wireless_filter_table("input"); %>
					
					
					<div class="submitFooter">
						<input type="button" name="save_button" value="Save Settings" onclick="to_submit_mac(this.form)" />
						<input type="reset" value="Cancel Changes" />
						<input onclick="self.close()" type="reset" value="Close" />
					</div>
				</div>
			</div>
		</form>
	</body>
</html>