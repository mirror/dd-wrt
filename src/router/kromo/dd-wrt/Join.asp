<% do_pagehead_nopwc("join.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

var SSID = "<% nvg("cur_ssid"); %>";

function to_send(url) {
	opener.focus();
	opener.location.href = url;
}
	//]]>
	</script>
	</head>

	<body onunload="to_send('Wireless_Basic.asp')">
		<div class="message">
			<form>
				<script type="text/javascript">
				//<![CDATA[
				document.write(join.mess1 + "&nbsp;" + SSID + "<br/>");
				document.write("<input class=\"button\" type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"self.close();\" />");
				//]]>
				</script>
			</form>
		</div>
	</body>
</html>
