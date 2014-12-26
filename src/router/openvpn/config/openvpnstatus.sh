#!/bin/sh
if [ "$(nvram get openvpn_enable)" = "1" ]; then
        echo -n "<table><tr><td colspan=5>VPN Server Stats: "
	# STATS
        /bin/echo "load-stats" | /usr/bin/nc 127.0.0.1 14 | grep SUCCESS | \
        awk -F " " '{print $2}'| awk -F "," '{print $1 ", " $2 ", " $3}'
        echo -e "<hr/></td></tr>\n"
        # CLIENT LIST
        /bin/echo "status 2" | /usr/bin/nc 127.0.0.1 14 | \
        awk '/HEADER,CLIENT_LIST/{printline = 1; next} /HEADER,ROUTING_TABLE/ {printline = 0} printline' | \
        awk -F "," 'BEGIN{print "<tr><th>Client</th><th>Remote IP:Port</th><th>Bytes Received</th><th>Bytes Sent</th><th>Connected Since</th></tr>\n"}{
                printf "<tr><td>%s</td><td>%s</td><td>%d</td><td>%d</td><td>%s</td></tr>\n", $2, $3, $5, $6, $7;
        }
        END{print "\n<tr><td colspan=5><br></td></tr>\n<tr><td colspan=5>VPN Server Routing Table<hr/></td></tr>\n"}'
        # ROUTING TABLE
        /bin/echo "status 2" | /usr/bin/nc 127.0.0.1 14 | \
        awk '/HEADER,ROUTING_TABLE/{printline = 1; next} /GLOBAL_STATS/ {printline = 0} printline' | \
        awk -F "," 'BEGIN{print "<tr><th>Client</th><th>Virtual Address</th><th colspan=2>Real Address</th><th>Last Ref</th></tr>\n"}{
                printf "<tr><td>%s</td><td>%s</td><td colspan=2>%s</td><td>%s</td></tr>\n", $3, $2, $4, $5;
        }
        END{print "\n"}'
        echo -e "</table>\n<br>\n";
fi
if [ "$(nvram get openvpncl_enable)" = "1" ]; then
/bin/echo "status 2" | /usr/bin/nc 127.0.0.1 16  | grep "bytes" | awk -F "," 'BEGIN{print "<table><tr><td colspan=2>VPN Client Stats<hr></td></tr>"}{
        printf "<tr>\n<td>%s</td><td>%d</td>\n</tr>", $1, $2;
}
END{print "</table>"}'
fi
