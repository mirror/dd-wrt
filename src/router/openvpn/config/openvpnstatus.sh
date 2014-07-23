#!/bin/sh
if [ "$(nvram get openvpn_enable)" = "1" ]; then
/bin/echo "status 2" | /usr/bin/nc 127.0.0.1 14  | grep "bytes" | awk -F "," 'BEGIN{print "<table><tr><td colspan=2>VPN Server Stats<hr></td></tr>"}{             
                                                                          
        printf "<tr>\n<td>%s</td><td>%d</td>\n</tr>", $1, $2;            
                                                         
}                                                        
END{print "</table>"}'
fi
if [ "$(nvram get openvpncl_enable)" = "1" ]; then
/bin/echo "status 2" | /usr/bin/nc 127.0.0.1 16  | grep "bytes" | awk -F "," 'BEGIN{print "<table><tr><td colspan=2>VPN Client Stats<hr></td></tr>"}{             
                                                                          
        printf "<tr>\n<td>%s</td><td>%d</td>\n</tr>", $1, $2;            
                                                         
}                                                        
END{print "</table>"}'
fi

