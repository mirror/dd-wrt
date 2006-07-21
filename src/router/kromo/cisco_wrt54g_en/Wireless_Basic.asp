<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless</title>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + wl_basic.titl;

function SelWL(num,F) {
	if ( num == 0)
		I = '0';
	else
		I = '1';
	wl_enable_disable(F,I);
}

function wl_enable_disable(F,I) {
	if (F.wl_ssid && F.wl0_channel){
		if( I == "0"){
			choose_disable(F.wl_ssid);
			choose_disable(F.wl0_channel);
			<% nvram_match("wl_mode", "ap", "choose_disable(F.wl_closed[0]);"); %>
			<% nvram_match("wl_mode", "ap", "choose_disable(F.wl_closed[1]);"); %>
		} else {
			choose_enable(F.wl_ssid);
			choose_enable(F.wl0_channel);
			<% nvram_match("wl_mode", "ap", "choose_enable(F.wl_closed[0]);"); %>
			<% nvram_match("wl_mode", "ap", "choose_enable(F.wl_closed[1]);"); %>
		}
	}
}

function vifs_add_submit(F,I) {
	F.iface.value = I;
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "add_vifs";
 	F.action.value = "Apply";
	F.submit();
}
function vifs_remove_submit(F,I) {
	F.iface.value = I;
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "remove_vifs";
 	F.action.value = "Apply";
	F.submit();
}

function initArray() {
    this.length = initArray.arguments.length;
    for (var i = 0; i < this.length; i++)
        this[i] = initArray.arguments[i];
}

function from10toradix(value,radix){
    var retval = '';
    var ConvArray = new initArray(0,1,2,3,4,5,6,7,8,9,'A','B','C','D','E','F');
    var intnum;
    var tmpnum;
    var i = 0;

    intnum = parseInt(value,10);
    if (isNaN(intnum)){
        retval = 'NaN';
    }else{
        while (intnum > 0.9){
            i++;
            tmpnum = intnum;
            // cancatinate return string with new digit:
            retval = ConvArray[tmpnum % radix] + retval;  
            intnum = Math.floor(tmpnum / radix);
            if (i > 100){
                // break infinite loops
                retval = 'NaN';
                break;
            }
        }
    }
    return retval;
}

function initWlTimer(hex_radio_on)
{
	tmpvar = parseInt(hex_radio_on,16);
	bin_radio_on = from10toradix(tmpvar,2);
	
	var color_red='#FF0000';
	var color_green='#00FF00';
	
	for(var i = 0; i < bin_radio_on.length; i++){
		if(bin_radio_on.charAt(i)==1){
			bgcolor=color_green;
			val=1;
		}else{
			bgcolor=color_red;
			val=0;
		}
		if(ie4){
			eval("document.all.td_" + i + ".style.backgroundColor = '" + bgcolor + "'");
			eval("document.all.td_" + i + ".value = '" + val + "'");
		}
		if(ns4) {
			eval("document.td_" + i + ".backgroundColor = '" + bgcolor + "'");
			eval("document.td_" + i + ".value = '" + val + "'");
		}
		if(ns6 || op) {
			eval("document.getElementById('td_" + i + "').style.backgroundColor = '" + bgcolor + "'");
			eval("document.getElementById('td_" + i + "').value = '" + val + "'");
		}
	}
}

function setWlTimer(id, state)
{
	var color_red='#FF0000';
	var color_green='#00FF00';
	
	if(id=='all'){
		if(state){
			bgcolor=color_green;
			val=1;
		}else{
			bgcolor=color_red;
			val=0;
		}
			
		for(var i = 0; i < 24; i++) {
			if(ie4){
				eval("document.all.td_" + i + ".style.backgroundColor = '" + bgcolor + "'");
				eval("document.all.td_" + i + ".value = '" + val + "'");
			}
			if(ns4){
				eval("document.td_" + i + ".backgroundColor = '" + bgcolor + "'");
				eval("document.td_" + i + ".value = '" + val + "'");
			}
			if(ns6 || op){
				eval("document.getElementById('td_" + i + "').style.backgroundColor = '" + bgcolor + "'");
				eval("document.getElementById('td_" + i + "').value = '" + val + "'");
			}
		}
	} else {
		if(ie4){
			if(eval("document.all." + id + ".value")==1){
				eval("document.all." + id + ".style.backgroundColor = '" + color_red + "'");
				eval("document.all." + id + ".value = '0'");
			}else{
				eval("document.all." + id + ".style.backgroundColor = '" + color_green + "'");
				eval("document.all." + id + ".value = '1'");
			}
		}
		if(ns4){
			if(eval("document." + id + ".value")==1){
				eval("document." + id + ".backgroundColor = '" + color_red + "'");
				eval("document." + id + ".value = '0'");
			}else{
				eval("document." + id + ".backgroundColor = '" + color_green + "'");
				eval("document." + id + ".value = '1'");
			}
		}
		if(ns6 || op){
			if(eval("document.getElementById('" + id + "').value")==1){
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_red + "'");
				eval("document.getElementById('" + id + "').value = '0'");
			}else{
				eval("document.getElementById('" + id + "').style.backgroundColor = '" + color_green + "'");
				eval("document.getElementById('" + id + "').value = '1'");
			}
		}
	}
}

function computeWlTimer()
{
	var bin_radio_on='';
	
	for(var i = 0; i < 24; i++){
		if(ie4){
			bin_radio_on=bin_radio_on + eval("document.all.td_" + i + ".value");
		}
		if(ns4) {
			bin_radio_on=bin_radio_on + eval("document.td_" + i + ".value");
		}
		if(ns6 || op) {
			bin_radio_on=bin_radio_on + eval("document.getElementById('td_" + i + "').value");
		}
	}
	
	tmpvar = parseInt(bin_radio_on,2);
	hex_radio_on = '0x' + from10toradix(tmpvar,16);
//	alert("bin_radio_on : " + bin_radio_on);
//	alert("hex_radio_on : " + hex_radio_on.toLowerCase());
	return hex_radio_on.toLowerCase();
}

function to_submit(F) {
	if(F.wl_ssid)
		if(F.wl_ssid.value == ""){
			alert(errmsg.err50);
			F.wl_ssid.focus();
			return false;
		}
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	F.radio_on_time.value = computeWlTimer();
	
	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
	wl_enable_disable(document.wireless,'<% nvram_else_match("wl0_gmode","-1","0","1"); %>');
	initWlTimer('<% nvram_get("radio_on_time"); %>');
});

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
          <div id="menu">
            <div id="menuMain">
              <ul id="menuMainList">
                <li><a href="index.asp"><% tran("bmenu.setup"); %></a></li>
                <li class="current"><span><% tran("bmenu.wireless"); %></span>
                  <div id="menuSub">
                      <ul id="menuSubList">
                      <li><span><% tran("bmenu.wirelessBasic"); %></span></li>
                      <li><a href="Wireless_radauth.asp"><% tran("bmenu.wirelessRadius"); %></a></li>
                      <li><a href="WL_WPATable.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
                      <li><a href="Wireless_MAC.asp"><% tran("bmenu.wirelessMac"); %></a></li>
                      <li><a href="Wireless_Advanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
                      <li><a href="Wireless_WDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
                    </ul>
                  </div>
                </li>
                <% nvram_invmatch("sipgate","1","<!--"); %>
                <li><a href="Sipath.asp"><% tran("bmenu.sipath"); %></a></li>
                <% nvram_invmatch("sipgate","1","-->"); %>
                <li><a href="Firewall.asp"><% tran("bmenu.security"); %></a></li>
                <li><a href="Filters.asp"><% tran("bmenu.accrestriction"); %></a></li>
                <li><a href="Forward.asp"><% tran("bmenu.applications"); %></a></li>
                <li><a href="Management.asp"><% tran("bmenu.admin"); %></a></li>
                <li><a href="Status_Router.asp"><% tran("bmenu.statu"); %></a></li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="wireless" action="apply.cgi" method="<% get_http_method(); %>">
		  <input type="hidden" name="submit_button" value="Wireless_Basic" />
		  <input type="hidden" name="submit_type" />
		  <input type="hidden" name="change_action" />
		  <input type="hidden" name="iface" />
		  <input type="hidden" name="radio_on_time" />
		  <input type="hidden" name="action" value="Apply" />
		        <% show_wireless(); %>
		    <br />
		    <div class="submitFooter">
                    <script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");</script>
                    <script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" onclick=\"initWlTimer('<% nvram_get("radio_on_time"); %>')\" />");</script>
                </div>
              </form>
          </div>
        </div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo"><h2><% tran("share.help"); %></h2></div>
						<dl>
							<dt class="term"><% tran("wl_basic.label2"); %>:</dt>
							<dd class="definition"><% tran("hwl_basic.right2"); %></dd>
							<dt class="term"><% tran("hwl_basic.right3"); %></dt>
							<dd class="definition"><% tran("hwl_basic.right4"); %></dt>
							<dt class="term"><% tran("wl_basic.legend2"); %></dt>
							<dd class="definition"><% tran("hwl_basic.right6"); %></dt>
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWireless.asp')"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>