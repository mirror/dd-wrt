# setup first
I add table ip t
I add chain ip t c
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"chain": {"family": "ip", "table": "t", "name": "c", "handle": 0}}}

# add set with elements, monitor output expectedly differs
I add map ip t s { typeof udp length . @ih,32,32 : verdict; flags interval; elements = { 20-80 . 0x14 : accept, 1-10 . 0xa : drop }; }
O add map ip t s { typeof udp length . @ih,32,32 : verdict; flags interval; }
O add element ip t s { 20-80 . 0x14 : accept }
O add element ip t s { 1-10 . 0xa : drop }
J {"add": {"map": {"family": "ip", "name": "s", "table": "t", "type": {"typeof": {"concat": [{"payload": {"protocol": "udp", "field": "length"}}, {"payload": {"base": "ih", "offset": 32, "len": 32}}]}}, "handle": 0, "map": "verdict", "flags": ["interval"]}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "s", "elem": {"set": [[{"concat": [{"range": [20, 80]}, 20]}, {"accept": null}]]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "s", "elem": {"set": [[{"concat": [{"range": [1, 10]}, 10]}, {"drop": null}]]}}}}
