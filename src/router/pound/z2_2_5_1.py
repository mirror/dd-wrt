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

  -Z path

    Unix only! This option is ignored on windows.

    If this option is specified, a separate managemnt process will
    be created that restarts Zope after a shutdown (or crash).
    The path must point to a pid file that the process will record its
    process id in. The path may be relative, in which case it will be
    relative to the Zope location.

    To prevent use of a separate management process, provide an
    empty string: -Z ''

  -t n

    The number of threads to use, if ZODB3 is used. The default is
    %(NUMBER_OF_THREADS)s.

  -i n

    Set the interpreter check interval. This integer value
    determines how often the interpreter checks for periodic things
    such as thread switches and signal handlers. The Zope default
    is 120, but you may want to experiment with other values that
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
  
    The username to run ZServer as. You may want to run ZServer as 'nobody'
    or some other user with limited resouces. The only works under Unix, and
    if ZServer is started by root. The default is: %(UID)s

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
  
    The encrypted Web server (HTTPS) port.  This defaults to %(HTTPS_PORT)s.
    The standard port for HTTP services is 443.  If this is a dash
    (e.g. -y -), then HTTPS is disabled.

    The number can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -y options can be provided to run multiple servers.

  -W port
  
    The "WebDAV source" port.  If this is a dash (e.g. -W -), then
    "WebDAV source" is disabled.  The default is disabled.  Note that
    this feature is a workaround for the lack of "source-link" support
    in standard WebDAV clients.

    The port can be preeceeded by an ip address follwed by a colon
    to specify an address to listen on. This allows different servers
    to listen on different addresses.

    Multiple -W options can be provided to run multiple servers.

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


import os, sys, getopt, string, codecs
# workaround to allow unicode encoding conversions in DTML
dummy = codecs.lookup('iso-8859-1')

sys.setcheckinterval(120)


program=sys.argv[0]
here=os.path.join(os.getcwd(), os.path.split(program)[0])
Zpid=''

########################################################################
# Configuration section

## General configuration options
##

# If you want run as a daemon, then uncomment the line below:
if sys.platform=='win32': Zpid=''
else: Zpid='zProcessManager.pid'

# This is the IP address of the network interface you want your servers to
# be visible from.  This can be changed to '' to listen on all interfaces.
IP_ADDRESS=''

# IP address of your DNS server. Set to '' if you do not want to resolve
# IP addresses. If you have DNS service on your local machine then you can
# set this to '127.0.0.1'
DNS_IP=''

# User id to run ZServer as. Note that this only works under Unix, and if
# ZServer is started by root.
UID='nobody'

# Log file location. If this is a relative path, then it is joined the
# the 'var' directory.
LOG_FILE='Z2.log'

## HTTP configuration
##

# Port for HTTP Server. The standard port for HTTP services is 80.
HTTP_PORT=8080

# HTTP enivornment settings.
HTTP_ENV={}

# Port for HTTPS Server. The standard port for HTTPS services is 443.
HTTPS_PORT=8443

# HTTPS enivornment settings.
HTTPS_ENV={}

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

#
########################################################################

########################################################################
# Handle command-line arguments:

def server_info(old, v, offset=0):
    # interpret v as a port or address/port and get new value
    if v == '-': v=''
    l=string.find(v, ':')
    if l >= 0:
        a=v[:l]
        v=v[l+1:]
    else:
        a=IP_ADDRESS

    if not v: return v
        
    try: 
        v=string.atoi(v)
        if v < 0: raise 'Invalid port', v
        v=v+offset
    except: raise 'Invalid port', v

    if type(old) is type(0): old=[(a,v)]
    else: old.append((a,v))

    return old    
    

try:
    if string.split(sys.version)[0] < '2.1':
        raise 'Invalid python version', string.split(sys.version)[0]

    opts, args = getopt.getopt(sys.argv[1:],
                               'hz:Z:t:i:a:d:u:w:y:W:f:p:m:Sl:2DP:rF:L:XM:')

    DEBUG=0
    READ_ONLY=0
    
    # Get environment variables
    for a in args:
        if string.find(a,'='):
            a=string.split(a,'=')
            o=a[0]
            v=string.join(a[1:],'=')
            if o: 
              os.environ[o]=v
              HTTP_ENV[o]=v
              HTTPS_ENV[o]=v
        else:
            raise 'Invalid argument', a

    for o, v in opts:
        if o=='-z': here=v
        elif o=='-Z':
            if v=='-': v=''
            Zpid=v
        elif o=='-r': READ_ONLY=1
        elif o=='-t':
            try: v=string.atoi(v)
            except: raise 'Invalid number of threads', v
            NUMBER_OF_THREADS=v

        elif o=='-i':
            try: v=string.atoi(v)
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
            MONITOR_PORT=HTTP_PORT=HTTPS_PORT=FTP_PORT=FCGI_PORT=0
            PCGI_FILE=''
        elif o=='-m':
            MONITOR_PORT=server_info(MONITOR_PORT, v)
        elif o=='-w':
            HTTP_PORT=server_info(HTTP_PORT, v)
        elif o=='-y':
            HTTPS_PORT=server_info(HTTPS_PORT, v)
        elif o=='-W':
            WEBDAV_SOURCE_PORT=server_info(WEBDAV_SOURCE_PORT, v)
        elif o=='-f':
            FTP_PORT=server_info(FTP_PORT, v)
        elif o=='-P':
            HTTP_PORT=server_info(HTTP_PORT, v, 80)
            HTTPS_PORT=server_info(HTTPS_PORT, v, 443)
            FTP_PORT=server_info(FTP_PORT, v, 21)

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

if sys.platform=='win32': Zpid=''

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


# from this point forward we can use the zope logger

# Import ZServer before we open the database or get at interesting
# application code so that ZServer's asyncore gets to be the
# official one. Also gets SOFTWARE_HOME, INSTANCE_HOME, and CLIENT_HOME
import ZServer

if Zpid and not READ_ONLY:
    import zdaemon, App.FindHomes, posix
    sys.ZMANAGED=1
    
    zdaemon.run(sys.argv, os.path.join(CLIENT_HOME, Zpid))

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
        DebugLogger.log=DebugLogger.DebugLogger(logfile).log

    # Import Zope (or Main)
    exec "import "+MODULE in {}

    # Location of the ZServer log file. This file logs all ZServer activity.
    # You may wish to create different logs for different servers. See
    # medusa/logger.py for more information.
    if not os.path.isabs(LOG_FILE):
        LOG_PATH=os.path.join(CLIENT_HOME, LOG_FILE)
    else:
        LOG_PATH=LOG_FILE

    # Location of the ZServer pid file. When ZServer starts up it will write
    # its PID to this file.
    PID_FILE=os.path.join(CLIENT_HOME, 'Z2.pid')


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
    elif os.environ.has_key('ZSYSLOG'):
        lg = logger.syslog_logger(os.environ['ZSYSLOG'])
        if os.environ.has_key("ZSYSLOG_FACILITY"):
            lg = logger.syslog_logger(os.environ['ZSYSLOG'],facility=os.environ['ZSYSLOG_FACILITY'])
        else:
            lg = logger.syslog_logger(os.environ['ZSYSLOG'])
    elif os.environ.has_key('ZSYSLOG_SERVER'):
        (addr, port) = string.split(os.environ['ZSYSLOG_SERVER'], ':')
        lg = logger.syslog_logger((addr, int(port)))
    else:
        lg = logger.file_logger(LOG_PATH)

    # HTTP Server
    if HTTP_PORT:
        if type(HTTP_PORT) is type(0): HTTP_PORT=((IP_ADDRESS, HTTP_PORT),)
        for address, port in HTTP_PORT:
            hs = zhttp_server(
                ip=address,
                port=port,
                resolver=rs,
                logger_object=lg)

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
            hs.install_handler(zh)

    # HTTPS Server
    if HTTPS_PORT:
        if type(HTTPS_PORT) is type(0): HTTPS_PORT=((IP_ADDRESS, HTTPS_PORT),)
        for address, port in HTTPS_PORT:
            hs = zhttp_server(
                ip=address,
                port=port,
                resolver=rs,
                logger_object=lg)

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
            hs.install_handler(zh)

    # WebDAV source Server (runs HTTP, but munges request to return
    #  'manage_FTPget').
    if WEBDAV_SOURCE_PORT:
        if type(WEBDAV_SOURCE_PORT) is type(0):
            WEBDAV_SOURCE_PORT=((IP_ADDRESS, WEBDAV_SOURCE_PORT),)
        for address, port in WEBDAV_SOURCE_PORT:
            hs = zhttp_server(
                ip=address,
                port=port,
                resolver=rs,
                logger_object=lg)

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

    # FTP Server
    if FTP_PORT:
        if type(FTP_PORT) is type(0): FTP_PORT=((IP_ADDRESS, FTP_PORT),)
        for address, port in FTP_PORT:
            FTPServer(
               module=MODULE,
               ip=address,
               port=port,
               resolver=rs,
               logger_object=lg)

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
            fcgiPort = string.atoi(FCGI_PORT)
        except ValueError:
            fcgiPath = FCGI_PORT
        zfcgi = FCGIServer(module=MODULE,
                           ip=IP_ADDRESS,
                           port=fcgiPort,
                           socket_file=fcgiPath,
                           resolver=rs,
                           logger_object=lg)


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
            if type(MONITOR_PORT) is type(0): 
                MONITOR_PORT=((IP_ADDRESS, MONITOR_PORT),)
            for address, port in MONITOR_PORT:
                monitor=secure_monitor_server(
                    password=pw,
                    hostname=address,
                    port=port)

    # Try to set uid to "-u" -provided uid.
    # Try to set gid to  "-u" user's primary group. 
    # This will only work if this script is run by root.
    try:
        import pwd
        try:
            try:    UID = string.atoi(UID)
            except: pass
            gid = None
            if type(UID) == type(""):
                uid = pwd.getpwnam(UID)[2]
                gid = pwd.getpwnam(UID)[3]
            elif type(UID) == type(1):
                uid = pwd.getpwuid(UID)[2]
                gid = pwd.getpwuid(UID)[3]
            else:
                raise KeyError 
            try:
                if gid is not None:
                    try:
                        os.setgid(gid)
                    except OSError:
                        pass
                os.setuid(uid)
            except OSError:
                pass
        except KeyError:
            zLOG.LOG("z2", zLOG.ERROR, ("can't find UID %s" % UID))
    except:
        pass



    # if it hasn't failed at this point, create a .pid file.
    if not READ_ONLY:
        pf = open(PID_FILE, 'w')
        pid=str(os.getpid())
        try: pid=str(os.getppid())+' '+pid
        except: pass
        pf.write(pid)
        pf.close()

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
