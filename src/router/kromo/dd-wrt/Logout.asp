<% do_pagehead(""); %>
	<script type="text/javascript">
	//<![CDATA[
function logout()
{
	var p = window.location.protocol + '//';
	window.location = 'logout:password@' + p + '/';
}
addEvent(window, "load", function() {
	setTimeout(logout, 2000);
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
