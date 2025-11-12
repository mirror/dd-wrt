# first the setup
I add table ip t
I add set ip t portrange { type inet_service; flags interval; }
I add set ip t portrange2 { type inet_service; flags interval; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"set": {"family": "ip", "name": "portrange", "table": "t", "type": "inet_service", "handle": 0, "flags": ["interval"]}}}
J {"add": {"set": {"family": "ip", "name": "portrange2", "table": "t", "type": "inet_service", "handle": 0, "flags": ["interval"]}}}

# make sure concurrent adds work
I add element ip t portrange { 1024-65535 }
I add element ip t portrange2 { 10-20 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange2", "elem": {"set": [{"range": [10, 20]}]}}}}
