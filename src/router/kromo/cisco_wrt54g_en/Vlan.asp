<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Virtual LAN</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + vlan.titl;

function to_submit(F) {
	F.submit_button.value = "Vlan";
	F.save_button.value = sbutton.saving;

	F.action.value = "Apply";
	apply(F);
}

function SelSpeed(F,I) {
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
	if(eval("F."+I+"vlan16").checked == true) {
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
		SelSpeed(document.vlan, "port"+k);
		SelVLAN(document.vlan, "port"+k);
	}
}
</script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li class="current"><span><script type="text/javascript">Capture(bmenu.setup)</script></span>
									<div id="menuSub">
										<ul id="menuSubList">
	  										<li><a href="index.asp"><script type="text/javascript">Capture(bmenu.setupbasic)</script></a></li>
											<li><a href="DDNS.asp"><script type="text/javascript">Capture(bmenu.setupddns)</script></a></li>
											<li><a href="WanMAC.asp"><script type="text/javascript">Capture(bmenu.setupmacclone)</script></a></li>
	  										<li><a href="Routing.asp"><script type="text/javascript">Capture(bmenu.setuprouting)</script></a></li>
	  										<li><span><script type="text/javascript">Capture(bmenu.setupvlan)</script></span></li>
  										</ul>
  									</div>
  								</li>
  								<li><a href="Wireless_Basic.asp"><script type="text/javascript">Capture(bmenu.wireless)</script></a></li>
								<% nvram_invmatch("sipgate","1","<!--"); %>
								<li><a href="Sipath.asp"><script type="text/javascript">Capture(bmenu.sipath)</script></a></li>
								<% nvram_invmatch("sipgate","1","-->"); %>
								<li><a href="Firewall.asp"><script type="text/javascript">Capture(bmenu.security)</script></a></li>
								<li><a href="Filters.asp"><script type="text/javascript">Capture(bmenu.accrestriction)</script></a></li>
								<li><a href="Forward.asp"><script type="text/javascript">Capture(bmenu.applications)</script></a></li>
								<li><a href="Management.asp"><script type="text/javascript">Capture(bmenu.admin)</script></a></li>
								<li><a href="Status_Router.asp"><script type="text/javascript">Capture(bmenu.statu)</script></a></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<div id="contents">
						<form name="vlan" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Vlan" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="commit" value="1" />
							<h2><script type="text/javascript">Capture(vlan.h2)</script></h2>
							<fieldset>
							   <legend><script type="text/javascript">Capture(vlan.legend)</script></legend>
							   <table id="vlan" class="table center vlan">
  								<tbody>
  									<tr>
  										<th rowspan="2"><script type="text/javascript">Capture(vlan.legend)</script></th>
  										<th colspan="5"><script type="text/javascript">Capture(share.port)</script></th>
  										<th rowspan="2"><script type="text/javascript">Capture(vlan.bridge)</script></th>
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
								<script type="text/javascript">document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />")</script>
								<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />")</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div id="logo">
							<h2><script type="text/javascript">Capture(share.help)</script></h2>
						</div>
						<br />
						<!-- <a href="javascript:openHelpWindow('HVlan.asp');"><script type="text/javascript">Capture(share.more)</script></a> -->
					</div>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info">Firmware: <script>document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");</script></div>
					<div class="info"><script type="text/javascript">Capture(share.time)</script>: <% get_uptime(); %></div>
					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
				</div>
			</div>
		</div>
	</body>
</html>