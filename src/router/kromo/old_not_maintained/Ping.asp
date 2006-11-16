<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Command Shell</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F,I) {
	if(valid(F,I)){
		F.submit_type.value = I;
		F.submit_button.value = "Ping";
		F.change_action.value = "gozila_cgi";
		F.firewall.value = "Saved";
		F.firewall.disabled = true;
		F.submit();
	}
}

function valid(F,I) {
	if(I == "start" && F.ping_ip.value == ""){
		alert("You must input a command to run!");
		F.ping_ip.focus();
		return false;
	}
	return true;
}

var value=0;

function Refresh() {
	refresh_time = 5000;
	if (value>=1) {
		window.location.replace("SysInfo.htm");
	}
	value++;
	timerID=setTimeout("Refresh()",refresh_time);
}

function init() {
	window.location.href = "#";
}

function exit() {
	//if(!confirm("Do you want to stop ping?"))
	//	self.close();
	//else{
		to_submit(document.ping,"stop");
		self.close();
	//}
}

		</script>
	</head>
	
	<body>
      <div class="popup">
	  	<form name="ping" action="apply.cgi">
			<input type="hidden" name="submit_button" value="Ping"/>
			<input type="hidden" name="submit_type" value="start"/>
			<input type="hidden" name="action" value="Apply"/>
			<input type="hidden" name="change_action" value="gozila_cgi"/>
			<input type="hidden" name="ping_times" value="1"/>
			<h2>Command Shell</h2>
            <h3>Commands</h3>
				<textarea style="width: 80%;" id="ping_ip" name="ping_ip" rows="10" maxlength="255"></textarea>
				<br/>
				<input type="button" name="ping" value="Cmd" onclick="to_submit(this.form,'start');"/>
				<table style="border: 1px solid #000; width: 80%;">
					<tr>
						<td><pre style="font-size: 1.5em;">
<script language="JavaScript">
var table = new Array(
<% dump_ping_log(""); %>
);
for(var i = 0; i < table.length; i++){
		document.writeln(table[i]);
}
</script></pre>
						</td>
					</tr>
				</table>
				<div class="submitFooterCmd">
					<input type="button" onclick="to_submit(this.form,'startup');" value="Save Startup" name="startup"/>
					<input type="button" onclick="to_submit(this.form,'firewall');" value="Save Firewall" name="firewall"/>
					<input type="button" onclick="exit()" value="Close"/>
				</div>
         </form>
      </div>
   </body>
</html>