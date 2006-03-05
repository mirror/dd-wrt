<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
	<title><% nvram_get("router_name"); %> - QoS</title>
	<link type="text/css" rel="stylesheet" href="style.css"/>
	<script type="text/JavaScript" src="common.js">{}</script>
	<script language="JavaScript">
	
function PortCheck(I) {
	d = parseInt(I.value, 10);
	if ( !( d<65536 && d>=0) ) {
		alert("Port value is out of range [0 - 65535]");
		I.value = I.defaultValue;
	}
}

function svcs_grey(sw_disabled,F) {
	F.add_svc.disabled = sw_disabled;
	for (i=0; i<F.svqos_nosvcs.value; i++) {
		eval("F.svqos_svcdel" + i).disabled = sw_disabled;
		eval("F.svqos_svcprio" + i).disabled = sw_disabled;
	}
}

function ips_grey(sw_disabled,F) {
	F.svqos_ipaddr0.disabled = sw_disabled;
	F.svqos_ipaddr1.disabled = sw_disabled;
	F.svqos_ipaddr2.disabled = sw_disabled;
	F.svqos_ipaddr3.disabled = sw_disabled;
	F.svqos_netmask.disabled = sw_disabled;
	for (i=0; i<F.svqos_noips.value; i++){
		eval("F.svqos_ipdel" + i).disabled = sw_disabled;
		eval("F.svqos_ipprio" + i).disabled = sw_disabled;
	}
}

function macs_grey(sw_disabled,F) {
	F.svqos_hwaddr0.disabled = sw_disabled;
	F.svqos_hwaddr1.disabled = sw_disabled;
	F.svqos_hwaddr2.disabled = sw_disabled;
	F.svqos_hwaddr3.disabled = sw_disabled;
	F.svqos_hwaddr4.disabled = sw_disabled;
	F.svqos_hwaddr5.disabled = sw_disabled;
	for (i=0; i<F.svqos_nomacs.value; i++){
		eval("F.svqos_macdel" + i).disabled = sw_disabled;
		eval("F.svqos_macprio" + i).disabled = sw_disabled;
	}
}

function port_grey(sw_disabled,F) {
	F.svqos_port1prio.disabled = sw_disabled;
	F.svqos_port2prio.disabled = sw_disabled;
	F.svqos_port3prio.disabled = sw_disabled;
	F.svqos_port4prio.disabled = sw_disabled;

	F.svqos_port1bw.disabled = sw_disabled;
	F.svqos_port2bw.disabled = sw_disabled;
	F.svqos_port3bw.disabled = sw_disabled;
	F.svqos_port4bw.disabled = sw_disabled;

}

function qos_grey(num,F) {
	var sw_disabled = (num == F.wshaper_enable[1].value) ? true : false;
	
	F._enable_game.disabled = sw_disabled;
	F.wshaper_uplink.disabled = sw_disabled;
	F.wshaper_downlink.disabled = sw_disabled;
	F.wshaper_dev.disabled = sw_disabled;
	F.add_svc_button.disabled = sw_disabled;
	F.edit_svc_button.disabled = sw_disabled;
	port_grey(sw_disabled, F);
	macs_grey(sw_disabled, F);
	ips_grey(sw_disabled, F);
	svcs_grey(sw_disabled, F);
}

function svc_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "QoS";
	F.submit_type.value = "add_svc";
 	F.action.value = "Apply";
	F.submit();
}

function ip_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "QoS";
	F.submit_type.value = "add_ip";
 	F.action.value = "Apply";
	F.submit();
}

function mac_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "QoS";
	F.submit_type.value = "add_mac";
 	F.action.value = "Apply";
	F.submit();
}

function service(id, name, port_start, port_end){
	this.id = id;
	this.name = name;
	this.start = port_start;
	this.end = port_end;
}

var sorton = function(x,y){
	if(x.name <  y.name) return -1;
	else if (x.name == y.name) return 0;
	else return 1;
}

services=new Array();
services_length=0;
/* Init. services data structure */
<% filter_port_services_get("all_list", "0"); %>
services.sort(sorton);

function to_submit(F) {
	if (F._enable_game.checked == false){
	    F.enable_game.value = 0;
	}else{
	    F.enable_game.value = 1;
	}
	    
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "QoS";
	F.submit_type.value = "save";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

 	F.action.value = "Apply";
	F.submit();
}

function init() {
	qos_grey(<% nvram_get("wshaper_enable"); %>,document.QoS);
}
		</script>
	</head>
	
	<body class="gui" onload="init()"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li><a href="Wireless_Basic.asp">Wireless</a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
								<li class="current"><span>Applications&nbsp;&amp;&nbsp;Gaming</span>
									<div id="menuSub">
										<ul id="menuSubList">
											<li><a href="Forward.asp">Port Range Forward</a></li>
											<li><a href="ForwardSpec.asp">Port Forwarding</a></li>
											<li><a href="Triggering.asp">Port Triggering</a></li>
											<li><a href="UPnP.asp">UPnP Forward</a></li>
											<li><a href="DMZ.asp">DMZ</a></li>
											<li><span>QoS</span></li>
										</ul>
									</div>
								</li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
            	<div id="main">
					<div id="contents">
						<form name="QoS" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="enable_game" value="1" />
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="action" value="Apply"/>
							<input type="hidden" name="commit" value="1"/>
							<h2>Quality Of Service (QoS )</h2>
							<div class="setting">
								<input type="radio" value="1" name="wshaper_enable" onclick="qos_grey(this.value,this.form)" <% nvram_match("wshaper_enable","1","checked"); %>>Enable</input>
								<input type="radio" value="0" name="wshaper_enable" onclick="qos_grey(this.value,this.form)" <% nvram_match("wshaper_enable","0","checked"); %>>Disable</input>
							</div>
							<div class="setting">
								<div class="label">Port</div>
								<input type="radio" name="wshaper_dev" value="WAN" <% nvram_match("wshaper_dev","WAN","checked"); %>>WAN</input>
								<input type="radio" name="wshaper_dev" value="LAN" <% nvram_match("wshaper_dev","LAN","checked"); %>>LAN & wLAN</input>
							</div>
							<div class="setting">
								<div class="label">&nbsp;</div>
								<input type="text" size="5" class="num" name="wshaper_uplink" value='<% nvram_get("wshaper_uplink"); %>'/> Uplink kbps
							</div>
							<div class="setting">
								<div class="label">&nbsp;</div>
								<input type="text" size="5" class="num" name="wshaper_downlink" value='<% nvram_get("wshaper_downlink"); %>'/> Downlink kbps
							</div>
							<div class="setting">
								<div class="label"></div>
								<input type="checkbox" name="_enable_game" value="1" <% nvram_match("enable_game", "1", "checked"); %>/> Optimize for Gaming
							</div>
							<fieldset>
								<legend>Services Priority</legend>
								<table class="table">
									<tr>
										<th>Delete</th>
										<th>Service</th>
										<th>Priority</th>
									</tr>
									<% get_qossvcs(); %>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2"><input type="button" name="add_svc_button" value="Add" onclick="svc_add_submit(this.form)"/>&nbsp;&nbsp;&nbsp;<select name="add_svc">
										<script language="Javascript">
										var i=0;
										for(i=0;i<services_length;i++)
										document.write("<option value="+services[i].name+">"+services[i].name+ " [ "+
										services[i].start+" ~ "+
										services[i].end + " ]" + "</option>");
										</script>
										</select></td>
									</tr>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2"><input type="button" name="edit_svc_button" value="Add/Edit Service" onclick="self.open('QOSPort_Services.asp','QOSPortServices','alwaysRised,resizable,scrollbars,width=630,height=360').focus()"/></td>
									</tr>
								</table>
							</fieldset>
							<br/>
							<fieldset>
								<legend>Netmask Priority</legend>
								<table class="table">
									<% get_qosips(); %>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2">
											<input type="button" value="Add" onclick="ip_add_submit(this.form)"/>&nbsp;&nbsp;&nbsp;
											<input size="3" maxlength="3" name="svqos_ipaddr0" value="0" onblur="valid_range(this,0,255,'IP')" class="num"/>.<input size="3" maxlength="3" name="svqos_ipaddr1" value="0" onblur="valid_range(this,0,255,'IP')" class="num"/>.<input size="3" maxlength="3" name="svqos_ipaddr2" value="0" onblur="valid_range(this,0,255,'IP')" class="num"/>.<input size="3" maxlength="3" name="svqos_ipaddr3" value="0" onblur="valid_range(this,0,255,'IP')" class="num"/>/
											<input size="3" maxlength="3" name="svqos_netmask" value="0" onblur="valid_range(this,0,32,'Netmask')" class="num"/>
										</td>
									</tr>
								</table>
								</fieldset>
								<br/>
								<fieldset>
									<legend>MAC Priority</legend>
									<table class="table">
										<% get_qosmacs(); %>
										<tr>
											<td>&nbsp;</td>
											<td colspan="2">
												<input type="button" value="Add" onclick="mac_add_submit(this.form)"/>&nbsp;&nbsp;&nbsp;
												<input name="svqos_hwaddr0" value="00" size="2" maxlength="2" onblur="valid_mac(this,0)" class="num"/>:<input name="svqos_hwaddr1" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num"/>:<input name="svqos_hwaddr2" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num"/>:<input name="svqos_hwaddr3" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num"/>:<input name="svqos_hwaddr4" value="00" size="2" maxlength="2" onBlur="valid_mac(this,1)" class="num"/>:<input name="svqos_hwaddr5" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num"/>
											</td>
										</tr>
									</table>
								</fieldset>
								<br/>
								<% show_default_level(); %>
								<fieldset>
									<legend>Ethernet Port Priority</legend>
									<table>
										<tr>
											<th>&nbsp;</th>
											<th>Priority</th>
											<th>Max&nbsp;Rate</th>
										</tr>
										<tr>
											<td>Port 1</td>
											<td>
												<select name="svqos_port1prio">
													<option value="10" <% nvram_match("svqos_port1prio", "10", "selected"); %>>Premium</option>
													<option value="20" <% nvram_match("svqos_port1prio", "20", "selected"); %>>Express</option>
													<option value="30" <% nvram_match("svqos_port1prio", "30", "selected"); %>>Standard</option>
													<option value="40" <% nvram_match("svqos_port1prio", "40", "selected"); %>>Bulk</option>
												</select>
											</td>
											<td>
												<select name="svqos_port1bw">
													<option value="0" <% nvram_match("svqos_port1bw", "0", "selected"); %>>Disable</option>
													<option value="256k" <% nvram_match("svqos_port1bw", "256k", "selected"); %>>256k</option>
													<option value="512k" <% nvram_match("svqos_port1bw", "512k", "selected"); %>>512k</option>
													<option value="1m" <% nvram_match("svqos_port1bw", "1m", "selected"); %>>1M</option>
													<option value="2m" <% nvram_match("svqos_port1bw", "2m", "selected"); %>>2M</option>
													<option value="5m" <% nvram_match("svqos_port1bw", "5m", "selected"); %>>5M</option>
													<option value="10m" <% nvram_match("svqos_port1bw", "10m", "selected"); %>>10M</option>
													<option value="20m" <% nvram_match("svqos_port1bw", "20m", "selected"); %>>20M</option>
													<option value="50m" <% nvram_match("svqos_port1bw", "50m", "selected"); %>>50M</option>
													<option value="full" <% nvram_match("svqos_port1bw", "full", "selected"); %>>100M</option>
												</select>
											</td>
										</tr>
										<tr>
											<td>Port 2</td>
											<td>
												<select name="svqos_port2prio">
													<option value="10" <% nvram_match("svqos_port2prio", "10", "selected"); %>>Premium</option>
													<option value="20" <% nvram_match("svqos_port2prio", "20", "selected"); %>>Express</option>
													<option value="30" <% nvram_match("svqos_port2prio", "30", "selected"); %>>Standard</option>
													<option value="40" <% nvram_match("svqos_port2prio", "40", "selected"); %>>Bulk</option>
												</select>
											</td>
											<td>
												<select name="svqos_port2bw">
													<option value="0" <% nvram_match("svqos_port2bw", "0", "selected"); %>>Disable</option>
													<option value="256k" <% nvram_match("svqos_port2bw", "256k", "selected"); %>>256k</option>
													<option value="512k" <% nvram_match("svqos_port2bw", "512k", "selected"); %>>512k</option>
													<option value="1m" <% nvram_match("svqos_port2bw", "1m", "selected"); %>>1M</option>
													<option value="2m" <% nvram_match("svqos_port2bw", "2m", "selected"); %>>2M</option>
													<option value="5m" <% nvram_match("svqos_port2bw", "5m", "selected"); %>>5M</option>
													<option value="10m" <% nvram_match("svqos_port2bw", "10m", "selected"); %>>10M</option>
													<option value="20m" <% nvram_match("svqos_port2bw", "20m", "selected"); %>>20M</option>
													<option value="50m" <% nvram_match("svqos_port2bw", "50m", "selected"); %>>50M</option>
													<option value="full" <% nvram_match("svqos_port2bw", "full", "selected"); %>>100M</option>
												</select>
											</td>
										</tr>
										<tr>
											<td>Port 3</td>
											<td>
												<select name="svqos_port3prio">
													<option value="10" <% nvram_match("svqos_port3prio", "10", "selected"); %>>Premium</option>
													<option value="20" <% nvram_match("svqos_port3prio", "20", "selected"); %>>Express</option>
													<option value="30" <% nvram_match("svqos_port3prio", "30", "selected"); %>>Standard</option>
													<option value="40" <% nvram_match("svqos_port3prio", "40", "selected"); %>>Bulk</option>
											</select>
											</td>
											<td>
												<select name="svqos_port3bw">
													<option value="0" <% nvram_match("svqos_port3bw", "0", "selected"); %>>Disable</option>
													<option value="256k" <% nvram_match("svqos_port3bw", "256k", "selected"); %>>256k</option>
													<option value="512k" <% nvram_match("svqos_port3bw", "512k", "selected"); %>>512k</option>
													<option value="1m" <% nvram_match("svqos_port3bw", "1m", "selected"); %>>1M</option>
													<option value="2m" <% nvram_match("svqos_port3bw", "2m", "selected"); %>>2M</option>
													<option value="5m" <% nvram_match("svqos_port3bw", "5m", "selected"); %>>5M</option>
													<option value="10m" <% nvram_match("svqos_port3bw", "10m", "selected"); %>>10M</option>
													<option value="20m" <% nvram_match("svqos_port3bw", "20m", "selected"); %>>20M</option>
													<option value="50m" <% nvram_match("svqos_port3bw", "50m", "selected"); %>>50M</option>
													<option value="full" <% nvram_match("svqos_port3bw", "full", "selected"); %>>100M</option>
												</select>
											</td>
										</tr>
										<tr>
											<td>Port 4</td>
											<td>
												<select name="svqos_port4prio">
													<option value="10" <% nvram_match("svqos_port4prio", "10", "selected"); %>>Premium</option>
													<option value="20" <% nvram_match("svqos_port4prio", "20", "selected"); %>>Express</option>
													<option value="30" <% nvram_match("svqos_port4prio", "30", "selected"); %>>Standard</option>
													<option value="40" <% nvram_match("svqos_port4prio", "40", "selected"); %>>Bulk</option>
												</select>
											</td>
											<td>
												<select name="svqos_port4bw">
													<option value="0" <% nvram_match("svqos_port4bw", "0", "selected"); %>>Disable</option>
													<option value="256k" <% nvram_match("svqos_port4bw", "256k", "selected"); %>>256k</option>
													<option value="512k" <% nvram_match("svqos_port4bw", "512k", "selected"); %>>512k</option>
													<option value="1m" <% nvram_match("svqos_port4bw", "1m", "selected"); %>>1M</option>
													<option value="2m" <% nvram_match("svqos_port4bw", "2m", "selected"); %>>2M</option>
													<option value="5m" <% nvram_match("svqos_port4bw", "5m", "selected"); %>>5M</option>
													<option value="10m" <% nvram_match("svqos_port4bw", "10m", "selected"); %>>10M</option>
													<option value="20m" <% nvram_match("svqos_port4bw", "20m", "selected"); %>>20M</option>
													<option value="50m" <% nvram_match("svqos_port4bw", "50m", "selected"); %>>50M</option>
													<option value="full" <% nvram_match("svqos_port4bw", "full", "selected"); %>>100M</option>
												</select>
											</td>
										</tr>
									</table>
								</fieldset>
								<br/>
								<div class="submitFooter">
									<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)"/>
									<input type="reset" value="Cancel Changes"/>
								</div>
							</form>
						</div>
					</div>
					<div id="statusInfo">
						<div class="info">Firmware: <% get_firmware_version(); %></div>
						<div class="info">Time: <% get_uptime(); %></div>
						<% nvram_match("wan_proto","disabled","<!--"); %>
						<div class="info">WAN IP: <% nvram_status_get("wan_ipaddr"); %></div>
						<% nvram_match("wan_proto","disabled","-->"); %>
                    	<div class="info"><% nvram_match("wan_proto","disabled","WAN disabled"); %></div>
					</div>
					<div id="helpContainer">
						<div id="help">
							<div id="logo"><h2>Help</h2></div>
							<dl>
								<dt class="term">Uplink: </dt>
								<dd class="definition">Set this to 80%-95% (max) of your total upload limit</dd>
								<dt class="term">Dnlink: </dt>
								<dd class="definition">Set this to 80%-100% (max) of your total download limit</dd>
								<dt class="term">Application Priority: </dt>
								<dd class="definition">You may control your data rate with respect to the application that is consuming bandwidth.</dd>
								<dt class="term">Netmask Priority: </dt>
								<dd class="definition">You may specify priority for all traffic from a given IP address or IP Range.</dd>
								<dt class="term">MAC Priority: </dt>
								<dd class="definition">You may specify priority for all traffic from a device on your network by giving the device a Device Name, specifying priority
									and entering its MAC address.</dd>
								<dt class="term">Ethernet Port Priority: </dt>
								<dd class="definition">You may control your data rate according to which physical LAN port your device is plugged into. You may assign Priorities
									accordingly for devices connected on LAN ports 1 through 4.</dd>
							</dl>
							<br/>
							<a target="_blank" href="help/HQos.asp">More...</a>
						</div>
					</div>
				</div>
			</div>
		</body>
	</html>