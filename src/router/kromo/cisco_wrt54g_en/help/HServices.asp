<% do_hpagehead(); %>
		<title>Help - Services</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Services</h2>
			<dl>
				
				<dt><% tran("service.dhcp_legend2"); %></dt>
				<dd>DHCPd assigns IP addresses to your local devices. While the main configuration is on the setup page you can program some nifty special functions here.
					<ul class="wide">
						<li>DHCPd &ndash; Disabling here will disable DHCPd on this router irrespective of the settings on the Setup screen.</li>
						<li>Used domain &ndash; You can select here which domain the DHCP clients should get as their local domain. This can be the WAN domain set on the Setup screen or the LAN domain which can be set here.</li>
						<li>LAN Domain &ndash; You can define here your local LAN domain which is used as local domain for DNSmasq and DHCP service if chosen above.</li>
						<li>Static allocations &ndash; If you want to assign certain hosts a specific address then you can define them here. This is also the way to add hosts with a fixed address to the router's local DNS service (DNSmasq).</li>
					</ul><br/>
					There are some extra options you can set by entering them in <em>Additional DHCPD Options</em>.
				</dd>
				
				<dt><% tran("service.dnsmasq_legend"); %></dt>
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
					</div>
				</dd>
				
				<dt><% tran("service.kaid_legend"); %></dt>
				<dd>Kai is a means of connecting platform games over the internet. Enable the service and then add XBox MAC addresses separeted with ";".<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>Xbox must be connected directly via one of the Ethernet ports of the router.</div>
					</div>
				</dd>
				
				<dt><% tran("service.pptp_legend"); %></dt>
				<dd>A VPN technology by Microsoft and remote access vendors. It is implemented in Windows XP. Configuring this allows you to access you LAN at home remotely.
					<ul class="wide">
						<li>Server IP &ndash; The IP address of your router</li>
						<li>Client IP &ndash; A range of IP addresses for remotely connected machines. This range should not overlap with the DHCP range (Example: 192.168.0.210-220).</li>
						<li>CHAP-Secrets &ndash; A list of usernames and passwords for the VPN login, one user per line (Example: joe * joespassword *). For more details look up the pppd man page.</li>
					</ul>
				</dd>
				
				<dt><% tran("service.pptpd_legend"); %></dt>
				<dd>A VPN Client that enable you to connect to VPN servers by Microsoft and remote access vendors. Configuring this allows the router to VPN into a remote network.
					<ul class="wide">
						<li>Server IP or DNS Name &ndash; The IP address or DNS Name of the VPN server that you would like to connect to (Example: www.MyServer.com). </li>
						<li>Remote Subnet &ndash; Remote Subnet of the network you are connecting to (Example: 192.168.2.0). </li>
						<li>Remote Subnet Mask &ndash; Remote Subnet Mask of the network you are connecting to (Example: 255.255.255.0). </li>
						<li>MPPE Encryption  &ndash; The type of security to use for the connection. If you are connecting to another DD-WRT router you need (Example: mppe required). But if you are connecting to a Windows VPN server you need (Example: mppe required,no40,no56,stateless) or (Example: mppe required,no40,no56,stateful) </li>
						<li>MTU &ndash; Default Maximum Transmission Unit (Default: 1450) </li>
						<li>MRU &ndash; Default Maximum Receiving Unit (Default: 1450) </li>
						<li>User Name &ndash; Enter the UserName that you will use to connect to the VPN server. If you are connecting to another Linux base PPTP server you just need to enter the UserName (Example: root). But if you are connecting to a Windows VPN server you need to enter the servername and username (Example: DOMAIN\\UserName). </li>
						<li>Password &ndash; Enter the password of the for the username </li>
					</ul>
				</dd>

				<dt><% tran("service.rflow_legend"); %></dt>
				<dd>RFlow Collector is a traffic monitoring and management tool that allows to watch a complete network of DD-WRT routers.<br/>
					<ul class="wide">
						<li>RFlow Default port is 2055</li>
						<li>MACupd Default port is 2056</li>
						<li>Interval = 10 seems messages will be sent to server each 10 seconds</li>
						<li>Interface : choose which interface to monitor</li>
					</ul><br/>
					<div class="note">
						<h4>Note</h4>
						<div>For each RFlow and MACupd server IP : enter the IP address of the listening server (win32 PC with RFlow Collector).</div>
					</div>
				</dd>
				
				<dt><% tran("service.rstats_legend"); %></dt>
				<dd>rstats is a bandwidth monitoring tool developped by Jonathan Zarate. The Adobe's SVG plugin is required to display bandwidth graphs. You can download this plugin <a href="http://www.adobe.com/svg/viewer/install/main.html">here</a><br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>Take care, that inspite of general purpose read/write not to store frequently changed files on <em>NVRAM</em> (non-volatile RAM) or <em>JFFS2</em> (Journaled flash file system) as this can cause flash wear out.</div>
					</div>
				</dd> 
				
				<dt><% tran("service.ssh_legend"); %></dt>
				<dd>Enabling SSHd allows you to access the Linux OS of your router with an SSH client (Putty works well on Windows, for example).
					<ul class="wide">
						<li>Password login &ndash; allow login with the router password (username is <tt>root</tt>)</li>
						<li>SSHd Port &ndash; the port number for SSHd (default is 22)</li>
						<li>Authorized Keys &ndash; here you paste your public keys to enable key-based login (more secure than a simple password)</li>
					</ul>
				</dd>
				
				<dt><% tran("service.syslog_legend"); %></dt>
				<dd>Enable Syslogd to capture system messages. By default they will be collected in the local file /var/log/messages. To send them to another system, enter the IP address of a remote syslog server.</dd>
				
				<dt><% tran("service.telnet_legend"); %></dt>
				<dd>Enable a telnet server to connect to the router with telnet. The username is <tt>root</tt> and the password is the router password.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>If you using the router in an untrusted environment (for example as a public hotspot), it is strongly recommended to use SSHd and deactivate telnet.</div>
					</div>
				</dd>
				<dd>Check all values and click <em>Save Settings</em> to save your settings. Click <em>Cancel Changes</em> to cancel your unsaved changes. Click <em>Reboot router</em> to reboot your router immediately.</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
