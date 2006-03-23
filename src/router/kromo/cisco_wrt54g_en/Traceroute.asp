<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>   
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %> - Traceroute Test</title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
function to_submit(F,I) {
	if(valid(F,I)){
		F.submit_type.value = I;
		F.submit_button.value = "Traceroute";
		F.change_action.value = "gozila_cgi";
		F.submit();
	}
}
function valid(F,I) {
	if(I == "start" && F.traceroute_ip.value == ""){
		alert("You must input an IP Address or Domain Name!");
		F.traceroute_ip.focus();
		return false;
	}

	return true;
}
var value=0;
function Refresh() {
	refresh_time = 5000;
	if (value>=1) {
		window.location.replace("Traceroute.asp");
	}
	value++;
	timerID = setTimeout("Refresh()",refresh_time);
}
function init() {
	window.location.href = "#";
	<% onload("Traceroute", "Refresh();"); %>
}
function exit() {
	//if(!confirm("Do you want to stop traceroute?"))
	//	self.close();
	//else{
		to_submit(document.traceroute,"stop");
		self.close();
	//}
}
</script></head>
   <body onload="init();">
      <form name="setup" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" value="Traceroute" /><input type="hidden" name="submit_type" value="start" /><input type="hidden" name="change_action" value="gozila_cgi" /><input type="hidden" name="action" value="Apply" /><h2>Traceroute Test</h2>
         <table>
            <tr>
               <th>IP Address or Domain Name:</th>
               <td><input maxLength="31" size="31" name="traceroute_ip" onBlur="valid_name(this,'IP')" value='<% nvram_selget("traceroute_ip"); %>' /></td>
               <td><input type="button" value="Traceroute" name="Traceroute" onclick="to_submit(this.form,'start');" /></td>
            </tr>
            <tr>
               <td colspan="3" class="output"><script language="JavaScript">
var table = new Array(
<% dump_traceroute_log(""); %>
);
	var i = 0;
	for(;;){
		if(!table[i]) {
			break;
		}
		document.write(table[i] + "<br/>");
		i = i + 1;
   }
</script></td>
            </tr>
            <tr>
               <td colspan="3"><input type="button" onclick="to_submit(this.form,'stop');" value="Stop" /><input type="button" onclick="to_submit(this.form,'clear');" value="Clear Log" /><input type="button" onclick="exit()" value="Close" /></td>
            </tr>
         </table>
      </form>
   </body>
</html>