<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wiviz Survey</title>
		<script type="text/javascript" src="js/wiviz.js"></script>
		<link type="text/css" rel="stylesheet" href="style/wiviz.css" />
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + " - Wiviz";

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
				<form name="channelform" action="apply.cgi" method="<% get_http_method(); %>" target="wivizGetFrame" />
                  		<input type="hidden" name="action" value="Apply" />
                  		<input type="hidden" name="change_action" value="gozila_cgi" />
                  		<input type="hidden" name="submit_button" value="Wiviz_Survey" />
	          		<input type="hidden" name="submit_type" value="Set" />
					Channel setting:
					<select id="channelsel" name="channelsel" onchange="channelSet()">
						<option value='nochange' <% nvram_match("channelsel","nochange","selected"); %> >No change</option>
						<option value='hop' <% nvram_match("channelsel","hop","selected"); %> >Hopping</option>
						<option value='1' <% nvram_match("channelsel","1","selected"); %> >1</option>
						<option value='2' <% nvram_match("channelsel","2","selected"); %> >2</option>
						<option value='3' <% nvram_match("channelsel","3","selected"); %> >3</option>
						<option value='4' <% nvram_match("channelsel","4","selected"); %> >4</option>
						<option value='5' <% nvram_match("channelsel","5","selected"); %> >5</option>
						<option value='6' <% nvram_match("channelsel","6","selected"); %> >6</option>
						<option value='7' <% nvram_match("channelsel","7","selected"); %> >7</option>
						<option value='8' <% nvram_match("channelsel","8","selected"); %> >8</option>
						<option value='9' <% nvram_match("channelsel","9","selected"); %> >9</option>
						<option value='10' <% nvram_match("channelsel","10","selected"); %> >10</option>
						<option value='11' <% nvram_match("channelsel","11","selected"); %> >11</option>
						<option value='12' <% nvram_match("channelsel","12","selected"); %> >12</option>
						<option value='13' <% nvram_match("channelsel","13","selected"); %> >13</option>
						<option value='14' <% nvram_match("channelsel","14","selected"); %> >14</option>
					</select>
					<br/>
					<span id="hopoptions" style="display: none">Time/channel: 
						<select name="hopdwell">
							<option value='500' <% nvram_match("hopdwell","500","selected"); %> >0.5 sec</option>
							<option value='1000' <% nvram_match("hopdwell","1000","selected"); %> >1 sec</option>
							<option value='2000' <% nvram_match("hopdwell","2000","selected"); %> >2 sec</option>
							<option value='5000' <% nvram_match("hopdwell","5000","selected"); %> >5 sec</option>
						</select>
						<br/>
						Hop sequence: 
						<select name="hopseq">
							<option value='1,3,6,8,11' <% nvram_match("hopseq","1,3,6,8,11","selected"); %> >1,3,6,8,11</option>
							<option value='1,3,6,8,11,14' <% nvram_match("hopseq","1,3,6,8,11,14","selected"); %> >1,3,6,8,11,14</option>
							<option value='1,6,11' <% nvram_match("hopseq","1,3,6,8,11","selected"); %> >1,6,11</option>
							<option value='1,2,3,4,5,6,7,8,9,10,11' <% nvram_match("hopseq","1,2,3,4,5,6,7,8,9,10,11","selected"); %> >1 to 11</option>
							<option value='1,2,3,4,5,6,7,8,9,10,11,12,13,14' <% nvram_match("hopseq","1,2,3,4,5,6,7,8,9,10,11,12,13,14","selected"); %> >1 to 14</option>
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
					
		</form>
	</body>
</html>