# first the setup
I add table ip t
I add map ip t m { typeof meta day . meta hour : verdict; flags interval; counter; }
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"map": {"family": "ip", "name": "m", "table": "t", "type": {"typeof": {"concat": [{"meta": {"key": "day"}}, {"meta": {"key": "hour"}}]}}, "handle": 0, "map": "verdict", "flags": ["interval"], "stmt": [{"counter": null}]}}}
