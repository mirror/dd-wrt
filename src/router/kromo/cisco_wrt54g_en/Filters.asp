<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Access Restrictions</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
<% filter_init(); %>
var summary_win = null;
var ipmac_win = null;

function dayall(F){
	if(F.day_all.checked == false) {
		I = 1;
	} else {
		I = 0;
	}
	day_enable_disable(F,I);
}

function day_enable_disable(F,I) {
	if(I == 1) {
		choose_enable(F.week0);
		choose_enable(F.week1);
		choose_enable(F.week2);
		choose_enable(F.week3);
		choose_enable(F.week4);
		choose_enable(F.week5);
		choose_enable(F.week6);
	} else if(I == 0) {
		choose_disable(F.week0);
		choose_disable(F.week1);
		choose_disable(F.week2);
		choose_disable(F.week3);
		choose_disable(F.week4);
		choose_disable(F.week5);
		choose_disable(F.week6);
	}
}

function timeall(F,I) {
	time_enable_disable(F,I);
}

function time_enable_disable(F,I){
	if(I == 1) {
		choose_enable(F.start_hour);
		choose_enable(F.start_min);
		choose_enable(F.start_time);
		choose_enable(F.end_hour);
		choose_enable(F.end_min);
		choose_enable(F.end_time);
	} else if(I == 0) {
		choose_disable(F.start_hour);
		choose_disable(F.start_min);
		choose_disable(F.start_time);
		choose_disable(F.end_hour);
		choose_disable(F.end_min);
		choose_disable(F.end_time);
	}
}

function ViewSummary() {
	summary_win = self.open('FilterSummary.asp','FilterSummary','alwaysRaised,resizable,scrollbars,width=700,height=480');
	summary_win.focus();
}

function ViewFilter() {
	ipmac_win = self.open('FilterIPMAC.asp','FilterTable','alwaysRaised,resizable,scrollbars,width=590,height=485');
	ipmac_win.focus();
}

function to_submit(F) {
	if(valid(F) == true) {
		F.submit_type.value = "save";
		F.submit_button.value = "Filters";
		F.action.value = "Apply";
		F.save_button.value = "Saved";
		F.save_button.disabled = true;
		F.submit();

	}
}

function to_save(F) {
	if(valid(F) == true) {
		F.submit_button.value = "Filters";
		F.change_action.value = "gozila_cgi";
		F.submit_type.value = "save";
		F.action.value = "Apply";
		F.submit();
	}
}

function to_delete(F) {
	if(confirm("Delete the Policy?")){
		F.submit_button.value = "Filters";
		F.change_action.value = "gozila_cgi";
		F.submit_type.value = "delete";
		F.action.value = "Apply";
		F.submit();
	}
}

function valid(F) {
	if(
		F.day_all.checked == false &&
		F.week0.checked == false &&
		F.week1.checked == false &&
		F.week2.checked == false &&
		F.week3.checked == false &&
		F.week4.checked == false &&
		F.week5.checked == false &&
		F.week6.checked == false
	){
		alert("You must at least select a day.");
		return false;
	}
	if(F.time_all[1].checked == true){
		start = (parseInt(F.start_time.value, 10)*12 + parseInt(F.start_hour.value, 10)) * 60 + parseInt(F.start_min.value, 10);
		end = (parseInt(F.end_time.value, 10)*12 + parseInt(F.end_hour.value, 10)) * 60 + parseInt(F.end_min.value, 10);
		if(end <= start){
			alert("The end time must be bigger than start time!");
			return false;
		}
	}
	if(F.f_status1[1].checked == true) {	// Disable
		F.f_status.value = "0";
	} else {					// Enable
		if(F.f_status2[1].checked == true) {	// Allow
			F.f_status.value = "2";
		} else {					// deny
			F.f_status.value = "1";
		}
	}
	if (F._filter_p2p)
	if (F._filter_p2p.checked == false){
	    F.filter_p2p.value = 0;
	}else{
	    F.filter_p2p.value = 1;
	}


	return true;
}

function SelFilter(num,F) {
	F.submit_button.value = "Filters";
	F.change_action.value = "gozila_cgi";
	F.f_id.value=F.f_id.options[num].value;
	F.submit();
}

function exit() {
	closeWin(summary_win);
	closeWin(ipmac_win);
}

function init() {
	day_enable_disable(document.filters,'<% filter_tod_get("day_all_init"); %>');
	time_enable_disable(document.filters,'<% filter_tod_get("time_all_init"); %>');
	setBlockedServicesValue();
	Status(document.filters, '<% filter_policy_get("f_status","onload_status"); %>');
	choose_disable(document.filters.port0_start);
	choose_disable(document.filters.port0_end);
	choose_disable(document.filters.port1_start);
	choose_disable(document.filters.port1_end);
	choose_disable(document.filters.port2_start);
	choose_disable(document.filters.port2_end);
	choose_disable(document.filters.port3_start);
	choose_disable(document.filters.port3_end);
}

function service(id, name, port_start, port_end, protocol) {
	this.id = id;
	this.name = name;
	this.start = port_start;
	this.end = port_end;
	this.protocol = protocol;
	this.deleted = false;
	this.modified = false;
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

servport_name0 = "<% filter_port_services_get("service", "0"); %>";
servport_name1 = "<% filter_port_services_get("service", "1"); %>";
servport_name2 = "<% filter_port_services_get("service", "2"); %>";
servport_name3 = "<% filter_port_services_get("service", "3"); %>";

function search_service_index(name) {
	for(var i=0; i<services_length ; i++){
		if(name == services[i].name){
			return i;
		}
	}

	return -1;
}

function write_service_options(name) {
	var index = search_service_index(name);
	for(var i=0 ; i<services_length ; i++){
		document.write("<option value=\""+services[i].name+"\"");
		if(i==index) {
			document.write(" selected=\"selected\"");
		}
		document.write(">"+services[i].name+"</option>");
	}
}

function setBlockedServicesValue() {
	var index;

	/* for service port 0 */
	index = search_service_index(servport_name0);
	if(index!=-1){
		document.filters.port0_start.value = services[index].start;
		document.filters.port0_end.value = services[index].end;
		document.filters.blocked_service0.selectedIndex = index+1; /* first will be none */
	}

	/* for service port 1 */
	index = search_service_index(servport_name1);
	if(index!=-1){
		document.filters.port1_start.value = services[index].start;
		document.filters.port1_end.value = services[index].end;
		document.filters.blocked_service1.selectedIndex = index+1; /* first will be none */
	}

	/* for service port 2 */
	index = search_service_index(servport_name2);
	if(index!=-1){
		document.filters.port2_start.value = services[index].start;
		document.filters.port2_end.value = services[index].end;
		document.filters.blocked_service2.selectedIndex = index+1; /* first will be none */
	}

	/* for service port 3 */
	index = search_service_index(servport_name3);
	if(index!=-1){
		document.filters.port3_start.value = services[index].start;
		document.filters.port3_end.value = services[index].end;
		document.filters.blocked_service3.selectedIndex = index+1; /* first will be none */
	}
}

function onChange_blockedServices(index, start, end) {
	index--
	if(index == -1) {
		start.value = '';
		end.value = '';
	} else {
		start.value = services[index].start;
		end.value = services[index].end;
	}
}

function Status(F,I) {
	var start = '';
	var end = '';
	var total = F.elements.length;
	for(i=0 ; i < total ; i++){
		if(F.elements[i].name == "blocked_service0")
			start = i;
		if(F.elements[i].name == "url5")
			end = i;
	}
	if(start == '' || end == '')
		return true;

	if(I == "deny" ) {
		for(i = start; i<=end ;i++) {
			choose_disable(F.elements[i]);
		}
	} else {
		for(i = start; i<=end ;i++) {
			choose_enable(F.elements[i]);
		}
		choose_disable(document.filters.port0_start);
		choose_disable(document.filters.port0_end);
		choose_disable(document.filters.port1_start);
		choose_disable(document.filters.port1_end);
		choose_disable(document.filters.port2_start);
		choose_disable(document.filters.port2_end);
		choose_disable(document.filters.port3_start);
		choose_disable(document.filters.port3_end);
	}
}
		</script>
	</head>

	<body class="gui" onunload="exit()" onload="init()"> <% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><a href="index.asp">Setup</a></li>
								<li><a href="Wireless_Basic.asp">Wireless</a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp">SIPatH</a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp">Security</a></li>
								<li class="current"><span>Access Restrictions</span>
									<div id="menuSub">
										<ul id="menuSubList"><% support_invmatch("PARENTAL_CONTROL_SUPPORT", "1", "<!--"); %>
											<li><a href="Parental_Control.asp">Parental Control</a></li>
											<% support_invmatch("PARENTAL_CONTROL_SUPPORT", "1", "-->"); %>
											<li><span>Internet Access</span></li>
										</ul>
									</div>
								</li>
								<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
								<li><a href="Management.asp">Administration</a></li>
								<li><a href="Status_Router.asp">Status</a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="filters" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button"/>
							<input type="hidden" name="action"/>
							<input type="hidden" name="change_action"/>
							<input type="hidden" name="submit_type"/>
							<input type="hidden" name="blocked_service"/>
							<input type="hidden" name="filter_web"/>
							<input type="hidden" name="filter_policy"/>
							<input type="hidden" name="filter_p2p"/>
							<input type="hidden" name="f_status"/>
							<h2>Internet Access</h2>
							<fieldset>
								<legend>Access Policy</legend>
								<div class="setting">
									<div class="label">Policy n&deg;</div>
									<select name="f_id" onchange="SelFilter(this.form.f_id.selectedIndex,this.form)"><% filter_policy_select(); %></select>
									<input type="button" value="Delete" onclick="to_delete(this.form)"/>
									<input type="button" value="Summary" onclick="ViewSummary()"/>
								</div>
								<div class="setting">
									<div class="label">Status</div>
									<input type="radio" value="enable" name="f_status1" <% filter_policy_get("f_status","enable"); %>/>Enable
									<input type="radio" value="disable" name="f_status1" <% filter_policy_get("f_status","disable"); %>/>Disable
								</div>
								<div class="setting">
									<div class="label">Enter Policy Name</div>
									<input maxlength="30" size="22" name="f_name" value="<% filter_policy_get('f_name',''); %>"/>
								</div>
								<div class="setting">
									<div class="label">PCs</div>
									<input type="button" value="Edit List of PCs" onclick="ViewFilter()"/>
								</div>
								<div class="setting">
									<div class="label">
										<input type="radio" name="f_status2" value="deny" onclick="Status(this.form,'deny')" <% filter_policy_get("f_status","deny"); %> /> Deny
									</div>
									Internet access during selected days and hours.
								</div>
								<div class="setting">
									<div class="label">
										<input type="radio" name="f_status2" value="allow" onclick="Status(this.form,'allow')" <% filter_policy_get("f_status","allow"); %> /> Allow
									</div>
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Days</legend>
								<div class="setting">
									<table>
										<tr>
											<td><input type="checkbox" value="1" name="day_all" onClick="dayall(this.form)" <% filter_tod_get("day_all"); %> /></td>
											<td><input type="checkbox" value="1" name="week0" <% filter_tod_get("week0"); %> /></td>
											<td><input type="checkbox" value="1" name="week1" <% filter_tod_get("week1"); %> /></td>
											<td><input type="checkbox" value="1" name="week2" <% filter_tod_get("week2"); %> /></td>
											<td><input type="checkbox" value="1" name="week3" <% filter_tod_get("week3"); %> /></td>
											<td><input type="checkbox" value="1" name="week4" <% filter_tod_get("week4"); %> /></td>
											<td><input type="checkbox" value="1" name="week5" <% filter_tod_get("week5"); %> /></td>
											<td><input type="checkbox" value="1" name="week6" <% filter_tod_get("week6"); %> /></td>
										</tr>
										<tr>
											<td>Everyday</td>
											<td>Sun</td>
											<td>Mon</td>
											<td>Tue</td>
											<td>Wed</td>
											<td>Thu</td>
											<td>Fri</td>
											<td>Sat</td>
										</tr>
									</table>
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Times</legend>
								<div class="setting">
									<div class="label">24 Hours</div>
									<input type="radio" value="1" name="time_all" onclick="timeall(this.form,'0')" <% filter_tod_get("time_all_en"); %> />
								</div>
								<div class="setting">
									<div class="label">From</div>
									<input type="hidden" name="allday" />
									<input type="radio" value="0" name="time_all" onclick="timeall(this.form,'1')" <% filter_tod_get("time_all_dis"); %> />
									<select name="start_hour"><% filter_tod_get("start_hour_12"); %></select>:<select name="start_min"><% filter_tod_get("start_min_5"); %></select>
									<select name="start_time">
										<option value="0" <% filter_tod_get("start_time_am"); %>>AM</option>
										<option value="1" <% filter_tod_get("start_time_pm"); %>>PM</option>
									</select> To <select name="end_hour"><% filter_tod_get("end_hour_12"); %></select>:<select name="end_min"><% filter_tod_get("end_min_5"); %></select>
									<select name="end_time">
										<option value="0" <% filter_tod_get("end_time_am"); %>>AM</option>
										<option value="1" <% filter_tod_get("end_time_pm"); %>>PM</option>
									</select>
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Blocked Services</legend>
								<div class="setting">
								<div class="label">Catch all P2P Protocols</div>
  								<input type="checkbox" name="_filter_p2p" value="1" <% nvram_match("filter_p2p", "1", "checked"); %>/>
								</div>
								<div class="setting">
									<select size="1" name="blocked_service0" onchange="onChange_blockedServices(blocked_service0.selectedIndex, port0_start, port0_end)">
										<option value="None" selected="selected">None</option>
										<script>write_service_options(servport_name0);</script>
									</select>
									<input maxLength="5" size="5" name="port0_start" class="num" readonly="readonly" /> ~<input maxLength="5" size="5" name="port0_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service1" onchange="onChange_blockedServices(blocked_service1.selectedIndex, port1_start, port1_end)">
										<option value="None" selected="selected">None</option>
										<script>write_service_options(servport_name1);</script>
									</select>
									<input maxLength="5" size="5" name="port1_start" class="num" readonly="readonly" /> ~<input maxLength="5" size="5" name="port1_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service2" onchange="onChange_blockedServices(blocked_service2.selectedIndex, port2_start, port2_end)">
										<option value="None" selected="selected">None</option>
										<script>write_service_options(servport_name2);</script>
									</select>
										<input maxLength="5" size="5" name="port2_start" class="num" readonly="readonly" /> ~<input maxLength="5" size="5" name="port2_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service3" onchange="onChange_blockedServices(blocked_service3.selectedIndex, port3_start, port3_end)">
										<option value="None" selected="selected">None</option>
										<script>write_service_options(servport_name3);</script>
									</select>
									<input maxLength="5" size="5" name="port3_start" class="num" readonly="readonly" /> ~<input maxLength="5" size="5" name="port3_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<input type="button" value="Add/Edit Service" onclick="self.open('Port_Services.asp','Port_Services','alwaysRised,resizable,scrollbars,width=630,height=360').focus()" />
								</div>
							</fieldset><br />
							<fieldset>
								<legend>Website Blocking by URL Address</legend>
								<input class="num" size="30" maxlength="79" name="host0" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","0"); %>" />
								<input class="num" size="30" maxlength="79" name="host1" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","1"); %>" />
								<br />
								<input class="num" size="30" maxlength="79" name="host2" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","2"); %>" />
								<input class="num" size="30" maxlength="79" name="host3" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","3"); %>" />
							</fieldset><br />
							<fieldset>
								<legend>Website Blocking by Keyword</legend>
								<input class="num" size="18" maxlength="79" name="url0" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","0"); %>" />
								<input class="num" size="18" maxlength="79" name="url1" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","1"); %>" />
								<input class="num" size="18" maxlength="79" name="url2" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","2"); %>" />
								<br />
								<input class="num" size="18" maxlength="79" name="url3" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","3"); %>" />
								<input class="num" size="18" maxlength="79" name="url4" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","4"); %>" />
								<input class="num" size="18" maxlength="79" name="url5" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","5"); %>" />
							</fieldset><br />
							<div class="submitFooter">
								<input type="button" name="save_button" value="Save Settings" onclick="to_submit(this.form)" />
								<input type="reset" value="Cancel Changes" />
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2>Help</h2></div>
						<dl>
							<dt class="term">Internet Access Policy: </dt>
							<dd class="definition">You may define up to 10 access policies. Click <em>Delete</em> to delete a policy or <em>Summary</em> to see a summary of the policy.</dd>
							<dt class="term">Status: </dt>
							<dd class="definition">Enable or disable a policy.</dd>
							<dt class="term">Policy Name: </dt>
							<dd class="definition">You may assign a name to your policy.</dd>
							<dt class="term">Days: </dt>
							<dd class="definition">Choose the day of the week you would like your policy to be applied.</dd>
							<dt class="term">Times: </dt>
							<dd class="definition">Enter the time of the day you would like your policy to apply.</dd>
							<dt class="term">Blocked Services: </dt>
							<dd class="definition">You may choose to block access to certain services. Click <em>Add/Edit</em> Services to modify these settings.</dd>
							<dt class="term">Website Blocking by URL: </dt>
							<dd class="definition">You can block access to certain websites by entering their URL.</dd>
							<dt class="term">Website Blocking by Keyword: </dt>
							<dd class="definition">You can block access to certain website by the keywords contained in their webpage.</dd>
						</dl>
						<br />
						<a target="_blank" href="help/HFilters.asp">More...</a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <% get_firmware_version(); %></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>