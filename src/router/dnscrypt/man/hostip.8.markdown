hostip(8) -- Resolve a host name to an IP address
=================================================

## SYNOPSIS

`hostip` [<options>] host_name

## DESCRIPTION

**hostip** sends a DNS query to a resolver, and prints the IP
addresses for the given host name.

It can be useful in order to retrieve IP addresses before
dnscrypt-proxy(8) is started.

## OPTIONS

  * `-6`, `--ipv6`: ask for AAAA records.

  * `-h`, `--help`: show usage.

  * `-r`, `--resolver-address=<ip>[:port]`: the resolver IP address
(default: 8.8.8.8, Google DNS).

  * `-V`, `--version`: show version number.

## SIMPLE USAGE EXAMPLE

    $ hostip www.example.com

## ADVANCED USAGE EXAMPLE

    $ hostip -6 -r 4.2.2.2 www.google.com

## EXIT STATUS

The `hostip` utility exits 0 on success, and > 0 if an error occurs.

## SEE ALSO

dnscrypt-proxy(8)
