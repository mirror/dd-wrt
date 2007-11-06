<% do_hpagehead("wol.titl"); %> 
   <body> 
      <div id="header"> 
         <div class="logo"> </div> 
         <div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div> 
      </div> 
      <div id="content"> 
         <h2><% tran("wol.h2"); %></h2> 
         <dl> 
            <dd>Wake-on-LANは、この機器のLAN側ネットワーク内のホストを遠隔地から起動するための機能です。</dd> 
               <dt class="term">検出ホスト</dt> 
               <dd class="definition">LAN側にあるホストを表示します。これらのListはDHCPサーバに管理されているもの、手動で追加されたものを含みます。
               <div class="note"> 
                  <h4>メモ</h4> 
                     <div>これらのMACアドレスは、LAN側ネットワークと同一ネットワークアドレス・マスクに含まれることを仮定しています。また、起動時に用いるのはUDPパケットです(指定しなかった場合、ポート番号は7を使用します)</div> 
               </div> 
            </dd> 
               <dt class="term"><% tran("wol.legend2"); %></dt> 
               <dd class="definition">実際にWOLを実行するために登録されたリストです(このリストは電源を切っても失われません)。このリストは自動的に登録されたものと手動登録されたものの両方が表示されます。
               <dt class="term"><% tran("wol.legend4"); %></dt> 
               <dd class="definition">
				「起動」ボタンをクリックすることによって、スリープ中のリスト内のホストを起動させることができます。
               <dt class="term"><% tran("wol.mac"); %></dt> 
               <dd class="definition">MACアドレスを列挙(アドレス間はスペースで区切る必要があります)することによって、これらの機器を一度に起動させることができます。

               <div class="note"> 
                  <h4>メモ</h4> 
                     <div>いずれのMACアドレスも次のフォーマットで入力する必要があります：xx:xx:xx:xx:xx:xx。</div> 
                  </div> 
               </dd> 
               <dt class="term"><% tran("wol.broadcast"); %></dt> 
                      <dd class="definition">MACアドレスに対応するブロードキャストIPを設定します</dd> 
               <dt class="term"><% tran("wol.udp"); %></dt> 
               <dd class="definition">送信するブロードキャストパケットのUDPポートを指定します。</dd> 
               <dt class="term"><% tran("sbutton.wol"); %></dt> 
               <dd class="definition">手動で起動を行います。起動時に使用したパラメータは<b>manual_wol_mac</b>, <b>manual_wol_network</b>, <b>manual_wol_port</b>の各NVRAMパラメータに保存されます。</dd> 
         </dl> 
      </div> 
   </body> 
</html> 

