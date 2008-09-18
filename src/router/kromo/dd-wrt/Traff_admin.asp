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
				
function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.save_button.value = sbutton.saving;
	applytake(F);
}
		
		//]]>
		</script>
	</head>

	<body>
				<form name="firmware" method="post" action="tadmin.cgi" enctype="multipart/form-data">
					<input type="hidden" name="submit_button" value="Traff_admin" />
					<input type="hidden" name="action" />
					<input type="hidden" name="change_action" />
					<input type="hidden" name="submit_type" />

				<h2><% tran("status_inet.traff"); %>: <% tran("status_inet.dataadmin"); %></h2>
				<br />		
					<div class="setting">
						<div class="label"><% tran("config.mess2"); %></div>
						<input type="file" name="file" size="40" />
				</div><br />
				<div class="submitFooter">
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