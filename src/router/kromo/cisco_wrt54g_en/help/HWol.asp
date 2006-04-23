<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 

<html> 
   <head> 
      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1"> 
      <link type="text/css" rel="stylesheet" href="/help/help.css"> 
      <title>Help - Local WOL</title> 
   </head> 
   <body> 
      <div id="header"> 
         <div class="logo"> </div> 
         <div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div> 
      </div> 
      <div id="content"> 
         <h2>Local WOL</h2> 
         <dl> 
            <dd>This page allows you to <em>Wake Up</em> hosts on your local network (i.e. locally connected to your WRT).</dd> 
               <dt class="term">Static Leases: </dt> 
               <dd class="definition">The Static Leases section attempts to <em>Wake Up</em> any host that you have a static DHCP lease for. 
               <div class="note"> 
                  <h4>Note</h4> 
                     <div>The attempt uses the MAC address, &quot;guesses&quot; the network broadcast address by assuming the host's IP address has a 255.255.255.0 netmask, and uses the UDP port specified in the UDP Port box.</div> 
               </div> 
            </dd> 
               <dt class="term">Mac Address: </dt> 
               <dd class="definition">Fill the MAC address(es) of the computer you would like to wake up and click <em>Wake Up</em> to send it the WOL <i>magic packet</i>. 
               <div class="note"> 
                  <h4>Note</h4> 
                     <div>Each MAC-ADDRESS is written as xx:xx:xx:xx:xx:xx, where xx is a hexadecimal number between 00 and ff which represents one byte of the address, which is in network byte order (big endian).</div> 
                  </div> 
               </dd> 
               <dt class="term">Network Broadcast: </dt> 
                      <dd class="definition">Broadcast to this IP address or hostname (typically you would want to make this your network's broadcast IP for locally waking up hosts.</dd> 
               <dt class="term">UDP Port: </dt> 
               <dd class="definition">Broadcast to this UDP port.</dd> 
               <dt class="term">Wake Up: </dt> 
               <dd class="definition">Besides attempting to <i>Wake Up</i> the specified hosts, clicking on the &quot;Wake Up&quot; button will save the MAC Address(es), Network Broadcast, and UDP Port values into the <b>local_wol_mac</b>, <b>local_wol_network</b>, and <b>local_wol_port</b> nvram variables and commits them to memory.</dd> 
         </dl> 
      </div> 
   </body> 
</html> 

