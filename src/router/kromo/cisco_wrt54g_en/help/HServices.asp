<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
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
				<dt>DHCPd</dt>
				<dd>DHCPd assigns IP addresses to your local devices. While the main configuration is on the setup page you can program some nifty special functions here.
					<ul class="wide">
						<li>DHCPd &ndash; Disabling here will disable DHCPd on this router irrespective of the settings on the Setup screen.</li>
						<li>Used domain &ndash; You can select here which domain the DHCP clients should get as their local domain. This can be the WAN domain set on the Setup screen or the LAN domain which can be set here.</li>
						<li>LAN Domain &ndash; You can define here your local LAN domain which is used as local domain for DNSmasq and DHCP service if chosen above.</li>
						<li>Static allocations &ndash; If you want to assign certain hosts a specific address then you can define them here. This is also the way to add hosts with a fixed address to the router's local DNS service (DNSmasq).</li>
					</ul>
				</dd>
				<dt>PPTP</dt>
				<dd>A VPN technology by Microsoft and remote access vendors. It is implemented in Windows XP. Configuring this allows you to access you LAN at home remotely.
					<ul class="wide">
						<li>Server IP &ndash; The IP address of your router</li>
						<li>Client IP &ndash; A range of IP addresses for remotely connected machines. This range should not overlap with the DHCP range (Example: 192.168.0.210-220).</li>
						<li>CHAP-Secrets &ndash; A list of usernames and passwords for the VPN login, one user per line (Example: joe * joespassword *). For more details look up the pppd man page.</li>
					</ul>
				</dd>
				<dt>SSHd</dt>
				<dd>Enabling SSHd allows you to access the Linux OS of your router with an SSH client (Putty works well on Windows, for example).
					<ul class="wide">
						<li>Password login &ndash; allow login with the router password (username is <tt>root</tt>)</li>
						<li>SSHd Port &ndash; the port number for SSHd (default is 22)</li>
						<li>Authorized Keys &ndash; here you paste your public keys to enable key-based login (more secure than a simple password)</li>
					</ul>
				</dd>
				<dt>Syslogd</dt>
				<dd>Enable Syslogd to capture and forward all messages to another system. Enter the IP-address of the server for the syslog messages.</dd>
				<dt>Telnetd</dt>
				<dd>Enable a telnet server to connect to the router with telnet. The username is <tt>root</tt> and the password is the router password.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>If you using the router in an untrusted environment (for example as a public hotspot), it is strongly recommended to use SSHd and deactivate telnet.</div>
					</div>
				</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings or click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HSetup.asp">Setup</a></li>
				<li><a href="HManagement.asp">Management</a></li>
			</ul>
		</div>
	</body>
</html>
