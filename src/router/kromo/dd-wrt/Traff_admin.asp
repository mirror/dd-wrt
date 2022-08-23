<% do_pagehead("status_inet.dataadmin"); %>
		<script type="text/javascript">
		//<![CDATA[

function ttraff_restore_submit(F) {
	if (F.file.value == "")	{
		alert(errmsg.err42);
		return false;
	}
	var len = F.file.value.length;
	var ext = new Array('.','b','a','k');
	var IMAGE = F.file.value.toLowerCase();
	for (i=0; i < 4; i++)	{
		if (ext[i] != IMAGE.charAt(len-4+i)){
			alert(errmsg.err61);
			return false;
		}
	}
	apply(F);
}

		//]]>
		</script>
	</head>

	<body class="popup_bg">
		<form name="firmware" method="post" action="tadmin.cgi" enctype="multipart/form-data">
			<input type="hidden" name="submit_button" value="Traff_admin" />

				<h2><% tran("status_inet.traff"); %></h2>
				<fieldset>
					<legend><% tran("status_inet.dataadmin"); %></legend>
					<div class="setting">
						<div class="label"><% tran("config.mess2"); %></div>
						<input type="file" name="file" size="40" />
					</div>
				</fieldset><br />
				<div id="footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input class=\"button\" type=\"button\" name=\"restore_button\" value=\"" + sbutton.restore + "\" onclick=\"ttraff_restore_submit(this.form);\" />");
					document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\"" + sbutton.clos + "\" onclick=\"self.close();\" />");
					//]]>
					</script>
				</div>
		</form>
	</body>
</html>
