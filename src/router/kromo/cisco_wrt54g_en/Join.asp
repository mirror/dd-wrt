<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
   <head>
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"/>
      <link type="text/css" rel="stylesheet" href="style.css"/>
      <script type="text/JavaScript" src="common.js">{}</script>
      <script language="JavaScript">

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