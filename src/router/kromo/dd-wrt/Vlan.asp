<% do_pagehead("vlan.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.save_button.value = sbutton.saving;
	applytake(F);
}

function vlan_add(F, I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "portvlan_add";
	apply(F);
}

function vlan_remove(F, I) {
	F.change_action.value="gozila_cgi";
	F.submit_type.value = "portvlan_remove";
	F.del_value.value=I;
	apply(F);
}

function SelSpeed(F,I) {
	var vlancount = <% nvg("portvlan_count"); %>;

	if(eval("F."+I+"vlan21000") && eval("F."+I+"vlan21000").checked==false) {
		for(i=0;i<vlancount;i++) {
			if (eval("F."+I+"vlan"+i)) {
				choose_disable(eval("F."+I+"vlan"+i));
			}
		}
		choose_disable(eval("F."+I+"vlan16000"));
		choose_disable(eval("F."+I+"vlan17000"));
		choose_disable(eval("F."+I+"vlan18000"));
		choose_disable(eval("F."+I+"vlan19000"));		
	} else {
		for(i=0;i<vlancount;i++) {
			if (eval("F."+I+"vlan"+i)) {
				choose_enable(eval("F."+I+"vlan"+i));
			}
		}
		choose_enable(eval("F."+I+"vlan16000"));
		choose_enable(eval("F."+I+"vlan17000"));
		choose_enable(eval("F."+I+"vlan18000"));
		choose_enable(eval("F."+I+"vlan19000"));

		SelVLAN(F,I);
		if (eval("F."+I+"vlan17000")) {
			if(eval("F."+I+"vlan17000").checked) {
				if (eval("F."+I+"vlan18000")) {
					eval("F."+I+"vlan18000").checked=true;
					choose_disable(eval("F."+I+"vlan18000"));
				}
				if (eval("F."+I+"vlan19000")) {
					eval("F."+I+"vlan19000").checked=true;
					choose_disable(eval("F."+I+"vlan19000"));
				}
				if (eval("F."+I+"vlan20000")) {
					eval("F."+I+"vlan20000").checked=true;
					choose_disable(eval("F."+I+"vlan20000"));
				}
			} else {
				if (eval("F."+I+"vlan18000"))
					choose_enable(eval("F."+I+"vlan18000"));

				if (eval("F."+I+"vlan19000"))
					choose_enable(eval("F."+I+"vlan19000"));

				if (eval("F."+I+"vlan20000"))
					choose_enable(eval("F."+I+"vlan20000"));
			}
			if (eval("F."+I+"vlan19000")) {
				if(eval("F."+I+"vlan18000").checked) {
					eval("F."+I+"vlan19000").checked=true;
					choose_disable(eval("F."+I+"vlan19000"));
				} else {
					choose_enable(eval("F."+I+"vlan19000"));
				}
			}
		}
	}
}

function SelVLAN(F,I) {
	var i,j=0;
	var vlancount = <% nvg("portvlan_count"); %>;
	if(!eval("F."+I+"vlan21000") || eval("F."+I+"vlan21000").checked == true) {
		if(eval("F."+I+"vlan16000") && eval("F."+I+"vlan16000").checked == true) {
			for(i=0;i<vlancount;i++) {
				if(eval("F."+I+"vlan"+i)) {
					choose_enable(eval("F."+I+"vlan"+i));
				}
			}
		} else {
			for(i=0;i<vlancount;i++) {
				if(eval("F."+I+"vlan"+i)) {
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
			}
			if(j==1) {
				for(i=0;i<vlancount;i++) {
					if(eval("F."+I+"vlan"+i)) {
						if(!(eval("F."+I+"vlan"+i).checked)) {
							choose_disable(eval("F."+I+"vlan"+i));
						} else {
							break;
						}
					}
				}
			}
		}
	}
}

function init() {
	for(var k=0; k<5; k++) {
		SelSpeed(document.vlan, "port"+k);
		SelVLAN(document.vlan, "port"+k);
	}
}

var update;

addEvent(window, "load", function() {
	update = new StatusbarUpdate();
	update.start();
});

addEvent(window, "unload", function() {
	update.stop();
});
	
		//]]>
		</script>
	</head>

	<body class="gui" onload="init()">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","Vlan.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="vlan" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="Vlan" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1" />
							<input type="hidden" name="del_value" />

							<h2><% tran("vlan.h2"); %></h2>
							<fieldset>
								<legend><% tran("vlan.legend"); %></legend>
								<table class="table vlan" summary="virtual lan table">
									<tbody>
										<% port_vlan_table(); %>
									</tbody>
							 </table>
						 </fieldset><br/>
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
						<br />
						<!-- <a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HVlan.asp');"><% tran("share.more"); %></a> -->
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
