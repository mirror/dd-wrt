<% do_pagehead(); %>
		<title><% nvram_get("router_name"); %></title>

<!-- Botho 22/04 : css_include() and js_include() correct in a temporary way the loss of style and javascript includes -->

		<style type="text/css">
<% css_include(); %>
		</style>
		<script type="text/javascript">//<![CDATA[
<% js_include(); %>
		
var submit_button = "<% get_web_page_name(); %>";
function to_submit()
{
	if(submit_button == ".asp")
		history.go(-1);
	else if(submit_button == "WL_WEPTable.asp")
		self.close();
	else
		document.location.href =  submit_button;
}
		//]]></script>
	</head>

   <body>
      <div class="message">
         <div>
            <form>
            	<% tran("fail.mess1"); %><br />
            	<script type="text/javascript">//<![CDATA[
document.write("<input type=\"button\" name=\"action\" value=\"" + sbutton.continu + "\" onclick=\"to_submit()\" />");
//]]></script>
            </form>
         </div>
      </div>
   </body>
</html>