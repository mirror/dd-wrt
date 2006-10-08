<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Virtual LAN</title>
		<script type="text/javascript"><![CDATA[

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
		]]></script>
	</head>

	<body class="gui" onload="init()">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","Vlan.asp"); %>
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
							   <table class="table center vlan" summary="virtual lan table">
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
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");
]]></script>
								<script type="text/javascript"><![CDATA[
document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />");
]]></script>
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
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>