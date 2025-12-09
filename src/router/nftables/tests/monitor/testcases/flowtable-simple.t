# setup first
I add table ip t
I add flowtable ip t ft { hook ingress priority 0; devices = { "lo" }; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"flowtable": {"family": "ip", "name": "ft", "table": "t", "handle": 0, "hook": "ingress", "prio": 0, "dev": "lo"}}}

I delete flowtable ip t ft
O -
J {"delete": {"flowtable": {"family": "ip", "name": "ft", "table": "t", "handle": 0}}}

I add flowtable ip t ft { hook ingress priority 0; devices = { "eth1", "lo" }; }
O -
J {"add": {"flowtable": {"family": "ip", "name": "ft", "table": "t", "handle": 0, "hook": "ingress", "prio": 0, "dev": ["eth1", "lo"]}}}

I delete flowtable ip t ft { hook ingress priority 0; devices = { "eth1" }; }
O -
J {"delete": {"flowtable": {"family": "ip", "name": "ft", "table": "t", "handle": 0, "hook": "ingress", "prio": 0, "dev": "eth1"}}}

I delete flowtable ip t ft
O -
J {"delete": {"flowtable": {"family": "ip", "name": "ft", "table": "t", "handle": 0}}}
