<% do_pagehead("qos.titl"); %>
		<script type="text/javascript">
		//<![CDATA[
		
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
	F.add_ipsprio_button.disabled = sw_disabled;
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
	F.add_macprio_button.disabled = sw_disabled;
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
	F.qos_type.disabled = sw_disabled;
	F.add_svc_button.disabled = sw_disabled;
	F.edit_svc_button.disabled = sw_disabled;
	<% nvram_match("portprio_support","0","/"); %><% nvram_match("portprio_support","0","/"); %>port_grey(sw_disabled, F);
	macs_grey(sw_disabled, F);
	ips_grey(sw_disabled, F);
	svcs_grey(sw_disabled, F);
}

function service(id, name, port_start, port_end) {
	this.id = id;
	this.name = name;
	this.start = port_start;
	this.end = port_end;
}

var sorton = function(x,y) {
	if(x.name <  y.name) return -1;
	else if (x.name == y.name) return 0;
	else return 1;
}

services=new Array();
services_length=0;
/* Init. services data structure */
<% filter_port_services_get("all_list", "0"); %>
services.sort(sorton);

function svc_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_svc";
	apply(F);
}

function ip_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_ip";
	apply(F);
}

function mac_add_submit(F) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "add_mac";
	apply(F);
}

function submitcheck(F) {
	if (F._enable_game.checked == false){
	    F.enable_game.value = 0;
	}else{
	    F.enable_game.value = 1;
	}
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
}

function to_submit(F) {
    submitcheck(F);
    apply(F);
}
function to_apply(F) {
    submitcheck(F);
    applytake(F);
}

var update;

addEvent(window, "load", function() {
	
	qos_grey(<% nvram_get("wshaper_enable"); %>,document.QoS);
	
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
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("ForwardSpec.asp","QoS.asp"); %>
				</div>
            	<div id="main">
					<div id="contents">
						<form name="QoS" action="apply.cgi" method="post" >
							<input type="hidden" name="submit_button" value="QoS" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							
							<input type="hidden" name="enable_game" value="1" />
							
							<h2><% tran("qos.h2"); %></h2>
							
							<fieldset>
  								<legend><% tran("qos.legend"); %></legend>
  								<div class="setting">
									<div class="label"><% tran("qos.srv"); %></div>
									<input class="spaceradio" type="radio" value="1" name="wshaper_enable" onclick="qos_grey(this.value,this.form)" <% nvram_selmatch("wshaper_enable", "1", "checked"); %> /><% tran("share.enable"); %>&nbsp;
									<input class="spaceradio" type="radio" value="0" name="wshaper_enable" onclick="qos_grey(this.value,this.form)" <% nvram_selmatch("wshaper_enable", "0", "checked"); %> /><% tran("share.disable"); %>
								</div>
								<div class="setting">
									<div class="label"><% tran("share.port"); %></div>
									<select name="wshaper_dev">
										<option value="WAN" <% nvram_selmatch("wshaper_dev", "WAN", "selected"); %>>WAN</option>
										<option value="LAN" <% nvram_selmatch("wshaper_dev", "LAN", "selected"); %>>LAN &amp; WLAN</option>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("qos.type"); %></div>
									<select name="qos_type">
										<option value="0" <% nvram_selmatch("qos_type", "0", "selected"); %>>HTB</option>
										<option value="1" <% nvram_selmatch("qos_type", "1", "selected"); %>>HFSC</option>
									</select>
								</div>
								<div class="setting">
									<div class="label"><% tran("qos.uplink"); %></div>
									<input type="text" size="5" class="num" name="wshaper_uplink" value="<% nvram_get("wshaper_uplink"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("qos.dnlink"); %></div>
									<input type="text" size="5" class="num" name="wshaper_downlink" value="<% nvram_get("wshaper_downlink"); %>" />
								</div>
								<div class="setting">
									<div class="label"><% tran("qos.gaming"); %></div>
									<input type="checkbox" name="_enable_game" value="1" <% nvram_checked("enable_game", "1"); %> />
								</div>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("qos.legend2"); %></legend>
								<table class="table" summary="services priority table">
									<tr>
										<th><% tran("share.del"); %></th>
										<th><% tran("share.srv"); %></th>
										<th><% tran("share.priority"); %></th>
									</tr>
									<% get_qossvcs(); %>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2">
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" name=\"add_svc_button\" value=\"" + sbutton.add + "\" onclick=\"svc_add_submit(this.form);\" />");
											//]]>
											</script>&nbsp;&nbsp;&nbsp;
											<select name="add_svc">
												<script type="text/javascript">
												//<![CDATA[
													var i=0;
													for(i=0;i<services_length;i++)
													document.write("<option value=\""+services[i].name+"\">"+services[i].name+ " [ "+
													services[i].start+" ~ "+
													services[i].end + " ]" + "</option>");
												//]]>
												</script>
											</select>
										</td>
									</tr>
								</table><br />
								<div class="center">
									<script type="text/javascript">
									//<![CDATA[
									document.write("<input class=\"button\" type=\"button\" name=\"edit_svc_button\" value=\"" + sbutton.edit_srv + "\" onclick=\"openWindow('Port_Services.asp', 630, 430);\" />");
									//]]>
									</script>&nbsp;&nbsp;&nbsp;
								</div>
								
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("qos.legend3"); %></legend>
								<table class="table" summary="IP addresses priority table">
									<% get_qosips(); %>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2">
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" name=\"add_ipsprio_button\" value=\"" + sbutton.add + "\" onclick=\"ip_add_submit(this.form);\" />");
											//]]>
											</script>&nbsp;&nbsp;&nbsp;
											<input size="3" maxlength="3" name="svqos_ipaddr0" value="0" onblur="valid_range(this,0,255,'IP')" class="num" />.<input size="3" maxlength="3" name="svqos_ipaddr1" value="0" onblur="valid_range(this,0,255,'IP')" class="num" />.<input size="3" maxlength="3" name="svqos_ipaddr2" value="0" onblur="valid_range(this,0,255,'IP')" class="num" />.<input size="3" maxlength="3" name="svqos_ipaddr3" value="0" onblur="valid_range(this,0,255,'IP')" class="num" />&nbsp;/&nbsp;
											<input size="3" maxlength="3" name="svqos_netmask" value="0" onblur="valid_range(this,0,32,share.subnet)" class="num" />
										</td>
									</tr>
								</table>
							</fieldset><br />
							
							<fieldset>
								<legend><% tran("qos.legend4"); %></legend>
								<table class="table" summary="MAC priority table">
									<% get_qosmacs(); %>
									<tr>
										<td>&nbsp;</td>
										<td colspan="2">
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input class=\"button\" type=\"button\" name=\"add_macprio_button\" value=\"" + sbutton.add + "\" onclick=\"mac_add_submit(this.form);\" />")
											//]]>
											</script>&nbsp;&nbsp;&nbsp;
											<input name="svqos_hwaddr0" value="00" size="2" maxlength="2" onblur="valid_mac(this,0)" class="num" />:<input name="svqos_hwaddr1" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num" />:<input name="svqos_hwaddr2" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num"/>:<input name="svqos_hwaddr3" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num" />:<input name="svqos_hwaddr4" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num" />:<input name="svqos_hwaddr5" value="00" size="2" maxlength="2" onblur="valid_mac(this,1)" class="num" />
										</td>
									</tr>
								</table>
							</fieldset><br />
							<% show_default_level(); %>
<% nvram_match("portprio_support","0","<!--"); %>							
							<fieldset>
								<legend><% tran("qos.legend5"); %></legend>
								<table>
									<tr>
										<th>&nbsp;</th>
										<th><% tran("share.priority"); %></th>
										<th><% tran("qos.maxrate_o"); %></th>
									</tr>
									<tr>
										<td><% tran("share.port"); %> 1</td>
										<td>
											<select name="svqos_port1prio">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"100\" <% nvram_selmatch("svqos_port1prio", "100", "selected"); %> >" + qos.prio_x + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"10\" <% nvram_selmatch("svqos_port1prio", "10", "selected"); %> >" + qos.prio_p + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"20\" <% nvram_selmatch("svqos_port1prio", "20", "selected"); %> >" + qos.prio_e + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"30\" <% nvram_selmatch("svqos_port1prio", "30", "selected"); %> >" + share.standard + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"40\" <% nvram_selmatch("svqos_port1prio", "40", "selected"); %> >" + qos.prio_b + "</option>");
												//]]>
												</script>
											</select>
										</td>
										<td>
											<select name="svqos_port1bw">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"0\" <% nvram_match("svqos_port1bw", "0", "selected"); %> >" + share.disable + "</option>");
												//]]>
												</script>
												<option value="256K" <% nvram_match("svqos_port1bw", "256K", "selected"); %>>256k</option>
												<option value="512K" <% nvram_match("svqos_port1bw", "512K", "selected"); %>>512k</option>
												<option value="1M" <% nvram_match("svqos_port1bw", "1M", "selected"); %>>1M</option>
												<option value="2M" <% nvram_match("svqos_port1bw", "2M", "selected"); %>>2M</option>
												<option value="5M" <% nvram_match("svqos_port1bw", "5M", "selected"); %>>5M</option>
												<option value="10M" <% nvram_match("svqos_port1bw", "10M", "selected"); %>>10M</option>
												<option value="20M" <% nvram_match("svqos_port1bw", "20M", "selected"); %>>20M</option>
												<option value="50M" <% nvram_match("svqos_port1bw", "50M", "selected"); %>>50M</option>
												<option value="FULL" <% nvram_match("svqos_port1bw", "FULL", "selected"); %>>100M</option>
											</select>
										</td>
									</tr>
									<tr>
										<td><% tran("share.port"); %> 2</td>
										<td>
											<select name="svqos_port2prio">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"100\" <% nvram_selmatch("svqos_port2prio", "100", "selected"); %> >" + qos.prio_x + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"10\" <% nvram_selmatch("svqos_port2prio", "10", "selected"); %> >" + qos.prio_p + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"20\" <% nvram_selmatch("svqos_port2prio", "20", "selected"); %> >" + qos.prio_e + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"30\" <% nvram_selmatch("svqos_port2prio", "30", "selected"); %> >" + share.standard + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"40\" <% nvram_selmatch("svqos_port2prio", "40", "selected"); %> >" + qos.prio_b + "</option>");
												//]]>
												</script>
											</select>
										</td>
										<td>
											<select name="svqos_port2bw">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"0\" <% nvram_selmatch("svqos_port2bw", "0", "selected"); %> >" + share.disable + "</option>");
												//]]>
												</script>
												<option value="256K" <% nvram_selmatch("svqos_port2bw", "256K", "selected"); %>>256k</option>
												<option value="512K" <% nvram_selmatch("svqos_port2bw", "512K", "selected"); %>>512k</option>
												<option value="1M" <% nvram_selmatch("svqos_port2bw", "1M", "selected"); %>>1M</option>
												<option value="2M" <% nvram_selmatch("svqos_port2bw", "2M", "selected"); %>>2M</option>
												<option value="5M" <% nvram_selmatch("svqos_port2bw", "5M", "selected"); %>>5M</option>
												<option value="10M" <% nvram_selmatch("svqos_port2bw", "10M", "selected"); %>>10M</option>
												<option value="20M" <% nvram_selmatch("svqos_port2bw", "20M", "selected"); %>>20M</option>
												<option value="50M" <% nvram_selmatch("svqos_port2bw", "50M", "selected"); %>>50M</option>
												<option value="FULL" <% nvram_selmatch("svqos_port2bw", "FULL", "selected"); %>>100M</option>
											</select>
										</td>
									</tr>
									<tr>
										<td><% tran("share.port"); %> 3</td>
										<td>
											<select name="svqos_port3prio">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"100\" <% nvram_selmatch("svqos_port3prio", "100", "selected"); %> >" + qos.prio_x + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"10\" <% nvram_selmatch("svqos_port3prio", "10", "selected"); %> >" + qos.prio_p + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"20\" <% nvram_selmatch("svqos_port3prio", "20", "selected"); %> >" + qos.prio_e + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"30\" <% nvram_selmatch("svqos_port3prio", "30", "selected"); %> >" + share.standard + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"40\" <% nvram_selmatch("svqos_port3prio", "40", "selected"); %> >" + qos.prio_b + "</option>");
												//]]>
												</script>
										</select>
										</td>
										<td>
											<select name="svqos_port3bw">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"0\" <% nvram_selmatch("svqos_port3bw", "0", "selected"); %> >" + share.disable + "</option>");
												//]]>
												</script>
												<option value="256K" <% nvram_selmatch("svqos_port3bw", "256K", "selected"); %>>256k</option>
												<option value="512K" <% nvram_selmatch("svqos_port3bw", "512K", "selected"); %>>512k</option>
												<option value="1M" <% nvram_selmatch("svqos_port3bw", "1M", "selected"); %>>1M</option>
												<option value="2M" <% nvram_selmatch("svqos_port3bw", "2M", "selected"); %>>2M</option>
												<option value="5M" <% nvram_selmatch("svqos_port3bw", "5M", "selected"); %>>5M</option>
												<option value="10M" <% nvram_selmatch("svqos_port3bw", "10M", "selected"); %>>10M</option>
												<option value="20M" <% nvram_selmatch("svqos_port3bw", "20M", "selected"); %>>20M</option>
												<option value="50M" <% nvram_selmatch("svqos_port3bw", "50M", "selected"); %>>50M</option>
												<option value="FULL" <% nvram_selmatch("svqos_port3bw", "FULL", "selected"); %>>100M</option>
											</select>
										</td>
									</tr>
									<tr>
										<td><% tran("share.port"); %> 4</td>
										<td>
											<select name="svqos_port4prio">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"100\" <% nvram_selmatch("svqos_port4prio", "100", "selected"); %> >" + qos.prio_x + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"10\" <% nvram_selmatch("svqos_port4prio", "10", "selected"); %> >" + qos.prio_p + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"20\" <% nvram_selmatch("svqos_port4prio", "20", "selected"); %> >" + qos.prio_e + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"30\" <% nvram_selmatch("svqos_port4prio", "30", "selected"); %> >" + share.standard + "</option>");
												//]]>
												</script>
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"40\" <% nvram_selmatch("svqos_port4prio", "40", "selected"); %> >" + qos.prio_b + "</option>");
												//]]>
												</script>
											</select>
										</td>
										<td>
											<select name="svqos_port4bw">
												<script type="text/javascript">
												//<![CDATA[
												document.write("<option value=\"0\" <% nvram_selmatch("svqos_port4bw", "0", "selected"); %> >" + share.disable + "</option>");
												//]]>
												</script>
												<option value="256K" <% nvram_selmatch("svqos_port4bw", "256K", "selected"); %>>256k</option>
												<option value="512K" <% nvram_selmatch("svqos_port4bw", "512K", "selected"); %>>512k</option>
												<option value="1M" <% nvram_selmatch("svqos_port4bw", "1M", "selected"); %>>1M</option>
												<option value="2M" <% nvram_selmatch("svqos_port4bw", "2M", "selected"); %>>2M</option>
												<option value="5M" <% nvram_selmatch("svqos_port4bw", "5M", "selected"); %>>5M</option>
												<option value="10M" <% nvram_selmatch("svqos_port4bw", "10M", "selected"); %>>10M</option>
												<option value="20M" <% nvram_selmatch("svqos_port4bw", "20M", "selected"); %>>20M</option>
												<option value="50M" <% nvram_selmatch("svqos_port4bw", "50M", "selected"); %>>50M</option>
												<option value="FULL" <% nvram_selmatch("svqos_port4bw", "FULL", "selected"); %>>100M</option>
											</select>
										</td>
									</tr>
								</table>
							</fieldset><br />
<% nvram_match("portprio_support","0","-->"); %>							
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
							<dt class="term"><% tran("hqos.right1"); %></dt>
							<dd class="definition"><% tran("hqos.right2"); %></dd>
							<dt class="term"><% tran("hqos.right3"); %></dt>
							<dd class="definition"><% tran("hqos.right4"); %></dd>
							<dt class="term"><% tran("qos.legend2"); %>:</dt>
							<dd class="definition"><% tran("hqos.right6"); %></dd>
							<dt class="term"><% tran("qos.legend3"); %>:</dt>
							<dd class="definition"><% tran("hqos.right8"); %></dd>
							<dt class="term"><% tran("qos.legend4"); %>:</dt>
							<dd class="definition"><% tran("hqos.right10"); %></dd>
							<dt class="term"><% tran("qos.legend5"); %>:</dt>
							<dd class="definition"><% tran("hqos.right12"); %></dd>
						</dl>
						<br/>
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HQos.asp');"><% tran("share.more"); %></a>
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