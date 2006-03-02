<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Management</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Management</h2>
			<dl>
				<dd>The Management screen allows you to change the router's settings. On this page you will find most of the configurable items of the DD-WRT router code.</dd>
				<dt>Router Password</dt>
				<dd>The new password must not exceed 32 characters in length and must not include any spaces. Enter the new password a second time to confirm it.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Default username is <tt>root</tt><br/>
						It is strongly recommended that you change the factory default password of the router, which is <tt>admin</tt>. All users who try to access the router's web-based utility or Setup Wizard will be prompted for the router's password.</div>
					</div>
				</dd>
				<dt>Remote Access</dt>
				<dd>This feature allows you to manage the router from a remote location, via the Internet. To disable this feature, keep the default setting, <em>Disable</em>. To enable this feature, select <em>Enable</em>, and use the specified port (default is 8080) on your PC to remotely manage the router. You must also change the router's default password to one of your own, if you haven't already.<br /><br />
					To remotely manage the router, enter <tt>http://xxx.xxx.xxx.xxx:8080</tt> (the x's represent the router's Internet IP address, and 8080 represents the specified port) in your web browser's address field. You will be asked for the router's password.<br /><br />
					If you use https you need to specify the url as <tt>https://xxx.xxx.xxx.xxx:8080</tt> (not all DD-WRT firmwares does support this without rebuilding with SSL support).<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>If the Remote Router Access feature is enabled, anyone who knows the router's Internet IP address and password will be able to alter the router's settings.</div>
					</div>
				</dd>
				
				<!--<dt>AP Watchdog</dt>
				<dd>The AP Watchdog enables a timer that will check to see if any clients are connected in the interval seconds given. If no clients are attached, the watchdog assumes the AP needs to be reset. When clients are connected this reset will not occur. The watchdog is intended for situations where the AP becomes unavailable due to interference or internal chip problems.</dd>-->
				<dt>Web Access</dt>
				<dd>This feature allows you to manage the router using either HTTP protocol or the HTTPS protocol.<br/>
				You can also activate or not the router information web page. It's now possible to password protect this page (same username and password than above)</dd>
				<dt>Boot Wait</dt>
				<dd>Boot Wait is a feature you will hopefully never need. It introduces a short delay while booting (5s). During this delay you can initiate the download of a new firmware if the one in the flash rom is not broken. Obviously this is only necessary if you can no longer reflash using the web interface because the installed firmware will not boot. See the DD-WRT documentation for more information.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>It is recommended that you enable the Boot Wait feature. This will help you recover in the future should you flash your router improperly.</div>
					</div>
				</dd>
				<dt>Cron</dt>
				<dd>The cron subsystem schedules execution of Linux commands. You'll need to use the command line or startup scripts to actually use this.</dd>
				<dt>DNS Masq</dt>
				<dd>DNSmasq is a local DNS server. It will resolve all host names known to the router from dhcp (dynamic and static) as well as forwarding and caching DNS entries from remote DNS servers. <em>Local DNS</em> enables DHCP clients on the LAN to resolve static and dynamic DHCP hostnames.<br/>
				There are some extra options you can set by entering them in <em>Additional DNS Options</em>. For example : <br/>
					<ul>
						<li>static allocation : dhcp-host=AB:CD:EF:11:22:33,192.168.0.10,myhost,myhost.domain,12h</li>
						<li>max lease number : dhcp-lease-max=2</li>
						<li>DHCP server IP range : dhcp-range=192.168.0.110,192.168.0.111,12h</li>
					</ul>
				<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>All options are saved in /tmp/dnsmasq.conf file. The format of this file consists of one option per line.<br/>The complete list of available options : <a href="http://thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html" target="_new">DNSMasq man</a>.</div>
					</div></dd>
				<dt>Loopback</dt>
				<dd>Enable / disable the loopback interface. The loopback interface makes your internal clients appear as if they are external. This is useful for testing things like DynDNS names. The loopback is an option because enabling it will break PPTP and Windows machine browsing by wireless clients.</dd>
				<dt>802.1x</dt>
				<dd>A limited 802.1x server needed to fulfill WPA handshake requirements to allow Windows XP clients to work with WPA.</dd>
				<dt>NTP Client</dt>
				<dd>Synchronize the clock of the router with an NTP time server.</dd>
				<dt>Resetbuttond</dt>
				<dd>The resetbuttond monitors the reset button and initiates actions depending on how long you press it.
					<ul>
						<li>Short press &ndash; Reset the router (reboot)</li>
						<li>Long press (&gt;5s) &ndash; Reboot and restore the factory default configuration.</li>
					</ul>
				</dd>
				<dt>Routing</dt>
				<dd>Routing enables the OSPF and RIP routing daemons if you have set up OSPF or RIP routing in the Advanced Routing page.</dd>
				<dt>UPnP (Universal Plug and Play)</dt>
				<dd>A Microsoft technology for automatic configuration of devices. You need this enabled if you want to connect the X-Box to the internet via your router.</dd>
				<dd>Check all values and click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes. Click <em>Reboot router</em> to reboot your router immediately.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HSetup.asp">Basic Setup</a></li>
				<li><a href="HServices.asp">Services</a></li>
			</ul>
		</div>
	</body>
</html>
