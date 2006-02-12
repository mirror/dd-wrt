<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Port Range Forwarding</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Port Range Forwarding</h2>
			<dl>
				<dd>Port Range Forwarding allows you to set up public services on your network, such as web servers, ftp servers, e-mail servers, or other specialized Internet applications. Specialized Internet applications are any applications that use Internet access to perform functions such as videoconferencing or online gaming. When users send this type of request to your network via the Internet, the router will forward those requests to the appropriate PC.<br /><br />
					If you only want to forward a single port, see <a href="HForward.asp">Port Forwarding</a>.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Any PC whose port is being forwarded must have a static IP address assigned to it because its IP address may change when using the DHCP function.</div>
					</div>
				</dd>
				<dd>To add a new Port Range Forwarding rule, click <i>Add</i> and fill in the fields below. To remove the last rule, click <i>Delete</i>.</dd>
				<dt>Application</dt>
				<dd>Enter the name of the application in the field provided.</dd>
				<dt>Start</dt>
				<dd>Enter the number of the first port of the range you want to seen by users on the Internet and forwarded to your PC.</dd>
				<dt>End</dt>
				<dd>Enter the number of the last port of the range you want to seen by users on the Internet and forwarded to your PC.</dd>
				<dt>Protocol</dt>
				<dd>Chose the right protocol <i>TCP</i>, <i>UDP</i> or <i>Both</i>. Set this to what the application requires.</dd>
				<dt>IP Address</dt>
				<dd>Enter the IP Address of the PC running the application.</dd>
				<dt>Enable</td>
				<dd>Click the <i>Enable</i> checkbox to enable port forwarding for the application.</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click the <i>Cancel Changes</i> button to cancel your unsaved changes.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Remember to save your changes before adding another forwarding rule.</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HForward.asp">Port Forwarding</a></li>
				<li><a href="HTrigger.asp">Port Triggering</a></li>
				<li><a href="HDMZ.asp">DMZ</a></li>
			</ul>
		</div>
	</body>
</html>
