<% do_hpagehead("radius.legend"); %>
	<body>
	<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("radius.h2"); %></h2>
			<dl>
				<dd>RADIUS (Remote Authentication Dial-In User Service)¤Ï¡¢¥Í¥Ã¥È¥ï¡¼¥¯¾å¤Îµ¡´ï¤ËÂÐ¤·¤ÆÇ§¾Ú¡¦¾µÇ§¤ò¹Ô¤¦¤¿¤á¤Î¥×¥í¥È¥³¥ë¤Ç¤¹¡£°ìÈÌÅª¤Ê´ë¶È¥Í¥Ã¥È¥ï¡¼¥¯¤Ë¤ÏÂçÄñ¥À¥¤¥ä¥ë¥¢¥Ã¥×Åù¤ÎÍÑÅÓ¸þ¤±¤ËRAIDUS¥µ¡¼¥Ð¤ò»ý¤Á¡¢¥æ¡¼¥¶Ç§¾Ú¤ò¹Ô¤¤¤Þ¤¹¡£¤³¤ÎÀ½ÉÊ¤Ë¤ª¤¤¤Æ¤â¡¢ÌµÀþLAN¤ËÀÜÂ³¤ò¹Ô¤¦¥æ¡¼¥¶¤ËÂÐ¤·¡¢Ç§¾Ú¤ò¹Ô¤¦¤³¤È¤¬¤Ç¤­¤Þ¤¹¡£<br /><br />
					<div class="note">
						<h4>¥á¥â</h4>
						<div>RADIUSµ¡Ç½¤ÏAP¥â¡¼¥ÉÀßÄê»þ¤Î¤ßÍøÍÑ¤Ç¤­¤Þ¤¹¡£</div>

					</div>
				</dd>
				
				<dt><% tran("radius.label2"); %></dt>
				<dd>RADIUS¥µ¡¼¥Ð¤ËÇ§¾ÚÍ×µá¤ò¹Ô¤Ã¤¿¤È¤­¡¢ÌµÀþLAN¥¯¥é¥¤¥¢¥ó¥È¤ÎMAC¥¢¥É¥ì¥¹¤òÍÑ¤¤¤ÆÇ§¾ÚÍ×µá¤ò¹Ô¤¤¤Þ¤¹¡£RADIUS¥µ¡¼¥Ð¤Î¥æ¡¼¥¶Ì¾¤Ï¼¡¤Î¥Õ¥©¡¼¥Þ¥Ã¥È¤Î¤¤¤º¤ì¤«¤Ç¤¢¤ëÉ¬Í×¤¬¤¢¤ê¤Þ¤¹¡£
					<ul class="wide">
						<li>aabbcc-ddeeff</li>
						<li>aabbccddeeff</li>
						<li>aa-bb-cc-dd-ee-ff</li>
					</ul>
				</dd>				
				
				<dt><% tran("radius.label3"); %> - <% tran("radius.label4"); %></dt>
				<dd>RAIDUS¥µ¡¼¥Ð¤ÎIP¥¢¥É¥ì¥¹¤ÈUDP¥Ý¡¼¥È¤òÀßÄê¤·¤Þ¤¹¡£</dd>
				
				<dt><% tran("radius.label5"); %></dt>
				<dd>Ç§¾ÚÌµ¤·¤Ç¥¢¥¯¥»¥¹²ÄÇ½¤Ê¥æ¡¼¥¶¤Î¿ô¤òÀßÄê¤·¤Þ¤¹</dd>
				
				<dt><% tran("radius.label6"); %></dt>
				<dd>RADIUS¥µ¡¼¥Ð¤ËÇ§¾Ú¤ò¹Ô¤¦¾ì¹ç¤Î¥Ñ¥¹¥ï¡¼¥É¤ò»ØÄê¤·¤Þ¤¹¡£Shared Secret¤«¡¢¥æ¡¼¥¶Ì¾¤ÈÆ±¤¸MAC¥¢¥É¥ì¥¹¥Õ¥©¡¼¥Þ¥Ã¥È¤òÍÑ¤¤¤ë¤³¤È¤â¤Ç¤­¤Þ¤¹¡£</dd>
				
				<dt><% tran("radius.label7"); %></dt>
				<dd>RADIUS¥µ¡¼¥Ð¤È¤ÎÄÌ¿®¤Ë»ÈÍÑ¤µ¤ì¤ë¶¦Í­¥­¡¼¤òÀßÄê¤·¤Þ¤¹¡£RADIUS¥µ¡¼¥Ð¤ÈÆ±ÍÍ¤Î¶¦Í­¥­¡¼¤òÀßÄê¤¹¤ëÉ¬Í×¤¬¤¢¤ê¤Þ¤¹¡£</dd>
				
				<dt><% tran("radius.label8"); %></dt>
				<dd>RADIUS¥µ¡¼¥Ð¤ÈÄÌ¿®¤Ç¤­¤Ê¤¯¤Ê¤Ã¤¿»þ¤Ë¡¢ÄÌ¿®¤Ç¤­¤ë¤è¤¦¤Ë¤Ê¤ë¤Þ¤ÇÇ§¾Ú¤òÄä»ß¤·¤Þ¤¹¡£¤³¤ÎÀßÄê¤Ï¤½¤¦¤¤¤Ã¤¿¾ì¹ç¤Ë¡¢ÄÌ¿®¤¬²ÄÇ½¤È¤¹¤ë¤³¤È¤¬¤Ç¤­¤Þ¤¹</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
				<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
				<li><a href="HWirelessAdvanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
			</ul>
		</div>
	</body>
</html>