<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Virtual LAN</title>
		<script type="text/javascript">

document.title = "<% nvram_get("router_name"); %>" + vlan.titl;

function to_submit(F) {
	F.submit_button.value = "Vlan";
	F.save_button.value = sbutton.saving;
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
								<li class="current"><span><% tran("bmenu.setup"); %></span>
									<div id="menuSub">
										<ul id="menuSubList">
	  										<li><a href="index.asp"><% tran("bmenu.setupbasic"); %></a></li>
											<li><a href="DDNS.asp"><% tran("bmenu.setupddns"); %></a></li>
											<li><a href="WanMAC.asp"><% tran("bmenu.setupmacclone"); %></a></li>
	  										<li><a href="Routing.asp"><% tran("bmenu.setuprouting"); %></a></li>
	  										<li><span><% tran("bmenu.setupvlan"); %></span></li>
  										</ul>
  									</div>
  								</li>
  								<li><a href="Wireless_Basic.asp"><% tran("bmenu.wireless"); %></a></li>
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
						<form name="vlan" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="Vlan" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="commit" value="1" />
							<h2><% tran("vlan.h2"); %></h2>
							<fieldset>
							   <legend><% tran("vlan.legend"); %></legend>
							   <table id="vlan" class="table center vlan">
  								<tbody>
  									<tr>
  										<th rowspan="2"><% tran("vlan.legend"); %></th>
  										<th colspan="5"><% tran("share.port"); %></th>
  										<th rowspan="2"><% tran("vlan.bridge"); %></th>
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
						<div><h2><% tran("share.help"); %></h2></div>
						<br />
						<!-- <a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HVlan.asp');"><% tran("share.more"); %></a> -->
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