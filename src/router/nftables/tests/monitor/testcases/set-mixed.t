# first the setup
I add table ip t
I add set ip t portrange { type inet_service; flags interval; }
I add set ip t ports { type inet_service; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"set": {"family": "ip", "name": "portrange", "table": "t", "type": "inet_service", "handle": 0, "flags": ["interval"]}}}
J {"add": {"set": {"family": "ip", "name": "ports", "table": "t", "type": "inet_service", "handle": 0}}}

# make sure concurrent adds work
I add element ip t portrange { 1024-65535 }
I add element ip t ports { 10 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "ports", "elem": {"set": [10]}}}}

# delete items again
I delete element ip t portrange { 1024-65535 }
I delete element ip t ports { 10 }
O -
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "ports", "elem": {"set": [10]}}}}
