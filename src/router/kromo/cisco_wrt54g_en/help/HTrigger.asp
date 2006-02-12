<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Port Triggering</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Port Triggering</h2>
			<dl>
				<dd>Port Triggering allows you to do port forwarding without setting a fixed PC. By setting Port Triggering rules, you can allow inbound traffic to arrive at a specific LAN host, using ports different than those used for the outbound traffic. This is called port triggering since the outbound traffic triggers to which ports inbound traffic is directed.<br /><br />
					If you want to forward ports to a PC with a static IP address, see <a href="HForward.asp">Port Forwarding</a> or <a href="Forward_range.asp">Port Range Forwarding</a>.</dd>
				<dd>To add a new Port Triggering rule, click <i>Add</i> and fill in the fields below. To remove the last rule, click <i>Delete</i>.</dd>
				<dt>Application</dt>
				<dd>Enter the name of the application in the field provided.</dd>
				<dt>Triggered Range</dt>
				<dd>Enter the number of the first and the last port of the range, which should be triggered. If a PC sends outbound traffic from those ports, incoming traffic on the <i>Forwarded Range</i> will be forwarded to that PC.</dd>
				<dt>Forwarded Range</dt>
				<dd>Enter the number of the first and the last port of the range, which should be forwareded from the Internet to the PC, which has triggered the <i>Triggered Range</i>.</dd>
				<dt>Enable</td>
				<dd>Click the <i>Enable</i> checkbox to enable port triggering for the application.</dd>
				<dd>Check all values and click <i>Save Settings</i> to save your settings. Click the <i>Cancel Changes</i> button to cancel your unsaved changes.<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Remember to save your changes before adding another triggering rule.</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HForward.asp">Port Forwarding</a></li>
				<li><a href="HForwardRange.asp">Port Range Forwarding</a></li>
				<li><a href="HDMZ.asp">DMZ</a></li>
			</ul>
		</div>
	</body>
</html>
