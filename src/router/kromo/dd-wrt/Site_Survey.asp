<% do_pagehead("survey.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function do_join (F,SSID) {
	F.wl_ssid.value = SSID;
	F.ath0_ssid.value = SSID;
	
	<% nvram_invmatch("wl_mode", "ap", "/"); %><% nvram_invmatch("wl_mode", "ap", "/"); %>F.wl_mode.value="<% nvram_match("wl_mode", "ap", "sta"); %>"
	<% nvram_invmatch("ath0_mode", "ap", "/"); %><% nvram_invmatch("ath0_mode", "ap", "/"); %>F.ath0_mode.value="<% nvram_match("ath0_mode", "ap", "sta"); %>"

	if (F.wl_ssid.value == "") {
		alert(errmsg.err47);
		return false;
	}
	apply(F);
}
		
		//]]>
		</script>
	</head>

	<body>
		<div id="bulle" class="bulle"></div>
		<form name="wireless" action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="Join" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="commit" value="1" />
			
			<input type="hidden" name="wl_ssid" />
			<input type="hidden" name="wl_mode" />
			<input type="hidden" name="ath0_ssid" />
			<input type="hidden" name="ath0_mode" />
			<h2><% tran("survey.h2"); %></h2>
			<table class="center table" cellspacing="5" id="survey_table">
				<tr>
				   <th width="25%"><% tran("share.ssid"); %></th>
				   <th width="7%">Mode</th>
				   <th width="19%"><% tran("share.mac"); %></th>
				   <th width="6%"><% tran("share.channel"); %></th>
				   <th width="6%"><% tran("share.rssi"); %></th>
				   <th width="6%"><% tran("share.noise"); %></th>
				   <th width="6%"><% tran("share.beacon"); %></th>
				   <th width="6%"><% tran("share.openn"); %></th>
				   <th width="6%"><% tran("share.dtim"); %></th>
				   <th width="6%"><% tran("share.rates"); %></th>
				   <th width="7%"><% tran("survey.thjoin"); %></th>
				</tr>
					<script type="text/javascript">
					//<![CDATA[
					
					var table = new Array(
					<% dump_site_survey(""); %>
					);
					
					if (table.length == 0) {
						document.write("<tr><td colspan=\"11\" align=\"center\">" + share.none + "</td></tr>");
					}
					else {
						for (var i = 0; i < table.length; i = i + 11) {
							document.write("<tr>");
							document.write("<td>"+table[i]+"</td>");
							document.write("<td align=\"left\">"+table[i+1]+"</td>");
							document.write("<td align=\"left\" style=\"cursor:pointer; text-decoration:underline;\" title=\"OUI Search\" onclick=\"getOUIFromMAC('" + table[i+2] + "')\" >"+table[i+2]+"</td>");
							document.write("<td align=\"right\">"+table[i+3]+"</td>");
							document.write("<td align=\"right\">"+table[i+4]+"</td>");
							document.write("<td align=\"right\">"+table[i+5]+"</td>");
							document.write("<td align=\"right\">"+table[i+6]+"</td>");
							document.write("<td align=\"right\" style=\"cursor:pointer; text-decoration:underline;\" onmouseover='DisplayDiv(this, event, 15, 15,\"" + table[i+8] + "\")' onmouseout=\"unDisplayDiv()\">"+table[i+7]+"</td>");
							document.write("<td align=\"right\">"+table[i+9]+"</td>");
							document.write("<td align=\"right\">"+table[i+10]+"</td>");
							document.write("<td align=\"right\"><input class=\"button\" type=\"button\" value=\"" + sbutton.join + "\" onclick='do_join(this.form,\"" + table[i] + "\")' /></td>");
							document.write("<\/tr>");
						}
					}
					
					//]]>
					</script>
			</table>
			<script type="text/javascript">
			//<![CDATA[
			var t = new SortableTable(document.getElementById('survey_table'), 1000);
			//]]>
			</script>
			<br />
			<div class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(0,0,0,0,1,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>
