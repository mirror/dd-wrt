<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Incoming Log Table</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
	</head>
	<body>
		<div class="popup">
			<form>
				<h2>Incoming Log Table</h2>
				<table class="table">
					<tr>
						<th>Source IP</th>
						<th>Destination Port Number</th>
					</tr>
					<% dumplog("incoming"); %>
				</table><br />
				<div class="submitFooter">
					<input type="button" value="Close" onclick="self.close()" />
					<input type="button" value="Refresh" onclick="window.location.reload()" />
				</div>
			</form>
		</div>
	</body>
</html>