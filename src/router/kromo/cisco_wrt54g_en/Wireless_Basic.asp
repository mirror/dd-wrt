<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Wireless</title>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + wl_basic.titl;

var wl_channel = '<% nvram_get("wl0_channel"); %>';
var wl_nctrlsb = '<% nvram_get("wl0_nctrlsb"); %>';
var wl_nbw = '<% nvram_get("wl0_nbw"); %>';
var wl_phytype = '<% nvram_get("wl0_phytype"); %>';
var wl_40m_disable = '<% nvram_get("wl0_40m_disable"); %>';


function SelWL(num,F) {
  if ( num == 0)
    I = "0";
  else
    I = "1";
  wl_enable_disable(F,I);
}

function create_wchannel_auto(F)
{
	F.wl0_wchannel.length = 1;
	
	F.wl0_wchannel[0] = new Option(share.auto);
	F.wl0_wchannel[0].value = "0";

}

function create_wchannel(F)
{
	var max_channel = '14';
	var wch;

	if(wl_nctrlsb == "lower") {
		wch = parseInt(wl_channel)+2;
	}
	else {
		wch = parseInt(wl_channel)-2;
	}

	F.wl0_wchannel.length = parseInt(max_channel)-4;

	for(ch=3 ; ch<=(parseInt(max_channel)-2) ; ch++){
		F.wl0_wchannel[ch-3] = new Option(ch);
		F.wl0_wchannel[ch-3].value = ch;
	}
	if(wch < 3 || wch > max_channel-2 || wch == "0")
		F.wl0_wchannel[0].selected = true;
	else
		F.wl0_wchannel[wch-3].selected = true;	
}

function InitBW(num,F)
{
	if(wl_channel == "0") {
		if(F.wl0_wchannel) choose_enable(F.wl0_wchannel);
		choose_enable(F.wl_schannel);

		if(F.wl0_wchannel) create_wchannel_auto(F)
	
	}
	else
		SelBW(num,F);
}

function SelBW(num,F)
{
	if (num == 0) {	// Auto
		if(F.wl0_wchannel) choose_enable(F.wl0_wchannel);
		choose_enable(F.wl0_channel);

		if(F.wl0_wchannel) create_wchannel_auto(F)
	
//		create_schannel_auto(F);
	}
	else if (num == 20){
		if(F.wl0_wchannel) choose_disable(F.wl0_wchannel);
		choose_enable(F.wl0_schannel);
		
		if(F.wl0_wchannel) create_wchannel(F)
	
//		create_schannel(F);
	}
	else {
		if(F.wl0_wchannel) choose_enable(F.wl0_wchannel);
		choose_enable(F.wl0_schannel);
		
		if(F.wl0_wchannel) create_wchannel(F);
		
		var curvalue = document.forms[0].wl0_wchannel[document.forms[0].wl0_wchannel.selectedIndex].value;

//		create_schannel2(curvalue,F);
	}
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

function to_submit(F) {
	if(F.wl_ssid)
		if(F.wl_ssid.value == ""){
			alert(errmsg.err50);
			F.wl_ssid.focus();
			return false;
		}
	if(F.wl0_nbw)
	{
	if(F.wl0_nbw.value == 0) { // Auto
		F.wl0_channel.value = 0;
	}
	else if(F.wl0_nbw.value == 20) { // 20MHz
		F.wl0_nctrlsb.value = "none";
		F.wl0_nbw.value = 20;
	}
	else { // 40MHz
		if(F.wl0_channel.selectedIndex == 0) {
			F.wl0_nctrlsb.value = "lower";
		}
		else {
			F.wl0_nctrlsb.value = "upper";
		}
		F.wl0_nbw.value = 40;
	}
	}
	F.change_action.value = "gozila_cgi";
	F.submit_button.value = "Wireless_Basic";
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
	wl_enable_disable(document.wireless,'<% nvram_else_match("wl0_gmode","-1","0","1"); %>');
	var wl_mode = "<% nvram_get("wl0_mode"); %>";
        if (wl_mode=="ap" || wl_mode=="infra")
	{
	    if (wl_phytype == 'n')
		InitBW('<% nvram_get("wl0_nbw"); %>' ,document.wireless);
	}
	var wl_net_mode = "<% nvram_get("wl0_net_mode"); %>";
	SelWL(wl_net_mode,document.wireless);
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
                  	<input type="hidden" name="wl0_nctrlsb" />
                  	<input type="hidden" name="iface" />
                  	<input type="hidden" name="action" value="Apply" />
                  	
                  	<% show_wireless(); %>
                  	
                  	<br />
                  	<div class="submitFooter">
                  		<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");</script>
                  		<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />");</script>
                  	</div>
                  </form>
                </div>
              </div>
              <div id="helpContainer">
              	<div id="help">
              		<div><h2><% tran("share.help"); %></h2></div>
              		<dl>
              			<dt class="term"><% tran("wl_basic.label2"); %>:</dt>
              			<dd class="definition"><% tran("hwl_basic.right2"); %></dd>
              			<dt class="term"><% tran("hwl_basic.right3"); %></dt>
              			<dd class="definition"><% tran("hwl_basic.right4"); %></dt>
              		</dl><br />
             		<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HWireless.asp')"><% tran("share.more"); %></a>
             	</div>
            </div>
						<div id="floatKiller"></div>
						<div id="statusInfo">
							<div class="info"><% tran("share.firmware"); %>: <script type="text/javascript">document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
							<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
							<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
						</div>
				</div>
		</div>
	</body>
</html>