<%% do_pagehead("survey.titl"); %%>
		<script type="text/javascript">
		//<![CDATA[

function do_join (F,SSID) {
	F.wl_ssid.value = SSID;

	if (F.wl_ssid.value == "") {
		alert(errmsg.err47);
		return false;
	}
	apply(F);
}
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
}
		
		//]]>
		</script>
	</head>

	<body class="popup_bg">
		<div id="bulle" class="bulle"></div>
		<form name="Join" action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="Join" />
			<input type="hidden" name="submit_type" value="Join" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<input type="hidden" name="commit" value="1" />
			
			<input type="hidden" name="wl_ssid" />
			<h2><%% tran("survey.h2"); %%></h2>
			<fieldset>
			<legend><%% tran("status_wireless.legend"); %%></legend>
			<table class="table" cellspacing="5" id="survey_table">
				<tr>
				   <th sortdir="up" width="20%"><%% tran("share.ssid"); %%></th>
				   <th sortdir="up" width="7%"><%% tran("share.mode"); %%></th>
				   <th sortdir="up" width="19%"><%% tran("share.mac"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.channel"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.frequency"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.stations"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.radioname"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.rssi"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.noise"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.quality"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.beacon"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.openn"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.dtim"); %%></th>
				   <th sortdir="up" width="5%"><%% tran("share.rates"); %%></th>
				   <th sortdir="up" width="10%" class="center"><%% tran("share.actiontbl"); %%></th>
				</tr>
					<script type="text/javascript">
					//<![CDATA[
					
					var table = new Array(
					<%% dump_site_survey("%s"); %%>
					);
					
					if (table.length == 0) {
						document.write("<tr><td colspan=\"15\" class=\"center\">" + share.none + "</td></tr>");
					}
					else {
						for (var i = 0; i < table.length; i = i + 15) {
							document.write("<tr>");
							document.write("<td>"+table[i]+"</td>");
							document.write("<td align=\"left\">"+table[i+1]+"</td>");
							document.write("<td align=\"left\" style=\"cursor:pointer; text-decoration:underline;\" title=\"OUI Search\" onclick=\"getOUIFromMAC('" + table[i+2] + "')\" >"+table[i+2]+"</td>");
							document.write("<td align=\"right\">"+table[i+3]+"</td>");
							document.write("<td align=\"right\">"+table[i+4]+"</td>");
							document.write("<td align=\"right\">"+table[i+5]+"</td>");
							document.write("<td align=\"left\">"+table[i+6]+"</td>");
							document.write("<td align=\"right\">"+table[i+7]+"</td>");
							document.write("<td align=\"right\">"+table[i+8]+"</td>");
							document.write("<td align=\"right\">"+table[i+9]+"</td>");
							document.write("<td align=\"right\">"+table[i+10]+"</td>");
							document.write("<td align=\"right\" style=\"cursor:pointer; text-decoration:underline;\" onmouseover='DisplayDiv(this, event, 15, 15,\"" + table[i+12] + "\")' onmouseout=\"unDisplayDiv()\">"+table[i+11]+"</td>");
							document.write("<td align=\"right\">"+table[i+13]+"</td>");
							document.write("<td align=\"right\">"+table[i+14]+"</td>");
							document.write("<td align=\"right\" class=\"center\"><input class=\"button\" type=\"button\" value=\"" + sbutton.join + "\" onclick='do_join(this.form,\"" + table[i] + "\")' /></td>");
							document.write("<\/tr>");
						}
					}
					
					//]]>
					</script>
			</table>
			</fieldset><br />
			<script type="text/javascript">
			//<![CDATA[
			var t = new SortableTable(document.getElementById('survey_table'), 1000);
			//]]>
			</script>
			<div id="footer" class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(0,0,0,0,1,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>
