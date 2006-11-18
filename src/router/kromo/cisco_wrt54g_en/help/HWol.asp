<% do_hpagehead(); %> 
      <title>Help - Local WOL</title> 
   </head> 
   <body> 
      <div id="header"> 
         <div class="logo"> </div> 
         <div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div> 
      </div> 
      <div id="content"> 
         <h2><% tran("wol.h2"); %></h2> 
         <dl> 
            <dd>This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your WRT).</dd> 
               <dt class="term">Available Hosts:</dt> 
               <dd class="definition">The Available Hosts section provides a list of hosts to add/remove from the WOL Addresses list. The list is a combination of any defined static hosts or automatically discovered DHCP clients.
               <div class="note"> 
                  <h4>Note</h4> 
                     <div>This table uses the MAC address, &quot;guesses&quot; the network broadcast address by assuming the host's IP address has the same netmask as the local router (lan_netmask), and uses the UDP port specified in the UDP Port box (in the Manual WOL section -- default is 7 if nothing is specified).</div> 
               </div> 
            </dd> 
               <dt class="term"><% tran("wol.legend2"); %></dt> 
               <dd class="definition">The WOL Addresses section allows individual hosts in the WOL list (stored in the <b>wol_hosts</b> nvram variable) to be <em>Woken Up</em>.  The list is a combination of selected (enabled) Available Hosts and manually added WOL hosts. 

               <dt class="term"><% tran("wol.legend4"); %></dt> 
               <dd class="definition">The Manual WOL section allows individual or a list of hosts to be woken up by clicking <em>Wake Up</em> to send it the WOL <i>magic packet</i>. 

               <dt class="term"><% tran("wol.mac"); %></dt> 
               <dd class="definition">Fill the MAC address(es) (either separated by spaces or one per line) of the computer(s) you would like to wake up.
               <div class="note"> 
                  <h4>Note</h4> 
                     <div>Each MAC-ADDRESS is written as xx:xx:xx:xx:xx:xx, where xx is a hexadecimal number between 00 and ff which represents one byte of the address, which is in network byte order (big endian).</div> 
                  </div> 
               </dd> 
               <dt class="term"><% tran("wol.broadcast"); %></dt> 
                      <dd class="definition">Broadcast to this IP address or hostname (typically you would want to make this your network's broadcast IP for locally waking up hosts.</dd> 
               <dt class="term"><% tran("wol.udp"); %></dt> 
               <dd class="definition">Broadcast to this UDP port.</dd> 
               <dt class="term"><% tran("sbutton.wol"); %></dt> 
               <dd class="definition">Besides attempting to <i>Wake Up</i> the manually specified host(s), clicking on the &quot;Wake Up&quot; button will save the MAC Address(es), Network Broadcast, and UDP Port values into the <b>manual_wol_mac</b>, <b>manual_wol_network</b>, and <b>manual_wol_port</b> nvram variables and commits them to memory.</dd> 
         </dl> 
      </div> 
   </body> 
</html> 

