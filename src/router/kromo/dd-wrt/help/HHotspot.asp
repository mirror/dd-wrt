<% do_hpagehead("hotspot.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("hotspot.h2"); %></h2>
			<dl>
				<dd>You can use the router as an Hotspot gateway (Chillispot solution) with authentication, accounting (Radius). ChilliSpot is an open source captive portal or wireless LAN access point controller. It is used for authenticating users of a wireless LAN. It supports web based login which is today's standard for public HotSpots and it supports Wireless Protected Access (WPA) which is the standard of the future. Authentication, authorization and accounting (AAA) is handled by your favorite radius server.</dd>
				
				<dt><% tran("hotspot.pserver"); %></dt>
				<dd>The IP addresses of radius server 1 and 2.</dd>
				
				<dt><% tran("hotspot.dns"); %></dt>
				<dd>DNS Server IP. It is used to inform the client about the DNS address to use for host name resolution. If this option is not given the system primary DNS is used.</dd>
				
				<dt><% tran("hotspot.url"); %></dt>
				<dd>URL of web server to use for authenticating clients.</dd>
				
				<dt><% tran("share.share_key"); %></dt>
				<dd>Radius shared secret for both servers. This secret should be changed in order not to compromise security.</dd>
				
				<dt><% tran("hotspot.dhcp"); %></dt>
				<dd>Ethernet interface to listen to for the downlink interface. This option must be specified.</dd>
				
				<dt><% tran("hotspot.radnas"); %></dt>
				<dd>Network access server identifier.</dd>
				
				<dt><% tran("hotspot.uam"); %></dt>
				<dd>Shared secret between uamserver and chilli. This secret should be set in order not to compromise security.</dd>
				
				<dt><% tran("hotspot.uamdns"); %></dt>
				<dd>Allow any DNS server. Normally unauthenticated clients are only allowed to communicate with the DNS servers specified by the dns1 and dns2 options. This option will allow the client to use all DNS servers. This is convenient for clients which are configured to use a fixed set of DNS servers.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>For security reasons this option should be combined with a destination NAT firewall rule which forwards all DNS requests to a given DNS server.</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.allowuam"); %></dt>
				<dd>IP addresses or network segments the client can access without first authenticating (Comma separated list of domain names). Example: www.chillispot.org,10.11.12.0/24 </dd>
				
				<dt><% tran("hotspot.macauth"); %></dt>
				<dd>If this option is given ChilliSpot will try to authenticate all users based on their mac address alone.</dd>
				
				<dt><% tran("hotspot.option"); %></dt>
				<dd>You can specify here additional Options.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>For more informations about the different options : <a href="http://www.chillispot.org/chilli.html">www.chillispot.org</a>.</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nocat_legend"); %></dt>
				<dd>Enabling the use of NoCatSplash allows you to redirect a client to a specific web page when connecting via wireless or wired.</dd>
				
				<dt><% tran("hotspot.nocat_gateway"); %></dt>
				<dd>The name of the gateway. Whatever you want to call it. "Joe's Pizza Shop and free DSL Cafe" for example. Use the variable $GatewayName in your splash.html page to display this.</dd>
				
				<dt><% tran("hotspot.nocat_home"); %></dt>
				<dd>Configures the Redirection URL after splash login</dd>

				<dt><% tran("hotspot.nocat_redirect"); %></dt>
				<dd>Enables the redirection to a specific Homepage after splash login, see above</dd>
				
				<dt><% tran("hotspot.nocat_allowweb"); %></dt>
				<dd>Space separated list of hostnames.
List any hosts (for example, the webserver with the splash page, or other websites) that you would like to allow clients to have web access to (TCP port 80 (HTTP) and 443 (HTTPS)) before they "log in" (before they click on "I Accept" in your splash page), however actual authentication is not supported.
List any webservers, that you would like connecting clients to be able to access, before clicking on I Agree on the initial nocatsplash screen. Such as the webserver hosting your EULA or Welcome Page, if it isn't the router itself.</dd>
				
				<dt><% tran("hotspot.nocat_docroot"); %></dt>
				<dd>Where all of the application templates (including SplashForm) are hiding (splash.html is the form displayed to users on capture).</dd>
				
				<dt><% tran("hotspot.nocat_splash"); %></dt>
				<dd>Optional URL to fetch dynamic remote splash page from. This should end with the /splash.html, or the name of your splash page.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>Leave empty if using a page stored on the router.</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nocat_port"); %></dt>
				<dd>Space separated list of ports. Specify TCP ports to denied access to when public class users login. All others will be allowed. If nothing is specified, access is granted to all ports to public class users.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>You should *always* exclude port 25 (SMTP), unless you want to run a portal for wanton spam sending. Users should have their own way of sending mail. It sucks, but that's the way it is.</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nocat_timeout"); %></dt>
				<dd>How much time, in seconds, elapses before the client has to see the splash screen again, and click on 'I Agree'. How often a client is shown the EULA or other designated splash page.</dd>
				
				<dt><% tran("hotspot.nocat_verbose"); %></dt>
				<dd>Log verbosity (to syslogd and /tmp/nocat.log). Syslogd service must be enabled.
					<ul>
						<li>0 is (almost) no logging.</li>
						<li>10 is log everything.</li>
						<li>5 is probably a safe middle road.</li>
					</ul>
				</dd>
				
				<dt><% tran("hotspot.nocat_route"); %></dt>
				<dd>Required only if you DO NOT want your gateway to act as a NAT. Enable this only if you're running a strictly routed network, and don't need the gateway to enable NAT for you. You would not normally use this option. So if you don't understand it, leave it Disabled</dd>
				<dd>Check all values and click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes. Click <em>Reboot router</em> to reboot your router immediately.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
				<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
				<li><a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a></li>
			</ul>
		</div>
	</body>
</html>