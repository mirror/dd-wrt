<%% do_pagehead("wl_active.titl"); %%>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F)
{
	if(valid_value(F)){
		F.submit_type.value="add_mac";
		F.submit();
		refreshParent();
	}
}
function to_apply(F)
{
	if(valid_value(F)){
		F.submit_type.value="add_mac";
		F.submit();
		refreshParent();
	}
}

function valid_value(F)
{
	var num = F.elements.length;
	var count = 0;

	for(i=0;i<num;i++){
		if(F.elements[i].type == "checkbox"){
			if(F.elements[i].checked == true)
				count = count + 1;
		}
	}
	if(count > 256){
		alert(errmsg.err44);
		return false;
	}
	return true;
}

addEvent(window, "load", function() {
	
	<%% onload("WL_ActiveTable-%s", "setTimeout('opener.window.location.reload();',500);"); %%>
	window.focus();

});

refreshParent = function() {
	var elements = opener.document.forms["macfilter"].elements;
	var url = elements["submit_button"].value + '-' + elements["ifname"].value + ".asp";
	opener.window.location = url;
}
		
		//]]>
		</script>
	</head>

	<body>
		<form action="apply.cgi<% insertpageToken(); %>" method="post">
			<input type="hidden" name="submit_button" value="WL_ActiveTable" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="ifname" value="%s" />
			<input type="hidden" name="commit" value="1" />
			
			<h2><%% tran("wl_active.h2"); %%></h2>
			<table>
				<tbody>
					<tr>
						<td><strong><%% tran("wl_active.active"); %%></strong></td>
						<td colspan="3">&nbsp;</td>
					</tr>
					<tr>
						<th><%% tran("dhcp.tclient"); %%></th>
						<th><%% tran("share.ip"); %%></th>
						<th><%% tran("share.mac"); %%></th>
						<th><%% tran("wl_active.h3"); %%></th>
					</tr>
					<%% wireless_active_table("online", "%s"); %%>
					<tr>
						<td colspan="4">&nbsp;</td>
					</tr>
					<tr>
						<td><strong><%% tran("wl_active.inactive"); %%></strong></td>
						<td colspan="3">&nbsp;</td>
					</tr>
					<tr>
						<th><%% tran("dhcp.tclient"); %%></th>
						<th><%% tran("share.ip"); %%></th>
						<th><%% tran("share.mac"); %%></th>
						<th><%% tran("wl_active.h3"); %%></th>
					</tr>
					<%% wireless_active_table("offline", "%s"); %%>
					<tr>
						<td colspan="4">&nbsp;</td>
					</tr>
				</tbody>
			</table><br />
			<div id="submit_footer" class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(1,0,0,0,1,1);
					var children = document.getElementById('submit_footer').childNodes;
					for(var i = 0; i < children.length; i++) {
						if(children[i].name == "apply_button") {
							document.getElementById('submit_footer').removeChild(children[i]);
						}
					}
					//]]>
					</script>
			</div>
		</form>
	</body>
</html>
