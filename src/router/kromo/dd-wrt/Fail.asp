<% do_pagehead(""); %>
	<title><% nvg("router_name"); %></title>

	<script type="text/javascript">
	//<![CDATA[
var submit_button = "<% get_web_page_name(); %>";

function to_submit() {
	if(submit_button == ".asp"){
		history.go(-1);
	}else{
		document.location.href =  submit_button;
	}
}
	//]]>
	</script>
	</head>

	<body>
		<div class="message">
				<form>
				<% tran("fail.mess1"); %><br /><br />
				<script type="text/javascript">
				//<![CDATA[
				document.write("<input class=\"button\" type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit();\" />");
				//]]>
				</script>
			</form>
		</div>
	</body>
</html>
