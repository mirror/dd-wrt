<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Advanced Interface Configuration</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
function to_submit(F) {
	F.submit_button.value = "IFConfig";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
 	F.submit();
}

function DelIP(F, IF, IP, ID) {
	F.mode.value = 1;
	F.delif.value = IF;
	F.delid.value = ID;
	F.delip.value = IP;
	F.submit_button.value = "IFConfig";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "delip";
	F.action.value = "Apply";
	F.submit();
}

function AddIP(F) {
	F.mode.value = 0;
	F.submit_button.value = "IFConfig";
	F.change_action.value = "gozila_cgi";
	F.submit_type.value = "delip";
	F.action.value = "Apply";
	F.submit();
}
function SelWAN(F,I) {
	SelMode(F,I,"3");
	choose_disable(eval("F."+I+"_route"));
	choose_disable(eval("F."+I+"_firewall"));
	for(i=0;i<eval("F."+I+"_dhcp").length;i++)
		choose_disable(eval("F."+I+"_dhcp")[i]);

	ifList=F.ifnames.value.split(" ");
	var i=0;
	for(i=0;i<ifList.length;i++) {
		if((ifList[i]==I)||ifList[i]=="none")
			continue;

		choose_enable(eval("F."+ifList[i]+"_route"));
		choose_enable(eval("F."+ifList[i]+"_firewall"));
		for(j=0;j<eval("F."+ifList[i]+"_dhcp").length;j++) {
			choose_enable(eval("F."+ifList[i]+"_dhcp")[j]);
		}
		for(j=0;j<eval("F."+ifList[i]+"_dhcp").length;j++) {
			if(eval("F."+ifList[i]+"_dhcp")[j].checked) {
				SelMode(F,ifList[i],eval("F."+ifList[i]+"_dhcp")[j].value);
			}
		}
	}
}
function SelMode(F,I,n) {
	switch(n) {
		case "0" :
			static_en_dis(F,I,1);
			dhcpd_en_dis(F,I,0);
			dhcpc_en_dis(F,I,0);
			break;
		case "1" :
			static_en_dis(F,I,1);
			dhcpd_en_dis(F,I,1);
			dhcpc_en_dis(F,I,0);
			break;
		case "2" :
			static_en_dis(F,I,0);
			dhcpd_en_dis(F,I,0);
			dhcpc_en_dis(F,I,1);
			break;
		case "3" :
			static_en_dis(F,I,0);
			dhcpd_en_dis(F,I,0);
			dhcpc_en_dis(F,I,0);
			break;
	}
}
function static_en_dis(F,I,t) {
	if(t==0) {
		for(i=1;i<5;i++) {
			choose_disable(eval("F."+I+"_ip"+i));
		}
		for(i=1;i<5;i++) {
			choose_disable(eval("F."+I+"_nm"+i));
		}
	} else {
		for(i=1;i<5;i++) {
			choose_enable(eval("F."+I+"_ip"+i));
		}
		for(i=1;i<5;i++) {
			choose_enable(eval("F."+I+"_nm"+i));
		}
	}
}
function dhcpd_en_dis(F,I,t) {
	if(t==0) {
		for(i=1;i<5;i++)
			choose_disable(eval("F."+I+"_dhcpd_ip"+i));
		choose_disable(eval("F."+I+"_dhcpd_max"));
		choose_disable(eval("F."+I+"_dhcpd_time"));
	} else {
		for(i=1;i<5;i++)
			choose_enable(eval("F."+I+"_dhcpd_ip"+i));
		choose_enable(eval("F."+I+"_dhcpd_max"));
		choose_enable(eval("F."+I+"_dhcpd_time"));
	}
}
function dhcpc_en_dis(F,I,t) {
	if(t==0)
		choose_disable(eval("F."+I+"_fqdn"));
	else
		choose_enable(eval("F."+I+"_fqdn"));
}
function init() {
	for(i=0;i<document.Vlan.is_wan.length;i++)
		if(document.Vlan.is_wan[i].checked)
			SelWAN(document.Vlan,document.Vlan.is_wan[i].value);
}
</script></head>
   <body class="gui" onload="init()"> <% showad(); %>
      <div id="wrapper">
         <div id="content">
            <div id="header">
               <div id="logo">
                  <h1><% show_control(); %></h1>
               </div>
               <div id="menu">
                  <div id="menuMain">
                     <ul id="menuMainList">
                        <li class="current"><a class="current" href="index.asp">Setup</a><div id="menuSub">
                              <ul id="menuSubList">
                                 <li><a href="index.asp">Basic Setup</a></li>
                                 <li><a href="DDNS.asp">DDNS</a></li>
                                 <li><a href="WanMAC.asp">MAC Address Clone</a></li>
                                 <li><a href="Routing.asp">Advanced Routing</a></li><% support_invmatch("HSIAB_SUPPORT", "1", "<!--"); %>
                                 <li><a href="HotSpot_Admin.asp">Hot Spot</a></li><% support_invmatch("HSIAB_SUPPORT", "1", "-->"); %>
                                 <li><a href="Vlan.asp">VLANs</a></li>
                                 <li><span>Interfaces</span></li>
                              </ul>
                           </div>
                        </li>
                        <li><a href="Wireless_Basic.asp">Wireless</a></li>
                        <% nvram_invmatch("sipgate","1","<!--"); %>
			<li><a href="Sipath.asp">SIPatH</a></li>
                        <% nvram_invmatch("sipgate","1","-->"); %>
			<li><a href="Firewall.asp">Security</a></li>
                        <li><a href="Filters.asp">Access Restrictions</a></li>
                        <li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
                        <li><a href="Management.asp">Administration</a></li>
                        <li><a href="Status_Router.asp">Status</a></li>
                     </ul>
                  </div>
               </div>
            </div>
            <div id="main">
               <div id="contents">
                  <form name="Vlan" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="submit_type" /><input type="hidden" name="change_action" /><input type="hidden" name="action" /><input type="hidden" name="delif" /><input type="hidden" name="delid" /><input type="hidden" name="delip" /><input type="hidden" name="mode" /><div><% if_config_table(); %></div><br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick=to_submit(this.form) /><input type="reset" value="Cancel Changes" /></div>
                  </form>
               </div>
            </div>
            <div id="helpContainer">
               <div id="help">
                  <div id="logo">
                     <h2>Help</h2>
                  </div><br />
                  <a href="javascript:openHelpWindow('HIFConfig.asp')">More...</a>
               </div>
            </div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><script type="text/javascript">Capture(share.firmware)</script>: <script type="text/javascript">document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info">Time: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>