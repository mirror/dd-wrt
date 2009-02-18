<% do_pagehead("wl_basic.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

var wl0_channel = '<% nvram_get("wl0_channel"); %>';
var wl0_nctrlsb = '<% nvram_get("wl0_nctrlsb"); %>';
var wl0_nbw = '<% nvram_get("wl0_nbw"); %>';
var wl0_phytype = '<% nvram_get("wl0_phytype"); %>';
var wl0_40m_disable = '<% nvram_get("wl0_40m_disable"); %>';
var wl1_channel = '<% nvram_get("wl1_channel"); %>';
var wl1_nctrlsb = '<% nvram_get("wl1_nctrlsb"); %>';
var wl1_nbw = '<% nvram_get("wl1_nbw"); %>';
var wl1_phytype = '<% nvram_get("wl1_phytype"); %>';
var wl1_40m_disable = '<% nvram_get("wl1_40m_disable"); %>';

function create_wchannel0_auto(F) {
	F.wl0_wchannel.length = 1;
	
	F.wl0_wchannel[0] = new Option(share.auto);
	F.wl0_wchannel[0].value = "0";
}

function create_wchannel1_auto(F) {
	F.wl1_wchannel.length = 1;
	
	F.wl1_wchannel[0] = new Option(share.auto);
	F.wl1_wchannel[0].value = "0";
}

function create_wchannel0(F) {
	var max_channel = '14';
	var wch;

	if(wl0_nctrlsb == "lower") {
		wch = parseInt(F.wl0_channel.value)+2;
	}
	else {
		wch = parseInt(F.wl0_channel.value)-2;
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

function create_wchannel1(F) {
	var max_channel = '14';
	var wch;

	if(wl1_nctrlsb == "lower") {
		wch = parseInt(F.wl1_channel.value)+2;
	}
	else {
		wch = parseInt(F.wl1_channel.value)-2;
	}

	F.wl1_wchannel.length = parseInt(max_channel)-4;

	for(ch=3 ; ch<=(parseInt(max_channel)-2) ; ch++){
		F.wl1_wchannel[ch-3] = new Option(ch);
		F.wl1_wchannel[ch-3].value = ch;
	}
	if(wch < 3 || wch > max_channel-2 || wch == "0")
		F.wl1_wchannel[0].selected = true;
	else
		F.wl1_wchannel[wch-3].selected = true;	
}

function InitBW0(num,F) {
	if(wl0_channel == "0") {
		if(F.wl0_wchannel) choose_enable(F.wl0_wchannel);
		choose_enable(F.wl0_schannel);

		if(F.wl0_wchannel) create_wchannel0_auto(F)
	
	}
	else
		SelBW0(num,F);
}

function InitBW1(num,F) {
	if(wl1_channel == "0") {
		if(F.wl1_wchannel) choose_enable(F.wl1_wchannel);
		choose_enable(F.wl1_schannel);

		if(F.wl1_wchannel) create_wchannel1_auto(F)
	
	}
	else
		SelBW1(num,F);
}

function SelBW0(num,F) {
	if (num == 0) {	// Auto
		if(F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);
			
		choose_enable(F.wl0_channel);
		if(F.wl0_wchannel)
			create_wchannel0_auto(F);
	}
	else if (num == 10 || num == 20) {
		if(F.wl0_wchannel)
			choose_disable(F.wl0_wchannel);
		
		choose_enable(F.wl0_schannel);
		if(F.wl0_wchannel)
			create_wchannel0(F);
	}
	else {
		if(F.wl0_wchannel)
			choose_enable(F.wl0_wchannel);
		
		choose_enable(F.wl0_schannel);
		if(F.wl0_wchannel)
			create_wchannel0(F);
	}
}

function SelBW1(num,F) {
	if (num == 0) {	// Auto
		if(F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);
			
		choose_enable(F.wl1_channel);
		if(F.wl1_wchannel)
			create_wchannel1_auto(F);
	}
	else if (num == 10 || num == 20) {
		if(F.wl1_wchannel)
			choose_disable(F.wl1_wchannel);
		
		choose_enable(F.wl1_schannel);
		if(F.wl1_wchannel)
			create_wchannel1(F);
	}
	else {
		if(F.wl1_wchannel)
			choose_enable(F.wl1_wchannel);
		
		choose_enable(F.wl1_schannel);
		if(F.wl1_wchannel)
			create_wchannel1(F);
	}
}




function vifs_add_submit(F,I) {
	F.iface.value = I;
	F.submit_type.value = "add_vifs";
	F.submit();
}

function vifs_remove_submit(F,I) {
	F.iface.value = I;
	F.submit_type.value = "remove_vifs";
	F.submit();
}

function submitcheck(F) {
	if(F.wl0_ssid)
		if(F.wl0_ssid.value == ""){
			alert(errmsg.err50);
			F.wl0_ssid.focus();
			return false;
		}
	if(F.wl1_ssid)
		if(F.wl1_ssid.value == ""){
			alert(errmsg.err50);
			F.wl1_ssid.focus();
			return false;
		}
	if(F.wl0_nbw)
	{
	if(F.wl0_nbw.value == 0) { // Auto
		F.wl0_channel.value = 0;
	}
	else if(F.wl0_nbw.value == 10) { // 10MHz
		F.wl0_nctrlsb.value = "none";
		F.wl0_nbw.value = 10;
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
	if(F.wl1_nbw)
	{
	if(F.wl1_nbw.value == 0) { // Auto
		F.wl1_channel.value = 0;
	}
	else if(F.wl1_nbw.value == 10) { // 10MHz
		F.wl1_nctrlsb.value = "none";
		F.wl1_nbw.value = 10;
	}
	else if(F.wl1_nbw.value == 20) { // 20MHz
		F.wl1_nctrlsb.value = "none";
		F.wl1_nbw.value = 20;
	}
	else { // 40MHz
		if(F.wl1_channel.selectedIndex == 0) {
			F.wl1_nctrlsb.value = "lower";
		}
		else {
			F.wl1_nctrlsb.value = "upper";
		}
		F.wl1_nbw.value = 40;
	}
	}
	F.submit_type.value = "save";
	F.save_button.value = sbutton.saving;
	return true;
}


function to_submit(F)
{
if (submitcheck(F))
    apply(F);
}
function to_apply(F)
{
if (submitcheck(F))
    applytake(F);
}


var update;

addEvent(window, "load", function() {

	var wl0_mode = "<% nvram_get("wl0_mode"); %>";
	   if (wl0_mode=="ap" || wl0_mode=="infra")
	{
	    if (wl0_phytype == 'n')
		InitBW0('<% nvram_get("wl0_nbw"); %>' ,document.wireless);
	}
	var wl1_mode = "<% nvram_get("wl1_mode"); %>";
	   if (wl1_mode=="ap" || wl1_mode=="infra")
	{
	    if (wl1_phytype == 'n')
		InitBW1('<% nvram_get("wl1_nbw"); %>' ,document.wireless);
	}
		
	update = new StatusbarUpdate();
	update.start();
	
});

addEvent(window, "unload", function() {
	update.stop();

});

		//]]>
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
			<% do_menu("Wireless_Basic.asp","Wireless_Basic.asp"); %>
		  </div>
		  <div id="main">
			<div id="contents">
			   <form name="wireless" action="apply.cgi" method="post">
				  	<input type="hidden" name="submit_button" value="Wireless_Basic" />
				  	<input type="hidden" name="action" value="Apply" />
				  	<input type="hidden" name="change_action" value="gozila_cgi" />
				  	<input type="hidden" name="submit_type" value="save" />

				  	<input type="hidden" name="wl0_nctrlsb" />
				  	<input type="hidden" name="wl1_nctrlsb" /> 
				  	<input type="hidden" name="iface" />
				  	
				  	<% show_wireless(); %>
				  	
				  	<br />
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
					<dl>
						<dt class="term"><% tran("wl_basic.label2"); %>:</dt>
						<dd class="definition"><% tran("hwl_basic.right2"); %></dd>
<% ifndef("ACK", "<!--"); %>
						<dt class="term"><% tran("hwl_basic.right3"); %></dt>
						<dd class="definition"><% tran("hwl_basic.right4"); %></dd>
<% ifndef("ACK", "-->"); %>
					</dl><br />
					<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HWireless.asp')"><% tran("share.more"); %></a>
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