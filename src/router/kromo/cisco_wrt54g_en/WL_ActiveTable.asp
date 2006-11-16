<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless Active Client MAC List</title>
		<script type="text/javascript">
		//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + wl_active.titl;

function MACAct(F)
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

function init() {
	<% onload("WL_ActiveTable", "setTimeout('opener.window.location.reload();',500);"); %>
	window.focus();
}
		
		//]]>
		</script>
	</head>
	
	<body onload="init()">
		<form action="apply.cgi" method="<% get_http_method(); %>">
			<input type="hidden" name="submit_button" value="WL_ActiveTable" />
			<input type="hidden" name="action" value="Apply" />
			<input type="hidden" name="change_action" value="gozila_cgi" />
			<input type="hidden" name="submit_type" />
			
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
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input class=\"button\" type=\"button\" name=\"action\" value=\"" + sbutton.update_filter + "\" onclick=\"MACAct(this.form);\" />");
					submitFooterButton(0,0,0,0,1,1);
					//]]>
					</script>
			</div>
		</form>
	</body>
</html>