##############################################################################
#
# Copyright (c) 2001 Zope Corporation and Contributors. All Rights Reserved.
#
# This software is subject to the provisions of the Zope Public License,
# Version 2.0 (ZPL).  A copy of the ZPL should accompany this distribution.
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY AND ALL EXPRESS OR IMPLIED
# WARRANTIES ARE DISCLAIMED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF TITLE, MERCHANTABILITY, AGAINST INFRINGEMENT, AND FITNESS
# FOR A PARTICULAR PURPOSE
#
##############################################################################
"""Zope 2 ZServer start-up file

Usage: %(program)s [options] [environment settings]

Options:

  -h

    Output this text.

  -z path

    The location of the Zope installation.
    The default is the location of this script, %(here)s.

  -Z 0 or 1

    UNIX only! This option is ignored on Windows.

    This option controls whether a management process will be created
    that restarts Zope after a shutdown or crash.
    
    If the argument to -Z is non-null (e.g. "-Z1" or "-Zyes"), a
    management process will be used.  If the argument to -Z is "-", or
    "0", (e.g. "-Z-" or "-Z0"), a management process will not be used.
    On UNIX, the default behavior is to create a separate management
    process (e.g. -Z1) if the -Z option is not specified.

    (Note: the -Z option in Zopes before Zope 2.6 used to be used to specify
    a pidfile name for the management process.  This pidfile no longer
    exists).

  -t n

    The number of threads to use, if ZODB3 is used. The default is
    %(NUMBER_OF_THREADS)s.

  -i n

    Set the interpreter check interval. This integer value
    determines how often the interpreter checks for periodic things
    such as thread switches and signal handlers. The Zope default
    is 500, but you may want to experiment with other values that
    may increase performance in your particular environment.

  -D

    Run in Zope debug mode.  This causes the Zope process not to
    detach from the controlling terminal, and is equivalent to
    supplying the environment variable setting Z_DEBUG_MODE=1

  -a ipaddress

    The IP address to listen on.  If this is an empty string
    (e.g. -a ''), then all addresses on the machine are used. The
    default is %(IP_ADDRESS)s.

  -d ipaddress

    IP address of your DNS server. If this is an empty string
    (e.g. -d ''), then IP addresses will not be logged. If you have
    DNS service on your local machine then you can set this to
    127.0.0.1.  The default is: %(DNS_IP)s.

  -u username or uid number

    The username to run ZServer as.  You may want to run ZServer as
    a dedicated user.  This only works under Unix, and if ZServer
    is started as root, and is required in that case.

  -P [ipaddress:]number

    Set the web, ftp and monitor port numbers simultaneously
    as offsets from the number.  The web port number will be number+80.
    The FTP port number will be number+21.  The monitor port number will
    be number+99.

    The number can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -P options can be provided to run multiple sets of servers.

  -w port

    The Web server (HTTP) port.  This defaults to %(HTTP_PORT)s. The
    standard port for HTTP services is 80.  If this is a dash
    (e.g. -w -), then HTTP is disabled.

    The number can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -w options can be provided to run multiple servers.

  -y port

    The Web secure server (HTTPS) port.  This defaults to %(HTTPS_PORT)s.
    The standard port for HTTPS services is 443.  If this is a dash
    (e.g. -y -), then HTTPS is disabled.

    The number can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -w options can be provided to run multiple servers.

  -W port

    The "WebDAV source" port.  If this is a dash (e.g. -w -), then
    "WebDAV source" is disabled.  The default is disabled.  Note that
    this feature is a workaround for the lack of "source-link" support
    in standard WebDAV clients.

    The port can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -W options can be provided to run multiple servers.

  -C
  --force-http-connection-close

    If present, this option causes Zope to close all HTTP connections,
    regardless of the 'Connection:' header (or lack of one) sent by
    the client.

  -f port

    The FTP port.  If this is a dash (e.g. -f -), then FTP
    is disabled.  The standard port for FTP services is 21.  The
    default is %(FTP_PORT)s.

    The port can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -f options can be provided to run multiple servers.

  -p path

    Path to the PCGI resource file.  The default value is
    %(PCGI_FILE)s, relative to the Zope location.  If this is a dash
    (-p -) or the file does not exist, then PCGI is disabled.

  -F path_or_port

    Either a port number (for inet sockets) or a path name (for unix
    domain sockets) for the FastCGI Server.  If the flag and value are
    not specified then the FastCGI Server is disabled.

  -m port

    The secure monitor server port. If this is a dash
    (-m -), then the monitor server is disabled. The monitor server
    allows interactive Python style access to a running ZServer. To
    access the server see medusa/monitor_client.py or
    medusa/monitor_client_win32.py. The monitor server password is the
    same as the Zope emergency user password set in the 'access'
    file. The default is to not start up a monitor server.

    The port can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -m options can be provided to run multiple servers.

  --icp port

    The ICP port. ICP can be used to distribute load between back-end
    zope servers, if you are using an ICP-aware front-end proxy such
    as Squid.

    The port can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple --icp options can be provided to run multiple servers.

  -l path

    Path to the ZServer log file. If this is a relative path then the
    log file will be written to the 'var' directory. The default is
    %(LOG_FILE)s.

  -r

    Run ZServer is read-only mode. ZServer won't write anything to disk.
    No log files, no pid files, nothing. This means that you can't do a
    lot of stuff like use PCGI, and zdaemon. ZServer will log hits to
    STDOUT and zLOG will log to STDERR.

  -L

    Enable locale (internationalization) support. The value passed for
    this option should be the name of the locale to be used (see your
    operating system documentation for locale information specific to
    your system). If an empty string is passed for this option (-L ''),
    Zope will set the locale to the user's default setting (typically
    specified in the $LANG environment variable). If your Python
    installation does not support the locale module, the requested
    locale is not supported by your system or an empty string was
    passed but no default locale can be found, an error will be raised
    and Zope will not start.

  -X

    Disable servers. This might be used to effectively disable all
    default server settings or previous server settings in the option
    list before providing new settings. For example to provide just a
    web server:

      %(program)s -X -w80

  -M file

    Save detailed logging information to the given file.
    This log includes separate entries for:

      - The start of a request,
      - The start of processing the request in an application thread,
      - The start of response output, and
      - The end of the request.

Environment settings are of the form: NAME=VALUE.

Note: you *must* use Python 2.1 or later!
"""


# This is required path hackery for the win32 binary distribution
# that ensures that the bundled python libraries are used. In a
# win32 binary distribution, the installer will have replaced the
# marker string with the actual software home. If that has not
# happened, then the path munging code is skipped.
swhome=r'INSERT_SOFTWARE_HOME'
if swhome != 'INSERT_SOFTWARE_HOME':
    import sys
    sys.path.insert(0, '%s/lib/python' % swhome)
    sys.path.insert(1, '%s/bin/lib' % swhome)
    sys.path.insert(2, '%s/bin/lib/plat-win' % swhome)
    sys.path.insert(3, '%s/bin/lib/win32' % swhome)
    sys.path.insert(4, '%s/bin/lib/win32/lib' % swhome)
    sys.path.insert(5, '%s' % swhome)


import os, sys, getopt, codecs, string
import socket

from types import StringType, IntType
# workaround to allow unicode encoding conversions in DTML
dummy = codecs.lookup('iso-8859-1')

sys.setcheckinterval(500)

program=sys.argv[0]
here=os.path.join(os.getcwd(), os.path.split(program)[0])

########################################################################
# Configuration section

## General configuration options
##

# This is the IP address of the network interface you want your servers to
# be visible from.  This can be changed to '' to listen on all interfaces.
IP_ADDRESS=''

# IP address of your DNS server. Set to '' if you do not want to resolve
# IP addresses. If you have DNS service on your local machine then you can
# set this to '127.0.0.1'
DNS_IP=''

# User id to run ZServer as. Note that this only works under Unix, and if
# ZServer is started by root. This no longer defaults to 'nobody' since
# that can lead to a Zope file compromise.
UID=None

# Log file location. If this is a relative path, then it is joined the
# the 'var' directory.
LOG_FILE='Z2.log'

## HTTP configuration
##

# HTTP enivornment settings.
HTTP_ENV={}

# Port for HTTP Server. The standard port for HTTP services is 80.
HTTP_PORT=8080

## HTTPS configuration
##

# HTTPS enivornment settings.
HTTPS_ENV={}

# Port for HTTP Server. The standard port for HTTP services is 80.
HTTPS_PORT=8443

# Should we close all HTTP connections, ignoring the (usually absent)
# 'Connection:' header?
FORCE_HTTP_CONNECTION_CLOSE=0

# Port for the special "WebDAV source view" HTTP handler.  There is no
# standard port for this handler, which is disabled by default.
WEBDAV_SOURCE_PORT=[]

## FTP configuration

# Port for the FTP Server. The standard port for FTP services is 21.
FTP_PORT=8021

## PCGI configuration

# You can configure the PCGI server manually, or have it read its
# configuration information from a PCGI info file.
PCGI_FILE='Zope.cgi'

## Monitor configuration
MONITOR_PORT=0

## ICP configuration
ICP_PORT=0

# Module to be published, which must be Main or Zope
MODULE='Zope'

# The size of the thread pool, if ZODB3 is used.
NUMBER_OF_THREADS=4

# Localization support
LOCALE_ID=None

# Socket path or port for the FastCGI Server
FCGI_PORT=None

# Detailed log file
DETAILED_LOG_FILE=''

# Use a daemon process
USE_DAEMON = 1

#
########################################################################

########################################################################
# Handle command-line arguments:

def server_info(old, v, offset=0):
    # interpret v as a port or address/port and get new value
    if v == '-': v=''
    l=v.find(':')
    if l >= 0:
        a=v[:l]
        v=v[l+1:]
    else:
        a=IP_ADDRESS

    if not v: return v

    try:
        v=int(v)
        if v < 0: raise 'Invalid port', v
        v=v+offset
    except: raise 'Invalid port', v

    if isinstance(old, IntType): old=[(a,v)]
    else: old.append((a,v))

    return old


try:
    python_version = sys.version.split()[0]
    if python_version < '2.1':
        raise 'Invalid python version', python_version
    if python_version[:3] == '2.1':
        if python_version[4:5] < '3':
            import warnings
            err = ('You are running Python version %s.  This Python version '
                   'has known bugs that may cause Zope to run improperly. '
                   'Consider upgrading to a Python in the 2.1 series '
                   'with at least version number 2.1.3.  (Note that Zope does '
                   'not yet run under any Python 2.2 version).' %
                   python_version)
            warnings.warn(err)
    if python_version[:3] == '2.2':
        import warnings
        err = ('You are running Python version %s.  This Python version '
               'has not yet been tested with Zope and you may experience '
               'operational problems as a result.  Consider using '
               'Python 2.1.3 instead.' % python_version)
        warnings.warn(err)


    opts, args = getopt.getopt(sys.argv[1:],
                               'hz:Z:t:i:a:d:u:w:y:W:f:p:m:Sl:2DP:rF:L:XM:C',
                               ['icp=', 'force-http-connection-close'
                               ])

    DEBUG=0
    READ_ONLY=0
    if sys.platform == 'win32':
        USE_DAEMON = 0
        

    # Get environment variables
    for a in args:
        if a.find('='):
            a=a.split('=')
            o=a[0]
            v='='.join(a[1:])
            if o:
                os.environ[o]=v
                HTTP_ENV[o]=v
                HTTPS_ENV[o]=v
        else:
            raise 'Invalid argument', a

    for o, v in opts:
        if o=='-z': here=v
        elif o=='-Z':
            if v in ('-', '0', ''):
                USE_DAEMON=0
            elif sys.platform != 'win32':
                USE_DAEMON = 1
        elif o=='-r': READ_ONLY=1
        elif o=='-t':
            try: v=int(v)
            except: raise 'Invalid number of threads', v
            NUMBER_OF_THREADS=v

        elif o=='-i':
            try: v=int(v)
            except: raise 'Invalid value for -i option', v
            sys.setcheckinterval(v)

        elif o=='-a': IP_ADDRESS=v
        elif o=='-d':
            if v=='-': v=''
            DNS_IP=v
        elif o=='-u': UID=v
        elif o=='-D':
            os.environ['Z_DEBUG_MODE']='1'
            DEBUG=1
        elif o=='-S': sys.ZMANAGED=1
        elif o=='-X':
            MONITOR_PORT=HTTP_PORT=HTTPS_PORT=FTP_PORT=FCGI_PORT=ICP_PORT=0
            WEBDAV_SOURCE_PORT=0
            PCGI_FILE=''
        elif o=='-m':
            MONITOR_PORT=server_info(MONITOR_PORT, v)
        elif o=='-w':
            HTTP_PORT=server_info(HTTP_PORT, v)
        elif o=='-y':
            HTTPS_PORT=server_info(HTTPS_PORT, v)
        elif o=='-C' or o=='--force-http-connection-close':
            FORCE_HTTP_CONNECTION_CLOSE=1
        elif o=='-W':
            WEBDAV_SOURCE_PORT=server_info(WEBDAV_SOURCE_PORT, v)
        elif o=='-f':
            FTP_PORT=server_info(FTP_PORT, v)
        elif o=='-P':
            HTTP_PORT=server_info(HTTP_PORT, v, 80)
            HTTPS_PORT=server_info(HTTPS_PORT, v, 443)
            FTP_PORT=server_info(FTP_PORT, v, 21)
        elif o=='--icp':
            ICP_PORT=server_info(ICP_PORT, v)

        elif o=='-p':
            if v=='-': v=''
            PCGI_FILE=v
        elif o=='-h':
            print __doc__ % vars()
            sys.exit(0)
        elif o=='-2': MODULE='Main'
        elif o=='-l': LOG_FILE=v
        elif o=='-L':
            if v: LOCALE_ID=v
            else: LOCALE_ID=''
        elif o=='-F':
            if v=='-': v=''
            FCGI_PORT=v
        elif o=='-M': DETAILED_LOG_FILE=v

except SystemExit: sys.exit(0)
except:
    print __doc__ % vars()
    print
    print 'Error:'
    print "%s: %s" % (sys.exc_type, sys.exc_value)
    sys.exit(1)

#
########################################################################

########################################################################
# OK, let's get going!

# Jigger path:
sys.path=[os.path.join(here,'lib','python'),here
          ]+filter(None, sys.path)



# Try to set the locale if specified on the command
# line. If the locale module is not available or the
# requested locale is not supported by the local
# machine, raise an error so that the user is made
# aware of the problem.

def set_locale(val):
    try:
        import locale
    except:
        raise SystemExit, (
            'The locale module could not be imported.\n'
            'To use localization options, you must ensure\n'
            'that the locale module is compiled into your\n'
            'Python installation.'
            )
    try:
        locale.setlocale(locale.LC_ALL, val)
    except:
        raise SystemExit, (
            'The specified locale is not supported by your system.\n'
            'See your operating system documentation for more\n'
            'information on locale support.'
            )
if LOCALE_ID is not None:
    set_locale(LOCALE_ID)

import zdaemon
# from this point forward we can use the zope logger
# importing ZDaemon before importing ZServer causes ZServer logging
# not to work.

# Import ZServer before we open the database or get at interesting
# application code so that ZServer's asyncore gets to be the
# official one. Also gets SOFTWARE_HOME, INSTANCE_HOME, and CLIENT_HOME
import ZServer

# install signal handlers if on posix
if os.name == 'posix':
    from Signals import Signals
    Signals.registerZopeSignals()

# Location of the ZServer pid file. When Zope starts up it will write
# its PID to this file.  If Zope is run under zdaemon control, zdaemon
# will write to this pidfile instead of Zope.
PID_FILE=os.path.join(CLIENT_HOME, 'Z2.pid')

if USE_DAEMON and not READ_ONLY:
    import App.FindHomes
    sys.ZMANAGED=1
    # zdaemon.run creates a process which "manages" the actual Zope
    # process (restarts it if it dies).  The management process passes along
    # signals that it receives to its child.
    zdaemon.run(sys.argv, os.path.join(CLIENT_HOME, PID_FILE))

os.chdir(CLIENT_HOME)

def _warn_nobody():
    zLOG.LOG("z2", zLOG.INFO, ("Running Zope as 'nobody' can compromise "
                               "your Zope files; consider using a "
                               "dedicated user account for Zope") )

try:
    # Import logging support
    import zLOG
    import ZLogger

    if READ_ONLY:
        if hasattr(zLOG, '_set_stupid_dest'):
            zLOG._set_stupid_dest(sys.stderr)
        else:
            zLOG._stupid_dest = sys.stderr
    else:
        zLOG.log_write = ZLogger.ZLogger.log_write

    if DETAILED_LOG_FILE:
        from ZServer import DebugLogger
        logfile=os.path.join(CLIENT_HOME, DETAILED_LOG_FILE)
        zLOG.LOG('z2', zLOG.BLATHER,
                 'Using detailed request log file %s' % logfile)
        DL=DebugLogger.DebugLogger(logfile)
        DebugLogger.log=DL.log
        DebugLogger.reopen=DL.reopen
        sys.__detailedlog=DL

    # Import Zope (or Main)
    if MODULE == 'Zope':
        import Zope
        Zope.startup()
    else:
        exec "import "+MODULE in {}

    # Location of the ZServer log file. This file logs all ZServer activity.
    # You may wish to create different logs for different servers. See
    # medusa/logger.py for more information.
    if not os.path.isabs(LOG_FILE):
        LOG_PATH=os.path.join(CLIENT_HOME, LOG_FILE)
    else:
        LOG_PATH=LOG_FILE

    # import ZServer stuff

    # First, we need to increase the number of threads
    if MODULE=='Zope':
        from ZServer import setNumberOfThreads
        setNumberOfThreads(NUMBER_OF_THREADS)

    from ZServer import resolver, logger, asyncore

    from ZServer import zhttp_server, zhttp_handler
    from ZServer.WebDAVSrcHandler import WebDAVSrcHandler
    from ZServer import PCGIServer,FTPServer,FCGIServer

    from ZServer import secure_monitor_server

    ## ZServer startup
    ##

    # Resolver and Logger, used by other servers
    if DNS_IP:
        rs = resolver.caching_resolver(DNS_IP)
    else:
        rs=None

    if READ_ONLY:
        lg = logger.file_logger('-') # log to stdout
        zLOG.LOG('z2', zLOG.BLATHER, 'Logging access log to stdout')
    elif os.environ.has_key('ZSYSLOG_ACCESS'):
        if os.environ.has_key("ZSYSLOG_ACCESS_FACILITY"):
            lg = logger.syslog_logger(
                os.environ['ZSYSLOG_ACCESS'],
                facility=os.environ['ZSYSLOG_ACCESS_FACILITY'])
        else:
            lg = logger.syslog_logger(os.environ['ZSYSLOG_ACCESS'])
        zLOG.LOG('z2', zLOG.BLATHER, 'Using local syslog access log')
    elif os.environ.has_key('ZSYSLOG_ACCESS_SERVER'):
        (addr, port) = os.environ['ZSYSLOG_ACCESS_SERVER'].split( ':')
        lg = logger.syslog_logger((addr, int(port)))
        zLOG.LOG('z2', zLOG.BLATHER, 'Using remote syslog access log')
    else:
        lg = logger.file_logger(LOG_PATH)
        zLOG.LOG('z2', zLOG.BLATHER, 'Using access log file %s' % LOG_PATH)
    sys.__lg = lg

    port_err=('\n\nZope wants to use %(socktype)s port %(port)s for its '
              '%(protocol)s service, but it is already in use by another '
              'application on this machine.  Either shut the application down '
              'which is using this port, or start Zope with a different '
              '%(protocol)s port via the "%(switch)s" command-line switch.\n')

    # HTTP Server
    if HTTP_PORT:
        if isinstance(HTTP_PORT, IntType): HTTP_PORT=((IP_ADDRESS, HTTP_PORT),)
        for address, port in HTTP_PORT:
            try:
                hs = zhttp_server(
                    ip=address,
                    port=port,
                    resolver=rs,
                    logger_object=lg)
            except socket.error, why:
                if why[0] == 98: # address in use
                    raise port_err % {'port':port,
                                      'socktype':'TCP',
                                      'protocol':'HTTP',
                                      'switch':'-w'}
                raise
            # Handler for a published module. zhttp_handler takes 3 arguments:
            # The name of the module to publish, and optionally the URI base
            # which is basically the SCRIPT_NAME, and optionally a dictionary
            # with CGI environment variables which override default
            # settings. The URI base setting is useful when you want to
            # publish more than one module with the same HTTP server. The CGI
            # environment setting is useful when you want to proxy requests
            # from another web server to ZServer, and would like the CGI
            # environment to reflect the CGI environment of the other web
            # server.
            zh = zhttp_handler(MODULE, '', HTTP_ENV)
            if FORCE_HTTP_CONNECTION_CLOSE:
                zh._force_connection_close = 1
            hs.install_handler(zh)

    # HTTPS Server
    if HTTPS_PORT:
        if isinstance(HTTPS_PORT, IntType): HTTPS_PORT=((IP_ADDRESS, HTTPS_PORT),)
        for address, port in HTTPS_PORT:
            try:
                hs = zhttp_server(
                    ip=address,
                    port=port,
                    resolver=rs,
                    logger_object=lg)
            except socket.error, why:
                if why[0] == 98: # address in use
                    raise port_err % {'port':port,
                                      'socktype':'TCP',
                                      'protocol':'HTTPS',
                                      'switch':'-y'}
                raise
            # Handler for a published module. zhttp_handler takes 3 arguments:
            # The name of the module to publish, and optionally the URI base
            # which is basically the SCRIPT_NAME, and optionally a dictionary
            # with CGI environment variables which override default
            # settings. The URI base setting is useful when you want to
            # publish more than one module with the same HTTP server. The CGI
            # environment setting is useful when you want to proxy requests
            # from another web server to ZServer, and would like the CGI
            # environment to reflect the CGI environment of the other web
            # server.

            try:
                del HTTPS_ENV['HTTP']
            except KeyError:
                pass
            HTTPS_ENV['HTTPS']='ON'

            zh = zhttp_handler(MODULE, '', HTTPS_ENV)
            if FORCE_HTTP_CONNECTION_CLOSE:
                zh._force_connection_close = 1
            hs.install_handler(zh)

    # WebDAV source Server (runs HTTP, but munges request to return
    #  'manage_FTPget').
    if WEBDAV_SOURCE_PORT:
        if isinstance(WEBDAV_SOURCE_PORT, IntType):
            WEBDAV_SOURCE_PORT=((IP_ADDRESS, WEBDAV_SOURCE_PORT),)
        for address, port in WEBDAV_SOURCE_PORT:
            try:
                hs = zhttp_server(
                    ip=address,
                    port=port,
                    resolver=rs,
                    logger_object=lg)
            except socket.error, why:
                if why[0] == 98: # address in use
                    raise port_err % {'port':port,
                                      'socktype':'TCP',
                                      'protocol':'WebDAV source',
                                      'switch':'-W'}
                raise

            # Handler for a published module. zhttp_handler takes 3 arguments:
            # The name of the module to publish, and optionally the URI base
            # which is basically the SCRIPT_NAME, and optionally a dictionary
            # with CGI environment variables which override default
            # settings. The URI base setting is useful when you want to
            # publish more than one module with the same HTTP server. The CGI
            # environment setting is useful when you want to proxy requests
            # from another web server to ZServer, and would like the CGI
            # environment to reflect the CGI environment of the other web
            # server.
            zh = WebDAVSrcHandler(MODULE, '', HTTP_ENV)
            hs.install_handler(zh)

            # enable document retrieval of the document source on the
            # standard HTTP port

            clients = os.environ.get('WEBDAV_SOURCE_PORT_CLIENTS')
            if clients:
                import re
                sys.WEBDAV_SOURCE_PORT_CLIENTS = re.compile(clients).search
            else:
                sys.WEBDAV_SOURCE_PORT_CLIENTS = None


    # FTP Server
    if FTP_PORT:
        if isinstance(FTP_PORT, IntType): FTP_PORT=((IP_ADDRESS, FTP_PORT),)
        for address, port in FTP_PORT:
            try:
                FTPServer(
                   module=MODULE,
                   ip=address,
                   port=port,
                   resolver=rs,
                   logger_object=lg)
            except socket.error, why:
                if why[0] == 98: # address in use
                    raise port_err % {'port':port,
                                      'socktype':'TCP',
                                      'protocol':'FTP',
                                      'switch':'-f'}
                raise

    # PCGI Server
    if PCGI_FILE and not READ_ONLY:
        PCGI_FILE=os.path.join(here, PCGI_FILE)
        if os.path.exists(PCGI_FILE):
            zpcgi = PCGIServer(
                module=MODULE,
                ip=IP_ADDRESS,
                pcgi_file=PCGI_FILE,
                resolver=rs,
                logger_object=lg)


    # FastCGI Server
    if FCGI_PORT and not READ_ONLY:
        fcgiPort = None
        fcgiPath = None
        try:
            fcgiPort = int(FCGI_PORT)
        except ValueError:
            fcgiPath = FCGI_PORT
        try:
            zfcgi = FCGIServer(module=MODULE,
                               ip=IP_ADDRESS,
                               port=fcgiPort,
                               socket_file=fcgiPath,
                               resolver=rs,
                               logger_object=lg)
        except socket.error, why:
            if why[0] == 98: # address in use
                raise port_err % {'port':fcgiPort,
                                  'socktype':'TCP',
                                  'protocol':'FastCGI',
                                  'switch':'-F'}
            raise


    # Monitor Server
    if MONITOR_PORT:
        from AccessControl.User import emergency_user
        if not hasattr(emergency_user, '__null_user__'):
            pw = emergency_user._getPassword()
        else:
            pw = None
            zLOG.LOG("z2", zLOG.WARNING, 'Monitor server not started'
                     ' because no emergency user exists.')
        if pw:
            if isinstance(MONITOR_PORT, IntType):
                MONITOR_PORT=((IP_ADDRESS, MONITOR_PORT),)
            for address, port in MONITOR_PORT:
                try:
                    monitor=secure_monitor_server(
                        password=pw,
                        hostname=address,
                        port=port)
                except socket.error, why:
                    if why[0] == 98: # address in use
                        raise port_err % {'port':port,
                                          'socktype':'TCP',
                                          'protocol':'monitor server',
                                          'switch':'-m'}
                    raise

    if ICP_PORT:
        if isinstance(ICP_PORT, IntType): ICP_PORT=((IP_ADDRESS, ICP_PORT),)
        from ZServer.ICPServer import ICPServer
        for address, port in ICP_PORT:
            try:
                ICPServer(address,port)
            except socket.error, why:
                if why[0] == 98: # address in use
                    raise port_err % {'port':port,
                                      'socktype':'UDP',
                                      'protocol':'ICP',
                                      'switch':'--icp'}
                raise

    if not USE_DAEMON and not READ_ONLY:
        if os.path.exists(PID_FILE): os.unlink(PID_FILE)
        pf = open(PID_FILE, 'w')
        pid='%s\n' % os.getpid()
        pf.write(pid)
        pf.close()

    # Warn if we were started as nobody.
    try:
        import pwd
        if os.getuid():
            if pwd.getpwuid(os.getuid())[0] == 'nobody':
                _warn_nobody()
    except:
        pass

    # Drop root privileges if we have them, and do some sanity checking
    # to make sure we're not starting with an obviously insecure setup.
    try:
        if os.getuid() == 0:
            try:
                import initgroups
            except:
                raise SystemExit, 'initgroups is required to safely setuid'
            if UID == None:
                raise SystemExit, ('A user was not specified to setuid '
                                   'to; fix this to start as root (see '
                                   'doc/SETUID.txt)')
            import stat
            client_home_stat = os.stat(CLIENT_HOME)
            client_home_faults = []
            if not (client_home_stat[stat.ST_MODE]&01000):
                client_home_faults.append('does not have the sticky bit set')
            if client_home_stat[stat.ST_UID] != 0:
                client_home_faults.append('is not owned by root')
            if client_home_faults:
                client_home_faults.append('fix this to start as root (see '
                                          'doc/SETUID.txt)')
                err = '%s %s' % (CLIENT_HOME, ', '.join(client_home_faults))
                raise SystemExit, err

            try:
                try:    UID = string.atoi(UID)
                except: pass
                gid = None
                if isinstance(UID, StringType):
                    uid = pwd.getpwnam(UID)[2]
                    gid = pwd.getpwnam(UID)[3]
                elif isinstance(UID, IntType):
                    uid = pwd.getpwuid(UID)[2]
                    gid = pwd.getpwuid(UID)[3]
                    UID = pwd.getpwuid(UID)[0]
                else:
                    raise KeyError
                if UID == 'nobody':
                    _warn_nobody()
                try:
                    initgroups.initgroups(UID, gid)
                    if gid is not None:
                        try:
                            os.setgid(gid)
                        except OSError:
                            pass
                    os.setuid(uid)
                except OSError:
                    pass
            except KeyError:
                zLOG.LOG("z2", zLOG.ERROR, ("Can't find UID %s" % UID))
    except AttributeError:
        pass
    except:
        raise

    # Check umask sanity if we're on posix.
    if os.name == 'posix' and not os.environ.get('Z_DEBUG_MODE'):
        # umask is silly, blame POSIX.  We have to set it to get its value.
        current_umask = os.umask(0)
        os.umask(current_umask)
        if current_umask != 077:
            current_umask = '%03o' % current_umask
            zLOG.LOG("z2", zLOG.INFO, (
                'Your umask of %s may be too permissive; for the security of '
                'your Zope data, it is recommended you use 077' % current_umask
                ))

except:
    # Log startup exception and tell zdaemon not to restart us.
    try:
        zLOG.LOG("z2", zLOG.PANIC, "Startup exception",
                 error=sys.exc_info())
    except: pass
    sys.exit(0)

# Start Medusa, Ye Hass!
sys.ZServerExitCode=0
asyncore.loop()
sys.exit(sys.ZServerExitCode)
