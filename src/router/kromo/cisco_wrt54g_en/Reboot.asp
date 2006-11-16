<% do_pagehead(); %>

		<script type="text/javascript">
		//<![CDATA[

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
		
		//]]>
		</script>
	</head>
	
	<body onload="init()" onunload="clearTimeout(timer)">
		<div class="message">
			<div>
			<form>
				<% tran("success.success_reboot"); %><br /><br />
					<div align="center">
						<script type="text/javascript">
						//<![CDATA[
							var bar1 = createBar(500,15,100,15,my_tab.scroll_count,"to_submit()");
	            bar1.togglePause();
	            //]]>
	            </script>
	        </div>
	        <div id="mess" style="display:none"><br /><br />
	        	<div style="text-align:left"><% tran("success.alert1"); %>
	        		<ul>
	        			<li><% tran("success.alert2"); %></li>
	        			<li><% tran("success.alert3"); %></li>
	        		</ul>
	        	</div>
	        	<script type="text/javascript">
	        	//<![CDATA[
	        		document.write("<input class=\"button\" type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit();\" />");
	        		if (browserName == "Microsoft Internet Explorer")
	        			document.write("<input class=\"button\" type=\"button\" name=\"close_button\" value=\"" + sbutton.clos + "\" onclick=\"opener=self;self.close();\" />");
	        	//]]>
	        	</script>
	        </div>
				</form>
			</div>
		</div>
   </body>
</html>