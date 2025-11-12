# setup first
I add table ip t
I add chain ip t c
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"chain": {"family": "ip", "table": "t", "name": "c", "handle": 0}}}

# add set with elements, monitor output expectedly differs
I add set ip t s { type inet_service; flags interval; elements = { 20, 30-40 }; }
O add set ip t s { type inet_service; flags interval; }
O add element ip t s { 20 }
O add element ip t s { 30-40 }
J {"add": {"set": {"family": "ip", "name": "s", "table": "t", "type": "inet_service", "handle": 0, "flags": ["interval"]}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "s", "elem": {"set": [20]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "s", "elem": {"set": [{"range": [30, 40]}]}}}}

# this would crash nft
I add rule ip t c tcp dport @s
O -
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"match": {"op": "==", "left": {"payload": {"protocol": "tcp", "field": "dport"}}, "right": "@s"}}]}}}

# test anonymous interval sets as well
I add rule ip t c tcp dport { 20, 30-40 }
O -
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"match": {"op": "==", "left": {"payload": {"protocol": "tcp", "field": "dport"}}, "right": {"set": [20, {"range": [30, 40]}]}}}]}}}

# ... and anon concat range
I add rule ip t c ether saddr . ip saddr { 08:00:27:40:f7:09 . 192.168.56.10-192.168.56.12 }
O -
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"match": {"op": "==", "left": {"concat": [{"payload": {"protocol": "ether", "field": "saddr"}}, {"payload": {"protocol": "ip", "field": "saddr"}}]}, "right": {"set": [{"concat": ["08:00:27:40:f7:09", {"range": ["192.168.56.10", "192.168.56.12"]}]}]}}}]}}}
