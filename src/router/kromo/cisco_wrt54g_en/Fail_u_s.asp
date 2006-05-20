<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %></title>
<!--	<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />	-->
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
<!--	<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript" src="lang_pack/english.js"></script>
		<script type="text/javascript" src="lang_pack/language.js"></script>		-->
		
<!-- Botho 22/04 : css_include() and js_include() correct in a temporary way the loss of style and javascript includes -->

		<style type="text/css">
<% css_include(); %>
		</style>
		<script type="text/javascript">
<% js_include(); %>
		
function to_submit()
{
	document.location.href = "Upgrade.asp";
}

		</script>
	</head>

   <body>
      <div class="message">
         <div>
            <form>
            	<script type="text/javascript">Capture(fail.mess2)</script><br />
				<script type="text/javascript">document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit()\" />")</script>
			</form>
         </div>
      </div>
   </body>
</html>