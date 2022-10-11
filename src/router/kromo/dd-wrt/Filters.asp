<% do_pagehead("filter.titl"); %>
	<script type="text/javascript">
	//<![CDATA[

<% filter_init(); %>
var summary_win = null;
var ipmac_win = null;

function dayall(F) {
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

function service(id, name, port_start, port_end, protocol, servicename) {
	this.id = id;
	this.name = name;
	this.start = port_start;
	this.end = port_end;
	this.protocol = protocol;
	this.deleted = false;
	this.modified = false;
	this.servicename = servicename;
}

var sorton = function(x,y) {
	if(x.name <  y.name) {
		return -1;
	} else { 
		if (x.name == y.name) {
			return 0;
		} else {
			return 1;
		}
	}
};

var services=new Array();
var services_length=0;
/* Init. services data structure */
<% filter_port_services_get("all_list", "0"); %>
services.sort(sorton);

<% gen_filters(); %>
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
		document.write(">"+services[i].name+" [ "+services[i].servicename+" ]"+"</option>");
	}
}

function setBlockedServicesValue() {
	var index;

	if (p2p_value) {
		if (p2p_value == "1") {
			document.filters._filter_p2p.checked = true;
		} else {
			document.filters._filter_p2p.checked = false;
		}
	}

	/* for service port 0 */
	for (i=0;i<document.filters.numfilters.value;i++) {
		index = search_service_index(eval("servport_name"+i));
		if(index!=-1) {
			eval("document.filters.port"+i+"_start").value = services[index].start;
			eval("document.filters.port"+i+"_end").value = services[index].end;
			eval("document.filters.blocked_service"+i).selectedIndex = index+1; /* first will be none */
		}
	}
}

function onchange_blockedServices(index, start, end) {
	index--;
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
			start = i - 1;
		if(F.elements[i].name == "url15")
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
		for(i = 0; i < F.numfilters.value; i++) {
			choose_disable(eval("document.filters.port"+i+"_start"));
			choose_disable(eval("document.filters.port"+i+"_end"));
		}
	}
	return true;
}

function filter_add_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "add_filter";
	apply(F);
}

function filter_remove_submit(F) {
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "remove_filter";
	apply(F);
}

function SelFilter(num,F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "sel_filter";
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
		F.apply_button.value = sbutton.applied;
		applytake(F);
	}
}

var update;

addEvent(window, "load", function() {
	stickControl(<% nvg("sticky_footer"); %>);
	day_enable_disable(document.filters, "<% filter_tod_get("day_all_init"); %>");
	time_enable_disable(document.filters, "<% filter_tod_get("time_all_init"); %>");
	setBlockedServicesValue();
	Status(document.filters, "<% filter_policy_get("f_status","onload_status"); %>");
	for(i = 0; i < document.filters.numfilters.value; i++) {
		choose_disable(eval("document.filters.port"+i+"_start"));
		choose_disable(eval("document.filters.port"+i+"_end"));
	}
	
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
						<form name="filters" action="apply.cgi" method="post" spellcheck="false">
							<input type="hidden" name="submit_button" value="Filters" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="numfilters" value="<% getnumfilters(); %>" />
							
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
									<div class="label"><% tran("share.intrface"); %></div>
									<% show_filterif("filter_if", "Any"); %>
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
										<% tran("share.deny"); %>
									</div>
									<input class="spaceradio" type="radio" name="f_status2" value="deny" onclick="Status(this.form,'deny')" <% filter_policy_get("f_status","deny"); %> /><% tran("filter.polallow"); %>
								</div>
								<div class="setting">
									<div class="label">
										<% tran("share.filter"); %>
									</div>
									<input class="spaceradio" type="radio" name="f_status2" value="allow" onclick="Status(this.form,'allow')" <% filter_policy_get("f_status","allow"); %> />
								</div>
								<br />
								<div class="setting">
									<div class="label"><% tran("filter.packetcount"); %></div>
									<% filter_getpacketcount(); %>&nbsp;
								</div>
								<br />
							</fieldset><br />
							<fieldset>
								<legend><% tran("filter.legend2"); %></legend>
								<div class="setting">
									<table class="table weekdays" summary="week days table">
										<tbody>
										<tr class="center">
											<td><% tran("share.everyday"); %></td>
											<td><% tran("share.sun_s"); %></td>
											<td><% tran("share.mon_s"); %></td>
											<td><% tran("share.tue_s"); %></td>
											<td><% tran("share.wed_s"); %></td>
											<td><% tran("share.thu_s"); %></td>
											<td><% tran("share.fri_s"); %></td>
											<td><% tran("share.sat_s"); %></td>
										</tr>
										<tr class="center">
											<td><input type="checkbox" value="1" name="day_all" onclick="dayall(this.form)" <% filter_tod_get("day_all"); %> /></td>
											<td><input type="checkbox" value="1" name="week0" <% filter_tod_get("week0"); %> /></td>
											<td><input type="checkbox" value="1" name="week1" <% filter_tod_get("week1"); %> /></td>
											<td><input type="checkbox" value="1" name="week2" <% filter_tod_get("week2"); %> /></td>
											<td><input type="checkbox" value="1" name="week3" <% filter_tod_get("week3"); %> /></td>
											<td><input type="checkbox" value="1" name="week4" <% filter_tod_get("week4"); %> /></td>
											<td><input type="checkbox" value="1" name="week5" <% filter_tod_get("week5"); %> /></td>
											<td><input type="checkbox" value="1" name="week6" <% filter_tod_get("week6"); %> /></td>
										</tr>
									</tbody>
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
									<select name="start_hour"><% filter_tod_get("start_hour_24"); %></select>:<select name="start_min"><% filter_tod_get("start_min_1"); %></select>&nbsp;
									<% tran("share.to"); %>
									&nbsp;<select name="end_hour"><% filter_tod_get("end_hour_24"); %></select>:<select name="end_min"><% filter_tod_get("end_min_1"); %></select>
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("filter.legend3"); %></legend>
								<div class="setting">
									<div class="label"><% tran("filter.catchall"); %></div>
									<input class="spaceradio" type="checkbox" name="_filter_p2p" value="1" <% nvc("filter_p2p", "1"); %> />
								</div>
								<table><tr class="center">
									<td><% show_filters(); %></td>
								</tr></table>
								<div class="center">
									<script type="text/javascript">
										//<![CDATA[
										document.write("<input class=\"button\" style=\"margin-right: .5em\" type=\"button\" value=\"" + sbutton.add + "\" onclick=\"filter_add_submit(this.form);\"/>");
										document.write("<input class=\"button\" style=\"margin-right: .5em\" type=\"button\" value=\"" + sbutton.del + "\" onclick=\"filter_remove_submit(this.form);\"/>");
										document.write("<input class=\"button\" type=\"button\" value=\"" + sbutton.filterSer + "\" onclick=\"openWindow('Port_Services.asp', 640, 500);\" />");
										//]]>
									</script>
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("filter.legend4"); %></legend>
								<div class="setting center">
									<input size="30" maxlength="79" name="host0" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","0"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host1" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","1"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host2" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","2"); %>" class="three_column three_column_last" />
								</div>
								<div class="setting center">
									<input size="30" maxlength="79" name="host3" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","3"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host4" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","4"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host5" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","5"); %>" class="three_column three_column_last" />
								</div>
								<div class="setting center">
									<input size="30" maxlength="79" name="host6" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","6"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host7" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","7"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host8" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","8"); %>" class="three_column three_column_last" />
								</div>
				 			<div class="setting center">
									<input size="30" maxlength="79" name="host9" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","9"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host10" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","10"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host11" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","11"); %>" class="three_column three_column_last" />
								</div>
							<div class="setting center">
									<input size="30" maxlength="79" name="host12" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","12"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host13" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","13"); %>" class="three_column" />
									<input size="30" maxlength="79" name="host14" onblur="valid_name(this,'URL')" value="<% filter_web_get("host","14"); %>" class="three_column three_column_last" />
								</div>
							</fieldset><br />
							<fieldset>
								<legend><% tran("filter.legend5"); %></legend>
								<div class="setting center">
									<input size="21" maxlength="79" name="url0" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","0"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url1" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","1"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url2" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","2"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url3" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","3"); %>" class="four_column four_column_last" />
								</div>
								<div class="setting center">
									<input size="21" maxlength="79" name="url4" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","4"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url5" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","5"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url6" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","6"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url7" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","7"); %>" class="four_column four_column_last" />
								</div>
								<div class="setting center">
									<input size="21" maxlength="79" name="url8" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","8"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url9" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","9"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url10" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","10"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url11" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","11"); %>" class="four_column four_column_last" />
								</div>
								<div class="setting center">
									<input size="21" maxlength="79" name="url12" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","12"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url13" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","13"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url14" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","14"); %>" class="four_column" />
									<input size="21" maxlength="79" name="url15" onblur="valid_name(this,'Keyword')" value="<% filter_web_get("url","15"); %>" class="four_column four_column_last" />
								</div>
							</fieldset><br />
							<div id="footer" class="submitFooter">
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
						<h2><% tran("share.help"); %></h2>
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
				<div class="info"><% tran("share.firmware"); %>:&nbsp;
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
