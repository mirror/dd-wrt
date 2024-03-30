# Pound

__Pound__ is a reverse proxy, load balancer and HTTPS front-end for Web
servers. It was developed to enable distributing the load among
several Web-servers and to allow for a convenient SSL wrapper for those Web
servers that do not offer it natively. __Pound__ is distributed under the
GNU General Public License, Version 3, or (at your option) any later
version.

The original version of __pound__ was written by Robert Segall at
[Apsis GmbH](https://web.archive.org/web/20221202094441/https://apsis.ch/).
In 2018, Sergey Poznyakoff added support for OpenSSL 1.x to the then current
version of the program (2.8).  This version of __pound__, hosted on
*github* was further modified by Rick O'Sullivan and Frank Schmirler,
who added WebSocket support.

On April 2020, Apsis started development of __pound__ 3.0 -
essentially an attempt to rewrite __pound__ from scratch, introducing
dependencies on some third-party software.

On 2022-09-19, Robert
[announced](https://groups.google.com/g/pound_proxy/c/O8xaIIODw18)
that he stops further development and maintenance of __pound__.  Following
that, Sergey decided to continue development of the program starting
from his fork.

## What Pound Is

1. a *reverse-proxy*: it passes requests from client browsers to one or
   more backend servers.
2. a *load balancer*: it distributes requests from client browsers among
   several backend servers, while keeping session information.
3. an *SSL wrapper*: it decrypts HTTPS requests from client browsers and
   passes them as plain HTTP to the backend servers.
4. an *HTTP/HTTPS sanitizer*: it verifies requests for correctness and
   accepts only well-formed ones.
5. a *fail-over server*: should a backend server fail, *pound* will take
   note of the fact and stop passing requests to it until it recovers.
6. a *request redirector*: requests may be distributed among servers
   according to the requested URL.

*Pound* is a very small program, easily audited for security
problems. It can run as setuid/setgid and/or in a chroot jail. *Pound*
does not access the hard-disk at all (except for reading certificate
files on start, if required) and should thus pose no security threat
to any machine.

## What Pound Is Not

1. __Pound__ is not a Web server: it serves no content itself, it only
   passes requests and responses back and forth between clients and
   actual web servers (*backends*).
2. __Pound__ is not a Web accelerator: no caching is done --
   every request is passed to a backend server "as is".

## Notice On Project Versioning

I took over __pound__ development at its 2.x branch.  The branch 3.x,
which emerged for a short time before the original project was
abandoned, I consider to be a failed experiment. To ensure consistent
versioning and avoid confusion, my versioning of __pound__ starts with
4.0.

## Documentation

Documentation in manpage format is available in the distribution.  A
copy of the documentation is [available online](https://www.gnu.org.ua/software/pound/pound.html).

## Build requirements

To build, __pound__ needs [OpenSSL](https://www.openssl.org/) version
1.1.x or 3.0.x.

As of current release, __pound__ still supports OpenSSL 1.0, but
this support will soon be discontinued.

If you compile it on a Debian-based system, you need to install the
`libssl-dev` package prior to building __pound__.

## Compilation

If you cloned __pound__ from the repository, you will need the
following tools in order to build it:

* [GNU Autoconf](http://www.gnu.org/software/automake), version 2.69 or later.
* [GNU Automake](http://www.gnu.org/software/autoconf), version 1.15 or later.

First, run

```sh
 ./bootstrap
```

This will prepare the necessary infrastructure files (`Makefile.in`'s
etc.)

If you are building __pound__ from a tarball, the above step is not
needed, since all the necessary files are already included in it.

To prepare __pound__ for compilation, run `./configure`.  Its command
line options will decide where on the filesystem the binary will be
installed, where it will look for its configuration file, etc.  When
run without options, the binary will be installed at `/usr/local/sbin`
and it will look for its configuration in file `/usr/local/etc/pound.cfg`.

If you run it as:

```sh
 ./configure --prefix=/usr --sysconfdir=/etc
```

then the binary will be installed at `/usr/sbin/pound` and it will
read its configuration from `/etc/pound.cfg`.

For a detailed discussion of `--prefix`, `--sysconfdir`, and other
generic configure options, refer to [Autoconf documentation](https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.71/html_node/Running-configure-Scripts.html).

Apart from the generic ones, there are also several *pound-specific*
configuration options:

* `--enable-pcreposix` or `--disable-pcreposix`

  Enable or disable the use of the `libpcreposix2` or `libpcreposix`
  library.  This is a library that makes it possible to use both POSIX
  extended and Perl-compatible regular expressions in __pound__ configuration
  file.

  By default, its presence is determined automatically; `libpcreposix2`
  is preferred over `libpcreposix`.  To force compiling with the older
  `libpcreposix`, use `--enable-pcreposix=pcre1`.

* `--enable-pthread-cancel-probe` or `--disable-pthread-cancel-probe`

  __Pound__ calls the `pthread_cancel` function as part of its shutdown
  sequence.  In GNU libc, this function tries to load shared library
  `libgcc_s.so.1`.  It will fail to do so, if the program is running in
  chroot (the `RootJail` statement is given), unless the library has
  previously been copied to the chroot directory.  To avoid this, __pound__
  will do a temptative call to `pthread_cancel` early, before chrooting,
  so that the necessary library will be loaded and remain available after
  `chroot`.  To determine whether to do this _pthread_cancel probe_ hack,
  `configure` checks if the program is going to be linked with GNU libc.

  These two options allow you to forcefully enable or disable this probe.
  For instance, you may wish to enable it, if another _libc_ implementation
  exhibits a similar behavior.

* `--with-maxbuf=`*n*

  Sets the value of `MAXBUF` parameter - the size of a generic buffer
  used internally by __pound__ for various needs.  The default is 4096.
  You will probably not want to change it.

* `--with-owner=`*user*

  Name of the system user who will own the __pound__ executable file.  When
  not supplied, the first name from the following list that exists in
  the `/etc/passwd` file will be used: `proxy`, `www`, `daemon`, `bin`,
  `sys`, `root`.

* `--with-group=`*group*

  Name of the system group who will own the __pound__ executable.  When
  not supplied, the first name from the following list that exists in
  the `/etc/passwd` file will be used: `proxy`, `www`, `daemon`, `bin`,
  `sys`, `root`.

* `--with-dh=`*n*

  Default DH parameter length.  Allowed values for *n* are 2048 (the
  default) and 1024.

  This option has no effect when compiling with OpenSSL 1.1 or later.

* `--with-ssl=`*directory*

  Directory under which OpenSSL is installed.  You will seldom need this
  option.  Most of the time `configure` is able to detect that location
  automatically.

* `--with-t_rsa=`*n*

  Sets default time interval for regeneration of RSA ephemeral keys.

  This option has no effect when compiling with OpenSSL 1.1 or later.

When configuration is finished, run

```sh
 make
```

When building from a git clone, the first run of this command can take
considerable time, if you are compiling with `OpenSSL` 1.0.  That's because
it involves generating DH parameters.

## Testing

Testing a reverse proxy in general, and __pound__ in particular, is not
a trivial task.  Testsuite in __pound__ was implemented quite recently
and is still somewhat experimental.  Notwithstanding that, it has
already helped to discover several important bugs that lurked in the
code.

To test __pound__ you will need [Perl](https://www.perl.org) version
5.26.3 or later, and the [IO::FDPass](https://metacpan.org/pod/IO::FDPass)
module.  To install the latter on a reasonably recent debian-based system,
run

```sh
 apt-get install libio-fdpass-perl
```

On other systems you may need to install it directly from *cpan* by
running

```sh
 cpan -i IO::FDPass
```

To run tests, type

```sh
 make check
```

from the top-level source directory.  On success, you will see
something like that:

```
## -------------------------- ##
## pound 4.5 test suite.      ##
## -------------------------- ##
  1: Configuration file syntax                       ok
  2: Basic request processing                        ok
  3: xHTTP                                           ok
  4: CheckURL                                        ok
  5: Custom Error Response                           ok
  6: MaxRequest                                      ok
  7: HeadRemove                                      ok
  8: AddHeader                                       ok
  9: RewriteLocation                                 ok
 10: HeadRequire                                     ok
 11: URL                                             ok

## ------------- ##
## Test results. ##
## ------------- ##

All 11 tests were successful.
```

If a test results in something other than `ok`, it leaves the detailed
diagnostics in files in the `tests/NN/testsuite.dir` directory, where
*NN* is the ordinal number of the test.  Pack them all into a single
tarball and send it over to <gray@gnu.org> for investigation.  See
also the section [Bug Reporting](#user-content-bug-reporting) below.

## Installation

If both building and testing succeeded, it's time to install __pound__.
To do so, run the following command as root:

```sh
 make install
```

## Configuration

__Pound__ looks for its configuration file in a location defined at
[compile time](#user-content-compilation), normally `/etc/pound.cfg`,
or `/usr/local/etc/pound.cfg`.  The configuration file syntax is discussed
in detail in the [manual](https://www.gnu.org.ua/software/pound/pound.html).
Here we will describe some example configurations.

Any __pound__ configuration must contain at least two parts:
a `ListenHTTP` (or `ListenHTTPS`) section, that declares a *frontend*,
i.e. the end of the proxy that is responsible for connection with the
outside world, and `Service` section with one or more `Backend` sections
within, which declares where the incoming requests should go.  The
`Service` section can be global or it can be located within the
`ListenHTTP` block.  Global `Service` sections can be shared between
two or more `ListenHTTP` sections.  Multiple `Service` sections can
be supplied, in which case the `Service` to use when handling a
particular HTTP request will be selected using the supplied criteria,
such as source IP address, URL, request header or the like.

### Simplest configuration

The following configuration instructs __pound__ to listen for incoming
HTTP requests on 192.0.2.1:80 and pass them to single backend on
10.10.0.1:8080.


```
ListenHTTP
	Address 192.0.2.1
	Port 80
	Service
		Backend
			Address 10.10.0.1
			Port 8080
		End
	End
End
```

Notice, that the two statements `Address`, and `Port` are in general
mandatory both in `ListenHTTP` and in `Backend`.  There are two
exceptions, however: if `Address` is a file name of a UNIX socket
file, or if an already opened socket is passed to __pound__ via the
`SocketFrom` statement.  These two cases are discussed below.

Argument to the `Address` statement can be an IPv4 or IPv6 address,
a hostname, that will be resolved at program startup, or a full
pathname of a UNIX socket file.

### HTTPS frontend

This example shows how to configure HTTPS frontend and redirect all
plain HTTP requests to it.  It assumes the domain name of the site
is `www.example.org` and its IP address is 192.0.2.1.

```
# Declare HTTP frontend
ListenHTTP
	Address 192.0.2.1
	Port 80
	Service
		# Redirect all requests to HTTPS.  The redirection
		# target has no path component, which means that the
		# path (and query parameters, if any) from the request
		# will be preserved.
		Redirect 301 https://www.example.org
	End
End

# Declare HTTPS frontend.
ListenHTTPS
	Address 192.0.2.1
	Port 443
	# Certificate file must contain the certificate, optional
	# certificate chain and the signature, in that order.
	Cert "/etc/ssl/priv/example.pem"
	# List of certificate authority certificates.
	CAlist /etc/ssl/acme/lets-encrypt-root.pem"
	# Disable obsolete protocols (SSLv2, SSLv3 and TLSv1).
	Disable TLSv1
	Service
		Backend
			Address 10.10.0.1
			Port 8080
		End
	End
End
```

### Virtual Hosts

To implement virtual hosts, one needs to instruct __pound__ to
route requests to different services depending on the values of
their `Host:` headers.  To do so, use the `Host` statement in the
`Service` section.

The argument to `Host` specifies the host name.  When an incoming request
arrives, it is compared with this value.  The `Service` section will be
used only if the value of the `Host:` header from the request matched the
argument to the `Host` statement.  By default, exact case-insensitive
comparison is used.

Let's assume that you have internal server 192.168.0.10 that is supposed to
serve the needs of virtual host *www.server0.com* and 192.168.0.11
that serves *www.server1.com*.  You want __pound__ to listen on address
192.0.2.1.  The configuration file would look like this:

```
ListenHTTP
	Address 192.0.2.1
	Port    80

	Service
		Host "www.server0.com"
		Backend
			Address 192.168.0.10
			Port    80
		End
	End

	Service
		Host "www.server1.com"
		Backend
			Address 192.168.0.11
			Port    80
		End
	End
End
```

The same can be done using `ListenHTTPS`.

If you want to use the same service for both the hostname and the
hostname prefixed with `www.`, you can either use the `Match` statement,
or a regular expression.

A `Match` statement groups several conditions using boolean shortcut
evaluation.  In the following example, boolean __or__ is used to group
two `Host` statements:

```
	Service
		Match OR
			Host "server0.com"
			Host "www.server0.com"
		End
		Backend
			Address 192.168.0.10
			Port    80
		End
	End
```

When this service is considered, the value of the `Host:` header from the
incoming request is matched against each host listed in the `Match OR`
statement.  If any value compares equal, the match succeeds and the service
is selected for processing the request.

By default, the `Host` directive uses exact case-insensitive string match.
This can be altered by supplying one or more options to it.  In the example
below, we use regular expression matching to achieve the same result as in
the configuration above:

```
	Service
		Host -re "^(www\\.)?server0\\.com$"
		Backend
			Address 192.168.0.10
			Port    80
		End
	End
```

Notice double-slashes: a slash is an escape character and must be escaped
if intended to be used literally.

### Sessions

__Pound__ is able to keep track of sessions between a client browser
and a backend server.  Unfortunately, HTTP is defined as a stateless
protocol, which complicates matters: many schemes have been invented
to allow keeping track of sessions, and none of them works
perfectly.  What's worse, sessions are critical in order to allow
web-based applications to function correctly - it is vital that once a
session is established all subsequent requests from the same browser
be directed to the same backend server.

Six possible ways of detecting a session have been implemented in
__pound__ (hopefully the most useful ones): by client address, by Basic
authentication (user id/password), by URL parameter, by cookie, by
HTTP parameter and by header value.

Session tracking is declared using the `Session` block in `Service`
section.  Only one `Session` can be used per `Service`.  The type of
session tracking is declared with the `Type` statement.

* `Type IP`:  Session tracking by address

  In this scheme __pound__ directs all requests from the same client
  IP address to the same backend server. Put the lines

  ```
  Session
	  Type    IP
	  TTL     300
  End
  ```

  in the configuration file to achieve this effect. The value indicates
  what period of inactivity is allowed before the session is discarded.

* `Type Basic`: by Basic Authentication

  In this scheme __pound__ directs all requests from the same user (as
  identified in the Basic Authentication header) to the same backend
  server. Put the lines

  ```
  Session
	  Type    Basic
	  TTL     300
  End
  ```

  in configuration file to achieve this effect. The value indicates what
  period of inactivity is allowed before the session is discarded.

  This type is a special case of the `Type Header`, described below.

  WARNING: given the constraints of the HTTP protocol it may very well be
  that the authenticated request will go to a different backend server than
  the one originally requesting it. Make sure all your servers support
  the same authentication scheme!

* `Type URL`: by URL parameter

  Quite often session information is passed through URL parameters
  (the browser is pointed to something like `http://xxx?id=123`).
  Put the lines

  ```
  Session
	  Type    URL
	  ID      "id"
	  TTL     300
  End
  ```

  to support this scheme and the sessions will be tracked based on the value
  of the `id` parameter.

* `Type Cookie`: by cookie value

  Applications that use this method pass a certain cookie back and
  forth. Add the lines

  ```
  Session
	  Type    Cookie
	  ID      "sess"
	  TTL     300
  End
  ```

  to your configuration file - the sessions will be tracked by the value of
  the `sess` cookie.

* `Type Parm`: by HTTP parameter value

  Applications that use this method pass an HTTP parameter
  (`http://x.y/z;parameter`) back and forth. Add the lines

  ```
  Session
	  Type    PARM
	  TTL     300
  End
  ```

  To your configuration file - sessions will be tracked by the value of
  the parameter.

* `Type Header`: by header value

  Applications that use this method pass a certain header back and
  forth. Add the lines

  ```
  Session
	  Type    Header
	  ID      "X-sess"
	  TTL     300
  End
  ```

  to your configuration file - the sessions will be tracked by the value of
  the `X-sess` header.

Please note the following restrictions on session tracking:

* Session tracking is always associated with a certain `Service`. Thus, each
  group may have other methods and parameters.
* There is no default session: if you have not defined any sessions, no
  session tracking will be done.
* Only one session definition is allowed per `Service`. If your application
  has alternative methods for sessions you will have to define a separate
  `Service` for each method.

A note on cookie injection: some applications have no session-tracking
mechanism at all but would still like to have the client always
directed to the same backend time after time. Some reverse proxies use
a mechanism called *cookie injection* in order to achieve this: a
cookie is added to backend responses and tracked by the reverse proxy.

__Pound__ was designed to be as transparent as possible, therefore this
mechanism is not supported. If you really need this sort of persistent
mapping use the client address session mechanism (`Type IP`), which
achieves the same result without changing the contents in any way.

### Logging

If __pound__ operates in daemon mode (the default), all diagnostics
goes to the syslog facility `daemon`.  __Pound__ switches to syslog right
before it disconnects from the controlling terminal.  Until then, it
sends its messages to the standard error.

By default only error and informative messages are logged.  The amount
of information logged is controlled by the `LogLevel` configuration
statement.  Possible settings are:

* `0`
  No logging.

* `1`
  Regular logging: only error conditions and important informative
  messages are logged.

* `2`
  Extended logging: show chosen backend servers as well.

* `3`
  Log requests using Apache-style Combined Log format.

* `4`
  Same as 3, but without the virtual host information.

* `5`
  Same as 4 but with information about the `Service` and `Backend`
  used.

The `LogLevel` statement can be global (effective for all listeners),
as well as per-listener.

## Socket Passing

__Pound__ can obtain socket to listen on from another program via
a UNIX socket.  This mode of operation is requested by the following
statement in `ListenHTTP` section:

```
  SocketFrom "/path/to/socket"
```

When this statement is present, neither `Address` nor `Port` may be
used in this listener.  __Pound__ will connect to the named socket and
obtain the socket descriptor from it.  Then it will start listening
for incoming requests on that socket.

This can be used both in `ListenHTTP` and `ListenHTTPS` sections.

Currently it is used in __pound__ testsuite.

## Request Modification

Normally, __pound__ passes all incoming requests to backends
verbatim.  Several request modification directives are provided, that
allow you to add or remove headers from the request.  The following
two groups of headers are added by default.  Each of them can be turned
off using the `HeaderOption` directive.

1. The _forwarded_ headers:

* `X-Forwarded-For:` header passes the actual IP address of the client
machine that sent the request.

* `X-Forwarded-Proto:` header contains the original protocol (`http` or
`https`).

* `X-Forwarded-Port:` header contains the port on the server that the
client connected to.

2. Second group contains _ssl_ headers that are added only if the client
connected using HTTPS.  The `X-SSL-Cipher` header is always present if
this header group is enabled.  The rest of headers below is added only if
the client certificate was supplied:

* `X-SSL-Cipher`: SSL version followed by a slash and active cipher algorithm.
* `X-SSL-Certificate`: the full client certificate (multi-line).
* `X-SSL-Issuer`: information about the certificate issuer (CA).
* `X-SSL-Subject`: information about the certificate owner.
* `X-SSL-notAfter`: end od validity date for the certificate.
* `X-SSL-notBefore`: start of validity date for the certificate.
* `X-SSL-serial`: certificate serial number (in decimal).

The `HeaderOption` directive can be used (either globally or in listener
block) to disable any or both of these groups, e.g.:
```
HeaderOption no-ssl forwarded
```

Any number of headers can be added or removed using the `HeaderAdd` and
`HeaderRemove` directives in the listener section.  The order in which these
directives are applied is:

1. Headers controlled by the `HeaderOption` directive are added.
2. Headers requested by `HeaderRemove` directives are removed.
3. Headers from `HeaderAdd` directives are added.

## ACME

__Pound__ offers built-in support for ACME (a.k.a. _LetsEncrypt_) [HTTP-01](https://letsencrypt.org/docs/challenge-types/#http-01-challenge) challenge type.
Thus, it can be used with any certificate controller to obtain SSL certificates
on the fly.

Assuming your certificate controller is configured to store challenges in
directory `/var/lib/pound/acme`, all you need to do is add the `ACME`
statement to the `ListenHTTP` block, for example:

```
ListenHTTP
	ACME "/var/lib/pound/acme"
	.
	.
	.
End
```

Now, each request whose URL ends in `/.well-known/acme-challenge/NAME`
will be served by directly by __pound__: it will send the content of
the file `/var/lib/pound/acme/NAME` as a reply.

## Using `RootJail`

The `RootJail` configuration directive instructs __pound__ to chroot
to the given directory at startup.  Normally, its use should be quite
straightforward:

```
RootJail "/var/pound"
```

__Pound__ tries to open all files and devices it needs before
chrooting.  There might be cases, however, when it is not enough
and you would need to copy certain system files to the chroot
directory.

### Notes for users of __pound__ versions prior to 4.7

When using `RootJail`, __pound__ does not remove its PID file
before shutting down.

If __pound__ displays the following message and aborts when being stopped:

```
libgcc_s.so.1 must be installed for pthread_cancel to work
```

then you need to copy that library to the `RootJail` directory, e.g.:

```sh
mkdir /var/pound/lib64
cp /usr/lib64/libgcc_s.so.1 /var/pound/lib64
```

or make sure it is loaded at program startup by defining the
`LD_PRELOAD` variable:

```sh
export LD_PRELOAD=/usr/lib64/libgcc_s.so
```

This problem was fixed in version 4.7 (see the description of the
`--enable-pthread-cancel-probe` configure option above).

## Bug-reporting

If you think you found a bug in __pound__ or in its documentation,
please send a mail to Sergey Poznyakoff <gray@gnu.org> (or
<gray+pound@gnu.org.ua>), or use the
[github issue tracker](https://github.com/graygnuorg/pound/issues).
