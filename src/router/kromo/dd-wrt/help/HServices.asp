<% do_hpagehead("service.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>Services</h2>
			<dl>
				
				<dt><% tran("service.dhcp_legend2"); %></dt>
				<dd>DHCPd assigns IP addresses to your local devices. While the main configuration is on the setup page you can program some nifty special functions here.
					<ul class="wide">
						<li>Used domain &ndash; You can select here which domain the DHCP clients should get as their local domain. This can be the WAN domain set on the Setup screen or the LAN domain which can be set here.</li>
						<li>LAN Domain &ndash; You can define here your local LAN domain which is used as local domain for DNSmasq and DHCP service if chosen above.</li>
						<li>Static Leases &ndash; If you want to assign certain hosts a specific address then you can define them here. This is also the way to add hosts with a fixed address to the router's local DNS service (DNSmasq).</li>
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
				<dd>Kai is a means of connecting platform games over the internet. Enable the service and then add XBox MAC addresses separeted with ";".
					<ul class="wide">
						<li><% tran("service.kaid_locdevnum"); %> &ndash; How many consoles to detect before the engine locks the pcap filter. Setting this to 0, means the engine will never lock - which means you can use any number of consoles, but you will notice a performance hit, if your network is busy with other traffic. The best thing to do here is to set the number to the number of consoles you own - that's why it defaults to 1 - because most people have just 1 console.</li>
						<li><% tran("service.kaid_uibind"); %> &ndash; Specifies which ip/port kaid will use to listen for controller UIs.</li>
					</ul><br/>
					<div class="note">
						<h4>Note</h4>
						<div>Xbox must be connected directly via one of the Ethernet ports of the router.</div>
					</div>
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
