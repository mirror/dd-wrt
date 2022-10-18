<% do_pagehead_nopwc("service.syslog_srv"); %>
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
	if( count > 49 )
		count = Number(count) - 50;
	do_refresh(count)
}

function do_show_next() {
	count = Number(count) + 50;
	do_refresh(count);
}

var update;

addEvent(window, "load", function() {
	if(document.getElementsByName("refresh_button").length) {
		document.getElementsByName("refresh_button")[0].disabled = true;
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
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Status_Router.asp","Syslog.asp"); %>
				</div>
				<!-- add padding to #contents so that submitfooter has padding on sides/bottom -->
				<div id="mainno" class="syslog-padding">
					<div id="contents">
						  <!-- see syslogd dir stylesheets for .syslog_bg padding/margins -->
							<script type="text/javascript">
								//<![CDATA[
								document.write("<iframe id=\"syslog\" src=\"" + load_file(0) + "\" title=\"" + share.sysloglegend + "\" width=\"100%\" height=\"810\" frameborder=\"0\" type=\"text/html\"></iframe>");
								//]]>
							</script>
							</div>
							<div class="submitFooter nostick">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.prev + "\" onclick=\"do_show_prev();\">");
								document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.next + "\" onclick=\"do_show_next();\">");
								var autoref = <% nvem("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(0,0,0,autoref);
								//]]>
								</script>
							</div>
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
