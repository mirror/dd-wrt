<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless Active Client MAC List</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + wl_filter.titl;

function MACAct(F)
{
	if(valid_value(F)){
		F.submit_button.value="WL_ActiveTable";
		F.change_action.value="gozila_cgi";
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
//		alert("The total checks exceed 128 counts.");
		alert(errmsg.err44);
		return false;
	}
	return true;
}

function init() {
	<% onload("WL_ActiveTable", "setTimeout('opener.window.location.reload();',500);"); %>
	window.focus();
}
		</script>
	</head>
	
	<body onload="init()">
		<form action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" />
			<input type="hidden" name="submit_type" />
			<input type="hidden" name="change_action" />
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
			</table></br />
			<div class="submitFooter">
					<script type="text/javascript">document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.update_filter + "\" onclick=\"MACAct(this.form)\" />")</script>
					<script type="text/javascript">document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />")</script>
					<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />")</script>
			</div>
		</form>
	</body>
</html>