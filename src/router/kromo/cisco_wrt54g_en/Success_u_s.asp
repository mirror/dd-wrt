<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %></title>

<!-- Botho 22/04 : css_include() and js_include() correct in a temporary way the loss of style and javascript includes -->
		<style type="text/css">
<% css_include(); %>
		</style>
		<script type="text/javascript"><![CDATA[
<% js_include(); %>

var clk = <% get_clkfreq(); %>;
var rest_default = <% nvram_get("sv_restore_defaults"); %>;
my_tab = new getTimeOut(clk, rest_default, <% getrebootflags(); %>);
var submit_button = "<% get_web_page_name(); %>";
var timer = setTimeout("message()", my_tab.wait_time * 1000);
var browserName=navigator.appName;

function to_submit()
{
	if(submit_button == ".asp")
		history.go(-1);
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

		]]></script>
	</head>
	
	<body onload="init()" onunload="clearTimeout(timer)">
		<div class="message">
			<div>
				<form name="success">
					<% tran("success.upgrade"); %><br /><br />
					<div align="center">
						<script type="text/javascript"><![CDATA[
							var bar1 = createBar(500,15,100,15,my_tab.scroll_count,"to_submit()");
							bar1.togglePause();
						]]></script>
	            	</div>
            		<div id="mess" style="display:none"><br /><br />
	            		<div style="text-align:left">
	            			<script type="text/javascript"><![CDATA[
	            				if (rest_default == 1) {
	            					Capture(success.alert_reset);
	            				}
	            				Capture(success.alert1);
	            			]]></script>
	            			<ul>
	            				<li><% tran("success.alert2"); %></li>
	            				<li><% tran("success.alert3"); %></li>
	            			</ul>
	            		</div>
		            	<script type="text/javascript"><![CDATA[
							document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit()\" />");
							if (browserName == "Microsoft Internet Explorer")
								document.write("<input type=\"button\" value=\"" + sbutton.clos + "\" onclick=\"opener=self;self.close();\" />");
						]]></script>
	            	</div>
				</form>
			</div>
		</div>
   </body>
</html>