<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Outgoing Log Table</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">
		
document.title = "<% nvram_get("router_name"); %>" + log_out.titl;

		</script>
	</head>
	<body>
		<div class="popup">
			<form>
				<h2><script type="text/javascript">Capture(log_out.h2)</script></h2>
				<table class="table">
					<tr>
						<th><script type="text/javascript">Capture(log_out.th_lanip)</script></th>
						<th><script type="text/javascript">Capture(log_out.th_wanip)</script></th>
						<th><script type="text/javascript">Capture(share.proto)</script></th>
						<th><script type="text/javascript">Capture(log_out.th_port)</script></th>
						<th><script type="text/javascript">Capture(share.rule)</script></th>
					</tr>
					<% dumplog("outgoing"); %>
				</table><br />
				<div class="submitFooter">
					<script type="text/javascript">document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />")</script>
					<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />")</script>
				</div>
			</form>
		</div>
	</body>
</html>