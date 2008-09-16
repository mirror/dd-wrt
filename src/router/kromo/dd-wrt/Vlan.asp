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

function SelSpeed(F,I) {
	if(eval("F."+I+"vlan20").checked==false) {
		for(i=0;i<20;i++) {
			choose_disable(eval("F."+I+"vlan"+i));
		}
	} else {
		for(i=0;i<20;i++) {
			choose_enable(eval("F."+I+"vlan"+i));
		}
	    SelVLAN(F,I);
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
}

function SelVLAN(F,I) {
	var i,j=0;
	if(eval("F."+I+"vlan20").checked == true) {
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
		<% showad(); %>
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