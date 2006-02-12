<div><% nvram_status_get("hidden1"); %><div class="setting"><div class="label">Login Status</div><script language="JavaScript">
var status1 = "<% nvram_status_get("status1"); %>";
var status2 = "<% nvram_status_get("status2"); %>";
if(status1 == "Status") {
	status1 = "Status";
}
if(status2 == "Connecting") {
	status2 = "Connecting";
} else if(status2 == "Disconnected") {
	status2 = "Disconnected";
} else if(status2 == "Connected") {
	status2 = "Connected";
}
document.write(status2);
document.write("");

var but_arg = "<% nvram_status_get("button1"); %>";
var wan_proto = "<% nvram_get("wan_proto"); %>";
var but_type = "";
if(but_arg == "Connect") {
	but_value = "Connect";
} else if(but_arg == "Disconnect") {
	but_value = "Disconnect";
}

but_type = but_arg +"_" + wan_proto;
document.write("<INPUT type=button value='"+but_value+"' onClick=Connect(this.form,'"+but_type+"') />");
</script></div>
<% nvram_status_get("hidden2"); %>
<% nvram_selmatch("wan_proto","disabled","<!--"); %>
<div class="setting"><div class="label">IP Address</div>
<% nvram_status_get("wan_ipaddr"); %></div>
<div class="setting"><div class="label">Subnet Mask</div>
<% nvram_status_get("wan_netmask"); %></div>
<div class="setting"><div class="label">Default Gateway</div>
<% nvram_status_get("wan_gateway"); %></div>
<div class="setting"><div class="label">DNS 1</div>
<% nvram_status_get("wan_dns0"); %></div>
<div class="setting"><div class="label">DNS 2</div>
<% nvram_status_get("wan_dns1"); %></div>
<div class="setting"><div class="label">DNS 3</div>
<% nvram_status_get("wan_dns2"); %></div>
<% nvram_selmatch("wan_proto","disabled","-->"); %>
</div>