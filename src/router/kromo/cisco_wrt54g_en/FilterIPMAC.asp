<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
   
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - List of PCs</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script><script language="JavaScript">
function to_submit(F) {
	F.submit_button.value = "FilterIPMAC";
	F.action.value = "Apply";
	F.save_button.value = "Saved";
	F.save_button.disabled = true;

	F.submit();
}
function valid_macs_all(I) {
	if(I.value == "") {
		return true;
	}
		
	if(I.value.length == 12) {
		valid_macs_12(I);
	} else if(I.value.length == 17) {
		valid_macs_17(I);
	} else {
		alert('The MAC Address length is not correct!!');
		I.value = I.defaultValue;
	}
}
</script></head>
   <body>
      <form name="ipfilter" action="apply.cgi" method="<% get_http_method(); %>"><input type="hidden" name="submit_button" /><input type="hidden" name="change_action" /><input type="hidden" name="action" /><input type="hidden" name="small_screen" /><input type="hidden" name="filter_ip_value" /><input type="hidden" name="filter_mac_value" /><h2>List of PCs</h2>
         <div>
            <h3>Enter MAC Address of the PCs in this format: xxxxxxxxxxxx</h3>
            <div class="setting">
               <div class="label">MAC 01</div><input class="num" size="17" maxLength="17" name="mac0" onBlur=valid_macs_all(this) value='<% filter_mac_get(0); %>' /></div>
            <div class="setting">
               <div class="label">MAC 02</div><input class="num" size="17" maxLength="17" name="mac1" onBlur=valid_macs_all(this) value='<% filter_mac_get(1); %>' /></div>
            <div class="setting">
               <div class="label">MAC 03</div><input class="num" size="17" maxLength="17" name="mac2" onBlur=valid_macs_all(this) value='<% filter_mac_get(2); %>' /></div>
            <div class="setting">
               <div class="label">MAC 04</div><input class="num" size="17" maxLength="17" name="mac3" onBlur=valid_macs_all(this) value='<% filter_mac_get(3); %>' /></div>
            <div class="setting">
               <div class="label">MAC 05</div><input class="num" size="17" maxLength="17" name="mac4" onBlur=valid_macs_all(this) value='<% filter_mac_get(4); %>' /></div>
            <div class="setting">
               <div class="label">MAC 06</div><input class="num" size="17" maxLength="17" name="mac5" onBlur=valid_macs_all(this) value='<% filter_mac_get(5); %>' /></div>
            <div class="setting">
               <div class="label">MAC 07</div><input class="num" size="17" maxLength="17" name="mac6" onBlur=valid_macs_all(this) value='<% filter_mac_get(6); %>' /></div>
            <div class="setting">
               <div class="label">MAC 08</div><input class="num" size="17" maxLength="17" name="mac7" onBlur=valid_macs_all(this) value='<% filter_mac_get(7); %>' /></div>
         </div><br /><div>
            <h3>Enter the IP Address of the PCs</h3>
            <div class="setting">
               <div class="label">IP 01</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip0" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",0); %>' /></div>
            <div class="setting">
               <div class="label">IP 02</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip1" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",1); %>' /></div>
            <div class="setting">
               <div class="label">IP 03</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip2" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",2); %>' /></div>
            <div class="setting">
               <div class="label">IP 04</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip3" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",3); %>' /></div>
            <div class="setting">
               <div class="label">IP 05</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip4" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",4); %>' /></div>
            <div class="setting">
               <div class="label">IP 06</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip5" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip",5); %>' /></div>
         </div><br /><div>
            <h3>Enter the IP Range of the PCs</h3>
            <div class="setting">
               <div class="label">IP Range 01</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip_range0_0" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip_range0_0",6); %>' />~<input class="num" size="3" maxLength="3" name="ip_range0_1" onBlur="onBlur=valid_range(this,0,254,'IP')" value='<% filter_ip_get("ip_range0_1",6); %>' /></div>
            <div class="setting">
               <div class="label">IP Range 02</div><% prefix_ip_get("lan_ipaddr",1); %><input class="num" size="3" maxLength="3" name="ip_range1_0" onBlur=valid_range(this,0,254,'IP') value='<% filter_ip_get("ip_range1_0",7); %>' />~<input class="num" size="3" maxLength="3" name="ip_range1_1" onBlur="onBlur=valid_range(this,0,254,'IP')" value='<% filter_ip_get("ip_range1_1",7); %>' /></div>
         </div><br /><div class="submitFooter"><input type="button" name="save_button" value="Save Settings" onClick=to_submit(this.form) /><input type="reset" value="Cancel Changes" /></div>
      </form>
   </body>
</html>