<% do_pagehead("sipath.titl"); %>>
		<script type="text/javascript">
		//<![CDATA[
		
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
	
	<body class="gui">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
				<% do_menu("Sipath.asp","Sipath.asp"); %>
				</div>
				<div id="main">
					<div id="contents">
		<h2>SIPath</h2>
		<p>Welcome to your <a href="http://sipath.sourceforge.net" target="new">SIP at
		Home</a> web interface for the
		<a href="http://sip-router.org/ser/" target="new">SIP Express Router</a>
		running on <a href="http://www.dd-wrt.com">DD-WRT Embedded Linux</a>.<br />
		<b>Short SIP at Home HowTo:</b><br />
		Standard configuration (/etc/ser/ser.cfg) will cause the router to function
		as an Outbound Proxy and local Registrar Server.<br />
		To enable the first function simply configure the routers IP address as
		<i>Outbound Proxy</i> in your SIP phones configuration.<br />
		Every registration your phone sends to your VoIP Provider will then pass
		through the SIP Express Router.<br />
		At the same time such a registration is forwarded to your Provider the router
		will remember you as being registered and will route calls to registered
		phones locally, so no internet traffic is being produced. A successful local
		registration can be checked on the Phonebook page in the table <i>Contacts</i>.
		All listed <i>aor</i>s (Address Of Record) may be called locally.<br />
		All authentication passwords of your SIP account are statelessly forwarded
		to the <i>SIP Server</i>.<br />
		If a callees number cannot be resolved locally, the call signalling will
		be forwarded to the designated destination, being the <i>SIP Server</i>
		you set in the calling phones configuration. This may imply that your VoIP
		Service Provider configured in your SIP phones configuration as <i>SIP Server</i>
		(or similar), may be connected and an external call may be established.<br />
		<br />
		If you cannot place or receive calls from or to your phones over the Internet
		or if your phones ring but you cannot talk to someone outside your network
		as soon as you raise the receiver, this may indicate that your firewall
		settings prevent the establishment of a RTP stream. In such a case, please
		refer to your firewalls configuration howto or manual.<br />
		<br />
		<b>Examples:</b><br />
		<u>John</u> has configured this routers IP as the Outbound Proxy in his
		SIP phone and did a reset. His username is <u>123456</u>, which is a registered
		account at an Internet VoIP Provider.<br />
		<u>Peter</u> also changed the setting of the Outbound Proxy of his SIP phone
		and restarted it. He also has a registered user account which is <u>234567</u>
		at the same provider as John.<br />
		<u>Chasey</u> has a user account at the same provider as John and Peters,
		yet she isn&#39;t connected to John and Peters SIP at Home Router. Chasey&#39;s
		username at the boys provider is <u>345678</u>.<br />
		<u>Marilyn</u>, living on the other side of town, only has a standard landline
		phone with the number <u>456789</u>.<br />
		<br />
		1. Scenario - Peter wants to call John:<br />
		So Peter dials 123456 with his SIP phone. The phone at first connects the
		SIP at Home Router. The router checks if 123456 is locally registered, which
		is the case. Peters phones call-invitation is being routed locally to Johns
		phone which thereby may start to ring. A RTP media stream is established
		between the two phones directly.<br />
		<br />
		2. Scenario - John wants to call Chasey:<br />
		John therefore dials 345678. The phone uses the SIP at Home Router as the
		next signalling instance (Outbound Proxy) but 345678 is locally unknown
		and therefore the call is being forwarded to the <i>SIP Server</i> John
		has set in his phones configuration. This <i>SIP Server</i> checks the number
		which in this case matches with the user account of Chasey. The call invitation
		is then forwarded to Chaseys SIP phone. A RTP media stream is established
		between John&#39;s and Chasey&#39;s phone, either with the SIP Provider proxying
		the media or not, depending on the Providers settings.<br />
		<br />
		3. Scenario - Peter wants to call Marilyn:<br />
		So he dials 00 as the prefix for landline calls and then 456789. The phone
		uses the SIP at Home Router as Outbound Proxy but 00456789 is locally unknown
		so the call is forwarded to the <i>SIP Server</i> Peter set in his phones
		settings. This SIP Server recognises from the prefix that Peter wants to
		place a call into the PSTN and directs the call to its gateway which connects
		to Marilyns phone. A RTP media stream is established between Peters phone,
		the Providers PSTN-Gateway over the PSTN to Marilyns phone.<br />
		<br />
		Please note that this software is under development and comes with no warranty.
		</p><br />
               </div>
            </div>
				<div id="helpContainer">
					<div id="help">
						<h2><% tran("share.help"); %></h2>
					<br />
					<!--<a href="javascript:openHelpWindow<% ifdef("MICRO","Ext"); %>('HSipath.asp')"><% tran("share.more"); %></a>-->
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
