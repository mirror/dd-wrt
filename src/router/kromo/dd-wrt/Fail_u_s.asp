<% do_pagehead(""); %>
		<title><% nvg("router_name"); %></title>

		<script type="text/javascript">
		//<![CDATA[
		function to_submit() {
			document.location.href = "Upgrade.asp";
		}
		//]]>
		</script>
	</head>

	<body>
		<div class="message">
			<div>
			<form>
				<% tran("fail.mess2"); %><br />
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input class=\"button\" type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit();\" />");
					//]]>
					</script>
				</form>
			</div>
		</div>
	</body>
</html>