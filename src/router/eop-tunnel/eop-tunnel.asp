<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - EOP Tunnel</title>
		<script type="text/javascript">
//<![CDATA[
document.title = "<% nvram_get("router_name"); %> EOP Tunnel";

function to_submit(F) {
	F.submit_button.value = "EOP";
//	F.save_button.value = "Saved";
	F.save_button.value = sbutton.saving;
	
	F.action.value = "Apply";
	apply(F);
}

addEvent(window, "load", function() {
		show_layer_ext(document.eop.oet1_en, 'idoet1', <% nvram_else_match("oet1_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet1_bridged, 'idbridged1', <% nvram_else_match("oet1_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet2_en, 'idoet2', <% nvram_else_match("oet2_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet2_bridged, 'idbridged2', <% nvram_else_match("oet2_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet3_en, 'idoet3', <% nvram_else_match("oet3_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet3_bridged, 'idbridged3', <% nvram_else_match("oet3_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet4_en, 'idoet4', <% nvram_else_match("oet4_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet4_bridged, 'idbridged4', <% nvram_else_match("oet4_bridged", "1", "0", "1"); %> == 1);
		show_layer_ext(document.eop.oet5_en, 'idoet5', <% nvram_else_match("oet5_en", "1", "1", "0"); %> == 1);
		show_layer_ext(document.eop.oet5_bridged, 'idbridged5', <% nvram_else_match("oet5_bridged", "1", "0", "1"); %> == 1);
});
		//]]>
		</script>

	</head>

	<body class="gui">
		<% showad(); %>
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("index.asp","eop-tunnel.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="eop" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" />
							<input type="hidden" name="action" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />

	<h2><% tran("service.eoip_legend"); %></h2>
<fieldset>
	<legend>Tunnel 1</legend>

		<div class="setting">
			<div class="label"><% tran("service.eoip_srv"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet1_en" <% nvram_selmatch("oet1_en","1","checked"); %> onclick="show_layer_ext(this, 'idoet1', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet1_en" <% nvram_selmatch("oet1_en","0","checked"); %> onclick="show_layer_ext(this, 'idoet1', false)" /><% tran("share.disable"); %>
		</div>
	<div id="idoet1">
		<div class="setting">
			<div class="label"><% tran("service.eoip_remoteIP"); %></div>
			<input type="hidden" name="oet1_rem" value="0.0.0.0"/>
			<input size="3" maxlength="3" class="num" name="oet1_rem_0" onblur="valid_range(this,0,255,service.eoip_remoteIP)" value="<% get_single_ip("oet1_rem","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_rem_1" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet1_rem","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_rem_2" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet1_rem","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_rem_3" onblur="valid_range(this,1,254,service.eoip_tunnelID)" value="<% get_single_ip("oet1_rem","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_tunnelID"); %></div>
			<input size="4" maxlength="3" class="num" name="oet1_id" onblur="valid_range(this,0,999,service.eoip_tunnelID)" value="<% nvram_get("oet1_id"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_comp"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet1_comp" <% nvram_selmatch("oet1_comp","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet1_comp" <% nvram_selmatch("oet1_comp","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_passtos"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet1_pt" <% nvram_selmatch("oet1_pt","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet1_pt" <% nvram_selmatch("oet1_pt","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_frag"); %></div>
			<input size="4" maxlength="4" class="num" name="oet1_fragment" onblur="valid_range(this,0,1500,service.eoip_frag)" value="<% nvram_get("oet1_fragment"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_mssfix"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet1_mssfix" <% nvram_selmatch("oet1_mssfix","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet1_mssfix" <% nvram_selmatch("oet1_mssfix","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_shaper"); %></div>
			<input size="6" maxlength="6" class="num" name="oet1_shaper" onblur="valid_range(this,0,100000,service.eoip_shaper)" value="<% nvram_get("oet1_shaper"); %>" />
		</div>	
		<div class="setting">
			<div class="label"><% tran("service.eoip_bridging"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet1_bridged" <% nvram_selmatch("oet1_bridged","1","checked"); %> onclick="show_layer_ext(this, 'idbridged1', false)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet1_bridged" <% nvram_selmatch("oet1_bridged","0","checked"); %> onclick="show_layer_ext(this, 'idbridged1', true)" /><% tran("share.disable"); %>
		</div>
		<div id="idbridged1">
			<div class="setting">
				<div class="label"><% tran("share.ip"); %></div>
				<input type="hidden" name="oet1_ip" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet1_ip_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet1_ip","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_ip_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet1_ip","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_ip_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet1_ip","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_ip_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("oet1_ip","3"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.subnet"); %></div>
				<input type="hidden" name="oet1_netmask" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet1_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet1_netmask","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet1_netmask","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet1_netmask","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet1_netmask_3" onblur="valid_range(this,0,254,share.subnet)" value="<% get_single_ip("oet1_netmask","3"); %>" />
			</div>
		</div>
	</div>
</fieldset><br/>
<fieldset>
	<legend>Tunnel 2</legend>

		<div class="setting">
			<div class="label"><% tran("service.eoip_srv"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet2_en" <% nvram_selmatch("oet2_en","1","checked"); %> onclick="show_layer_ext(this, 'idoet2', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet2_en" <% nvram_selmatch("oet2_en","0","checked"); %> onclick="show_layer_ext(this, 'idoet2', false)" /><% tran("share.disable"); %>
		</div>
	<div id="idoet2">
		<div class="setting">
			<div class="label"><% tran("service.eoip_remoteIP"); %></div>
			<input type="hidden" name="oet2_rem" value="0.0.0.0"/>
			<input size="3" maxlength="3" class="num" name="oet2_rem_0" onblur="valid_range(this,0,255,service.eoip_remoteIP)" value="<% get_single_ip("oet2_rem","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_rem_1" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet2_rem","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_rem_2" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet2_rem","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_rem_3" onblur="valid_range(this,1,254,service.eoip_tunnelID)" value="<% get_single_ip("oet2_rem","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_tunnelID"); %></div>
			<input size="4" maxlength="3" class="num" name="oet2_id" onblur="valid_range(this,0,999,service.eoip_tunnelID)" value="<% nvram_get("oet2_id"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_comp"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet2_comp" <% nvram_selmatch("oet2_comp","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet2_comp" <% nvram_selmatch("oet2_comp","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_passtos"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet2_pt" <% nvram_selmatch("oet2_pt","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet2_pt" <% nvram_selmatch("oet2_pt","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_frag"); %></div>
			<input size="4" maxlength="4" class="num" name="oet2_fragment" onblur="valid_range(this,0,1500,service.eoip_frag)" value="<% nvram_get("oet2_fragment"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_mssfix"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet2_mssfix" <% nvram_selmatch("oet2_mssfix","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet2_mssfix" <% nvram_selmatch("oet2_mssfix","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_shaper"); %></div>
			<input size="6" maxlength="6" class="num" name="oet2_shaper" onblur="valid_range(this,0,100000,service.eoip_shaper)" value="<% nvram_get("oet2_shaper"); %>" />
		</div>	
		<div class="setting">
			<div class="label"><% tran("service.eoip_bridging"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet2_bridged" <% nvram_selmatch("oet2_bridged","1","checked"); %> onclick="show_layer_ext(this, 'idbridged2', false)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet2_bridged" <% nvram_selmatch("oet2_bridged","0","checked"); %> onclick="show_layer_ext(this, 'idbridged2', true)" /><% tran("share.disable"); %>
		</div>
		<div id="idbridged2">
			<div class="setting">
				<div class="label"><% tran("share.ip"); %></div>
				<input type="hidden" name="oet2_ip" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet2_ip_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet2_ip","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_ip_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet2_ip","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_ip_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet2_ip","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_ip_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("oet2_ip","3"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.subnet"); %></div>
				<input type="hidden" name="oet2_netmask" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet2_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet2_netmask","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet2_netmask","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet2_netmask","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet2_netmask_3" onblur="valid_range(this,0,254,share.subnet)" value="<% get_single_ip("oet2_netmask","3"); %>" />
			</div>
		</div>
	</div>
</fieldset><br/>
<fieldset>
	<legend>Tunnel 3</legend>

		<div class="setting">
			<div class="label"><% tran("service.eoip_srv"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet3_en" <% nvram_selmatch("oet3_en","1","checked"); %> onclick="show_layer_ext(this, 'idoet3', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet3_en" <% nvram_selmatch("oet3_en","0","checked"); %> onclick="show_layer_ext(this, 'idoet3', false)" /><% tran("share.disable"); %>
		</div>
	<div id="idoet3">
		<div class="setting">
			<div class="label"><% tran("service.eoip_remoteIP"); %></div>
			<input type="hidden" name="oet3_rem" value="0.0.0.0"/>
			<input size="3" maxlength="3" class="num" name="oet3_rem_0" onblur="valid_range(this,0,255,service.eoip_remoteIP)" value="<% get_single_ip("oet3_rem","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_rem_1" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet3_rem","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_rem_2" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet3_rem","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_rem_3" onblur="valid_range(this,1,254,service.eoip_tunnelID)" value="<% get_single_ip("oet3_rem","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_tunnelID"); %></div>
			<input size="4" maxlength="3" class="num" name="oet3_id" onblur="valid_range(this,0,999,service.eoip_tunnelID)" value="<% nvram_get("oet3_id"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_comp"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet3_comp" <% nvram_selmatch("oet3_comp","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet3_comp" <% nvram_selmatch("oet3_comp","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_passtos"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet3_pt" <% nvram_selmatch("oet3_pt","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet3_pt" <% nvram_selmatch("oet3_pt","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_frag"); %></div>
			<input size="4" maxlength="4" class="num" name="oet3_fragment" onblur="valid_range(this,0,1500,service.eoip_frag)" value="<% nvram_get("oet3_fragment"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_mssfix"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet3_mssfix" <% nvram_selmatch("oet3_mssfix","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet3_mssfix" <% nvram_selmatch("oet3_mssfix","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_shaper"); %></div>
			<input size="6" maxlength="6" class="num" name="oet3_shaper" onblur="valid_range(this,0,100000,service.eoip_shaper)" value="<% nvram_get("oet3_shaper"); %>" />
		</div>	
		<div class="setting">
			<div class="label"><% tran("service.eoip_bridging"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet3_bridged" <% nvram_selmatch("oet3_bridged","1","checked"); %> onclick="show_layer_ext(this, 'idbridged3', false)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet3_bridged" <% nvram_selmatch("oet3_bridged","0","checked"); %> onclick="show_layer_ext(this, 'idbridged3', true)" /><% tran("share.disable"); %>
		</div>
		<div id="idbridged3">
			<div class="setting">
				<div class="label"><% tran("share.ip"); %></div>
				<input type="hidden" name="oet3_ip" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet3_ip_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet3_ip","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_ip_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet3_ip","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_ip_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet3_ip","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_ip_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("oet3_ip","3"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.subnet"); %></div>
				<input type="hidden" name="oet3_netmask" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet3_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet3_netmask","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet3_netmask","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet3_netmask","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet3_netmask_3" onblur="valid_range(this,0,254,share.subnet)" value="<% get_single_ip("oet3_netmask","3"); %>" />
			</div>
		</div>
	</div>
</fieldset><br/>
<fieldset>
	<legend>Tunnel 4</legend>

		<div class="setting">
			<div class="label"><% tran("service.eoip_srv"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet4_en" <% nvram_selmatch("oet4_en","1","checked"); %> onclick="show_layer_ext(this, 'idoet4', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet4_en" <% nvram_selmatch("oet4_en","0","checked"); %> onclick="show_layer_ext(this, 'idoet4', false)" /><% tran("share.disable"); %>
		</div>
	<div id="idoet4">
		<div class="setting">
			<div class="label"><% tran("service.eoip_remoteIP"); %></div>
			<input type="hidden" name="oet4_rem" value="0.0.0.0"/>
			<input size="3" maxlength="3" class="num" name="oet4_rem_0" onblur="valid_range(this,0,255,service.eoip_remoteIP)" value="<% get_single_ip("oet4_rem","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_rem_1" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet4_rem","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_rem_2" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet4_rem","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_rem_3" onblur="valid_range(this,1,254,service.eoip_tunnelID)" value="<% get_single_ip("oet4_rem","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_tunnelID"); %></div>
			<input size="4" maxlength="3" class="num" name="oet4_id" onblur="valid_range(this,0,999,service.eoip_tunnelID)" value="<% nvram_get("oet4_id"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_comp"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet4_comp" <% nvram_selmatch("oet4_comp","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet4_comp" <% nvram_selmatch("oet4_comp","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_passtos"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet4_pt" <% nvram_selmatch("oet4_pt","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet4_pt" <% nvram_selmatch("oet4_pt","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_frag"); %></div>
			<input size="4" maxlength="4" class="num" name="oet4_fragment" onblur="valid_range(this,0,1500,service.eoip_frag)" value="<% nvram_get("oet4_fragment"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_mssfix"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet4_mssfix" <% nvram_selmatch("oet4_mssfix","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet4_mssfix" <% nvram_selmatch("oet4_mssfix","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_shaper"); %></div>
			<input size="6" maxlength="6" class="num" name="oet4_shaper" onblur="valid_range(this,0,100000,service.eoip_shaper)" value="<% nvram_get("oet4_shaper"); %>" />
		</div>	
		<div class="setting">
			<div class="label"><% tran("service.eoip_bridging"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet4_bridged" <% nvram_selmatch("oet4_bridged","1","checked"); %> onclick="show_layer_ext(this, 'idbridged4', false)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet4_bridged" <% nvram_selmatch("oet4_bridged","0","checked"); %> onclick="show_layer_ext(this, 'idbridged4', true)" /><% tran("share.disable"); %>
		</div>
		<div id="idbridged4">
			<div class="setting">
				<div class="label"><% tran("share.ip"); %></div>
				<input type="hidden" name="oet4_ip" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet4_ip_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet4_ip","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_ip_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet4_ip","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_ip_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet4_ip","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_ip_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("oet4_ip","3"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.subnet"); %></div>
				<input type="hidden" name="oet4_netmask" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet4_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet4_netmask","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet4_netmask","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet4_netmask","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet4_netmask_3" onblur="valid_range(this,0,254,share.subnet)" value="<% get_single_ip("oet4_netmask","3"); %>" />
			</div>
		</div>
	</div>
</fieldset><br/>
<fieldset>
	<legend>Tunnel 5</legend>

		<div class="setting">
			<div class="label"><% tran("service.eoip_srv"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet5_en" <% nvram_selmatch("oet5_en","1","checked"); %> onclick="show_layer_ext(this, 'idoet5', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet5_en" <% nvram_selmatch("oet5_en","0","checked"); %> onclick="show_layer_ext(this, 'idoet5', false)" /><% tran("share.disable"); %>
		</div>
	<div id="idoet5">
		<div class="setting">
			<div class="label"><% tran("service.eoip_remoteIP"); %></div>
			<input type="hidden" name="oet5_rem" value="0.0.0.0"/>
			<input size="3" maxlength="3" class="num" name="oet5_rem_0" onblur="valid_range(this,0,255,service.eoip_remoteIP)" value="<% get_single_ip("oet5_rem","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_rem_1" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet5_rem","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_rem_2" onblur="valid_range(this,0,255,service.eoip_tunnelID)" value="<% get_single_ip("oet5_rem","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_rem_3" onblur="valid_range(this,1,254,service.eoip_tunnelID)" value="<% get_single_ip("oet5_rem","3"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_tunnelID"); %></div>
			<input size="4" maxlength="3" class="num" name="oet5_id" onblur="valid_range(this,0,999,service.eoip_tunnelID)" value="<% nvram_get("oet5_id"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_comp"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet5_comp" <% nvram_selmatch("oet5_comp","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet5_comp" <% nvram_selmatch("oet5_comp","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_passtos"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet5_pt" <% nvram_selmatch("oet5_pt","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet5_pt" <% nvram_selmatch("oet5_pt","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_frag"); %></div>
			<input size="4" maxlength="4" class="num" name="oet5_fragment" onblur="valid_range(this,0,1500,service.eoip_frag)" value="<% nvram_get("oet5_fragment"); %>" />
		</div>			
		<div class="setting">
			<div class="label"><% tran("service.eoip_mssfix"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet5_mssfix" <% nvram_selmatch("oet5_mssfix","1","checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet5_mssfix" <% nvram_selmatch("oet5_mssfix","0","checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("service.eoip_shaper"); %></div>
			<input size="6" maxlength="6" class="num" name="oet5_shaper" onblur="valid_range(this,0,100000,service.eoip_shaper)" value="<% nvram_get("oet5_shaper"); %>" />
		</div>	
		<div class="setting">
			<div class="label"><% tran("service.eoip_bridging"); %></div>
			<input class="spaceradio" type="radio" value="1" name="oet5_bridged" <% nvram_selmatch("oet5_bridged","1","checked"); %> onclick="show_layer_ext(this, 'idbridged5', false)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="oet5_bridged" <% nvram_selmatch("oet5_bridged","0","checked"); %> onclick="show_layer_ext(this, 'idbridged5', true)" /><% tran("share.disable"); %>
		</div>
		<div id="idbridged5">
			<div class="setting">
				<div class="label"><% tran("share.ip"); %></div>
				<input type="hidden" name="oet5_ip" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet5_ip_0" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet5_ip","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_ip_1" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet5_ip","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_ip_2" onblur="valid_range(this,0,255,share.ip)" value="<% get_single_ip("oet5_ip","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_ip_3" onblur="valid_range(this,1,254,share.ip)" value="<% get_single_ip("oet5_ip","3"); %>" />
			</div>
			<div class="setting">
				<div class="label"><% tran("share.subnet"); %></div>
				<input type="hidden" name="oet5_netmask" value="0.0.0.0"/>
				<input size="3" maxlength="3" class="num" name="oet5_netmask_0" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet5_netmask","0"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_netmask_1" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet5_netmask","1"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_netmask_2" onblur="valid_range(this,0,255,share.subnet)" value="<% get_single_ip("oet5_netmask","2"); %>" />.<input size="3" maxlength="3" class="num" name="oet5_netmask_3" onblur="valid_range(this,0,254,share.subnet)" value="<% get_single_ip("oet5_netmask","3"); %>" />
			</div>
		</div>
	</div>
</fieldset><br/>
							<div class="submitFooter">
							 <script type="text/javascript">
							 //<![CDATA[
							 document.write("<input type=\"button\" name=\"save_button\" value=\"" + sbutton.save + "\" onclick=\"to_submit(this.form)\" />");
							 //]]>
							 </script>
							 <script type="text/javascript">
							 //<![CDATA[
							 document.write("<input type=\"reset\" value=\"" + sbutton.cancel + "\" />");
							 //]]>
							 </script>
							 <script type="text/javascript">
							 //<![CDATA[
							 document.write("<input type=\"button\" name=\"refresh_button\" value=\"" + <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %> + "\" onclick=\"window.location.reload()\">");
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
						</dl><br />
						<a href="javascript:openHelpWindow<% nvram_selmatch("dist_type","micro","Ext"); %>('HEOP.asp');"><% tran("share.more"); %></a>
					</div>
				</div>
				<div id="floatKiller"></div>
				<% do_statusinfo(); %>
			</div>
		</div>
	</body>
</html>