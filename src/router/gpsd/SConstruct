### SCons build recipe for the GPSD project

# Important targets:
#
# build     - build the software (default)
# dist      - make distribution tarball
# install   - install programs, libraries, and manual pages
# uninstall - undo an install
#
# check     - run regression and unit tests.
# audit     - run code-auditing tools
# testbuild - test-build the code from a tarball
# release   - ship a release
#
# Setting the DESTDIR environment variable will prefix the install destinations
# without changing the --prefix prefix.

# Unfinished items:
# * Out-of-directory builds: see http://www.scons.org/wiki/UsingBuildDir

# Release identification begins here
gpsd_version = "3.9"

# library version
libgps_version_current   = 20
libgps_version_revision  = 0
libgps_version_age       = 0
libgpsd_version_current  = 21
libgpsd_version_revision = 0
libgpsd_version_age      = 0

# Release identification ends here

# Hosting information (mainly used for templating web pages) begins here
# Each variable foo has a corresponding @FOO@ expanded in .in files.
# There are no project-dependent URLs or references to the hosting site
# anywhere else in the distribution; preserve this property!
sitename   = "Savannah"
sitesearch = "catb.org"
website    = "http://catb.org/gpsd" 
mainpage   = "https://savannah.nongnu.org/projects/gpsd/"
webupload  = "login.ibiblio.org:/public/html/catb/gpsd"
cgiupload  = "thyrsus.com:/home/www/thyrsus.com/cgi-bin/"
scpupload  = "dl.sv.nongnu.org:/releases/gpsd/"
mailman    = "http://lists.nongnu.org/mailman/listinfo/"
admin      = "https://savannah.nongnu.org/project/admin/?group=gpsd"
download   = "http://download-mirror.savannah.gnu.org/releases/gpsd/"
bugtracker = "https://savannah.nongnu.org/bugs/?group=gpsd"
browserepo = "http://git.savannah.gnu.org/cgit/gpsd.git"
clonerepo  = "https://savannah.nongnu.org/git/?group=gpsd"
gitrepo    = "git://git.savannah.nongnu.org/gpsd.git"
webform    = "http://www.thyrsus.com/cgi-bin/gps_report.cgi"
formserver = "www@thyrsus.com"
devmail    = "gpsd-dev@lists.nongnu.org"
# Hosting information ends here

EnsureSConsVersion(2,0,1)

import copy, os, sys, glob, re, platform, time
from distutils import sysconfig
from distutils.util import get_platform
import SCons

# replacement for functions from the commands module, which is deprecated.
from subprocess import PIPE, STDOUT, Popen
def _getstatusoutput(cmd, input=None, cwd=None, env=None):
    pipe = Popen(cmd, shell=True, cwd=cwd, env=env, stdout=PIPE, stderr=STDOUT)
    (output, errout) = pipe.communicate(input=input)
    status = pipe.returncode
    return (status, output)
def _getoutput(cmd, input=None, cwd=None, env=None):
    return _getstatusoutput(cmd, input, cwd, env)[1]


#
# Build-control options
#

# Start by reading configuration variables from the cache
opts = Variables('.scons-option-cache')

systemd = os.path.exists("/usr/share/systemd/system")

# Set distribution-specific defaults here
imloads = True

# Does our platform has a working memory-barrier instruction?
# The shared-memory export won't be reliable without it.
mfence = (platform.machine() in ('x86_64',))
    
boolopts = (
    # GPS protocols
    ("nmea",          True,  "NMEA support"),
    ("ashtech",       True,  "Ashtech support"),
    ("earthmate",     True,  "DeLorme EarthMate Zodiac support"),
    ("evermore",      True,  "EverMore binary support"),
    ("fv18",          True,  "San Jose Navigation FV-18 support"),
    ("garmin",        True,  "Garmin kernel driver support"),
    ("garmintxt",     True,  "Garmin Simple Text support"),
    ("geostar",       True,  "Geostar Protocol support"),
    ("itrax",         True,  "iTrax hardware support"),
    ("mtk3301",       True,  "MTK-3301 support"),
    ("navcom",        True,  "Navcom support"),
    ("oncore",        True,  "Motorola OnCore chipset support"),
    ("sirf",          True,  "SiRF chipset support"),
    ("superstar2",    True,  "Novatel SuperStarII chipset support"),
    ("tnt",           True,  "True North Technologies support"),
    ("tripmate",      True,  "DeLorme TripMate support"),
    ("tsip",          True,  "Trimble TSIP support"),
    ("ubx",           True,  "UBX Protocol support"),
    ("fury",          True,  "Jackson Labs Fury and Firefly support"),
    ("nmea2000",      True,  "NMEA2000/CAN support"),
    # Non-GPS protocols
    ("aivdm",         True,  "AIVDM support"),
    ("gpsclock",      True,  "GPSClock support"),
    ("ntrip",         True,  "NTRIP support"),
    ("oceanserver",   True,  "OceanServer support"),
    ("rtcm104v2",     True,  "rtcm104v2 support"),
    ("rtcm104v3",     True,  "rtcm104v3 support"),
    # Time service
    ("ntpshm",        True,  "NTP time hinting support"),
    ("pps",           True,  "PPS time syncing support"),
    # Export methods
    ("socket_export", True,  "data export over sockets"),
    ("dbus_export",   False, "enable DBUS export support"),
    ("shm_export",    mfence,"export via shared memory"),
    # Communication
    ('usb',           True,  "libusb support for USB devices"),
    ("bluez",         True,  "BlueZ support for Bluetooth devices"),
    ("ipv6",          True,  "build IPv6 support"),
    ("netfeed",       True,  "build support for handling TCP/IP data sources"),
    ("passthrough",   True,  "build support for passing through JSON"),
    # Other daemon options
    ("force_global",  False, "force daemon to listen on all addressses"),
    ("timing",        False, "latency timing support"),
    ("control_socket",True,  "control socket for hotplug notifications"),
    ("systemd",       systemd, "systemd socket activation"),
    # Client-side options
    ("clientdebug",   True,  "client debugging support"),
    ("oldstyle",      True,  "oldstyle (pre-JSON) protocol support"),
    ("libgpsmm",      True,  "build C++ bindings"),
    ("libQgpsmm",     True,  "build QT bindings"),
    ("reconfigure",   True,  "allow gpsd to change device settings"),
    ("controlsend",   True,  "allow gpsctl/gpsmon to change device settings"),
    ("cheapfloats",   True,  "float ops are cheap, compute error estimates"),
    ("squelch",       False, "squelch gpsd_report/gpsd_hexdump to save cpu"),
    ("ncurses",       True,  "build with ncurses"),
    # Build control
    ("shared",        True,  "build shared libraries, not static"),
    ("implicit_link", imloads,"implicit linkage is supported in shared libs"),
    ("python",        True,  "build Python support and modules."),
    ("debug",         False, "include debug information in build"),
    ("profiling",     False, "build with profiling enabled"),
    ("coveraging",    False, "build with code coveraging enabled"),
    ("strip",         True,  "build with stripping of binaries enabled"),
    ("chrpath",       True,  "use chrpath to edit library load paths"),
    )
for (name, default, help) in boolopts:
    opts.Add(BoolVariable(name, help, default))

nonboolopts = (
    ("gpsd_user",           "nobody",      "privilege revocation user",),
    ("gpsd_group",          "dialout",     "privilege revocation group"),
    ("prefix",              "/usr/local",  "installation directory prefix"),
    ("limited_max_clients", 0,             "maximum allowed clients"),
    ("limited_max_devices", 0,             "maximum allowed devices"),
    ("fixed_port_speed",    0,             "fixed serial port speed"),
    ("fixed_stop_bits",     0,             "fixed serial port stop bits"),
    ("target",              "",            "cross-development target"),
    ("sysroot",             "",            "cross-development system root"),
    )
for (name, default, help) in nonboolopts:
    opts.Add(name, help, default)

pathopts = (
    ("sysconfdir",          "etc",           "system configuration directory"),
    ("bindir",              "bin",           "application binaries directory"),
    ("includedir",          "include",       "header file directory"),
    ("libdir",              "lib",           "system libraries"),
    ("sbindir",             "sbin",          "system binaries directory"),
    ("mandir",              "share/man",     "manual pages directory"),
    ("docdir",              "share/doc",     "documents directory"),
    ("udevdir",             "/lib/udev",     "udev rules directory"),
    ("pkgconfig",           "$libdir/pkgconfig", "pkgconfig file directory"),
    )
for (name, default, help) in pathopts:
    opts.Add(PathVariable(name, help, default, PathVariable.PathAccept))


#
# Environment creation
#
import_env = (
    "DISPLAY",         # Required for dia to run under scons
    "GROUPS",          # Required by gpg
    "HOME",            # Required by gpg
    "LOGNAME",         # LOGNAME is required for the flocktest production.
    'PATH',            # Required for ccache and Coverity scan-build
    'PKG_CONFIG_PATH', # Set .pc file directory in a crossbuild
    'STAGING_DIR',     # Required by the OpenWRT and CeroWrt builds.
    'STAGING_PREFIX',  # Required by the OpenWRT and CeroWrt builds.
    )
envs = {}
for var in import_env:
    if var in os.environ:
        envs[var] = os.environ[var]
envs["GPSD_HOME"] = os.getcwd()

env = Environment(tools=["default", "tar", "textfile"], options=opts, ENV=envs)
opts.Save('.scons-option-cache', env)
env.SConsignFile(".sconsign.dblite")

for (name, default, help) in pathopts:
    env[name] = env.subst(env[name])

env['VERSION'] = gpsd_version
env['PYTHON'] = sys.executable

# Set defaults from environment.  Note that scons doesn't cope well
# with multi-word CPPFLAGS/LDFLAGS/SHLINKFLAGS values; you'll have to
# explicitly quote them or (better yet) use the "=" form of GNU option
# settings.
env['STRIP'] = "strip"
env['PKG_CONFIG'] = "pkg-config"
env['CHRPATH'] = 'chrpath'
for i in ["AR", "ARFLAGS", "CCFLAGS", "CFLAGS", "CC", "CXX", "CXXFLAGS", "STRIP", "PKG_CONFIG", "CHRPATH", "LD", "TAR"]:
    if os.environ.has_key(i):
        j = i
        if i == "LD":
            i = "SHLINK"
        if i in ("CFLAGS", "CCFLAGS", "LINKFLAGS"):
            env.Replace(**{j: Split(os.getenv(i))})
        else:
            env.Replace(**{j: os.getenv(i)})
for flag in ["LDFLAGS", "LINKFLAGS", "SHLINKFLAGS", "CPPFLAGS"]:
    if os.environ.has_key(flag):
        env.MergeFlags({flag : [os.getenv(flag)]})


# Keep scan-build options in the environment
for key, value in os.environ.iteritems():
    if key.startswith('CCC_'):
        env.Append(ENV={key:value})

# FIXME: Set up compilation for gcov - doesn't work yet.
if env['coveraging']:
    env['CFLAGS'].append(['-fprofile-arcs', '-ftest-coverage'])

# Placeholder so we can kluge together something like VPATH builds.
# $SRCDIR replaces occurrences for $(srcdir) in the autotools build.
env['SRCDIR'] = '.'

def announce(msg):
    if not env.GetOption("silent"):
        print msg

# We need to define -D_GNU_SOURCE
env.Append(CFLAGS='-D_GNU_SOURCE')

# DESTDIR environment variable means user wants to prefix the installation root.
DESTDIR = os.environ.get('DESTDIR', '')

def installdir(dir, add_destdir=True):
    # use os.path.join to handle absolute paths properly.
    wrapped = os.path.join(env['prefix'], env[dir])
    if add_destdir:
        wrapped = os.path.normpath(DESTDIR + os.path.sep + wrapped)
    wrapped.replace("/usr/etc", "/etc")
    return wrapped

# Honor the specified installation prefix in link paths.
if not env["sysroot"]:
    env.Prepend(LIBPATH=[installdir('libdir')])
else:
    env.Prepend(LIBPATH=[env["sysroot"] + installdir('libdir')])
if env["shared"]:
    env.Prepend(RPATH=[installdir('libdir')])

# Give deheader a way to set compiler flags
if 'MORECFLAGS' in os.environ:
    env.Append(CFLAGS=Split(os.environ['MORECFLAGS']))

# Don't change CCFLAGS if already set by environment.
if not 'CCFLAGS' in os.environ:
    # Should we build with profiling?
    if env['profiling']:
        env.Append(CCFLAGS=['-pg'])
        env.Append(LDFLAGS=['-pg'])
    # Should we build with debug symbols?
    if env['debug']:
        env.Append(CCFLAGS=['-g'])
        env.Append(CCFLAGS=['-O0'])
    else:
        env.Append(CCFLAGS=['-O2'])

# Get a slight speedup by not doing automatic RCS and SCCS fetches.
env.SourceCode('.', None)

## Cross-development

devenv = (("ADDR2LINE", "addr2line"),
          ("AR","ar"),
          ("AS","as"),
          ("CHRPATH", "chrpath"),
          ("CXX","c++"),
          ("CXXFILT","c++filt"),
          ("CPP","cpp"),
          ("GXX","g++"),
          ("CC","gcc"),
          ("GCCBUG","gccbug"),
          ("GCOV","gcov"),
          ("GPROF","gprof"),
          ("LD", "ld"),
          ("NM", "nm"),
          ("OBJCOPY","objcopy"),
          ("OBJDUMP","objdump"),
          ("RANLIB", "ranlib"),
          ("READELF","readelf"),
          ("SIZE", "size"),
          ("STRINGS", "strings"),
          ("STRIP", "strip"))

if env['target']:
    for (name, toolname) in devenv:
        env[name] = env['target'] + '-' + toolname

if env['sysroot']:
    env.MergeFlags({"CFLAGS": ["--sysroot=%s" % env['sysroot']]})
    env.MergeFlags({"LINKFLAGS": ["--sysroot=%s" % env['sysroot']]})

## Build help

Help("""Arguments may be a mixture of switches and targets in any order.
Switches apply to the entire build regardless of where they are in the order.
Important switches include:

    prefix=/usr     probably what you want for production tools

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable.  Delete it to start fresh.  Current option
values can be listed with 'scons -h'.

""" + opts.GenerateHelpText(env, sort=cmp))

if "help" in ARGLIST:
    Return()

## Configuration

def CheckPKG(context, name):
    context.Message( 'Checking for %s... ' % name )
    ret = context.TryAction('%s --exists \'%s\'' % (env['PKG_CONFIG'], name))[0]
    context.Result( ret )
    return ret

def CheckExecutable(context, testprogram, check_for):
    context.Message( 'Checking for %s... ' %(check_for,))
    ret = context.TryAction(testprogram)[0]
    context.Result( ret )
    return ret

# Stylesheet URLs for making HTML and man pages from DocBook XML.
docbook_url_stem = 'http://docbook.sourceforge.net/release/xsl/current/'
docbook_man_uri = docbook_url_stem + 'manpages/docbook.xsl'
docbook_html_uri = docbook_url_stem + 'html/docbook.xsl'

def CheckXsltproc(context):
    context.Message('Checking that xsltproc can make man pages... ')
    ofp = open("xmltest.xml", "w")
    ofp.write('''
       <refentry id="foo.1">
      <refmeta>
        <refentrytitle>foo</refentrytitle>
        <manvolnum>1</manvolnum>
        <refmiscinfo class='date'>9 Aug 2004</refmiscinfo>
      </refmeta>
      <refnamediv id='name'>
        <refname>foo</refname>
        <refpurpose>check man page generation from docbook source</refpurpose>
      </refnamediv>
    </refentry>
''')
    ofp.close()
    probe = "xsltproc --nonet --noout '%s' xmltest.xml" % (docbook_man_uri,)
    ret = context.TryAction(probe)[0]
    os.remove("xmltest.xml")
    if os.path.exists("foo.1"):
        os.remove("foo.1")
    context.Result( ret )
    return ret

def CheckCompilerOption(context, option):
    context.Message( 'Checking if compiler accepts %s ...' %(option,) )
    old_CFLAGS=context.env['CFLAGS']
    context.env.Append(CFLAGS=option)
    ret = context.TryLink("""
        int main(int argc, char **argv) {
            return 0;
        }
    """,'.c')
    if not ret:
        context.env.Replace(CFLAGS=old_CFLAGS)
    context.Result(ret)
    return ret

config = Configure(env, custom_tests = { 'CheckPKG' : CheckPKG,
                                         'CheckExecutable' : CheckExecutable,
                                         'CheckXsltproc' : CheckXsltproc,
                                         'CheckCompilerOption' : CheckCompilerOption})


# If supported by the compiler, enable all warnings except uninitialized and
# missing-field-initializers, which we can't help triggering because
# of the way some of the JSON-parsing code is generated.
# Also not including -Wcast-qual and -Wimplicit-function-declaration,
# because we can't seem to keep scons from passing it to g++.
for option in ('-Wextra','-Wall', '-Wno-uninitialized','-Wno-missing-field-initializers',
               '-Wcast-align','-Wmissing-declarations', '-Wmissing-prototypes',
               '-Wstrict-prototypes', '-Wpointer-arith', '-Wreturn-type'):
    config.CheckCompilerOption(option)


env.Prepend(LIBPATH=[os.path.realpath(os.curdir)])
if env["shared"]:
    if env["chrpath"] and config.CheckExecutable('$CHRPATH -v', 'chrpath'):
        # Tell generated binaries to look in the current directory for
        # shared libraries so we can run regression tests without
        # hassle. Should be handled sanely by scons on all systems.  Not
        # good to use '.' or a relative path here; it's a security risk.
        # At install time we use chrpath to edit this out of RPATH.
        env.Prepend(RPATH=[os.path.realpath(os.curdir)])
    else:
        print "chrpath is not available or use of it has been disabled."

confdefs = ["/* gpsd_config.h.  Generated by scons, do not hand-hack.  */\n"]

confdefs.append('#define VERSION "%s"\n' % gpsd_version)

confdefs.append('#define GPSD_URL "%s"\n' % website)

cxx = config.CheckCXX()

# define a helper function for pkg-config - we need to pass
# --static for static linking, too.
if env["shared"]:
    pkg_config = lambda pkg: ['!%s --cflags --libs %s' %(env['PKG_CONFIG'], pkg, )]
else:
    pkg_config = lambda pkg: ['!%s --cflags --libs --static %s' %(env['PKG_CONFIG'], pkg, )]

# The actual distinction here is whether the platform has ncurses in the
# base system or not. If it does, pkg-config is not likely to tell us
# anything useful. FreeBSD does, Linux doesn't. Most likely other BSDs
# are like FreeBSD.
ncurseslibs= []
if env['ncurses']:
    if config.CheckPKG('ncurses'):
        ncurseslibs = pkg_config('ncurses')
    elif config.CheckExecutable('ncurses5-config --version', 'ncurses5-config'):
        ncurseslibs = ['!ncurses5-config --libs --cflags']
    elif config.CheckExecutable('ncursesw5-config --version', 'ncursesw5-config'):
        ncurseslibs = ['!ncursesw5-config --libs --cflags']
    elif sys.platform.startswith('freebsd'):
        ncurseslibs= [ '-lncurses' ]
    elif sys.platform.startswith('openbsd'):
        ncurseslibs= [ '-lcurses' ]

if env['usb']:
    # In FreeBSD except version 7, USB libraries are in the base system
    if config.CheckPKG('libusb-1.0'):
        confdefs.append("#define HAVE_LIBUSB 1\n")
        try:
            usblibs = pkg_config('libusb-1.0')
        except OSError:
            announce("pkg_config is confused about the state of libusb-1.0.")
            usblibs = []
    elif sys.platform.startswith("freebsd"):
        confdefs.append("#define HAVE_LIBUSB 1\n")
        usblibs = [ "-lusb"]
    else:
        confdefs.append("/* #undef HAVE_LIBUSB */\n")
        usblibs = []
else:
    confdefs.append("/* #undef HAVE_LIBUSB */\n")
    usblibs = []
    env["usb"] = False

if config.CheckLib('librt'):
    confdefs.append("#define HAVE_LIBRT 1\n")
    # System library - no special flags
    rtlibs = ["-lrt"]
else:
    confdefs.append("/* #undef HAVE_LIBRT */\n")
    rtlibs = []

if config.CheckLib('libcap'):
    confdefs.append("#define HAVE_LIBCAP 1\n")
    # System library - no special flags
    caplibs = ["-lcap"]
else:
    confdefs.append("/* #undef HAVE_LIBCAP */\n")
    caplibs = []

if env['dbus_export'] and config.CheckPKG('dbus-1'):
    confdefs.append("#define HAVE_DBUS 1\n")
    dbus_libs = pkg_config('dbus-1')
else:
    confdefs.append("/* #undef HAVE_DBUS */\n")
    dbus_libs = []
    env["dbus_export"] = False

if env['bluez'] and config.CheckPKG('bluez'):
    confdefs.append("#define HAVE_BLUEZ 1\n")
    bluezlibs = pkg_config('bluez')
else:
    confdefs.append("/* #undef HAVE_BLUEZ */\n")
    bluezlibs = []
    env["bluez"] = False

# ntpshm is required for pps support
if env['pps'] and env['ntpshm'] and config.CheckHeader("sys/timepps.h"):
    confdefs.append("#define HAVE_SYS_TIMEPPS_H 1\n")
    announce("You have kernel PPS available.")
else:
    confdefs.append("/* #undef HAVE_SYS_TIMEPPS_H */\n")
    announce("You do not have kernel PPS available.")
    # Don't turn off PPS here, we might be using the non-kernel version

if config.CheckHeader(["bits/sockaddr.h", "linux/can.h"]):
    confdefs.append("#define HAVE_LINUX_CAN_H 1\n")
    announce("You have kernel CANbus available.")
else:
    confdefs.append("/* #undef HAVE_LINUX_CAN_H */\n")
    announce("You do not have kernel CANbus available.")
    env["nmea2000"] = False

# check function after libraries, because some function require library
# for example clock_gettime() require librt on Linux
for f in ("daemon", "strlcpy", "strlcat", "clock_gettime"):
    if config.CheckFunc(f):
        confdefs.append("#define HAVE_%s 1\n" % f.upper())
    else:
        confdefs.append("/* #undef HAVE_%s */\n" % f.upper())

# Map options to libraries required to support them that might be absent.
optionrequires = {
    "bluez": ["libbluetooth"],
    "dbus_export" : ["libdbus-1"],
    }

keys = map(lambda x: (x[0],x[2]), boolopts) + map(lambda x: (x[0],x[2]), nonboolopts) + map(lambda x: (x[0],x[2]), pathopts)
keys.sort()
for (key,help) in keys:
    value = env[key]
    if value and key in optionrequires:
        for required in optionrequires[key]:
            if not config.CheckLib(required):
                announce("%s not found, %s cannot be enabled." % (required,key))
                value = False
                break

    confdefs.append("/* %s */" % help)
    if type(value) == type(True):
        if value:
            confdefs.append("#define %s_ENABLE 1\n" % key.upper())
        else:
            confdefs.append("/* #undef %s_ENABLE */\n" % key.upper())
    elif value in (0, "", "(undefined)"):
        confdefs.append("/* #undef %s */\n" % key.upper())
    else:
        if value.isdigit():
            confdefs.append("#define %s %s\n" % (key.upper(), value))
        else:
            confdefs.append("#define %s \"%s\"\n" % (key.upper(), value))

if config.CheckFunc("pselect"):
    confdefs.append("/* #undef COMPAT_SELECT */\n")
else:
    confdefs.append("#define COMPAT_SELECT\n")

confdefs.append('''
/* will not handle pre-Intel Apples that can run big-endian 
   __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are define in some gcc versions
  only, probably depending on the architecture. Try to use endian.h if
  the gcc way fails - endian.h also doesn not seem to be available on all
  platforms.
*/
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#else /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
#undef WORDS_BIGENDIAN
#else
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#undef WORDS_BIGENDIAN
#else
#error "unable to determine endianess!"
#endif /* __BYTE_ORDER */
#endif /* __LITTLE_ENDIAN__ */
#endif /* __BIG_ENDIAN__ */

/* Some libcs do not have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif
#ifndef HAVE_STRLCPY
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif

#define GPSD_CONFIG_H
''')


manbuilder = mangenerator = htmlbuilder = None
if config.CheckXsltproc():
    mangenerator = 'xsltproc'
    build = "xsltproc --nonet %s $SOURCE >$TARGET"
    htmlbuilder = build % docbook_html_uri
    manbuilder = build % docbook_man_uri
elif WhereIs("xmlto"):
    mangenerator = 'xmlto'
    xmlto = "xmlto %s $SOURCE || mv `basename $TARGET` `dirname $TARGET`"
    htmlbuilder = xmlto % "html-nochunks"
    manbuilder = xmlto % "man"
else:
    announce("Neither xsltproc nor xmlto found, documentation cannot be built.")
if manbuilder:
    env['BUILDERS']["Man"] = Builder(action=manbuilder)
    env['BUILDERS']["HTML"] = Builder(action=htmlbuilder,
                                      src_suffix=".xml", suffix=".html")

qt_network = env['libQgpsmm'] and config.CheckPKG('QtNetwork')

env = config.Finish()

# Be explicit about what we're doing.
changelatch = False 
for (name, default, help) in boolopts + nonboolopts + pathopts:
    if env[name] != env.subst(default):
        if not changelatch:
            announce("Altered configuration variables:")
            changelatch = True
        announce("%s = %s (default %s): %s" % (name, env[name], env.subst(default), help))
if not changelatch:
    announce("All configuration flags are defaulted.")

# Gentoo systems can have a problem with the Python path
if os.path.exists("/etc/gentoo-release"):
    announce("This is a Gentoo system.")
    announce("Adjust your PYTHONPATH to see library directories under /usr/local/lib")

# Should we build the Qt binding?
if cxx and qt_network:
    qt_env = env.Clone()
    qt_env.MergeFlags('-DUSE_QT')
    if qt_network:
        try:
            qt_env.MergeFlags(pkg_config('QtNetwork'))
        except OSError:
            announce("pkg_config is confused about the state of QtNetwork.")
            qt_env = None
else:
    qt_env = None

## Two shared libraries provide most of the code for the C programs

libgps_version_soname = libgps_version_current - libgps_version_age
libgps_version = "%d.%d.%d" %(libgps_version_soname, libgps_version_age, libgps_version_revision)
libgpsd_version_soname = libgpsd_version_current - libgpsd_version_age
libgpsd_version = "%d.%d.%d" %(libgpsd_version_soname, libgpsd_version_age, libgpsd_version_revision)

libgps_sources = [
    "ais_json.c",
    "bits.c",
    "daemon.c",
    "gpsutils.c",
    "gpsdclient.c",
    "gps_maskdump.c",
    "hex.c",
    "json.c",
    "libgps_core.c",
    "libgps_dbus.c",
    "libgps_json.c",
    "libgps_shm.c",
    "libgps_sock.c",
    "netlib.c",
    "rtcm2_json.c",
    "shared_json.c",
    "strl.c",
]

if cxx and env['libgpsmm']:
    libgps_sources.append("libgpsmm.cpp")

libgpsd_sources = [
    "bsd_base64.c",
    "crc24q.c",
    "gpsd_json.c",
    "geoid.c",
    "isgps.c",
    "libgpsd_core.c",
    "net_dgpsip.c",
    "net_gnss_dispatch.c",
    "net_ntrip.c",
    "packet.c",
    "pseudonmea.c",
    "serial.c",
    "subframe.c",
    "timebase.c",
    "drivers.c",
    "driver_ais.c",
    "driver_evermore.c",
    "driver_garmin.c",
    "driver_garmin_txt.c",
    "driver_geostar.c",
    "driver_italk.c",
    "driver_navcom.c",
    "driver_nmea0183.c",
    "driver_nmea2000.c",
    "driver_oncore.c",
    "driver_rtcm2.c",
    "driver_rtcm3.c",
    "driver_sirf.c",
    "driver_superstar2.c",
    "driver_tsip.c",
    "driver_ubx.c",
    "driver_zodiac.c",
]

# Cope with scons's failure to set SONAME in its builtins.
# Inspired by Richard Levitte's (slightly buggy) code at
# http://markmail.org/message/spttz3o4xrsftofr

def VersionedSharedLibrary(env, libname, version, lib_objs=[], parse_flags=[]):
    platform = env.subst('$PLATFORM')
    shlib_pre_action = None
    shlib_suffix = env.subst('$SHLIBSUFFIX')
    shlib_post_action = None
    shlink_flags = SCons.Util.CLVar(env.subst('$SHLINKFLAGS'))

    if platform == 'posix':
        ilib_suffix = shlib_suffix + '.' + version
        (major, age, revision) = version.split(".")
        soname = "lib" + libname + shlib_suffix + "." + major
        shlink_flags += [ '-Wl,-Bsymbolic', '-Wl,-soname=%s' % soname ]
    elif platform == 'cygwin':
        ilib_suffix = shlib_suffix
        shlink_flags += [ '-Wl,-Bsymbolic',
                          '-Wl,--out-implib,${TARGET.base}.a' ]
    elif platform == 'darwin':
        ilib_suffix = '.' + version + shlib_suffix
        shlink_flags += [ '-current_version', '%s' % version,
                          '-compatibility_version', '%s' % version,
                          '-undefined', 'dynamic_lookup' ]

    ilib = env.SharedLibrary(libname,lib_objs,
                            SHLIBSUFFIX=ilib_suffix,
                            SHLINKFLAGS=shlink_flags, parse_flags=parse_flags)

    if platform == 'darwin':
        if version.count(".") != 2:
            # We need a library name in libfoo.x.y.z.dylib form to proceed
            raise ValueError
        lib = 'lib' + libname + '.' + version + '.dylib'
        lib_no_ver = 'lib' + libname + '.dylib'
        # Link libfoo.x.y.z.dylib to libfoo.dylib
        env.AddPostAction(ilib, 'rm -f %s; ln -s %s %s' % (
            lib_no_ver, lib, lib_no_ver))
        env.Clean(lib, lib_no_ver)
    elif platform == 'posix':
        if version.count(".") != 2:
            # We need a library name in libfoo.so.x.y.z form to proceed
            raise ValueError
        lib = "lib" + libname + ".so." + version
        suffix_re = '%s\\.[0-9\\.]*$' % re.escape(shlib_suffix)
        # For libfoo.so.x.y.z, links libfoo.so libfoo.so.x.y libfoo.so.x
        major_name = shlib_suffix + "." + lib.split(".")[2]
        minor_name = major_name + "." + lib.split(".")[3]
        for linksuffix in [shlib_suffix, major_name, minor_name]:
            linkname = re.sub(suffix_re, linksuffix, lib)
            env.AddPostAction(ilib, 'rm -f %s; ln -s %s %s' % (
                linkname, lib, linkname))
            env.Clean(lib, linkname)

    return ilib

def VersionedSharedLibraryInstall(env, destination, libs):
    platform = env.subst('$PLATFORM')
    shlib_suffix = env.subst('$SHLIBSUFFIX')
    ilibs = env.Install(destination, libs)
    if platform == 'posix':
        suffix_re = '%s\\.[0-9\\.]*$' % re.escape(shlib_suffix)
        for lib in map(str, libs):
            if lib.count(".") != 4:
                # We need a library name in libfoo.so.x.y.z form to proceed
                raise ValueError
            # For libfoo.so.x.y.z, links libfoo.so libfoo.so.x.y libfoo.so.x
            major_name = shlib_suffix + "." + lib.split(".")[2]
            minor_name = major_name + "." + lib.split(".")[3]
            for linksuffix in [shlib_suffix, major_name, minor_name]:
                linkname = re.sub(suffix_re, linksuffix, lib)
                env.AddPostAction(ilibs, 'cd %s; rm -f %s; ln -s %s %s' % (destination, linkname, lib, linkname))
            env.Clean(lib, linkname)
    return ilibs

if not env["shared"]:
    def Library(env, target, sources, version, parse_flags=[]):
        return env.StaticLibrary(target, sources, parse_flags=parse_flags)
    LibraryInstall = lambda env, libdir, sources: env.Install(libdir, sources)
else:
    def Library(env, target, sources, version, parse_flags=[]):
        return VersionedSharedLibrary(env=env,
                                     libname=target,
                                     version=version,
                                     lib_objs=sources,
                                     parse_flags=parse_flags)
    LibraryInstall = lambda env, libdir, sources: \
                     VersionedSharedLibraryInstall(env, libdir, sources)

# Klugery to handle sonames ends

compiled_gpslib = Library(env=env,
                          target="gps",
                          sources=libgps_sources,
                          version=libgps_version,
                          parse_flags=dbus_libs + rtlibs)
env.Clean(compiled_gpslib, "gps_maskdump.c")

compiled_gpsdlib = Library(env=env,
                           target="gpsd",
                           sources=libgpsd_sources,
                           version=libgpsd_version,
                           parse_flags=usblibs + rtlibs + bluezlibs)

libraries = [compiled_gpslib, compiled_gpsdlib]

if qt_env:
    qtobjects = []
    qt_flags = qt_env['CFLAGS']
    for c_only in ('-Wmissing-prototypes', '-Wstrict-prototypes'):
        if c_only in qt_flags:
            qt_flags.remove(c_only)
    # Qt binding object files have to be renamed as they're built to avoid
    # name clashes with the plain non-Qt object files. This prevents the
    # infamous "Two environments with different actions were specified
    # for the same target" error.
    for src in libgps_sources:
        if src not in ('ais_json.c','json.c','libgps_json.c','rtcm2_json.c','shared_json.c'):
            compile_with = qt_env['CXX']
            compile_flags = qt_flags
        else:
            compile_with = qt_env['CC']
            compile_flags = qt_env['CFLAGS']
        qtobjects.append(qt_env.SharedObject(src.split(".")[0] + '-qt', src,
                                             CC=compile_with,                                             CFLAGS=compile_flags,
                                             parse_flags=dbus_libs))
    compiled_qgpsmmlib = Library(qt_env, "Qgpsmm", qtobjects, libgps_version)
    libraries.append(compiled_qgpsmmlib)

# The libraries have dependencies on system libraries

gpslibs = ["-lgps", "-lm"]
gpsdlibs = ["-lgpsd"] + usblibs + bluezlibs + gpslibs + caplibs

# We need to be able to make a static client library for ad-hoc testing.
# (None of the normal targets relies on this.)  You can use this
# with a build command like
#
# g++ --static streamtest.cpp libgps.a -lrt -o streamtest
#
# When you link with this library you will get warnings that look like this:
# warning: Using 'getprotobyname' in statically linked applications requires
#          at runtime the shared libraries from the glibc version used for
#          linking
# The final executable will build but not be portable.

env.StaticLibrary(target='libgps.a',
                  source=libgps_sources,
                  parse_flags=dbus_libs + rtlibs)

# Source groups

gpsd_sources = ['gpsd.c','ntpshm.c','shmexport.c','dbusexport.c']

if env['systemd']:
    gpsd_sources.append("sd_socket.c")

gpsmon_sources = [
    'gpsmon.c',
    'monitor_italk.c',
    'monitor_nmea.c',
    'monitor_oncore.c',
    'monitor_sirf.c',
    'monitor_superstar2.c',
    'monitor_tnt.c',
    'monitor_ubx.c',
    'monitor_garmin.c',
    ]

## Production programs

# FIXME: What we really want is for libm to be linked when libgps is.
# VersionedSharedLibrary accomplishes this for its case, but we don't
# know how to force it when linking staticly.
#
# It turns out there are two cases where we need to force this.  Some
# distributions don't do implicit linking by design.  See the test
# code for implicit_link.
#
if not env['shared'] or not env["implicit_link"]:
    env.MergeFlags("-lm")

# FIXME: Part of an attempt to support coverage testing.
if env['coveraging']:
    env.MergeFlags("-lgcov")

gpsd_env = env.Clone()
gpsd_env.MergeFlags("-pthread")

gpsd = gpsd_env.Program('gpsd', gpsd_sources,
                        parse_flags = gpsdlibs + dbus_libs)
env.Depends(gpsd, [compiled_gpsdlib, compiled_gpslib])

gpsdecode = env.Program('gpsdecode', ['gpsdecode.c'], parse_flags=gpsdlibs)
env.Depends(gpsdecode, [compiled_gpsdlib, compiled_gpslib])

gpsctl = env.Program('gpsctl', ['gpsctl.c'], parse_flags=gpsdlibs)
env.Depends(gpsctl, [compiled_gpsdlib, compiled_gpslib])

gpsdctl = env.Program('gpsdctl', ['gpsdctl.c'], parse_flags=gpslibs)
env.Depends(gpsdctl, compiled_gpslib)

gpsmon = env.Program('gpsmon', gpsmon_sources,
                     parse_flags=gpsdlibs + ncurseslibs + ['-lm'])
env.Depends(gpsmon, [compiled_gpsdlib, compiled_gpslib])

gpspipe = env.Program('gpspipe', ['gpspipe.c'], parse_flags=gpslibs)
env.Depends(gpspipe, compiled_gpslib)

gpxlogger = env.Program('gpxlogger', ['gpxlogger.c'], parse_flags=gpslibs)
env.Depends(gpxlogger, compiled_gpslib)

lcdgps = env.Program('lcdgps', ['lcdgps.c'], parse_flags=gpslibs)
env.Depends(lcdgps, compiled_gpslib)

cgps = env.Program('cgps', ['cgps.c'], parse_flags=gpslibs + ncurseslibs)
env.Depends(cgps, compiled_gpslib)

binaries = [gpsd, gpsdecode, gpsctl, gpsdctl, gpspipe, gpxlogger, lcdgps]
if ncurseslibs:
    binaries += [cgps, gpsmon]

# Test programs
test_float = env.Program('test_float', ['test_float.c'])
test_geoid = env.Program('test_geoid', ['test_geoid.c'], parse_flags=gpsdlibs)
env.Depends(test_geoid, [compiled_gpsdlib, compiled_gpslib])
test_json = env.Program('test_json', ['test_json.c'], parse_flags=gpslibs)
env.Depends(test_json, compiled_gpslib)
test_mkgmtime = env.Program('test_mkgmtime', ['test_mkgmtime.c'], parse_flags=gpslibs)
env.Depends(test_mkgmtime, compiled_gpslib)
test_trig = env.Program('test_trig', ['test_trig.c'], parse_flags=["-lm"])
test_packet = env.Program('test_packet', ['test_packet.c'], parse_flags=gpsdlibs)
env.Depends(test_packet, [compiled_gpsdlib, compiled_gpslib])
test_bits = env.Program('test_bits', ['test_bits.c'], parse_flags=gpslibs)
env.Depends(test_bits, [compiled_gpsdlib, compiled_gpslib])
test_gpsmm = env.Program('test_gpsmm', ['test_gpsmm.cpp'], parse_flags=gpslibs)
env.Depends(test_gpsmm, compiled_gpslib)
test_libgps = env.Program('test_libgps', ['test_libgps.c'], parse_flags=gpslibs)
env.Depends(test_libgps, compiled_gpslib)
testprogs = [test_float, test_trig, test_bits, test_packet,
             test_mkgmtime, test_geoid, test_libgps]
if env['socket_export']:
    testprogs.append(test_json)
if cxx and env["libgpsmm"]:
    testprogs.append(test_gpsmm)

# Python programs
if not env['python']:
    python_built_extensions = []
else:
    python_progs = ["gpscat", "gpsfake", "gpsprof", "xgps", "xgpsspeed", "gegps"]
    python_modules = Glob('gps/*.py') 

    # Build Python binding
    #
    python_extensions = {
        "gps" + os.sep + "packet" : ["gpspacket.c", "packet.c", "isgps.c",
                                        "driver_rtcm2.c", "strl.c", "hex.c", "crc24q.c"],
        "gps" + os.sep + "clienthelpers" : ["gpsclient.c", "geoid.c", "gpsdclient.c", "strl.c"]
    }

    python_env = env.Clone()
    vars = sysconfig.get_config_vars('CC', 'CXX', 'OPT', 'BASECFLAGS', 'CCSHARED', 'LDSHARED', 'SO', 'INCLUDEPY', 'LDFLAGS')
    for i in range(len(vars)):
        if vars[i] is None:
            vars[i] = []
    (cc, cxx, opt, basecflags, ccshared, ldshared, so_ext, includepy, ldflags) = vars
    # in case CC/CXX was set to the scan-build wrapper,
    # ensure that we build the python modules with scan-build, too
    if env['CC'] is None or env['CC'].find('scan-build') < 0:
        python_env['CC'] = cc
    else:
        python_env['CC'] = ' '.join([env['CC']] + cc.split()[1:])
    if env['CXX'] is None or env['CXX'].find('scan-build') < 0:
        python_env['CXX'] = cxx
    else:
        python_env['CXX'] = ' '.join([env['CXX']] + cxx.split()[1:])

    ldshared=ldshared.replace('-fPIE', '')
    ldshared=ldshared.replace('-pie', '')
    python_env.Replace(SHLINKFLAGS=[],
                       LDFLAGS=ldflags,
                       LINK = ldshared,
                       SHLIBPREFIX="",
                       SHLIBSUFFIX=so_ext,
                       CPPPATH=[includepy],
                       CPPFLAGS=opt,
                       CFLAGS=basecflags,
                       CXXFLAGS=basecflags)

    python_objects={}
    python_compiled_libs = {}
    for ext, sources in python_extensions.iteritems():
        python_objects[ext] = []
        for src in sources:
            python_objects[ext].append(
                python_env.NoCache(
                    python_env.SharedObject(
                        src.split(".")[0] + '-py_' + '_'.join(['%s' %(x) for x in sys.version_info]) + so_ext, src
                    )
                )
            )
        python_compiled_libs[ext] = python_env.SharedLibrary(ext, python_objects[ext])
    python_built_extensions = python_compiled_libs.values()
    python_egg_info_source = """Metadata-Version: 1.0
Name: gps
Version: %s
Summary: Python libraries for the gpsd service daemon
Home-page: %s
Author: the GPSD project
Author-email: %s
License: BSD
Description: The gpsd service daemon can monitor one or more GPS devices connected to a host computer, making all data on the location and movements of the sensors available to be queried on TCP port 2947.
Platform: UNKNOWN
""" %(gpsd_version, website, devmail)
    python_egg_info = python_env.Textfile(target="gps-%s.egg-info" %(gpsd_version, ), source=python_egg_info_source)

env.Command(target = "packet_names.h", source="packet_states.h", action="""
    rm -f $TARGET &&\
    sed -e '/^ *\([A-Z][A-Z0-9_]*\),/s//   \"\\1\",/' <$SOURCE >$TARGET &&\
    chmod a-w $TARGET""")

# build timebase.h
def timebase_h(target, source, env):
    from leapsecond import make_leapsecond_include
    f = open(target[0].abspath, 'w')
    f.write(make_leapsecond_include(source[0].abspath))
    f.close()
env.Command(target="timebase.h", source="leapseconds.cache",
            action=timebase_h)

env.Textfile(target="gpsd_config.h", source=confdefs)
env.Textfile(target="gpsd.h", source=[File("gpsd.h-head"), File("gpsd_config.h"), File("gpsd.h-tail")])

env.Command(target="gps_maskdump.c", source=["maskaudit.py", "gps.h", "gpsd.h"], action='''
    rm -f $TARGET &&\
        $PYTHON $SOURCE -c $SRCDIR >$TARGET &&\
        chmod a-w $TARGET''')

env.Command(target="ais_json.i", source="jsongen.py", action='''\
    rm -f $TARGET &&\
    $PYTHON $SOURCE --ais --target=parser >$TARGET &&\
    chmod a-w $TARGET''')

# generate revision.h
if 'dev' in gpsd_version:
    (st, rev) = _getstatusoutput('git describe')
    if st != 0:
        from datetime import datetime
        rev = datetime.now().isoformat()[:-4]
else:
    rev = gpsd_version
revision='#define REVISION "%s"\n' %(rev.strip(),)
env.Textfile(target="revision.h", source=[revision])

generated_sources = ['packet_names.h', 'timebase.h', 'gpsd.h', "ais_json.i",
                     'gps_maskdump.c', 'revision.h', 'gpsd.php']

# leapseconds.cache is a local cache for information on leapseconds issued
# by the U.S. Naval observatory. It gets kept in the repository so we can
# build without Internet access.
from leapsecond import save_leapseconds
leapseconds_cache_rebuild = lambda target, source, env: save_leapseconds(target[0].abspath)
if 'dev' in gpsd_version or not os.path.exists('leapseconds.cache'):
    leapseconds_cache = env.Command(target="leapseconds.cache",
                                source="leapsecond.py",
                                action=leapseconds_cache_rebuild)
    env.Clean(leapseconds_cache, "leapsecond.pyc")
    env.NoClean(leapseconds_cache)
    env.Precious(leapseconds_cache)

# Instantiate some file templates.  We'd like to use the Substfile builtin
# but it doesn't seem to work in scons 1.20
def substituter(target, source, env):
    substmap = (
        ('@VERSION@',    gpsd_version),
        ('@prefix@',     env['prefix']),
        ('@libdir@',     env['libdir']),
        ('@PYTHON@',     sys.executable),
        ('@DATE@',       time.asctime()),
        ('@MASTER',      'DO NOT HAND_HACK! THIS FILE IS GENERATED@'),
        ('@SITENAME@',   sitename),
        ('@SITESEARCH@', sitesearch),
        ('@WEBSITE@',    website),
        ('@MAINPAGE@',   mainpage),
        ('@WEBUPLOAD@',  webupload),
        ('@CGIUPLOAD@',  cgiupload),
        ('@SCPUPLOAD@',  scpupload),
        ('@MAILMAN@',    mailman),
        ('@ADMIN@',      admin),
        ('@DOWNLOAD@',   download),
        ('@BUGTRACKER@', bugtracker),
        ('@BROWSEREPO@', browserepo),
        ('@CLONEREPO@',  clonerepo),
        ('@GITREPO@',    gitrepo),
        ('@WEBFORM@',    webform),
        ('@FORMSERVER@', formserver),
        ('@DEVMAIL@',    devmail),
        ('@LIBGPSVERSION@', libgps_version),
        )
    sfp = open(str(source[0]))
    content = sfp.read()
    sfp.close()
    for (s, t) in substmap:
        content = content.replace(s, t)
    m = re.search("@[A-Z]+@", content)
    if m and m.group(0) not in map(lambda x: x[0], substmap):
        print >>sys.stderr, "Unknown subst token %s in %s." % (m.group(0), sfp.name)
    tfp = open(str(target[0]), "w")
    tfp.write(content)
    tfp.close()

templated = glob.glob("*.in") + glob.glob("*/*.in") + glob.glob("*/*/*.in")

for fn in templated:
    builder = env.Command(source=fn, target=fn[:-3], action=substituter)
    env.AddPostAction(builder, 'chmod -w $TARGET')
    if fn.endswith(".py.in"):
        env.AddPostAction(builder, 'chmod +x $TARGET')

# Documentation

base_manpages = {
    "gpsd.8" : "gpsd.xml",
    "gpsd_json.5" : "gpsd_json.xml",
    "gps.1" : "gps.xml",
    "cgps.1" : "gps.xml",
    "gpsinit.8": "gpsinit.xml",
    "lcdgps.1" : "gps.xml",
    "libgps.3" : "libgps.xml",
    "libgpsmm.3" : "libgpsmm.xml",
    "libQgpsmm.3" : "libgpsmm.xml",
    "libgpsd.3" : "libgpsd.xml",
    "gpsmon.1": "gpsmon.xml",
    "gpsctl.1" : "gpsctl.xml",
    "gpsdctl.8" : "gpsdctl.xml",
    "gpspipe.1" : "gpspipe.xml",
    "gpsdecode.1" : "gpsdecode.xml",
    "srec.5" : "srec.xml",
    }
python_manpages = {
    "gpsprof.1" : "gpsprof.xml",
    "gpsfake.1" : "gpsfake.xml",
    "gpscat.1" : "gpscat.xml",
    "xgpsspeed.1" : "gps.xml",
    "xgps.1" : "gps.xml",
    "gegps.1" : "gps.xml",
    }

manpage_targets = []
if manbuilder:
    for (man, xml) in base_manpages.items() + python_manpages.items():
        manpage_targets.append(env.Man(source=xml, target=man))

## Where it all comes together

build = env.Alias('build', [libraries, binaries, python_built_extensions, "gpsd.php", manpage_targets])
env.Clean(build,
          map(glob.glob,("*.[oa]", "*.os", "*.os.*", "*.gcno", "*.pyc", "gps/*.pyc")) + \
          generated_sources + \
          map(lambda f: f[:-3], templated) + \
          [".sconf_temp"])
env.Default(*build)

if qt_env:
    build_qt = qt_env.Alias('build', [compiled_qgpsmmlib])
    qt_env.Default(*build_qt)

if env['python']:
    build_python = python_env.Alias('build', [python_built_extensions])
    python_env.Default(*build_python)

## Installation and deinstallation

# Not here because too distro-specific: udev rules, desktop files, init scripts

# It's deliberate that we don't install gpsd.h. It's full of internals that
# third-party client programs should not see.
headerinstall = [ env.Install(installdir('includedir'), x) for x in ("libgpsmm.h", "gps.h")]

binaryinstall = []
binaryinstall.append(env.Install(installdir('sbindir'), [gpsd, gpsdctl]))
binaryinstall.append(env.Install(installdir('bindir'),  [gpsdecode, gpsctl, gpspipe, gpxlogger, lcdgps]))
if ncurseslibs:
    binaryinstall.append(env.Install(installdir('bindir'), [cgps, gpsmon]))
binaryinstall.append(LibraryInstall(env, installdir('libdir'), compiled_gpslib))
binaryinstall.append(LibraryInstall(env, installdir('libdir'), compiled_gpsdlib))
if qt_env:
    binaryinstall.append(LibraryInstall(qt_env, installdir('libdir'), compiled_qgpsmmlib))

if env["shared"] and env["chrpath"]:
    env.AddPostAction(binaryinstall, '$CHRPATH -r "%s" "$TARGET"' \
                      % (installdir('libdir', False), ))

if not env['debug'] and not env['profiling'] and env['strip']:
    env.AddPostAction(binaryinstall, '$STRIP $TARGET')

if not env['python']:
    python_install = []
else:
    python_lib_dir = sysconfig.get_python_lib(plat_specific=1)
    python_module_dir = python_lib_dir + os.sep + 'gps'
    python_extensions_install = python_env.Install( DESTDIR + python_module_dir,
                                                    python_built_extensions)
    if not env['debug'] and not env['profiling'] and env['strip']:
        python_env.AddPostAction(python_extensions_install, '$STRIP $TARGET')

    python_modules_install = python_env.Install( DESTDIR + python_module_dir,
                                                python_modules)

    python_progs_install = python_env.Install(installdir('bindir'), python_progs)

    python_egg_info_install = python_env.Install(DESTDIR + python_lib_dir,
                                                 python_egg_info)
    python_install = [  python_extensions_install,
                        python_modules_install,
                        python_progs_install,
                        python_egg_info_install]

pc_install = [ env.Install(installdir('pkgconfig'), x) for x in ("libgps.pc", "libgpsd.pc") ]
if qt_env:
    pc_install.append(qt_env.Install(installdir('pkgconfig'), 'Qgpsmm.pc'))
    pc_install.append(qt_env.Install(installdir('libdir'), 'libQgpsmm.prl'))


maninstall = []
for manpage in base_manpages.keys() + python_manpages.keys():
    if not manbuilder and not os.path.exists(manpage):
        continue
    section = manpage.split(".")[1]
    dest = os.path.join(installdir('mandir'), "man"+section, manpage)
    maninstall.append(env.InstallAs(source=manpage, target=dest))
install = env.Alias('install', binaryinstall + maninstall + python_install + pc_install + headerinstall)

def Uninstall(nodes):
    deletes = []
    for node in nodes:
        if node.__class__ == install[0].__class__:
            deletes.append(Uninstall(node.sources))
        else:
            deletes.append(Delete(str(node)))
    return deletes
uninstall = env.Command('uninstall', '', Flatten(Uninstall(Alias("install"))) or "")
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

# Target selection for '.' is badly broken. This is a general scons problem,
# not a glitch in this particular recipe. Avoid triggering the bug.

def error_action(target, source, env):
    from SCons.Errors import UserError
    raise UserError, "Target selection for '.' is broken."
AlwaysBuild(Alias(".", [], error_action))

# Utility productions

def Utility(target, source, action):
    target = env.Command(target=target, source=source, action=action)
    env.AlwaysBuild(target)
    env.Precious(target)
    return target

# Report splint warnings
# Note: test_bits.c is unsplintable because of the PRI64 macros.
splintopts = "-I/usr/include/libusb-1.0 +quiet"
# splint does not know about multi-arch, work around that
ma_status, ma = _getstatusoutput('dpkg-architecture -qDEB_HOST_MULTIARCH')
if ma_status == 0:
    splintopts = '-I/usr/include/%s %s' %(ma.strip(),splintopts)
env['SPLINTOPTS']=splintopts

def Splint(target,sources, description, params):
    return Utility(target,sources+generated_sources,[
            '@echo "Running splint on %s..."'%description,
            '-splint $SPLINTOPTS %s %s'%(" ".join(params)," ".join(sources)),
            ])

splint_table = [
    ('splint-daemon',gpsd_sources,'daemon', ['-exportlocal', '-redef']),
    ('splint-libgpsd',libgpsd_sources,'libgpsd', ['-exportlocal', '-redef']),
    ('splint-libgps',libgps_sources,'user-side libraries', ['-exportlocal',
                                                            '-fileextensions',
                                                            '-redef']),
    ('splint-cgps',['cgps.c'],'cgps', ['-exportlocal']),
    ('splint-gpsctl',['gpsctl.c'],'gpsctl', ['']),
    ('splint-gpsdctl',['gpsdctl.c'],'gpsdctl', ['']),
    ('splint-gpsmon',gpsmon_sources,'gpsmon', ['-exportlocal']),
    ('splint-gpspipe',['gpspipe.c'],'gpspipe', ['']),
    ('splint-gpsdecode',['gpsdecode.c'],'gpsdecode', ['']),
    ('splint-gpxlogger',['gpxlogger.c'],'gpxlogger', ['']),
    ('splint-test_packet',['test_packet.c'],'test_packet test harness', ['']),
    ('splint-test_mkgmtime',['test_mkgmtime.c'],'test_mkgmtime test harness', ['']),
    ('splint-test_geoid',['test_geoid.c'],'test_geoid test harness', ['']),
    ('splint-test_json',['test_json.c'],'test_json test harness', ['']),
    ]

for (target,sources,description,params) in splint_table:
    env.Alias('splint',Splint(target,sources,description,params))

Utility("cppcheck", ["gpsd.h", "packet_names.h"],
        "cppcheck --template gcc --enable=all --inline-suppr --suppress='*:driver_proto.c' --force $SRCDIR")

# Sanity-check Python code. TODO: add xgps for the complete set.
Utility("pychecker", ["jsongen.py", "maskaudit.py", python_built_extensions],
        ['''for f in  gpsprof gpscat gpsfake gegps; do ln -s $$f $$f.py; done; \
        pychecker --no-classattr --no-callinit jsongen.py leapsecond.py maskaudit.py gpsprof.py gpscat.py gpsfake.py gegps.py gps/*.py;
        for f in gpsprof gpscat gpsfake gegps; do rm $$f.py $$f.pyc; done'''])
Utility("pylint", ["jsongen.py", "maskaudit.py", python_built_extensions],
        ['''pylint --output-format=parseable --reports=n --include-ids=y --disable=F0001,C0103,C0111,C0301,C0302,C0322,C0324,C0323,C0321,R0201,R0902,R0903,R0904,R0911,R0912,R0913,R0914,R0915,W0201,W0401,W0141,W0142,W0603,W0614,W0621,E1101,E1102 jsongen.py leapsecond.py maskaudit.py gpsprof.py gpscat.py gpsfake.py gegps.py gps/*.py xgps'''])

# Check the documentation for bogons, too
Utility("xmllint", glob.glob("*.xml"),
    "for xml in $SOURCES; do xmllint --nonet --noout --valid $$xml; done")

# Use deheader to remove headers not required.  If the statistics line
# ends with other than '0 removed' there's work to be done.
Utility("deheader", generated_sources, [
    'deheader -x cpp -x contrib -x gpspacket.c -x gpsclient.c -x monitor_proto.c -i gpsd_config.h -i gpsd.h -m "MORECFLAGS=\'-Werror -Wfatal-errors -DDEBUG -DPPS_ENABLE\' scons -Q"',
        ])

# Perform all sanity checks.
audit = env.Alias('audit',
                  ['splint',
                   'cppcheck',
                   'xmllint',
                   'valgrind-audit',
                   ])

#
# Regression tests begin here
#
# Note that the *-makeregress targets re-create the *.log.chk source
# files from the *.log source files.

# Unit-test the bitfield extractor
bits_regress = Utility('bits-regress', [test_bits], [
    '$SRCDIR/test_bits --quiet'
    ])

# Check that all Python modules compile properly 
if env['python']:
    def check_compile(target, source, env):
        for pyfile in source:
            'cp %s tmp.py'%(pyfile)
            '%s -tt -m py_compile tmp.py' %(sys.executable, )
            'rm -f tmp.py tmp.pyc'
    python_compilation_regress = Utility('python-compilation-regress',
            Glob('*.py') + python_modules + python_progs + ['SConstruct'], check_compile)
else:
    python_compilation_regress = None

# using regress-drivers requires socket_export being enabled.
if env['socket_export']:
    # Regression-test the daemon
    gps_regress = [Utility("gps-regress", [gpsd, python_built_extensions],
            '$SRCDIR/regress-driver test/daemon/*.log')]

    # Test that super-raw mode works. Compare each logfile against itself
    # dumped through the daemon running in R=2 mode.  (This test is not
    # included in the normal regressions.)
    Utility("raw-regress", [gpsd, python_built_extensions],
        '$SRCDIR/regress-driver test/daemon/*.log')

    # Build the regression tests for the daemon.
    # Note: You'll have to do this whenever the default leap second
    # changes in timebase.h.  The problem is in the SiRF tests;
    # that driver relies on the default until it gets the current
    # offset from subframe data.
    Utility('gps-makeregress', [gpsd, python_built_extensions],
        '$SRCDIR/regress-driver -b test/daemon/*.log')
else:
    gps_regress = []

# To build an individual test for a load named foo.log, put it in
# test/daemon and do this:
#    regress-driver -b test/daemon/foo.log

# Regression-test the RTCM decoder.
rtcm_regress = Utility('rtcm-regress', [gpsdecode], [
    '@echo "Testing RTCM decoding..."',
    '@for f in $SRCDIR/test/*.rtcm2; do '
        'echo "Testing $${f}..."; '
        'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -u -j <$${f} >$${TMPFILE}; '
        'diff -ub $${f}.chk $${TMPFILE}; '
        'rm -f $${TMPFILE}; '
    'done;',
    '@echo "Testing idempotency of JSON dump/decode for RTCM2"',
    '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
    '$SRCDIR/gpsdecode -u -e -j <test/synthetic-rtcm2.json >$${TMPFILE}; '
        'grep -v "^#" test/synthetic-rtcm2.json | diff -ub - $${TMPFILE}; '
        'rm -f $${TMPFILE}; ',
        ])

# Rebuild the RTCM regression tests.
Utility('rtcm-makeregress', [gpsdecode], [
    'for f in $SRCDIR/test/*.rtcm2; do '
        '$SRCDIR/gpsdecode -j <$${f} >$${f}.chk; '
    'done'
        ])

# Regression-test the AIVDM decoder.
aivdm_regress = Utility('aivdm-regress', [gpsdecode], [
    '@echo "Testing AIVDM decoding w/ CSV format..."',
    '@for f in $SRCDIR/test/*.aivdm; do '
        'echo "Testing $${f}..."; '
        'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -u -c <$${f} >$${TMPFILE}; '
        'diff -ub $${f}.chk $${TMPFILE} || echo "Test FAILED!"; '
        'rm -f $${TMPFILE}; '
    'done;',
    '@echo "Testing AIVDM decoding w/ JSON unscaled format..."',
    '@for f in $SRCDIR/test/*.aivdm; do '
        'echo "  Testing $${f}..."; '
        'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -u -j <$${f} >$${TMPFILE}; '
        'diff -ub $${f}.ju.chk $${TMPFILE} || echo "Test FAILED!"; '
        'rm -f $${TMPFILE}; '
    'done;',
    '@echo "Testing AIVDM decoding w/ JSON scaled format..."',
    '@for f in $SRCDIR/test/*.aivdm; do '
        'echo "  Testing $${f}..."; '
        'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -j <$${f} >$${TMPFILE}; '
        'diff -ub $${f}.js.chk $${TMPFILE} || echo "Test FAILED!"; '
        'rm -f $${TMPFILE}; '
    'done;',
    '@echo "Testing idempotency of unscaled JSON dump/decode for AIS"',
    '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
    '$SRCDIR/gpsdecode -u -e -j <$SRCDIR/test/sample.aivdm.ju.chk >$${TMPFILE}; '
        'grep -v "^#" $SRCDIR/test/sample.aivdm.ju.chk | diff -ub - $${TMPFILE}; '
        'rm -f $${TMPFILE}; ',
    # Parse the unscaled json reference, dump it as scaled json, 
    # and finally compare it with the scaled json reference
    '@echo "Testing idempotency of scaled JSON dump/decode for AIS"',
    '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
    '$SRCDIR/gpsdecode -e -j <$SRCDIR/test/sample.aivdm.ju.chk >$${TMPFILE}; '
        'grep -v "^#" $SRCDIR/test/sample.aivdm.js.chk | diff -ub - $${TMPFILE}; '
        'rm -f $${TMPFILE}; ',
        ])

# Rebuild the AIVDM regression tests.
Utility('aivdm-makeregress', [gpsdecode], [
    'for f in $SRCDIR/test/*.aivdm; do '
        '$SRCDIR/gpsdecode -u -c <$${f} > $${f}.chk; '
        '$SRCDIR/gpsdecode -u -j <$${f} > $${f}.ju.chk; '
        '$SRCDIR/gpsdecode -j  <$${f} > $${f}.js.chk; '
    'done',
        ])

# Regression-test the packet getter.
packet_regress = Utility('packet-regress', [test_packet], [
    '@echo "Testing detection of invalid packets..."',
    '$SRCDIR/test_packet | diff -u $SRCDIR/test/packet.test.chk -',
    ])

# Rebuild the packet-getter regression test
Utility('packet-makeregress', [test_packet], [
    '$SRCDIR/test_packet >$SRCDIR/test/packet.test.chk',
    ])

# Rebuild the geoid test
Utility('geoid-makeregress', [test_geoid], [
    '$SRCDIR/test_geoid 37.371192 122.014965 >$SRCDIR/test/geoid.test.chk'])

# Regression-test the geoid tester.
geoid_regress = Utility('geoid-regress', [test_geoid], [
    '@echo "Testing the geoid model..."',
    '$SRCDIR/test_geoid 37.371192 122.014965 | diff -u $SRCDIR/test/geoid.test.chk -',
    ])

# Regression-test the Maidenhead Locator
maidenhead_locator_regress = Utility('maidenhead-locator-regress', [python_built_extensions], [
    '@echo "Testing the Maidenhead Locator conversion..."',
    '$SRCDIR/test_maidenhead.py >/dev/null',
    ])

# Regression-test the calendar functions
time_regress = Utility('time-regress', [test_mkgmtime], [
    '$SRCDIR/test_mkgmtime'
    ])

# Regression test the unpacking code in libgps
unpack_regress = Utility('unpack-regress', [test_libgps], [
    '@echo "Testing the client-library sentence decoder..."',
    '$SRCDIR/regress-driver -c $SRCDIR/test/clientlib/*.log',
    ])

# Build the regression test for the sentence unpacker
Utility('unpack-makeregress', [test_libgps], [
    '@echo "Rebuilding the client sentence-unpacker tests..."',
    '$SRCDIR/regress-driver -c -b $SRCDIR/test/clientlib/*.log'
    ])

# Unit-test the JSON parsing
json_regress = Utility('json-regress', [test_json], [
    '$SRCDIR/test_json'
    ])

# Run a valgrind audit on the daemon  - not in normal tests
valgrind_audit = Utility('valgrind-audit',
    ['$SRCDIR/valgrind-audit.py', python_built_extensions, gpsd],
    './valgrind-audit.py'
    )

# Run test builds on remote machines
flocktest = Utility("flocktest", [], "cd devtools; ./flocktest " + gitrepo)

# Run all normal regression tests
check = env.Alias('check', [
    python_compilation_regress,
    bits_regress,
    rtcm_regress,
    aivdm_regress,
    packet_regress,
    geoid_regress,
    maidenhead_locator_regress,
    time_regress,
    unpack_regress,
    json_regress,
    ] + gps_regress)

env.Alias('testregress', check)

# The website directory
#
# None of these productions are fired by default.
# The content they handle is the GPSD website, not included in release tarballs.

webpages = Split('''www/installation.html
    www/gpscat.html www/gpsctl.html www/gpsdecode.html 
    www/gpsd.html www/gpsd_json.html www/gpsfake.html www/gpsmon.html 
    www/gpspipe.html www/gpsprof.html www/gps.html www/gpsinit.html
    www/libgpsd.html www/libgpsmm.html www/libgps.html
    www/srec.html
    www/AIVDM.html www/NMEA.html
    www/protocol-evolution.html www/protocol-transition.html
    www/client-howto.html www/writing-a-driver.html
    www/hardware.html
    www/performance/performance.html
    www/internals.html
    www/cycle.png
    ''') + map(lambda f: f[:-3], glob.glob("www/*.in"))

www = env.Alias('www', webpages)

# Paste 'scons --quiet validation-list' to a batch validator such as
# http://htmlhelp.com/tools/validator/batch.html.en 
def validation_list(target, source, env):
    for page in glob.glob("www/*.html"):
        if not '-head' in page:
            fp = open(page)
            if "Valid HTML" in fp.read():
                print os.path.join(website, os.path.basename(page))
            fp.close()
Utility("validation-list", [www], validation_list)

# How to update the website
upload_web = Utility("upload_web", [www],
                     ['rsync --exclude="*.in" -avz www/ ' + webupload,
                      'scp README TODO NEWS ' + webupload,
                      'chmod ug+w,a+x www/gps_report.cgi',
                      'scp www/gps_report.cgi ' + cgiupload + "gps_report.cgi"])

# When the URL declarations change, so must the generated web pages
for fn in glob.glob("www/*.in"):
    env.Depends(fn[:-3], "SConstruct")

# asciidoc documents
if env.WhereIs('asciidoc'):
    env.Command("www/installation.html",
                "INSTALL",
                ["asciidoc -o www/installation.html INSTALL"])
    for stem in ['AIVDM', 'NMEA',
                 'protocol-evolution', 'protocol-transition',
                 'client-howto']:
        env.Command('www/%s.html' % stem, 'www/%s.txt' % stem,    
                    ['asciidoc -a toc -o www/%s.html www/%s.txt' % (stem, stem)])

if htmlbuilder:
    # Manual pages
    for xml in glob.glob("*.xml"):
        env.HTML('www/%s.html' % xml[:-4], xml)
    
    # DocBook documents
    for stem in ['writing-a-driver', 'performance/performance']:
        env.HTML('www/%s.html' % stem, 'www/%s.xml' % stem)

    # The internals manual.
    # Doesn't capture dependencies on the subpages
    env.HTML('www/internals.html', '$SRCDIR/doc/internals.xml')

# The hardware page
env.Command('www/hardware.html', ['gpscap.py',
                                  'www/hardware-head.html',
                                  'gpscap.ini',
                                  'www/hardware-tail.html'],
            ['(cat www/hardware-head.html; $PYTHON gpscap.py; cat www/hardware-tail.html) >www/hardware.html'])

# The diagram editor dia is required in order to edit the diagram masters
Utility("www/cycle.png", ["www/cycle.dia"], ["dia -e www/cycle.png www/cycle.dia"])

# Experimenting with pydoc.  Not yet fired by any other productions.

if env['python']:
    env.Alias('pydoc', "www/pydoc/index.html")

    # We need to run epydoc with the Python version we built the modules for.
    # So we define our own epydoc instead of using /usr/bin/epydoc
    EPYDOC = "python -c 'from epydoc.cli import cli; cli()'"
    env.Command('www/pydoc/index.html', python_progs + glob.glob("*.py")  + glob.glob("gps/*.py"), [
        'mkdir -p www/pydoc',
        EPYDOC + " -v --html --graph all -n GPSD $SOURCES -o www/pydoc",
            ])

# Productions for setting up and performing udev tests.
#
# Requires root. Do "udev-install", then "tail -f /var/log/syslog" in
# another window, then run 'scons udev-test', then plug and unplug the
# GPS ad libitum.  All is well when you get fix reports each time a GPS
# is plugged in.

Utility('udev-install', 'install', [
    'mkdir -p ' + DESTDIR + env['udevdir'] + '/rules.d',
    'cp $SRCDIR/gpsd.rules ' + DESTDIR + env['udevdir'] + '/rules.d/25-gpsd.rules',
    'cp $SRCDIR/gpsd.hotplug ' + DESTDIR + env['udevdir'],
    'chmod a+x ' + DESTDIR + env['udevdir'] + '/gpsd.hotplug',
        ])

Utility('udev-uninstall', '', [
    'rm -f %s/gpsd.hotplug' % env['udevdir'],
    'rm -f %s/rules.d/25-gpsd.rules' % env['udevdir'],
        ])

Utility('udev-test', '', [
    '$SRCDIR/gpsd -N -n -F /var/run/gpsd.sock -D 5',
        ])

# Release machinery begins here
#
# We need to be in the actual project repo (i.e. not doing a -Y build)
# for these productions to work.

if os.path.exists("gpsd.c") and os.path.exists(".gitignore"):
    distfiles = _getoutput(r"git ls-files | grep -v '^www/'").split()
    if ".gitignore" in distfiles:
        distfiles.remove(".gitignore")
    distfiles += generated_sources
    distfiles += base_manpages.keys() + python_manpages.keys()
    distfiles.remove("gpsd.h")
    if "packaging/rpm/gpsd.spec" not in distfiles:
        distfiles.append("packaging/rpm/gpsd.spec")

    # How to build a tarball.
    tarball = env.Command('tarball', distfiles, [
        '@tar --transform "s:^:gpsd-${VERSION}/:" -czf gpsd-${VERSION}.tar.gz $SOURCES',
        '@ls -l gpsd-${VERSION}.tar.gz',
        ])
    env.Clean(tarball, ["gpsd-${VERSION}.tar.gz", "packaging/rpm/gpsd.spec"])

    # Make RPM from the specfile in packaging
    Utility('dist-rpm', tarball, 'rpmbuild -ta gpsd-${VERSION}.tar.gz')

    # Make sure build-from-tarball works.
    testbuild = Utility('testbuild', [tarball], [
        'tar -xzvf gpsd-${VERSION}.tar.gz',
        'cd gpsd-${VERSION}; scons',
        'rm -fr gpsd-${VERSION}',
        ])

    releasecheck = env.Alias('releasecheck', [
        testbuild,
        check,
        audit,
        flocktest,
        ])

    # This is how to ship a release to the hosting site.
    # The chmod copes with the fact that scp will give a
    # replacement the permissions of the *original*...
    upload_release = Utility('upload-release', [tarball], [
            'gpg -b gpsd-${VERSION}.tar.gz',
            'chmod ug=rw,o=r gpsd-${VERSION}.tar.gz gpsd-${VERSION}.tar.gz.sig',
            'scp gpsd-${VERSION}.tar.gz gpsd-${VERSION}.tar.gz.sig ' + scpupload,
            ])

    # How to tag a release
    tag_release = Utility('tag-release', [], [
        'git tag -s -m "Tagged for external release ${VERSION}" release-${VERSION}'
        ])
    upload_tags = Utility('upload-tags', [], ['git push --tags'])

    # Local release preparation. This production will require Internet access,
    # but it doesn't do any uploads or public repo mods.
    #
    # Note that tag_release has to fire early, otherwise the value of REVISION
    # won't be right when revision.h is generated for the tarball. 
    releaseprep = env.Alias("releaseprep",
                            [Utility("distclean", [], ["rm -f revision.h"]),
                             tag_release,
                             tarball])
    # Undo local release preparation
    Utility("undoprep", [], ['rm -f gpsd-${VERSION}.tar.gz;',
                             'git tag -d release-${VERSION};'])

    # All a buildup to this.
    env.Alias("release", [releaseprep,
                          upload_release,
                          upload_tags,
                          upload_web])

    # Experimental release mechanics using shipper
    # This will ship a freecode metadata update 
    ship_release = Utility("ship_release",
                           [tarball],
                           ['shipper -u -m --exclude "login.ibiblio.org:/public/html/catb/esr/"'])
    env.Alias("ship", [releaseprep,
                          ship_release,
                          upload_tags])

# The following sets edit modes for GNU EMACS
# Local Variables:
# mode:python
# End:
