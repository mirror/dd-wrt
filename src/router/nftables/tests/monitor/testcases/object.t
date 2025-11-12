# first the setup
I add table ip t
O -
J {"add": {"table": {"family": "ip", "name": "t", "handle": 0}}}

I add counter ip t c
O add counter ip t c { packets 0 bytes 0 }
J {"add": {"counter": {"family": "ip", "name": "c", "table": "t", "handle": 0, "packets": 0, "bytes": 0}}}

I delete counter ip t c
O -
J {"delete": {"counter": {"family": "ip", "name": "c", "table": "t", "handle": 0, "packets": 0, "bytes": 0}}}

# FIXME: input/output shouldn't be asynchronous here
I add quota ip t q 25 mbytes
O add quota ip t q { 25 mbytes }
J {"add": {"quota": {"family": "ip", "name": "q", "table": "t", "handle": 0, "bytes": 26214400, "used": 0, "inv": false}}}

I delete quota ip t q
O -
J {"delete": {"quota": {"family": "ip", "name": "q", "table": "t", "handle": 0, "bytes": 26214400, "used": 0, "inv": false}}}

# FIXME: input/output shouldn't be asynchronous here
I add limit ip t l rate 1/second
O add limit ip t l { rate 1/second }
J {"add": {"limit": {"family": "ip", "name": "l", "table": "t", "handle": 0, "rate": 1, "per": "second", "burst": 5}}}

I delete limit ip t l
O -
J {"delete": {"limit": {"family": "ip", "name": "l", "table": "t", "handle": 0, "rate": 1, "per": "second", "burst": 5}}}

I add ct helper ip t cth { type "sip" protocol tcp; l3proto ip; }
O -
J {"add": {"ct helper": {"family": "ip", "name": "cth", "table": "t", "handle": 0, "type": "sip", "protocol": "tcp", "l3proto": "ip"}}}

I delete ct helper ip t cth
O -
J {"delete": {"ct helper": {"family": "ip", "name": "cth", "table": "t", "handle": 0, "type": "sip", "protocol": "tcp", "l3proto": "ip"}}}

I add ct timeout ip t ctt { protocol udp; l3proto ip; policy = { unreplied : 15s, replied : 12s }; }
O -
J {"add": {"ct timeout": {"family": "ip", "name": "ctt", "table": "t", "handle": 0, "protocol": "udp", "l3proto": "ip", "policy": {"unreplied": 15, "replied": 12}}}}

I delete ct timeout ip t ctt
O -
J {"delete": {"ct timeout": {"family": "ip", "name": "ctt", "table": "t", "handle": 0, "protocol": "udp", "l3proto": "ip", "policy": {"unreplied": 15, "replied": 12}}}}
