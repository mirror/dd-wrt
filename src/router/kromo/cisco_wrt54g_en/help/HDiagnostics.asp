<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1">
		<link type="text/css" rel="stylesheet" href="help.css">
		<title>Help - Command Shell</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2>Command Shell</h2>
			<dl>
				<dd>You are able to run command lines directly via the Webinterface. Just open the popup window by clicking <em>Run</em>.</dd>
				<dt>Commands</dt>
				<dd>Fill the text area with your command click <em>Cmd</em> to submit.</dd>				
				<dt>Save Startup</dt>
				<dd>You can save some command lines to be executed at startup's router. Fill the text area with commands (only one command by row) and click <em>Save Startup</em>.</dd>
				<dt>Save Firewall</dt>
				<dd>Each time the firewall is started, it can run some custom iptables instructions. Fill the text area with firewall's instructions (only one command by row) and click <em>Save Firewall</em>.<br/><br/>
					<div class="note">
						<h4>Note</h4>
						<div>
							<ul class="wide">
								<li>Startup commands are stored in nvram rc_startup variable</li>
								<li>Firewall commands are stored in nvram rc_firewall variable</li>
							</ul>
						</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4>See also</h4>
			<ul>
				<li><a href="HManagement.asp">Management</a></li>
			</ul>
		</div>
	</body>
</html>
