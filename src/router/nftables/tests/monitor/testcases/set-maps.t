# first the setup
I add table ip t
I add map ip t portip { type inet_service: ipv4_addr; flags interval; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"map": {"family": "ip", "name": "portip", "table": "t", "type": "inet_service", "handle": 0, "map": "ipv4_addr", "flags": ["interval"]}}}

I add element ip t portip { 80-100: 10.0.0.1 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portip", "elem": {"set": [[{"range": [80, 100]}, "10.0.0.1"]]}}}}

I add element ip t portip { 1024-65535: 10.0.0.1 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portip", "elem": {"set": [[{"range": [1024, 65535]}, "10.0.0.1"]]}}}}
