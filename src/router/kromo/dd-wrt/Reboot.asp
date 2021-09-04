<% do_pagehead(""); %>

<script type="text/javascript">
//<![CDATA[
var clk = <% get_clkfreq("1"); %>;
var rest_default = <% nvg("sv_restore_defaults"); %>;
var submit_button = "<% get_web_page_name(); %>";
var timer = setTimeout("message()", <% getboottime(); %> * 1000);
var browserName=navigator.appName;

function to_submit()
{
	if(submit_button == ".asp") {
		history.go(-1);
	} else {
		document.location.href =  submit_button;
	}
}
function to_apply()
{
	if(submit_button == ".asp") {
		history.go(-1);
	} else {
		document.location.href =  submit_button;
	}
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

addEvent(window, "load", function() {
	bar1.togglePause();
});

addEvent(window, "unload", function() {
	clearTimeout(timer);
});
		
//]]>
</script>
	</head>
	
	<body>
		<div class="message">
			<div>
			<form>
				<% tran("success.success_reboot"); %><br /><br />
				<div align="center">
					<script type="text/javascript">
					//<![CDATA[
						var bar1 = createBar(500,15,100,15,<% getboottime(); %> / 5 - 3,"to_submit()");
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
<% footer(); %></html>