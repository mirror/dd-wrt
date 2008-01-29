<% do_pagehead("anchorfree.title"); %>
		<script type="text/javascript">
		//<![CDATA[

function validate_anchorfree(F)
{
if (F.af_enable)
    {
    if (valid_email(F.af_email.value,1,1)==false)
      {
	    return false;
      }
    if (F.af_publish.value=="1")
	{
	if (F.af_address.value.length==0)
	    {
	    alert("a valid address field must be provided if this hotspot should be published on the wifi hotspot map!");
	    return false;
	    }
	if (F.af_city.value.length == 0 && F.af_zip.value.length == 0)
	    {
	    alert("a valid city or zip/postal code must be provided if this hotspot should be published on the wifi hotspot map!");
	    return false;
	    }
	}
    if (F.af_category.value=="0")
	{
	alert("please select a advertising category for your hotspot");
	return false;
	}
    if (F.af_country.value=="null")
	{
	alert("please select a country for your hotspot");
	return false;
	}
    }
return true;
}

function to_submit(F) {
	if (validate_anchorfree(F)==true)
	{
	    F.change_action.value = "";
	    F.submit_type.value = "";
	    F.save_button.value = sbutton.saving;
	    apply(F);
	}
}
function to_apply(F) {
	if (validate_anchorfree(F)==true)
	{
	    F.change_action.value = "";
	    F.submit_type.value = "";
	    F.save_button.value = sbutton.saving;
	    applytake(F);
	}
}


var update;

addEvent(window, "load", function() {
	show_layer_ext(document.setup.af_enable, 'idanchorfree', <% nvram_else_match("af_enable", "1", "1", "0"); %> == 1);
	show_layer_ext(document.setup.af_ssid, 'idanchorfreessid', <% nvram_else_match("af_ssid", "1", "1", "0"); %> == 1);
	update = new StatusUpdate("AnchorFree.live.asp", <% nvram_get("refresh_time"); %>);
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
					<div id="logo"><h1><% show_control(); %></h1></div>
					<% do_menu("Services.asp","AnchorFree.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
						<form name="setup" action="apply.cgi" method="<% get_http_method(); %>">
							<input type="hidden" name="submit_button" value="AnchorFree" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							
							<h2><% tran("anchorfree.anchorfree_revenue"); %></h2>
<fieldset>
	<legend><% tran("anchorfree.anchorfree_revenue"); %></legend>
	<div class="setting">
		<div class="label"><% tran("anchorfree.anchorfree"); %></div>
		<input class="spaceradio" type="radio" value="1" name="af_enable" <% nvram_selmatch("af_enable","1", "checked"); %> onclick="show_layer_ext(this, 'idanchorfree', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" value="0" name="af_enable" <% nvram_selmatch("af_enable","0", "checked"); %> onclick="show_layer_ext(this, 'idanchorfree', false)" /><% tran("share.disable"); %>
	</div>
	<div id="idanchorfree" >
		<div class="setting">
			<div class="label"><% tran("anchorfree.email"); %></div>
			<input class="text" maxLength="100" size="25" name="af_email" value="<% nvram_get("af_email"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.ssid"); %></div>
			<input class="spaceradio" type="radio" value="1" name="af_ssid" <% nvram_selmatch("af_ssid","1", "checked"); %> onclick="show_layer_ext(this, 'idanchorfreessid', true)" /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="af_ssid" <% nvram_selmatch("af_ssid","0", "checked"); %> onclick="show_layer_ext(this, 'idanchorfreessid', false)" /><% tran("share.disable"); %>
		</div>
		<div id="idanchorfreessid">
			<div class="setting">
				<div class="label"><% tran("anchorfree.ssid_name"); %></div>
				<input class="text" maxLength="32" size="32" name="af_ssid_name" value="<% nvram_get("af_ssid_name"); %>" />
			</div>	
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.address_1"); %></div>
			<input class="text" maxLength="100" size="64" name="af_address" value="<% nvram_get("af_address"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.address_2"); %></div>
			<input class="text" maxLength="100" size="64" name="af_address_2" value="<% nvram_get("af_address_2"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.city"); %></div>
			<input class="text" maxLength="100" size="32" name="af_city" value="<% nvram_get("af_city"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.zip"); %></div>
			<input class="text" maxLength="6" size="6" name="af_zip" value="<% nvram_get("af_zip"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.state"); %></div>
			<input class="text" maxLength="32" size="32" name="af_state" value="<% nvram_get("af_state"); %>" />
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.country"); %></div>
			<% show_countrylist("af_country"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.category"); %></div>
			<select name="af_category" >
			<option value="0" <% nvram_selected("af_category", "0"); %> >Please choose...</option>
			<option value="112" <% nvram_selected("af_category", "112"); %> >Airports</option>
			<option value="115" <% nvram_selected("af_category", "115"); %> >Bars, Pubss, & Clubs</option>
			<option value="118" <% nvram_selected("af_category", "118"); %> >Law Firms</option>
			<option value="127" <% nvram_selected("af_category", "127"); %> >Apartments</option>
			<option value="137" <% nvram_selected("af_category", "137"); %> >Art Museums & Galleries</option>
			<option value="145" <% nvram_selected("af_category", "145"); %> >Auto Sales & Services</option>
			<option value="171" <% nvram_selected("af_category", "171"); %> >Bakeries</option>
			<option value="174" <% nvram_selected("af_category", "174"); %> >Banks</option>
			<option value="175" <% nvram_selected("af_category", "175"); %> >Convention, Conference, & Banquet</option>
			<option value="187" <% nvram_selected("af_category", "187"); %> >Billiards</option>
			<option value="195" <% nvram_selected("af_category", "195"); %> >Bookstores</option>
			<option value="214" <% nvram_selected("af_category", "214"); %> >Cafes</option>
			<option value="215" <% nvram_selected("af_category", "215"); %> >Campgrounds</option>
			<option value="247" <% nvram_selected("af_category", "247"); %> >Churches & Religious Organisations</option>
			<option value="257" <% nvram_selected("af_category", "257"); %> >Schools & Universities</option>
			<option value="266" <% nvram_selected("af_category", "266"); %> >Computer Sales & Services</option>
			<option value="304" <% nvram_selected("af_category", "304"); %> >Spas & Massage Therapy</option>
			<option value="317" <% nvram_selected("af_category", "317"); %> >Healthcare</option>
			<option value="338" <% nvram_selected("af_category", "338"); %> >Movies, Theatres, & Entertainment Venues</option>
			<option value="370" <% nvram_selected("af_category", "370"); %> >Gas Stations</option>
			<option value="376" <% nvram_selected("af_category", "376"); %> >Retail</option>
			<option value="379" <% nvram_selected("af_category", "379"); %> >Golf Courses</option>
			<option value="383" <% nvram_selected("af_category", "383"); %> >Government</option>
			<option value="386" <% nvram_selected("af_category", "386"); %> >Grocery & Food Stores</option>
			<option value="391" <% nvram_selected("af_category", "391"); %> >Health & Fitness Clubs</option>
			<option value="410" <% nvram_selected("af_category", "410"); %> >Hotels,Motels & Lodging</option>
			<option value="415" <% nvram_selected("af_category", "415"); %> >Ice Cream & Yogurt</option>
			<option value="416" <% nvram_selected("af_category", "416"); %> >Ice Rinks</option>
			<option value="425" <% nvram_selected("af_category", "425"); %> >Internet Access Provider</option>
			<option value="442" <% nvram_selected("af_category", "442"); %> >Laundry Services</option>
			<option value="449" <% nvram_selected("af_category", "449"); %> >Libraries</option>
			<option value="469" <% nvram_selected("af_category", "469"); %> >Mass Transit</option>
			<option value="486" <% nvram_selected("af_category", "486"); %> >Mobile Home Parks</option>
			<option value="533" <% nvram_selected("af_category", "533"); %> >Outdoor Parks</option>
			<option value="549" <% nvram_selected("af_category", "549"); %> >Pharmacies</option>
			<option value="589" <% nvram_selected("af_category", "589"); %> >Restaurants</option>
			<option value="650" <% nvram_selected("af_category", "650"); %> >Tourist Attractions</option>
			<option value="654" <% nvram_selected("af_category", "654"); %> >Travel Agents</option>
			<option value="745" <% nvram_selected("af_category", "745"); %> >Office Building</option>
			<option value="746" <% nvram_selected("af_category", "746"); %> >Other</option>
			<option value="747" <% nvram_selected("af_category", "747"); %> >Mall, Outlet</option>
			<option value="748" <% nvram_selected("af_category", "748"); %> >Hospital, Clinic</option>
			<option value="749" <% nvram_selected("af_category", "749"); %> >Drink Stand</option>
			<option value="750" <% nvram_selected("af_category", "750"); %> >Homeoffice</option>
			</select>
		</div>		
		<div class="setting">
			<div class="label"><% tran("anchorfree.publish"); %></div>
			<input class="spaceradio" type="radio" value="1" name="af_publish" <% nvram_selmatch("af_publish","1", "checked"); %> /><% tran("share.enable"); %>&nbsp;
			<input class="spaceradio" type="radio" value="0" name="af_publish" <% nvram_selmatch("af_publish","0", "checked"); %> /><% tran("share.disable"); %>
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.serviceid"); %></div>
			<span id="af_serviceid"><% nvram_get("af_serviceid"); %></span>
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.servicestatus"); %></div>
			<span id="af_servicestatus"><% nvram_get("af_servicestatus"); %></span>
		</div>
	   </div>
</fieldset><br />
							<div class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								var autoref = <% nvram_else_match("refresh_time","0","sbutton.refres","sbutton.autorefresh"); %>;
								submitFooterButton(1,1,0,autoref);
								//]]>
								</script>
							</div>
						</form>
					</div>
				</div>
				<div id="helpContainer">
					<div id="help">
						<div><h2><% tran("share.help"); %></h2></div><br/>
						<dl>
							<dt class="term"><% tran("hanchorfree.right1"); %></dt>
							<dd class="definition"><% tran("hanchorfree.right2"); %></dd>
							<dt class="term"><% tran("hanchorfree.right3"); %></dt>
							<dd class="definition"><% tran("hanchorfree.right4"); %></dd>
							<dd class="definition"><% tran("hanchorfree.right5"); %></dd>
							<dt class="term"><% tran("hanchorfree.right6"); %></dt>
							<dd class="definition"><% tran("hanchorfree.right7"); %></dd>
							<dt class="term"><% tran("hanchorfree.right8"); %></dt>
							<dd class="definition"><% tran("hanchorfree.right9"); %></dd>
						</dl><br />
						<a href="javascript:openHelpWindow<% ifdef("MICRO","Ext"); %>('HAnchorFree.asp');"><% tran("share.more"); %></a>
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
