<% do_pagehead("filter.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

<% filter_init(); %>
var summary_win = null;
var ipmac_win = null;

function dayall(F) {
	if(F.day_all.checked == false)
		I = 1;
	else
		I = 0;
	
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
		choose_enable(F.end_hour);
		choose_enable(F.end_min);
	} else if(I == 0) {
		choose_disable(F.start_hour);
		choose_disable(F.start_min);
		choose_disable(F.end_hour);
		choose_disable(F.end_min);
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
		F.week6.checked == false) {
			alert(filter.mess2);
			return false;
	}
	if(F.time_all[1].checked == true){
		start = (parseInt(F.start_hour.value, 10)) * 60 + parseInt(F.start_min.value, 10);
		end = (parseInt(F.end_hour.value, 10)) * 60 + parseInt(F.end_min.value, 10);
		if(end <= start){
			alert(filter.mess3);
			return false;
		}
	}
	if(F.f_status1[1].checked == true) {		// Disable
		F.f_status.value = "0";
	} else {									// Enable
		if(F.f_status2[1].checked == true) {	// Allow
			F.f_status.value = "2";
		} else {								// deny
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

var services=new Array();
var services_length=0;
/* Init. services data structure */
<% filter_port_services_get("all_list", "0"); %>
services.sort(sorton);

var servport_name0 = "<% filter_port_services_get("service", "0"); %>";
var servport_name1 = "<% filter_port_services_get("service", "1"); %>";
var servport_name2 = "<% filter_port_services_get("service", "2"); %>";
var servport_name3 = "<% filter_port_services_get("service", "3"); %>";
var p2p_value = "<% filter_port_services_get("p2p", "0"); %>";

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

	if (p2p_value)
	if (p2p_value == "1")
	    document.filters._filter_p2p.checked = true;
	else
	    document.filters._filter_p2p.checked = false;

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

function onchange_blockedServices(index, start, end) {
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
	for(var i=0 ; i < total ; i++){
		if(F.elements[i].name == "_filter_p2p")
			start = i;
		if(F.elements[i].name == "url7")
			end = i;
	}
	if(start == '' || end == '') return true;

	if(I == "deny" ) {
		for(i = start; i<=end; i++) {
			choose_disable(F.elements[i]);
		}
	} else {
		for(i = start; i<=end; i++) {
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
	return true;
}

function SelFilter(num,F) {
	F.change_action.value="gozila_cgi";
	F.f_id.value=F.f_id.options[num].value;
	apply(F);
}

function to_delete(F) {
	if(confirm(filter.mess1)) {
		F.change_action.value="gozila_cgi";
		F.submit_type.value = "delete";
		F.submit();
	}
}

function to_submit(F) {
	if(valid(F) == true) {
		F.change_action.value = "";
		F.submit_type.value = "";
		F.save_button.value = sbutton.saving;
		apply(F);
	}
}
function to_apply(F) {
	if(valid(F) == true) {
		F.change_action.value = "";
		F.submit_type.value = "";
		F.save_button.value = sbutton.saving;
		applytake(F);
	}
}

var update;

addEvent(window, "load", function() {
	
	day_enable_disable(document.filters, "<% filter_tod_get("day_all_init"); %>");
	time_enable_disable(document.filters, "<% filter_tod_get("time_all_init"); %>");
	setBlockedServicesValue();
	Status(document.filters, "<% filter_policy_get("f_status","onload_status"); %>");
	choose_disable(document.filters.port0_start);
	choose_disable(document.filters.port0_end);
	choose_disable(document.filters.port1_start);
	choose_disable(document.filters.port1_end);
	choose_disable(document.filters.port2_start);
	choose_disable(document.filters.port2_end);
	choose_disable(document.filters.port3_start);
	choose_disable(document.filters.port3_end);
	
	update = new StatusbarUpdate();
	update.start();

});

addEvent(window, "unload", function() {
	update.stop();

});
	
		//]]>
		</script>
	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo">
						<h1><% show_control(); %></h1>
					</div>
					<% do_menu("Filters.asp","Filters.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="filters" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="Filters" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							
							<input type="hidden" name="blocked_service" />
							<input type="hidden" name="filter_web" />
							<input type="hidden" name="filter_policy" />
							<input type="hidden" name="filter_p2p" />
							<input type="hidden" name="f_status" />
							<h2><% tran("filter.h2"); %></h2>
							
							<fieldset>
								<legend><% tran("filter.legend"); %></legend>
								<div class="setting">
									<div class="label"><% tran("filter.pol"); %></div>
									<select name="f_id" onchange="SelFilter(this.form.f_id.selectedIndex,this.form)"><% filter_policy_select(); %></select>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.del + "\" onclick=\"to_delete(this.form);\" />");
									document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.summary + "\" onclick=\"openWindow('FilterSummary.asp', 700, 480);\" />");
									//]]>
									</script>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.statu"); %></div>
									<input class="spaceradio" type="radio" value="enable" name="f_status1" <% filter_policy_get("f_status","enable"); %>/><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="disable" name="f_status1" <% filter_policy_get("f_status","disable"); %>/><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("filter.polname"); %></div>
									<input maxlength="30" size="22" name="f_name" value="<% filter_policy_get("f_name",""); %>"/>
								</div>
								<div class="setting">
									<div class="label"><% tran("filter.pcs"); %></div>
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.filterIP + "\" onclick=\"openWindow('FilterIPMAC.asp', 590, 730);\" />");
									//]]>
									</script>
								</div>
								<div class="setting">
									<div class="label">
										<input class="spaceradio" type="radio" name="f_status2" value="deny" onclick="Status(this.form,'deny')" <% filter_policy_get("f_status","deny"); %> /><% tran("share.deny"); %>
									</div>
									<% tran("filter.polallow"); %>
								</div>
								<div class="setting">
									<div class="label">
										<input class="spaceradio" type="radio" name="f_status2" value="allow" onclick="Status(this.form,'allow')" <% filter_policy_get("f_status","allow"); %> /><% tran("share.filter"); %>
									</div>
								</div>
								<br />
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("filter.legend2"); %></legend>
								<div class="setting">
									<table summary="week days table">
										<tr>
											<td align="center"><% tran("share.everyday"); %></td>
											<td align="center"><% tran("share.sun_s"); %></td>
											<td align="center"><% tran("share.mon_s"); %></td>
											<td align="center"><% tran("share.tue_s"); %></td>
											<td align="center"><% tran("share.wed_s"); %></td>
											<td align="center"><% tran("share.thu_s"); %></td>
											<td align="center"><% tran("share.fri_s"); %></td>
											<td align="center"><% tran("share.sat_s"); %></td>
										</tr>
										<tr>
											<td align="center"><input type="checkbox" value="1" name="day_all" onclick="dayall(this.form)" <% filter_tod_get("day_all"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week0" <% filter_tod_get("week0"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week1" <% filter_tod_get("week1"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week2" <% filter_tod_get("week2"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week3" <% filter_tod_get("week3"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week4" <% filter_tod_get("week4"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week5" <% filter_tod_get("week5"); %> /></td>
											<td align="center"><input type="checkbox" value="1" name="week6" <% filter_tod_get("week6"); %> /></td>
										</tr>
									</table>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("filter.time"); %></legend>
								<div class="setting">
									<div class="label"><% tran("filter.h24"); %></div>
									<input class="spaceradio" type="radio" value="1" name="time_all" onclick="timeall(this.form,'0')" <% filter_tod_get("time_all_en"); %> />
								</div>
								<div class="setting">
									<div class="label"><% tran("share.from"); %></div>
									<input type="hidden" name="allday" />
									<input class="spaceradio" type="radio" value="0" name="time_all" onclick="timeall(this.form,'1')" <% filter_tod_get("time_all_dis"); %> />
									<select name="start_hour"><% filter_tod_get("start_hour_24"); %></select>:<select name="start_min"><% filter_tod_get("start_min_1"); %></select>&nbsp;&nbsp;&nbsp;
									<% tran("share.to"); %>&nbsp;
									<select name="end_hour"><% filter_tod_get("end_hour_24"); %></select>:<select name="end_min"><% filter_tod_get("end_min_1"); %></select>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("filter.legend3"); %></legend>
								<div class="setting">
									<div class="label"><% tran("filter.catchall"); %></div>
									<input class="spaceradio" type="checkbox" name="_filter_p2p" value="1" <% nvram_checked("filter_p2p", "1"); %> />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service0" onchange="onchange_blockedServices(blocked_service0.selectedIndex, port0_start, port0_end)">
										<option value="None" selected="selected">None</option>
										<script type="text/javascript">
										//<![CDATA[
										write_service_options(servport_name0);
										//]]>
										</script>
									</select>
									<input maxLength="5" size="5" name="port0_start" class="num" readonly="readonly" /> ~ <input maxLength="5" size="5" name="port0_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service1" onchange="onchange_blockedServices(blocked_service1.selectedIndex, port1_start, port1_end)">
										<option value="None" selected="selected">None</option>
										<script type="text/javascript">
										//<![CDATA[
										write_service_options(servport_name1);
										//]]>
										</script>
									</select>
									<input maxLength="5" size="5" name="port1_start" class="num" readonly="readonly" /> ~ <input maxLength="5" size="5" name="port1_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service2" onchange="onchange_blockedServices(blocked_service2.selectedIndex, port2_start, port2_end)">
										<option value="None" selected="selected">None</option>
										<script type="text/javascript">
										//<![CDATA[
										write_service_options(servport_name2);
										//]]>
										</script>
									</select>
										<input maxLength="5" size="5" name="port2_start" class="num" readonly="readonly" /> ~ <input maxLength="5" size="5" name="port2_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<select size="1" name="blocked_service3" onchange="onchange_blockedServices(blocked_service3.selectedIndex, port3_start, port3_end)">
										<option value="None" selected="selected">None</option>
										<script type="text/javascript">
										//<![CDATA[
										write_service_options(servport_name3);
										//]]>
										</script>
									</select>
									<input maxLength="5" size="5" name="port3_start" class="num" readonly="readonly" /> ~ <input maxLength="5" size="5" name="port3_end" class="num" readonly="readonly" />
								</div>
								<div class="setting">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.filterSer + "\" onclick=\"openWindow('Port_Services.asp', 640, 430);\" />");
									//]]>
									</script>
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("filter.legend4"); %></legend>
								<div class="setting center">
									<input size="30" maxlength="79" name="host0" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","0"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host1" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","1"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host2" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","2"); %>" />
								</div>
								<div class="setting center">
									<input size="30" maxlength="79" name="host3" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","3"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host4" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","4"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host5" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","5"); %>" />
								</div>
								<div class="setting center">
									<input size="30" maxlength="79" name="host6" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","6"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host7" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","7"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="30" maxlength="79" name="host8" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","8"); %>" />
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("filter.legend5"); %></legend>
								<div class="setting center">
									<input size="21" maxlength="79" name="url0" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","0"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url1" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","1"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url2" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","2"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url3" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","3"); %>" />
								</div>
								<div class="setting center">
									<input size="21" maxlength="79" name="url4" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","4"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url5" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","5"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url6" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","6"); %>" />&nbsp;&nbsp;&nbsp;
									<input size="21" maxlength="79" name="url7" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","7"); %>" />
								</div>
							</fieldset><br />
							
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								submitFooterButton(1,1);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("filter.legend"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right2"); %></dd>
							<dt class="term"><% tran("share.statu"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right4"); %></dd>
							<dt class="term"><% tran("filter.polname"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right6"); %></dd>
							<dt class="term"><% tran("filter.legend2"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right8"); %></dd>
							<dt class="term"><% tran("filter.time"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right10"); %></dd>
							<dt class="term"><% tran("filter.legend3"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right12"); %></dd>
							<dt class="term"><% tran("filter.legend4"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right14"); %></dd>
							<dt class="term"><% tran("filter.legend5"); %>:</dt>
							<dd class="definition"><% tran("hfilter.right16"); %></dd>
						</dl>
						<br />
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HFilters.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
				<div class="info"><% tran("share.firmware"); %>: 
					<script type="text/javascript">
					//<![CDATA[
					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
					//]]>
					</script>
				</div>
				<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
				<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>
