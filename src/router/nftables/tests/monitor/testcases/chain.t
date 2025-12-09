I add table inet t
O -
J {"add": {"table": {"family": "inet", "name": "t", "handle": 0}}}

I add chain inet t c
O -
J {"add": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0}}}

I delete chain inet t c
O -
J {"delete": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0}}}

I add chain inet t c { type filter hook input priority filter; }
O add chain inet t c { type filter hook input priority 0; policy accept; }
J {"add": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0, "type": "filter", "hook": "input", "prio": 0, "policy": "accept"}}}

I delete chain inet t c
O -
J {"delete": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0}}}

I add chain inet t c { type filter hook ingress priority filter; devices = { "lo" }; }
O add chain inet t c { type filter hook ingress devices = { "lo" } priority 0; policy accept; }
J {"add": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0, "dev": "lo", "type": "filter", "hook": "ingress", "prio": 0, "policy": "accept"}}}

I delete chain inet t c
O -
J {"delete": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0}}}

I add chain inet t c { type filter hook ingress priority filter; devices = { "eth1", "lo" }; }
O add chain inet t c { type filter hook ingress devices = { "eth1", "lo" } priority 0; policy accept; }
J {"add": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0, "dev": ["eth1", "lo"], "type": "filter", "hook": "ingress", "prio": 0, "policy": "accept"}}}

I delete chain inet t c { type filter hook ingress priority filter; devices = { "eth1" }; }
O delete chain inet t c { type filter hook ingress devices = { "eth1" } priority 0; policy accept; }
J {"delete": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0, "dev": "eth1", "type": "filter", "hook": "ingress", "prio": 0, "policy": "accept"}}}

I delete chain inet t c
O -
J {"delete": {"chain": {"family": "inet", "table": "t", "name": "c", "handle": 0}}}


