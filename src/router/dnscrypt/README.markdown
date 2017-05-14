[![Build Status](https://travis-ci.org/jedisct1/dnscrypt-proxy.png?branch=master)](https://travis-ci.org/jedisct1/dnscrypt-proxy?branch=master)
[![Make a donation to support this project](https://img.shields.io/badge/donate-PayPal-green.svg?style=flat)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=paypalrecovery-a@pureftpd.org&lc=US&item_name=Donation+to+the+DNSCrypt+project)

[![DNSCrypt](https://raw.github.com/jedisct1/dnscrypt-proxy/master/dnscrypt-small.png)](https://dnscrypt.org)
============

DNSCrypt is a protocol for securing communications between a client
and a DNS resolver, using high-speed high-security elliptic-curve
cryptography.

While not providing end-to-end security, it protects the local network, which
is often the weakest point of the chain, against man-in-the-middle attacks.

`dnscrypt-proxy` is a client-implementation of the protocol. It
requires a [DNSCrypt server](https://www.dnscrypt.org/#dnscrypt-server) on
the other end.

Online documentation
--------------------

* [dnscrypt-proxy documentation](https://github.com/jedisct1/dnscrypt-proxy/wiki/)
* [dnscrypt website](https://dnscrypt.org).

Download and integrity check
----------------------------

dnscrypt-proxy can be downloaded here:
[dnscrypt-proxy download](https://download.dnscrypt.org/dnscrypt-proxy/).

Signatures can be verified with [Minisign](https://jedisct1.github.io/minisign/):

    $ minisign -VP RWQf6LRCGA9i53mlYecO4IzT51TGPpvWucNSCh1CBM0QTaLn73Y7GFO3 -m dnscrypt-proxy-1.9.5.tar.bz2

Plugins
-------

Aside from implementing the protocol, dnscrypt-proxy can be extended
with plug-ins, and gives a lot of control on the local DNS traffic:

- Review the DNS traffic originating from your network in real time,
and detect compromised hosts and applications phoning home.
- Locally block ads, trackers, malware, spam, and any website whose
domain names or IP addresses match a set of rules you define.
- Prevent queries for local zones from being leaked.
- Reduce latency by caching resposes and avoiding requesting IPv6
addresses on IPv4-only networks.
- Force traffic to use TCP, to route it through TCP-only tunnels or
Tor.
