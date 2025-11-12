# first the setup
I add table ip t
I add set ip t portrange { type inet_service; flags interval; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"set": {"family": "ip", "name": "portrange", "table": "t", "type": "inet_service", "handle": 0, "flags": ["interval"]}}}

# adding some ranges
I add element ip t portrange { 1-10 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1, 10]}]}}}}
I add element ip t portrange { 1024-65535 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
I add element ip t portrange { 20-30, 40-50 }
O add element ip t portrange { 20-30 }
O add element ip t portrange { 40-50 }
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [20, 30]}]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [40, 50]}]}}}}

# test flushing -> elements are removed in reverse
I flush set ip t portrange
O delete element ip t portrange { 1024-65535 }
O delete element ip t portrange { 40-50 }
O delete element ip t portrange { 20-30 }
O delete element ip t portrange { 1-10 }
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [40, 50]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [20, 30]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1, 10]}]}}}}

# make sure lower scope boundary works
I add element ip t portrange { 0-10 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [0, 10]}]}}}}

# make sure half open before other element works
I add element ip t portrange { 1024-65535 }
I add element ip t portrange { 100-200 }
O add element ip t portrange { 100-200 }
O add element ip t portrange { 1024-65535 }
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [100, 200]}]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}

# make sure deletion of elements works
I delete element ip t portrange { 0-10 }
O -
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [0, 10]}]}}}}
I delete element ip t portrange { 100-200 }
I delete element ip t portrange { 1024-65535 }
O -
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [100, 200]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}

# make sure mixed add/delete works
I add element ip t portrange { 10-20 }
I add element ip t portrange { 1024-65535 }
I delete element ip t portrange { 10-20 }
O -
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [10, 20]}]}}}}
J {"add": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [1024, 65535]}]}}}}
J {"delete": {"element": {"family": "ip", "table": "t", "name": "portrange", "elem": {"set": [{"range": [10, 20]}]}}}}
