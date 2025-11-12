# first the setup
I add table ip t
I add chain ip t c
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}
J {"add": {"chain": {"family": "ip", "table": "t", "name": "c", "handle": 0}}}

I add rule ip t c accept
O -
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"accept": null}]}}}

I add rule ip t c tcp dport { 22, 80, 443 } accept
O -
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"match": {"op": "==", "left": {"payload": {"protocol": "tcp", "field": "dport"}}, "right": {"set": [22, 80, 443]}}}, {"accept": null}]}}}

I insert rule ip t c counter accept
O insert rule ip t c counter packets 0 bytes 0 accept
J {"insert": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"counter": {"packets": 0, "bytes": 0}}, {"accept": null}]}}}

I replace rule ip t c handle 2 accept comment "foo bar"
O delete rule ip t c handle 2
O add rule ip t c handle 5 accept comment "foo bar"
J {"delete": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "expr": [{"accept": null}]}}}
J {"add": {"rule": {"family": "ip", "table": "t", "chain": "c", "handle": 0, "comment": "foo bar", "expr": [{"accept": null}]}}}

I add counter ip t cnt
O add counter ip t cnt { packets 0 bytes 0 }
J {"add": {"counter": {"family": "ip", "name": "cnt", "table": "t", "handle": 0, "packets": 0, "bytes": 0}}}
