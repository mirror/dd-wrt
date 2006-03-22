<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
      <title><% nvram_get("router_name"); %> - Outgoing Log Table</title>
      <link type="text/css" rel="stylesheet" href="style.css" /><script type="text/JavaScript" src="common.js">{}</script></head>
   <body onload="window.focus()">
      <div class="popup">
         <form>
            <h2>Outgoing Log Table</h2>
            <table class="table">
               <tr>
                  <th>LAN IP</th>
                  <th>Destination URL/IP</th>
                  <th>Service/Port Number</th>
               </tr><% dumplog("outgoing"); %>
            </table><input onclick=self.close() type="reset" value="Close" /><input type="button" onclick="window.location.reload()" value=" Refresh " /></form>
      </div>
   </body>
</html>