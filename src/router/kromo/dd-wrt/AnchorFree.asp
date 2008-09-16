<% do_pagehead("anchorfree.titl"); %>
		<script type="text/javascript">
		//<![CDATA[

function validate_anchorfree(F)
{
if (F.af_enable.item(0).checked == true)
    {
    if (valid_email(F.af_email.value,1,1)==false)
      {
	    return false;
      }
    if (F.af_publish.item(0).checked == true)
	{
	if (F.af_address.value.length==0)
	    {
	    alert(anchorfree.validaddr);
	    return false;
	    }
	if (F.af_city.value.length == 0 && F.af_zip.value.length == 0)
	    {
	    alert(anchorfree.validcity);
	    return false;
	    }
	}
    if (F.af_category.value == 0)
	{
	alert(anchorfree.validcat);
	return false;
	}
    if (F.af_country.value == "null")
	{
	alert(anchorfree.validcountry);
	return false;
	}
    if (F._af_agree.checked == false)
	{
	alert(anchorfree.validterms);
	return false;	
	}else
	{
	F.af_agree.value = 1;
	}
    }else{
	F.af_agree.value = 0;
    
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
						<form name="setup" action="apply.cgi" method="post">
							<input type="hidden" name="submit_button" value="AnchorFree" />
							<input type="hidden" name="action" value="Apply" />
							<input type="hidden" name="change_action" />
							<input type="hidden" name="submit_type" />
							<input type="hidden" name="commit" value="1"/>
							<input type="hidden" name="af_agree"/>
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
		<div class="setting">
				<div class="label"><% tran("anchorfree.agreement"); %></div>
					<textarea cols="60" rows="20" >
HOTSPOT MONETIZATION SERVICES AGREEMENT
This HotSpot Monetization Services Agreement (the “Agreement”) is a legal agreement between you (“Affiliate” or “you”) and AnchorFree, Inc., a Delaware corporation with a place of business at 260 Santa Ana Ct., Sunnyvale, CA 94085, USA (“AnchorFree”) and exclusively governs AnchorFree’s provision and Affiliate’s receipt of the Monetization Services (as defined below).

BEFORE YOU CLICK ON THE “I ACCEPT AND AGREE” BUTTON ON THE MONETIZATION PROGRAM SIGN-UP FORM, CAREFULLY READ THE TERMS AND CONDITIONS OF THIS AGREEMENT. BY COMPLETING AND SUBMITTING THE MONETIZATION PROGRAM SIGN-UP FORM, YOU ARE AGREEING, AND IF DOING SO ON BEHALF OF ANY ENTITY, AGREEING ON BEHALF OF SUCH ENTITY, TO BE BOUND BY THE TERMS AND CONDITIONS OF THIS AGREEMENT. IF YOU DO NOT AGREE TO DO SO, DO NOT COMPLETE AND SUBMIT THE MONETIZATION PROGRAM SIGN-UP FORM AND NO AGREEMENT SHALL BE DEEMED ENTERED INTO BY AND BETWEEN ANCHORFREE AND AFFILIATE WITH RESPECT TO THE
MONETIZATION SERVICES.

1. Definitions. 
(a) “Content” means any and all information, text, images, audio, video, graphics and other materials available on the World Wide Web. 
(b) “Customer” means any third-party person or entity visiting or otherwise accessing Content on the World Wide Web via a HotSpot. 
(c) ”HotSpot” means each of the wireless access points operated by Affiliate during the term that serve as a gateway to the World Wide Web. 
(d) “Monetization Services” means AnchorFree’s proprietary method of monitoring, tracking and documenting Customer access to and use of Content through a HotSpot, which access and use is free of charge to the Customer but results in AnchorFree’s receipt of payment from providers of Content. 
(e) “Net Revenue” means amounts actually received by AnchorFree from third party providers of Content based on a Customer’s access and use of such Content. 

2. Configuration.In conjunction with this Agreement, Affiliate will either enter into the Equipment Loan and License Agreement or the Software License Agreement with AnchorFree to enable the Monetization Services. 

3. Other Requirements. Affiliate understands and agrees that its eligibility to utilize the Monetization Services with respect to any HotSpot shall continue only for so long as the connectivity to the World Wide Web through such HotSpot is provided free of charge to each Customer. Affiliate hereby represents, warrants and covenants that each HotSpot of Affiliate covered by this Agreement shall be solely for public access HotSpots that do not require payment or other consideration by a Customer to access the World Wide Web through such HotSpot. 

4. Fees and Payments.
4.1 Payments. You shall receive a payment related to the number of valid clicks on Ads, valid impressions of Ads, and/or valid completions of survey in connection with Your router, in each case as determined by AnchorFree for its participants in the Program. Unless otherwise agreed to by the parties in writing (including by electronic mail), payments to You shall be sent by AnchorFree within approximately thirty (30) days after the end of each calendar month that Ads or Surveys are running on Your router if Your earned balance is $100 or more. In the event the Agreement is terminated, AnchorFree shall pay Your earned balance to You within approximately ninety (90) days after the end of the calendar month in which the Agreement is terminated by You (following AnchorFree's receipt of Your written request, including by email, to terminate the Agreement) or by AnchorFree. In no event, however, shall AnchorFree make payments for any earned balance less than $10. 

Notwithstanding the foregoing, AnchorFree shall not be liable for any payment based on:
 
(a) any amounts which result from invalid display reporting, invalid Survey events, or invalid clicks or impressions on Ads generated by any person, bot, automated program or similar device, as reasonably determined by AnchorFree, including without limitation through any clicks or impressions 
(i) originating from Your IP addresses or computers under Your control, 
(ii) solicited by payment of money, false representation, or request for end users to click on Ads, or 
(iii) solicited by payment of money, false representation, or any illegal or otherwise invalid request for end users to complete Survey Events; 

(b) Ads or Surveys delivered to end users whose browsers have JavaScript disabled; 

(c) Ads benefiting charitable organizations and other placeholder or transparent Ads that AnchorFree may deliver; 

(d) AnchorFree advertisements for its own products and/or services (excluding payments based on completed Referral Events); or 
(e) clicks co-mingled with a significant number of invalid clicks described in (a) above, or as a result of any breach of this Agreement by You for any applicable pay period. 

AnchorFree reserves the right to withhold payment or charge back Your account due to any of the foregoing or any breach of this Agreement by You, pending AnchorFree's reasonable investigation of any of the foregoing or any breach of this Agreement by You, or in the event that an advertiser whose Ads are displayed in connection with Your Router(s) defaults on payment for such Ads to AnchorFree. In addition, if You are past due on any payment to AnchorFree in connection with any AnchorFree program, AnchorFree reserves the right to withhold payment until all outstanding payments have been made or to offset amounts owed to You in connection with the Program by amounts owed by You to AnchorFree. To ensure proper payment, You are solely responsible for providing and maintaining accurate contact and payment information associated with Your account. For U.S. taxpayers, this information includes withoutlimitation a valid U.S. tax identification number and a fully-completed Form W-9. For non-U.S. taxpayers, this information includes without limitation either a signed certification that the taxpayer does not have U.S. Activities or a fully-completed Form W-8 or other form, which may require a valid U.S. tax identification number, as required by the U.S. tax authorities. Any bank fees related to returned or cancelled checks due to a contact or payment information error or omission may be deducted from the newly issued payment. You agree to pay all applicable taxes or charges imposed by any government entity in connection with Your participation in the Program. AnchorFree may change its pricing and/or payment structure at any time. If You dispute any payment made under the Program, You must notify AnchorFree in writing within thirty (30) days of any such payment; failure to so notify AnchorFree shall result in the waiver by You of any claim relating to any such disputed payment. Payment shall be calculated solely based on records maintained by AnchorFree. No other measurements or statistics of any kind shall be accepted by AnchorFree or have any effect under this Agreement. The payments made under this Agreement are for use by You only and may not be transferred or in any manner passed on to any third party (i.e., distributed to Routers managed by You that require separate payments) unless expressly authorized in writing by AnchorFree (including by electronic mail).

4.2 Taxes.  Each Party will be responsible for taxes based on its own net income, employment taxes with respect to its own employees, and for taxes on any property it owns or leases.

4.3 Expenses.  Each party will bear its own expenses in connection with the performance of this Agreement.  Except as specifically set forth above in this Section 4, neither party will owe any financial consideration to the other party under this Agreement. 

5. Term and Termination. 
5.1 Term.  Unless earlier terminated as set forth in this Section 5, this Agreement will remain in force for a period of eighteen (18) months after the date of Affiliate’s acceptance of the terms and conditions of this Agreement as provided for below. 

5.2 Default.  If either party materially defaults in the performance of any of its material obligations hereunder and if any such default is not corrected within thirty (30) days after notice in writing, then the non-defaulting party, at its option, may, in addition to any other remedies it may have, thereupon terminate this Agreement by giving written notice of termination to the defaulting party. 

5.3 Convenience.  Either party may terminate this Agreement at any time upon sixty (60) days’ prior written notice to the other party. 

5.4 Insolvency.  This Agreement may be terminated by either party, upon written notice: 
(i) upon the institution by the other party of insolvency, receivership or bankruptcy proceedings or any other proceedings for the settlement of its debts; 
(ii) upon the institution of such proceedings against the other party, which are not dismissed or otherwise resolved in its favor within sixty (60) days thereafter; 
(iii) upon the other party's making a general assignment for the benefit of creditors, or 
(iv) upon the other party's dissolution or ceasing to conduct business in the ordinary course. 

5.5 Effect of Termination or Expiration. 
(i) Survival.  The parties’ rights and obligations of Sections 1, 4
(solely with respect to amounts owed but unpaid on the effective date of termination or expiration of this Agreement), 5.5, 6 and 7 will survive any termination or expiration of this Agreement.
(ii) Return of Materials.  No later than thirty (30) days after the date of expiration or termination of this Agreement, each party shall return, or if instructed by the other party, destroy, all information, documents, and materials exchanged between the parties (including all copies thereof) and each party shall furnish the other with a certificate signed by an executive officer of such party verifying that the same has been done.
(iii) Limitation of Liability.  In the event of termination by either
party in accordance with any of the provisions of this Agreement, neither party will be liable to the other, because of such termination, for compensation, reimbursement or damages on account of the loss of prospective profits or anticipated sales or on account of expenditures, investments, or commitments in connection with the business or goodwill. 

6. Warranties and Limitations of Liability. 
6.1 Each party hereby represents and warrants that: 
(i) it has the right to enter into this Agreement; and, if applicable, it is a corporation duly organized, validly existing, and in good standing under the laws of the state of its incorporation; it has the corporate power and authority for, and has by all necessary corporate action authorized, the execution and delivery of this Agreement, and the performance of its obligations hereunder; 
(ii) the execution, performance and delivery of this Agreement by such party will not conflict with or violate or result in any breach of, or constitute a default under, any contract, agreement or other obligation of such party; and
(iii) its activities in connection with this Agreement will not violate any applicable law, ordinance, statute, or regulation, including without limitation, laws concerning defamation, libel, unfair competition, discrimination or false advertising.

6.2 Disclaimer.  EXCEPT FOR THE EXPRESS LIMITED WARRANTY SET FORTH IN THIS SECTION 7, ANCHORFREE MAKES NO OTHER WARRANTIES, EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, AND SPECIFICALLY DISCLAIMS, ON ITS OWN BEHALF ON AND BEHALF OF ITS SUPPLIERS AND LICENSORS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT AND THAT THE MONETIZATION SERVICES WILL BE UNINTERRUPTED OR ERROR FREE.  AFFILIATE UNDERSTANDS AND ACKNOWLEDGES THAT THERE IS NO GUARANTEE THAT ANY MINIMUM LEVEL OF REVENUE, OR ANY REVENUE, WILL BE GENERATED AS A RESULT OF THIS AGREEMENT. 

6.3 Limitation of Liability.  NEITHER PARTY WILL BE LIABLE TO THE OTHER PARTY FOR ANY SPECIAL, CONSEQUENTIAL, EXEMPLARY OR INCIDENTAL DAMAGES (INCLUDING LOST OR ANTICIPATED REVENUES OR PROFITS RELATING TO THE SAME, OR COST OF PROCUREMENT OFSUBSTITUTE PRODUCTS, SERVICES, OR TECHNOLOGY), ARISING FROM ANY CLAIM RELATING TO THIS AGREEMENT OR THE SUBJECT MATTER HEREOF, WHETHER SUCH CLAIM IS BASED ON WARRANTY, CONTRACT, TORT (INCLUDING NEGLIGENCE) OR OTHERWISE, EVEN IF AN AUTHORIZED REPRESENTATIVE OF SUCH PARTY IS ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 

7. Miscellaneous.
No amendment or modification hereof shall be valid or binding upon the parties unless made in writing and signed by the duly authorized representatives of both parties.  In the event that any provision(s) of this Agreement shall be held to be unenforceable, this Agreement will continue in full force and effect without said provision and the parties will negotiate in good faith to amend this Agreement to reflect the original intent of the parties.  This Agreement shall be governed in all respects by the substantive laws of the State of California (excluding conflict of laws rules). The parties hereby and irrevocably consent to the personal and exclusive jurisdiction of courts located in Santa Clara County, California.  Affiliate may not assign this Agreement (by operation of law, including change of control, merger, sale of assets or otherwise) without the prior written consent of AnchorFree, and any prohibited assignment or sublicense shall be null and void.  AnchorFree may assign this Agreement without restriction in connection with a change of control, merger or sale of assets.  This Agreement constitutes the complete and exclusive understanding and agreement of the parties relating to the subject matter hereof and supersedes all prior understandings, proposals, agreements, negotiations, and discussions between the parties, whether written or oral.  Subject to the foregoing, this Agreement will be binding upon and will inure to the benefit of the parties’ permitted successors and/or assignees.  Waiver by either party of a breach of any provision of this Agreement or the failure by either party to exercise any right hereunder shall not operate or be construed as a waiver of any subsequent breach of that right or as a waiver of any other right.
</textarea>
		</div>
		<div class="setting">
			<div class="label"><% tran("anchorfree.agree"); %></div>
			<input type="checkbox" value="1" name="_af_agree" <% nvram_checked("af_agree", "1"); %> />
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
						<a href="javascript:openHelpWindow<% ifdef("EXTHELP","Ext"); %>('HAnchorFree.asp');"><% tran("share.more"); %></a>
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
