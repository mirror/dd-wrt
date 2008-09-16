<% do_pagehead("wl_active.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F)
{
	if(valid_value(F)){
		F.submit_type.value="add_mac";
		F.submit();
	}
}
function to_apply(F)
{
	if(valid_value(F)){
		F.submit_type.value="add_mac";
		F.submit();
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
	if(count > 128){
		alert(errmsg.err44);
		return false;
	}
	return true;
}

addEvent(window, "load", function() {
	
	<% onload("WL_ActiveTable", "setTimeout('opener.window.location.reload();',500);"); %>
	window.focus();

});
		
		//]]>
		</script>
	</head>

	<body>
		<form action="apply.cgi" method="post">
			<input type="hidden" name="submit_button" value="WL_ActiveTable" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="commit" value="1" />
			
			<h2><% tran("wl_active.h2"); %></h2>
			<table>
				<tbody>
					<tr>
						<td><strong><% tran("wl_active.active"); %></strong></td>
						<td colspan="3">&nbsp;</td>
					</tr>
					<tr>
						<th><% tran("dhcp.tclient"); %></th>
						<th><% tran("share.ip"); %></th>
						<th><% tran("share.mac"); %></th>
						<th><% tran("wl_active.h3"); %></th>
					</tr>
					<% wireless_active_table("online"); %>
					<tr>
						<td colspan="4">&nbsp;</td>
					</tr>
					<tr>
						<td><strong><% tran("wl_active.inactive"); %></strong></td>
						<td colspan="3">&nbsp;</td>
					</tr>
					<tr>
						<th><% tran("dhcp.tclient"); %></th>
						<th><% tran("share.ip"); %></th>
						<th><% tran("share.mac"); %></th>
						<th><% tran("wl_active.h3"); %></th>
					</tr>
					<% wireless_active_table("offline"); %>
					<tr>
						<td colspan="4">&nbsp;</td>
					</tr>
				</tbody>
			</table><br />
			<div class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(1,0,0,0,1,1);
					//]]>
					</script>
			</div>
		</form>
	</body>
</html>
