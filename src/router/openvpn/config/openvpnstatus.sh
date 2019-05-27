#!/bin/sh
if [ "$(nvram get openvpn_enable)" = "1" ]; then
PORT=`grep "^management " /tmp/openvpn/openvpn.conf | awk '{print $3}'`
if [ x${PORT} = x ]
then
	PORT=14
fi
        echo -n "<table><tr><td colspan=5>VPN Server Stats: "
	# STATS
        /bin/echo "load-stats" | /usr/bin/nc 127.0.0.1 ${PORT} | grep SUCCESS | \
        awk -F " " '{print $2}'| awk -F "," '{print $1 ", " $2 ", " $3}'
        echo -e "<hr/></td></tr>\n"
        # CLIENT LIST
        /bin/echo "status 2" | /usr/bin/nc 127.0.0.1 ${PORT} | \
        awk '/HEADER,CLIENT_LIST/{printline = 1; next} /HEADER,ROUTING_TABLE/ {printline = 0} printline' | \
        awk -F "," 'BEGIN{print "<tr><th>Client</th><th>Remote IP:Port</th><th>Bytes Received</th><th>Bytes Sent</th><th>Connected Since</th></tr>\n"}{
                printf "<tr><td>%s</td><td>%s</td><td>%d</td><td>%d</td><td>%s</td></tr>\n", $2, $3, $6, $7, $8;
        }
        END{print "\n<tr><td colspan=5><br></td></tr>\n<tr><td colspan=5>VPN Server Routing Table<hr/></td></tr>\n"}'
        # ROUTING TABLE
        /bin/echo "status 2" | /usr/bin/nc 127.0.0.1 ${PORT} | \
        awk '/HEADER,ROUTING_TABLE/{printline = 1; next} /GLOBAL_STATS/ {printline = 0} printline' | \
        awk -F "," 'BEGIN{print "<tr><th>Client</th><th>Virtual Address</th><th colspan=2>Real Address</th><th>Last Ref</th></tr>\n"}{
                printf "<tr><td>%s</td><td>%s</td><td colspan=2>%s</td><td>%s</td></tr>\n", $3, $2, $4, $5;
        }
        END{print "\n"}'
        echo -e "</table>\n<br>\n";
fi
if [ "$(nvram get openvpncl_enable)" = "1" ]; then
PORT=`grep "^management " /tmp/openvpncl/openvpn.conf | awk '{print $3}'`
if [ x${PORT} = x ]
then
	PORT=16
fi
/bin/echo "status 2" | /usr/bin/nc 127.0.0.1 ${PORT}  | grep "bytes" | awk -F "," 'BEGIN{print "<table><tr><td colspan=2>VPN Client Stats<hr></td></tr>"}{
        printf "<tr>\n<td>%s</td><td>%d</td>\n</tr>", $1, $2;
}
END{print "</table>"}'
fi
