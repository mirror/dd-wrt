<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
	<title><% nvram_get("router_name"); %> - Advanced Interface Configuration</title>
	<link type="text/css" rel="stylesheet" href="style.css"/>
	<script type="text/JavaScript" src="common.js">{}</script>
	<script language="JavaScript">
	
function to_submit(F) {
	F.submit_button.value = "Vlan";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;
	F.submit();
}

function SelSpeed(F,I) {
	return;

	if(eval("F."+I+"vlan17").checked) {
		eval("F."+I+"vlan18").checked=true;
		eval("F."+I+"vlan19").checked=true;
		choose_disable(eval("F."+I+"vlan18"));
		choose_disable(eval("F."+I+"vlan19"));
	} else {
		choose_enable(eval("F."+I+"vlan18"));
		choose_enable(eval("F."+I+"vlan19"));
	}
}

function SelVLAN(F,I) {
	var j=0;
	if(eval("F."+I+"vlan16").checked) {
		for(i=0;i<16;i++)
			choose_enable(eval("F."+I+"vlan"+i));
	} else {
		for(i=0;i<16;i++) {
			if(j==1) {
				eval("F."+I+"vlan"+i).checked=false;
				choose_disable(eval("F."+I+"vlan"+i));
			} else {
				choose_enable(eval("F."+I+"vlan"+i));
			}
			if(eval("F."+I+"vlan"+i).checked == true) {
				j=1;
			}
		}
		if(j==1) {
			for(i=0;i<16;i++) {
				if(!(eval("F."+I+"vlan"+i).checked)) {
					choose_disable(eval("F."+I+"vlan"+i));
				} else {
					break;
				}
			}
		}
	}
}

function init() {
	for(k=0;k<5;k++) {
		SelSpeed(document.Vlan, "port"+k);
		SelVLAN(document.Vlan, "port"+k);
	}
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
									<li class="current"><span>Setup</span>
										<div id="menuSub">
											<ul id="menuSubList">
												<li><a href="index.asp">Basic Setup</a></li>
												<li><a href="DDNS.asp">DDNS</a></li>
												<li><a href="WanMAC.asp">MAC Address Clone</a></li>
												<li><a href="Routing.asp">Advanced Routing</a></li>
												<% support_invmatch("HSIAB_SUPPORT", "1", "<!--"); %>
												<li><a href="HotSpot_Admin.asp">Hot Spot</a></li>
												<% support_invmatch("HSIAB_SUPPORT", "1", "-->"); %>
												<li><span>VLANs</span></li>
											</ul>
										</div>
									</li>
									<li><a href="Wireless_Basic.asp">Wireless</a></li>
									<% nvram_invmatch("sipgate","1","<!--"); %>
									<li><a href="Sipath.asp">SIPatH</a></li>
									<% nvram_invmatch("sipgate","1","-->"); %>
									<li><a href="Firewall.asp">Security</a></li>
									<li><a href='<% support_elsematch("PARENTAL_CONTROL_SUPPORT", "1", "Parental_Control.asp", "Filters.asp"); %>'>Access Restrictions</a></li>
									<li><a href="Forward.asp">Applications&nbsp;&amp;&nbsp;Gaming</a></li>
									<li><a href="Management.asp">Administration</a></li>
									<li><a href="Status_Router.asp">Status</a></li>
								</ul>
							</div>
						</div>
					</div>
					<div id="main">
					<div id="contents">
						<form name="static" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Vlan"/>
							<input type="hidden" name="action" value="Apply"/>
							<input type="hidden" name="commit" value="1"/>
							<h2>Virtual Local Area Network (VLAN)</h2>
							<fieldset>
							   <legend>VLAN</legend>
							   <table id="vlan" class="table center">
  								<tbody>
  									<tr>
  										<th rowspan="2">VLAN</th>
  										<th colspan="5">Port</th>
  										<th rowspan="2">Assigned To<br />Bridge</th>
  									</tr>
  									<tr>
  										<th>W</th>
  										<th>1</th>
  										<th>2</th>
  										<th>3</th>
  										<th>4</th>
  									</tr>
  									<% port_vlan_table(); %>
  								</tbody>
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
						<br/>
						<!--<a target="_blank" href="help/HVlan.asp">More...</a>-->
					</div>
				</div>
			</div>
		</div>
	</body>
</html>