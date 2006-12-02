<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Site Survey</title>
		<script type="text/javascript" src="js/wiviz.js"></script>
		<link type="text/css" rel="stylesheet" href="style/wiviz.css" />
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + survey.titl;

function do_join (F,SSID) {
	F.wl_ssid.value = SSID;
	<% nvram_invmatch("wl_mode", "ap", "/"); %><% nvram_invmatch("wl_mode", "ap", "/"); %>F.wl_mode.value="<% nvram_match("wl_mode", "ap", "sta"); %>"

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
		<form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" value="Join" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="commit" value="1" />
			
			<input type="hidden" name="wl_ssid" />
			<input type="hidden" name="wl_mode" />
			<h2><% tran("survey.h2"); %></h2>
			
			<div class='floater'><% tran("share.statu"); %>:<span id="status" class="status">Monitoring</span>
			<br/>
			<input type="button" id="togglelisten" value="Stop monitoring" onclick="toggleListen()" />
			<p>
				<form name="channelform" action="/cgi-bin/wiviz-set.cgi" method="get" target="wivizGetFrame" />Channel setting:
					<select id="channelsel" name="channelsel" onchange="channelSet()">
						<option value='nochange' selected>No change</option>
						<option value='hop'>Hopping</option>
						<option>1</option>
						<option>2</option>
						<option>3</option>
						<option>4</option>
						<option>5</option>
						<option>6</option>
						<option>7</option>
						<option>8</option>
						<option>9</option>
						<option>10</option>
						<option>11</option>
						<option>12</option>
						<option>13</option>
						<option>14</option>
					</select>
					<br/>
					<span id="hopoptions" style="display: none">Time/channel: 
						<select name="hopdwell">
							<option value='500'>0.5 sec</option>
							<option selected value='1000'>1 sec</option>
							<option value='2000'>2 sec</option>
							<option value='5000'>5 sec</option>
						</select>
						<br/>
						Hop sequence: 
						<select name="hopseq">
							<option selected>1,3,6,8,11</option>
							<option>1,3,6,8,11,14</option>
							<option>1,6,11</option>
							<option value='1,2,3,4,5,6,7,8,9,10,11'>1 to 11</option>
							<option value='1,2,3,4,5,6,7,8,9,10,11,12,13,14'>1 to 14</option>
						</select>
						<br/>
						<input type="submit" value="Set" />
					</span>
				</form>
			</div>
			
			<center>
				<div id="infodiv" class="main">
					<table height="100%" width="100%">
						<tr>
							<td>
								<span id="pips" style="position: relative"></span>
								<span id="content" style="position: relative"></span>
							</td>
						</tr>
					</table>
				</div>
			</center>
			<span id="debug" style="display: none"></span>
			<iframe style="display:none" id="wivizGetFrame" name="wivizGetFrame" src="about:blank"></iframe>
			<script type="text/javascript">scan_thread();</script>
					
			
			
<!--	
        	<table class="center table" cellspacing="5">
				<tr>
				   <th width="31%"><% tran("share.ssid"); %></th>
				   <th width="20%"><% tran("share.mac"); %></th>
				   <th width="7%"><% tran("share.channel"); %></th>
				   <th width="7%"><% tran("share.rssi"); %></th>
				   <th width="7%"><% tran("share.noise"); %></th>
				   <th width="7%"><% tran("share.beacon"); %></th>
				   <th width="7%"><% tran("share.openn"); %></th>
				   <th width="7%"><% tran("share.dtim"); %></th>
				   <th width="7%"><% tran("share.rates"); %></th>
				   <th width="10%"><% tran("survey.thjoin"); %></th>
				</tr>
					<script type="text/javascript">
					//<![CDATA[
					
					var table = new Array(
					<% dump_site_survey(""); %>
					);
					
					if (table.length == 0) {
						document.write("<tr><td colspan=\"10\" align=\"center\">" + share.none + "</td></tr>");
					}
					else {
						for (var i = 0; i < table.length; i = i + 9) {
							document.write("<tr>");
							document.write("<td>"+table[i]+"</td>");
							document.write("<td align=\"left\" style=\"cursor:pointer; text-decoration:underline;\" title=\"OUI Search\" onclick=\"getOUIFromMAC('" + table[i+1] + "')\" >"+table[i+1]+"</td>");
							document.write("<td align=\"right\">"+table[i+2]+"</td>");
							document.write("<td align=\"right\">"+table[i+3]+"</td>");
							document.write("<td align=\"right\">"+table[i+4]+"</td>");
							document.write("<td align=\"right\">"+table[i+5]+"</td>");
							document.write("<td align=\"left\">"+table[i+6]+"</td>");
							document.write("<td align=\"right\">"+table[i+7]+"</td>");
							document.write("<td align=\"right\">"+table[i+8]+"</td>");
							document.write("<td align=\"right\"><input class=\"button\" type=\"button\" value=\"" + sbutton.join + "\" onclick='do_join(this.form,\"" + table[i] + "\")' /></td>");
							document.write("<\/tr>");
						}
					}
					
					//]]>
					</script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(0,0,0,0,1,1);
				//]]>
				</script>
			</div>
-->
		</form>
	</body>
</html>