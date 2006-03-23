<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
		<title><% nvram_get("router_name"); %></title>
		<link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style.css" />
		<!--[if IE]><link type="text/css" rel="stylesheet" href="style/<% nvram_get("router_style"); %>/style_ie.css" /><![endif]-->
		<script type="text/javascript" src="common.js"></script>
		<script type="text/javascript">
var submit_button = '<% get_web_page_name(); %>';

function to_send(url)
{
	opener.focus();
	opener.location.href = url;
}
      </script>
    </head>

    <body onunload="to_send('Wireless_Basic.asp')">
      <div class="message">
         <div>
            <form>Successfully joined the network as a client.<br/>
              <input type="button" name="action" value="Continue" OnClick="self.close()"/>
            </form>
         </div>
      </div>
   </body>
</html>