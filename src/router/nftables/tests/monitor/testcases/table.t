I add table ip t
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}

I delete table ip t
O -
J {"delete": {"table": {"family": "ip", "name": "t", "handle": 0}}}

I add table ip t { comment "foo bar"; flags dormant; }
O add table ip t { flags dormant; }
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0, "flags": ["dormant"], "comment": "foo bar"}}}

I delete table ip t
O -
J {"delete": {"table": {"family": "ip", "name": "t", "handle": 0}}}
