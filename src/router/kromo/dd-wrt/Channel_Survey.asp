<% do_pagehead("survey.titl2"); %>
		<script type="text/javascript">
		//<![CDATA[
addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
});
		//]]>
		</script>
	</head>

	<body>
		<div id="bulle" class="bulle"></div>
		<form name="Join" action="apply.cgi" method="post">
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<input type="hidden" name="commit" value="1" />
			<h2><% tran("survey.h3"); %></h2>
			<table class="table" cellspacing="5" id="survey_table">
				<thead>
					<tr>
						<th sortdir="up" width="4%"><% tran("share.frequency"); %></th>
						<th sortdir="up" width="3%"><% tran("share.channel"); %></th>
						<th sortdir="up" width="3%"><% tran("share.noise"); %></th>
						<th sortdir="up" width="3%"><% tran("share.quality"); %></th>
						<th sortdir="up" width="5%"><% tran("status_wireless.active"); %></th>
						<th sortdir="up" width="5%"><% tran("status_wireless.busy"); %></th>
						<th sortdir="up" width="5%"><% tran("status_wireless.rx_time"); %></th>
						<th sortdir="up" width="5%"><% tran("status_wireless.tx_time"); %></th>
					</tr>
				</thead>
				<tbody>
					<script type="text/javascript">
					//<![CDATA[
					var table = new Array(
						<% dump_channel_survey(""); %>
					);
					if (table.length == 0) {
						document.write("<tr><td colspan=\"12\" class=\"center\">" + share.none + "</td></tr>");
					}
					else {
						for (var i = 0; i < table.length; i = i + 8) {
							document.write("<tr>");
							document.write("<td class=\"center\">"+table[i]+"</td>");
							document.write("<td class=\"center\">"+table[i+1]+"</td>");
							document.write("<td class=\"center\">"+table[i+2]+"</td>");
							document.write("<td class=\"center\">"+table[i+3]+"</td>");
							document.write("<td class=\"center\">"+table[i+4]+"</td>");
							document.write("<td class=\"center\">"+table[i+5]+"</td>");
							document.write("<td class=\"center\">"+table[i+6]+"</td>");
							document.write("<td class=\"center\">"+table[i+7]+"</td>");
							document.write("<\/tr>");
						}
					}
					//]]>
					</script>
				</tbody>
			</table>
			<script type="text/javascript">
			//<![CDATA[
			var t = new SortableTable(document.getElementById('survey_table'), 1000);
			//]]>
			</script>
			<br />
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
