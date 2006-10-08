<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %> - Join</title>
		<script type="text/javascript">
//<![CDATA[

document.title = "<% nvram_get("router_name"); %>" + join.titl;

var SSID = "<% nvram_get("wl_ssid"); %>";

function to_send(url)
{
	opener.focus();
	opener.location.href = url;
}
      
//]]>
</script>
    </head>

    <body onunload="to_send('Wireless_Basic.asp')">
      <div class="message">
         <div>
            <form>
            	<script type="text/javascript">
//<![CDATA[

document.write(join.mess1 + "&nbsp;" + SSID);

//]]>
</script><br/>
            	<script type="text/javascript">
//<![CDATA[

document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"self.close()\" />");

//]]>
</script>
            </form>
         </div>
      </div>
   </body>
</html>