<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Routing Table</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>
		<script type="text/javascript">

setMetaContent();
document.title = "<% nvram_get("router_name"); %>" + routetbl.titl;

		</script>
	</head>

	<body>
		<form>
			<h2><script type="text/javascript">Capture(routetbl.h2)</script></h2>
			<table>
				<tr>
					<th><script type="text/javascript">Capture(routetbl.th1)</script></th>
					<th><script type="text/javascript">Capture(share.subnet)</script></th>
					<th><script type="text/javascript">Capture(share.gateway)</script></th>
					<th><script type="text/javascript">Capture(share.intrface)</script></th>
				</tr>
				<script language="JavaScript">
					var table = new Array(<% dump_route_table(""); %>);
					
					if(table.length == 0) {
						document.write("<tr><td align=\"center\" colspan=\"4\">- " + share.none + " -</td></tr>");
					} else {
						for(var i = 0; i < table.length; i = i+4) {
							if(table[i+3] == "LAN")
								table[i+3] = "LAN &amp; WLAN";
							else if(table[i+3] == "WAN")
								table[i+3] = "WAN";
							document.write("<tr><td>"+table[i]+"</td><td>"+table[i+1]+"</td><td>"+table[i+2]+"</td><td>"+table[i+3]+"</td></tr>");
						}
					}
				</script>
			</table><br />
			<div class="submitFooter">
				<script type="text/javascript">document.write("<input type=\"button\" name=\"button\" value=\"" + sbutton.refres + "\" onclick=\"window.location.reload()\" />")</script>
				<script type="text/javascript">document.write("<input type=\"reset\" value=\"" + sbutton.clos + "\" onclick=\"self.close()\" />")</script>
			</div>
		</form>
	</body>
</html>