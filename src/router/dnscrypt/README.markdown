[![DNSCrypt](https://raw.github.com/dyne/dnscrypt-proxy/master/dnscrypt-small.png)](https://dowse.eu)

[![Build Status](https://travis-ci.org/dyne/dnscrypt-proxy.png?branch=master)](https://travis-ci.org/dyne/dnscrypt-proxy)

## Status of the project

This project was taken offline by its creator and maintainer Frank Denis on the 6th December 2017, after announcing in November 2017 that [the project needs a new maintainer](https://twitter.com/jedisct1/status/928942292202860544).

The old webpage [dnscrypt.org]() now points to a new domain, endorsing the usage of competing protocol "DNS-over-TLS" and competing software in particular the "getdns" library and an immature implementation that could substitute dnscrypt-proxy, called "stubby".

The new website also links a [critical analysis of DNSCrypt vs DNS-over-TLS protocols](https://tenta.com/blog/post/2017/12/dns-over-tls-vs-dnscrypt) by a company marketing their own open-source Android web browser and offering a new DNS resolver implemented in Go.

While this sounds all very new and exciting to us, at Dyne.org we already rely on DNSCrypt-proxy for our project [Dowse.eu]() and are intentioned to maintain this software unless a viable and mature alternative arises, supporting our application of it in Dowse.

We intend to maintain the DNSCrypt-proxy codebase without the intention of adding any new features, just patch bugs. We are also available to archive older versions and setup the website and the wiki, if we are given these archives. Frank Denis: if you are reading this please contact us on info@dyne.org. It won't take long and we are happy to keep your project alive, many thanks for all the fish so far!

Anyone running a DNSCrypt server, interested in the future of this software, willing to share more insights or wanting to help with development and documentation: be welcome to [join our dnscrypt mailinglist](https://mailinglists.dyne.org/cgi-bin/mailman/listinfo/dnscrypt) where we are setting up a campfire for all of us to make a sustainable plan and take collectively informed decisions.

## What is DNSCrypt

DNSCrypt is a protocol for securing communications between a client
and a DNS resolver, using high-speed high-security elliptic-curve
cryptography.

While not providing end-to-end security, it protects the local network, which
is often the weakest point of the chain, against man-in-the-middle attacks.

`dnscrypt-proxy` is a client-implementation of the protocol. It
requires a DNS server available via the [DNSCrypt
wrapper](https://github.com/cofyc/dnscrypt-wrapper) to function. A
number of public DNSCrypt servers are already available.

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
