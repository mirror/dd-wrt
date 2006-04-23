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

//var wait_time = <% webs_get("wait_time"); %> * 1000;		//milliseconds => seconds
//var scroll_count = <% nvram_get("scroll_count"); %>;
var wait_time = 40 * 1000;
var scroll_count = 5;
var submit_button = "<% get_web_page_name(); %>";
var timer = setTimeout("message()", wait_time);
var browserName=navigator.appName;

function to_submit()
{
	if(submit_button == "")
		history.go(-1);
	else if(submit_button == "WL_WEPTable.asp")
		self.close();
	else
		document.location.href =  submit_button;
}

function message()
{
	clearTimeout(timer);
	bar1.togglePause();
	setElementVisible("mess", true);
	if (browserName == "Microsoft Internet Explorer") {
		document.execCommand("Stop");
	}
	else {
		window.stop();
	}
}

function init()
{
	bar1.togglePause();
}

		</script>
	</head>
	
	<body onload="init()" onunload="clearTimeout(timer)">
		<div class="message">
			<div>
				<form name="success">
					<script type="text/javascript">Capture(success.mess2)</script><br /><br />
					<div align="center">
	            		<script type="text/javascript">
	            			var bar1 = createBar(500,15,100,15,scroll_count,"to_submit()");
	            			bar1.togglePause();
	            		</script>
	            	</div>
            		<div id="mess" style="display: none">
            			<br /><br /><script type="text/javascript">Capture(success.mess5)</script><br /><br />
            			<script type="text/javascript">
            				if (browserName == "Microsoft Internet Explorer")
            					document.write("<input type=\"button\" value=\"" + sbutton.clos + "\" onclick=\"opener=self;self.close();\" />");
           				</script>
            		</div>
<!--			<script type="text/javascript">document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit()\" />")</script> -->
				</form>
			</div>
		</div>
   </body>
</html>