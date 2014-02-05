<% do_pagehead("service.syslog_srv"); %>
		<script type="text/javascript">
		//<![CDATA[

var count = 0;

function load_file(count) {
	return "/syslog.cgi?" + count;
}

function do_refresh() {
	var f = document.getElementById('syslog');
	f.src = load_file(count);
}
function do_show_prev() {
	if( count > 99 )
		count = Number(count) - 100;
	do_refresh(count)
}

function do_show_next() {
	count = Number(count) + 100;
	do_refresh(count);
}

var update;

addEvent(window, "load", function() {
	if(document.getElementsByName("refresh_button")) {
		document.getElementsByName("refresh_button")[0].disabled = true;
		document.getElementsByName("refresh_button")[0].style.background = '#DADADA';
		document.getElementsByName("refresh_button")[0].style.cursor = "default";
	}
	
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
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Status_Router.asp","Syslog.asp"); %>
				</div>
				<div id="mainno">
					<div id="contents">						
							<script type="text/javascript">
								//<![CDATA[
								document.write("<iframe id=\"syslog\" src=\"" + load_file(0) + "\" width=\"850\" height=\"1000\" frameborder=\"0\" type=\"text/html\"></iframe>");
								//]]>
							</script>
							<div class="center">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"submit_button\" value=\"            < Prev\" onclick=\"do_show_prev();\">");
								document.write("<input class=\"button\" type=\"submit_button\" value=\"            Next >\" onclick=\"do_show_next();\">");
								//]]>
								</script>
							</div>
							<p><p>
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
								//]]>
								</script>
							</div>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
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