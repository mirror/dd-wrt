<% do_hpagehead("wol.titl"); %> 
   <body> 
      <div id="header"> 
         <div class="logo"> </div> 
         <div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div> 
      </div> 
      <div id="content"> 
         <h2><% tran("wol.h2"); %></h2> 
         <dl> 
            <dd>Wake-on-LAN¤Ï¡¢¤³¤Îµ¡´ï¤ÎLANÂ¦¥Í¥Ã¥È¥ï¡¼¥¯Æâ¤Î¥Û¥¹¥È¤ò±ó³ÖÃÏ¤«¤éµ¯Æ°¤¹¤ë¤¿¤á¤Îµ¡Ç½¤Ç¤¹¡£</dd> 
               <dt class="term">¸¡½Ð¥Û¥¹¥È</dt> 
               <dd class="definition">LANÂ¦¤Ë¤¢¤ë¥Û¥¹¥È¤òÉ½¼¨¤·¤Þ¤¹¡£¤³¤ì¤é¤ÎList¤ÏDHCP¥µ¡¼¥Ð¤Ë´ÉÍý¤µ¤ì¤Æ¤¤¤ë¤â¤Î¡¢¼êÆ°¤ÇÄÉ²Ã¤µ¤ì¤¿¤â¤Î¤ò´Þ¤ß¤Þ¤¹¡£
               <div class="note"> 
                  <h4>¥á¥â</h4> 
                     <div>¤³¤ì¤é¤ÎMAC¥¢¥É¥ì¥¹¤Ï¡¢LANÂ¦¥Í¥Ã¥È¥ï¡¼¥¯¤ÈÆ±°ì¥Í¥Ã¥È¥ï¡¼¥¯¥¢¥É¥ì¥¹¡¦¥Þ¥¹¥¯¤Ë´Þ¤Þ¤ì¤ë¤³¤È¤ò²¾Äê¤·¤Æ¤¤¤Þ¤¹¡£¤Þ¤¿¡¢µ¯Æ°»þ¤ËÍÑ¤¤¤ë¤Î¤ÏUDP¥Ñ¥±¥Ã¥È¤Ç¤¹(»ØÄê¤·¤Ê¤«¤Ã¤¿¾ì¹ç¡¢¥Ý¡¼¥ÈÈÖ¹æ¤Ï7¤ò»ÈÍÑ¤·¤Þ¤¹)</div> 
               </div> 
            </dd> 
               <dt class="term"><% tran("wol.legend2"); %></dt> 
               <dd class="definition">¼ÂºÝ¤ËWOL¤ò¼Â¹Ô¤¹¤ë¤¿¤á¤ËÅÐÏ¿¤µ¤ì¤¿¥ê¥¹¥È¤Ç¤¹(¤³¤Î¥ê¥¹¥È¤ÏÅÅ¸»¤òÀÚ¤Ã¤Æ¤â¼º¤ï¤ì¤Þ¤»¤ó)¡£¤³¤Î¥ê¥¹¥È¤Ï¼«Æ°Åª¤ËÅÐÏ¿¤µ¤ì¤¿¤â¤Î¤È¼êÆ°ÅÐÏ¿¤µ¤ì¤¿¤â¤Î¤ÎÎ¾Êý¤¬É½¼¨¤µ¤ì¤Þ¤¹¡£
               <dt class="term"><% tran("wol.legend4"); %></dt> 
               <dd class="definition">
				¡Öµ¯Æ°¡×¥Ü¥¿¥ó¤ò¥¯¥ê¥Ã¥¯¤¹¤ë¤³¤È¤Ë¤è¤Ã¤Æ¡¢¥¹¥ê¡¼¥×Ãæ¤Î¥ê¥¹¥ÈÆâ¤Î¥Û¥¹¥È¤òµ¯Æ°¤µ¤»¤ë¤³¤È¤¬¤Ç¤­¤Þ¤¹¡£
               <dt class="term"><% tran("wol.mac"); %></dt> 
               <dd class="definition">MAC¥¢¥É¥ì¥¹¤òÎóµó(¥¢¥É¥ì¥¹´Ö¤Ï¥¹¥Ú¡¼¥¹¤Ç¶èÀÚ¤ëÉ¬Í×¤¬¤¢¤ê¤Þ¤¹)¤¹¤ë¤³¤È¤Ë¤è¤Ã¤Æ¡¢¤³¤ì¤é¤Îµ¡´ï¤ò°ìÅÙ¤Ëµ¯Æ°¤µ¤»¤ë¤³¤È¤¬¤Ç¤­¤Þ¤¹¡£

               <div class="note"> 
                  <h4>¥á¥â</h4> 
                     <div>¤¤¤º¤ì¤ÎMAC¥¢¥É¥ì¥¹¤â¼¡¤Î¥Õ¥©¡¼¥Þ¥Ã¥È¤ÇÆþÎÏ¤¹¤ëÉ¬Í×¤¬¤¢¤ê¤Þ¤¹¡§xx:xx:xx:xx:xx:xx¡£</div> 
                  </div> 
               </dd> 
               <dt class="term"><% tran("wol.broadcast"); %></dt> 
                      <dd class="definition">MAC¥¢¥É¥ì¥¹¤ËÂÐ±þ¤¹¤ë¥Ö¥í¡¼¥É¥­¥ã¥¹¥ÈIP¤òÀßÄê¤·¤Þ¤¹</dd> 
               <dt class="term"><% tran("wol.udp"); %></dt> 
               <dd class="definition">Á÷¿®¤¹¤ë¥Ö¥í¡¼¥É¥­¥ã¥¹¥È¥Ñ¥±¥Ã¥È¤ÎUDP¥Ý¡¼¥È¤ò»ØÄê¤·¤Þ¤¹¡£</dd> 
               <dt class="term"><% tran("sbutton.wol"); %></dt> 
               <dd class="definition">¼êÆ°¤Çµ¯Æ°¤ò¹Ô¤¤¤Þ¤¹¡£µ¯Æ°»þ¤Ë»ÈÍÑ¤·¤¿¥Ñ¥é¥á¡¼¥¿¤Ï<b>manual_wol_mac</b>, <b>manual_wol_network</b>, <b>manual_wol_port</b>¤Î³ÆNVRAM¥Ñ¥é¥á¡¼¥¿¤ËÊÝÂ¸¤µ¤ì¤Þ¤¹¡£</dd> 
         </dl> 
      </div> 
   </body> 
</html> 

