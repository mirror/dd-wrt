dnscrypt-proxy(8) -- A DNSCrypt forwarder
=========================================

## SYNOPSIS

`dnscrypt-proxy <config file>`

`dnscrypt-proxy [<option>, ...]`

## DESCRIPTION

**dnscrypt-proxy** accepts DNS requests, authenticates and encrypts
them using dnscrypt and forwards them to a remote dnscrypt-enabled
resolver.

Replies from the resolver are expected to be authenticated or else
they will be discarded.

The proxy verifies the replies, decrypts them, and transparently
forwards them to the local stub resolver.

`dnscrypt-proxy` listens to `127.0.0.1` / port `53` by default.

## OPTIONS (ignored when a configuration file is provided)

  * `-R`, `--resolver-name=<name>`: name of the resolver to use, from
    the list of available resolvers (see `-L`). Or `random` for a random
    resolver accessible over IPv4, that doesn't log and supports DNSSEC.

  * `-a`, `--local-address=<ip>[:port]`: what local IP the daemon will listen
    to, with an optional port. The default port is 53.

  * `-d`, `--daemonize`: detach from the current terminal and run the server
    in background.

  * `-E`, `--ephemeral-keys`: By default, queries are always sent with the
    same public key, allowing providers to link this public key to the
    different IP addresses you are using. This option requires extra
    CPU cycles, but mitigates this by computing an ephemeral key pair for
    every query. Use it if you are not using your own server, and the
    remote server is logging your activity, and your client IP address is
    frequently changing. Not enabled by default because it may be slow,
    especially on non-Intel CPUs.

  * `-K`, `--client-key=<file>`: use a static client secret key stored in
    `<file>`.

  * `-L`, `--resolvers-list=<file>`: path to the CSV file containing
    the list of available resolvers, and the parameters to use them.

  * `-l`, `--logfile=<file>`: log events to this file instead of the
    standard output.

  * `-m`, `--loglevel=<level>`: don't log events with priority above
    this level after the service has been started up. Default is `6`,
    the value for `LOG_INFO`. Valid values are `0` (system is unusable),
    `1` (action must be taken immediately), `2` (critical conditions),
    `3` (error conditions), `4` (warning conditions),
    `5` (normal but significant condition), `6` (informational) and
    `7` (debug-level messages).

  * `-p`, `--pidfile=<file>`: write the PID number to a file.

  * `-X`, `--plugin=<plugin_name>[,<options>]`: enable a plugin.

  * `-N`, `--provider-name=<FQDN>`: the fully-qualified name of the
    dnscrypt certificate provider (for private resolvers).

  * `-k`, `--provider-key=<key>`: specify the provider public key
    (for private resolvers).

  * `-r`, `--resolver-address=<ip>[:port]`: a DNSCrypt-capable resolver IP
    address with an optional port (for private resolvers).
    The default port is 443.

  * `-S`, `--syslog`: if a log file hasn't been set, log diagnostic messages to
    syslog instead of printing them. `--daemonize` implies `--syslog`.

  * `-Z`, `--syslog-prefix=prefix`: specify a string of message to insert at
    the beginning of every line sent to syslog. This implies --syslog.

  * `-n`, `--max-active-requests=<count>`: set the maximum number of
    simultaneous active requests. The default value is 250.

  * `-u`, `--user=<user name>`: chroot(2) to this user's home directory
    and drop privileges.

  * `-t`, `--test=<margin>`: don't actually start the proxy, but check that
    a valid certificate can be retrieved from the server and that it
    will remain valid for the next <margin> minutes. The exit code is 0
    if a valid certificate can be used, 2 if no valid certificates can be used,
    3 if a timeout occurred, and 4 if a currently valid certificate is
    going to expire before <margin>. The margin is always specified in
    minutes.

  * `-T`, `--tcp-only`: always use TCP. A connection made using UDP
    will get a truncated response, so that the (stub) resolver retries using
    TCP.

  * `-e`, `--edns-payload-size=<bytes>`: transparently add an OPT
    pseudo-RR to outgoing queries in order to enable the EDNS0
    extension mechanism. The payload size is the size of the largest
    response we accept from the resolver before retrying over TCP.
    This feature is enabled by default, with a payload size of 1252
    bytes. Any value below 512 disables it.

  * `-I`, `--ignore-timestamps`: ignore timestamps when validating certificates.
    Never enable this option unless you know you really need it (routers without
    a clock battery).

  * `-V`, `--version`: show version number.

  * `-h`, `--help`: show usage.

A public key is 256-bit long, and it has to be specified as a hexadecimal
string, with optional columns.

## COMMON USAGE EXAMPLE

    $ dnscrypt-proxy /etc/dnscrypt.conf

## COMMON USAGE EXAMPLE WITHOUT A CONFIGURATION FILE

    $ dnscrypt-proxy --daemonize --resolver-name=...

The resolver name is the first column (Name) in the CSV file.

## BUGS AND SUPPORT

Please report issues with DNSCrypt itself to https://dnscrypt.org/issues

## SEE ALSO

hostip(8)
