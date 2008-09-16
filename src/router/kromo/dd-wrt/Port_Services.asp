<% do_pagehead("portserv.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

var ref_page = window.opener.location;

function service(id, name, port_start, port_end, protocol) {
	this.id = id;
	this.name = name;
	this.start = port_start;
	this.end = port_end;
	this.protocol = protocol;
	this.deleted = false;
	this.modified = false;
}

services=new Array();
services_length=0;
/* Init. services data structure */
<% filter_port_services_get("user_list", "0"); %>

function setValue() {
	document.PortServices.ServiceTable.selectedIndex = 0;
	if (services_length == 0)	return;
	document.PortServices.Add_Service_Name.value =
		services[document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex].value].name;
	switch(services[document.PortServices.ServiceTable.options[
		document.PortServices.ServiceTable.selectedIndex].value].protocol){
	 case "icmp":
	 case 1:
		document.PortServices.Add_Service_Protocol.selectedIndex = 0;
		break;
	 default:
	 case "tcp":
	 case 6:
		document.PortServices.Add_Service_Protocol.selectedIndex = 1;
		break;
	 case "udp":
	 case 17:
		document.PortServices.Add_Service_Protocol.selectedIndex = 2;
		break;
	 case "both":
	 case 23:
	 	document.PortServices.Add_Service_Protocol.selectedIndex = 3;
		break;
	}
	document.PortServices.Add_Service_Port_S.value =
		services[document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex].value].start;
	document.PortServices.Add_Service_Port_E.value =
		services[document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex].value].end;
	onchange_Prot(document.PortServices.Add_Service_Protocol.options[document.PortServices.Add_Service_Protocol.selectedIndex].value);
}

function onchange_ServiceTable(index, name, protocol, start, end){
	name.value = services[index].name;
	switch(services[index].protocol){
	 case "icmp":
	 case 1:
	 case "1":
		protocol.selectedIndex = 0;
      break;
	 case "tcp":
	 case 6:
	 case "6":
		protocol.selectedIndex = 1;
		break;
	 case "udp":
	 case 17:
	 case "17":
		protocol.selectedIndex = 2;
		break;
	 case "both":
	 case 23:
	 case "23":
		protocol.selectedIndex = 3;
		break;
	}
	
	onchange_Prot(document.PortServices.Add_Service_Protocol.options[document.PortServices.Add_Service_Protocol.selectedIndex].value);
	
	start.value = services[index].start;
	end.value = services[index].end;
}

function onclick_Delete() {
	if(!confirm(errmsg.err53))
		return false;
	var index = document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex].value;
	services[index].deleted = true;
	document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex] = null;
	document.PortServices.ServiceTable.selectedIndex = 0;
	setValue();
}

function onclick_Modify(){
	if(!confirm(errmsg.err53))
		return false;
	var index1 = document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.selectedIndex].value;
	var index2 = document.PortServices.ServiceTable.selectedIndex;
	services[index1].modified = true;
	services[index1].name = document.PortServices.Add_Service_Name.value;
	services[index1].protocol = document.PortServices.Add_Service_Protocol.options[
							document.PortServices.Add_Service_Protocol.selectedIndex].value.valueOf();
	services[index1].start = document.PortServices.Add_Service_Port_S.value;
	services[index1].end = document.PortServices.Add_Service_Port_E.value;

	document.PortServices.ServiceTable.options[index2].text = services[index1].name + " [ "+
				services[index1].start+" ~ "+services[index1].end+" ]";

}

function onclick_Add(){
	if(document.PortServices.Add_Service_Name.value == ""){
		alert(errmsg.err54);
		document.PortServices.Add_Service_Name.focus();
		return false;
	}
 	for(i=0 ; i<services.length ; i++){
		if(services[i].deleted == true)	continue;
		if(document.PortServices.Add_Service_Name.value == services[i].name){
			alert(errmsg.err55);
			document.PortServices.Add_Service_Name.focus();
			return false;
		}
 	}

	if(!confirm(errmsg.err53))
		return false;

	if(document.PortServices.Add_Service_Port_S.value > document.PortServices.Add_Service_Port_E.value){
		var start = document.PortServices.Add_Service_Port_S.value;
		var end = document.PortServices.Add_Service_Port_E.value;
		document.PortServices.Add_Service_Port_S.value = end;
		document.PortServices.Add_Service_Port_E.value = start;
	}
	if(document.PortServices.Add_Service_Protocol[
                                        document.PortServices.Add_Service_Protocol.selectedIndex].value == "1"){	// icmp
		document.PortServices.Add_Service_Port_S.value = "0";
		document.PortServices.Add_Service_Port_E.value = "0";
	}

	services[services_length] = new service(
		-1,
		document.PortServices.Add_Service_Name.value, document.PortServices.Add_Service_Port_S.value,
		document.PortServices.Add_Service_Port_E.value,
		document.PortServices.Add_Service_Protocol.options[
			document.PortServices.Add_Service_Protocol.selectedIndex].value
	);
	services[services_length].modified = true;
	document.PortServices.ServiceTable.options[document.PortServices.ServiceTable.length] =
							new Option(services[services_length].name +
							" [ "+services[services_length].start+" ~ "+services[services_length].end+" ]",
							services_length, 0, 1);
	services_length++;
}

function submitcheck(F) {
 	var i=0;
	var index=0;
	var svclen=0;
	var svc;

	F.services_length.value0 = 0;
	F.services_length.value1 = 0;
	F.services_length.value2 = 0;
	F.services_length.value3 = 0;
	F.services_length.value4 = 0;
	F.services_length.value5 = 0;
	F.services_length.value6 = 0;
	F.services_length.value7 = 0;

 	for(i=0; i<services.length; i++) {

		if(services[i].deleted == true){
			continue;
		}
 		svc = services[i].start+":"+services[i].end+":"+services[i].protocol+":"+services[i].name+"(&nbsp;)";
		eval("svclen = F.services_array"+index+".value.length + svc.length");
		if( svclen > 1024) {
		   index = index + 1;
		   svclen = 0;
		}
 		eval("F.services_array"+index+".value += svc");
 	}
	F.change_action.value="gozila_cgi";
	
}

function to_submit(F)
{
submitcheck(F);
apply(F);
}

function to_apply(F)
{
submitcheck(F);
applytake(F);
}

function onchange_Prot(I) {
	if(I == "1") {
		setElementsActive("Add_Service_Port_S", "Add_Service_Port_E", false);
	} else {
		setElementsActive("Add_Service_Port_S", "Add_Service_Port_E", true);
	}
}

addEvent(window, "load", function() {
   setValue();
   if (ref_page == "Filters.asp") {
      document.PortServices.submit_button.value = "Port_Services";
      document.PortServices.submit_type.value = "save_services";
   } else {
      document.PortServices.submit_button.value = "QOSPort_Services";
      document.PortServices.submit_type.value = "save_qosservices";
   }
});

addEvent(window, "unload", function() {
	top.opener.window.location.href =  ref_page;
});
		
		//]]>
		</script>
	</head>
	<body>
		<form name="PortServices" action="apply.cgi" method="post" >
		<input type="hidden" name="submit_button" />
		<input type="hidden" name="action" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		
		<input type="hidden" name="services_array" />
		<input type="hidden" name="services_array0" />
		<input type="hidden" name="services_array1" />
		<input type="hidden" name="services_array2" />
		<input type="hidden" name="services_array3" />
		<input type="hidden" name="services_array4" />
		<input type="hidden" name="services_array5" />
		<input type="hidden" name="services_array6" />
		<input type="hidden" name="services_array7" />
		<input type="hidden" name="services_length" />
		<input type="hidden" name="services_length0" />
		<input type="hidden" name="services_length1" />
		<input type="hidden" name="services_length2" />
		<input type="hidden" name="services_length3" />
		<input type="hidden" name="services_length4" />
		<input type="hidden" name="services_length5" />
		<input type="hidden" name="services_length6" />
		<input type="hidden" name="services_length7" />
		<div>
		    <h2><% tran("portserv.h2"); %></h2>
			<div class="setting">
				<div class="label"><% tran("share.srv"); %></div>
				<input name="Add_Service_Name" size="12" maxlength="18" onblur="valid_name(this,share.srv)" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.proto"); %></div>
				<select name="Add_Service_Protocol" onchange="onchange_Prot(Add_Service_Protocol.options[Add_Service_Protocol.selectedIndex].value)" >
					<option value="1">ICMP</option>
					<option value="6">TCP</option>
					<option value="17">UDP</option>
					<option value="23">TCP &amp; UDP</option>
				</select>
			</div>
			<div class="setting">
				<div class="label"><% tran("share.port_range"); %></div>
				<input name="Add_Service_Port_S" size="5" maxlength="5" onblur="valid_range(this,1,65535,share.port_range)" class="num" value="0" /> ~ <input name="Add_Service_Port_E" size="5" maxlength="5" onblur="valid_range(this,1,65535,share.port_range)" class="num" value="0" />
			</div>
				<div class="setting">
					<script type="text/javascript">
					//<![CDATA[
					document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"onclick_Add();\" />");
					document.write("<input class=\"button\" type=\"button\" name=\"mod_button\" value=\"" + sbutton.modify + "\" onclick=\"onclick_Modify();\" />");
					document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.del + "\" onclick=\"onclick_Delete();\" />");
					//]]>
					</script>
				</div>
				<div class="setting">
					<select name="ServiceTable" size="12" onchange="onchange_ServiceTable(ServiceTable.options[ServiceTable.selectedIndex].value,Add_Service_Name, Add_Service_Protocol, Add_Service_Port_S, Add_Service_Port_E)" >
						<script type="text/javascript">
						//<![CDATA[

							var i=0;
							for(i=0;i<services_length;i++)
								document.write("<option value="+i+">"+services[i].name+ " [ "+
								services[i].start+" ~ "+
								services[i].end + " ]" + "</option>");
							
							//]]>
							</script>
					</select>
				</div>
			</div><br />
			<div class="submitFooter">
				<script type="text/javascript">
				//<![CDATA[
				submitFooterButton(1,1);
				//]]>
				</script>
			</div>
		</form>
	</body>
</html>