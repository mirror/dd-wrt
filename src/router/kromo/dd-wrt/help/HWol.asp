<% do_hpagehead("wol.titl"); %> 
   <body> 
      <div id="header"> 
         <div class="logo"> </div> 
         <div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div> 
      </div> 
      <div id="content"> 
         <h2><% tran("wol.h2"); %></h2> 
         <dl> 
		<% tran("hwol.page1"); %>
               <dt class="term"><% tran("wol.legend2"); %></dt> 
		<% tran("hwol.page2"); %>
               <dt class="term"><% tran("wol.legend4"); %></dt> 
		<% tran("hwol.page3"); %>

               <dt class="term"><% tran("wol.mac"); %></dt> 
		<% tran("hwol.page4"); %>
               <dt class="term"><% tran("wol.broadcast"); %></dt> 
		<% tran("hwol.page5"); %>
               <dt class="term"><% tran("wol.udp"); %></dt> 
		<% tran("hwol.page6"); %>
               <dt class="term"><% tran("sbutton.wol"); %></dt> 
		<% tran("hwol.page7"); %>
         </dl> 
      </div> 
   </body> 
<% footer(); %></html> 

