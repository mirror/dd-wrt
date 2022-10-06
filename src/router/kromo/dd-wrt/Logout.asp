<% do_pagehead(""); %>
	<script type="text/javascript">
	//<![CDATA[
function do_logout()
{
	var p = window.location.protocol + '//';
	window.location = p + 'logout:password@' + window.location.host + '/';
}
addEvent(window, "load", function() {
	setTimeout(do_logout, 2000);
});
	//]]>
	</script>
</head>

	<body>
		<div class="message">
			<form>
				<% tran("logout.message"); %><br />
			</form>
		</div>
	</body>
</html>
