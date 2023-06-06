# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project('dbus',
    'c',
    version: '1.15.6',
    meson_version: '>=0.56',
)

project_url = 'https://gitlab.freedesktop.org/dbus/dbus'

cc = meson.get_compiler('c')

windows = import('windows')
pkgconfig = import('pkgconfig')

config = configuration_data()
# Used for dbus-arch-deps.h, separated from config.h
arch_config = configuration_data()
# Non-quoted variables
data_config = configuration_data()

install_emptydirs = []
install_symlinks = []

###############################################################################
# Project configuration

not_found = dependency('', required: false)

version = meson.project_version()
config.set_quoted('VERSION',    version)
data_config.set('VERSION',      version)
data_config.set('DBUS_VERSION', version)

ver_array = version.split('.')
arch_config.set('DBUS_VERSION', version)
arch_config.set('DBUS_MAJOR_VERSION', ver_array[0])
arch_config.set('DBUS_MINOR_VERSION', ver_array[1])
arch_config.set('DBUS_MICRO_VERSION', ver_array[2])

config.set_quoted('DBUS_DAEMON_NAME', 'dbus-daemon')

###############################################################################
# libtool versioning - this applies to libdbus
# http://sources.redhat.com/autobook/autobook/autobook_91.html#SEC91

## increment if the interface has additions, changes, removals.
lt_current = 40
## increment any time the source changes;
## set to 0 if you increment CURRENT
lt_revision = 0
## increment if any interfaces have been added;
## set to 0 if any interfaces have been changed or removed.
## removal has precedence over adding, so set to 0 if both happened.
lt_age = 37

soversion = (lt_current - lt_age)
version_info = '@0@.@1@.@2@'.format(soversion, lt_age, lt_revision)

data_config.set('SOVERSION', soversion)

###############################################################################
# System detection

python = find_program('python3')

host_os = host_machine.system()

platform_windows = host_os.contains('windows')

if platform_windows
    # On Windows, we use C++ constructors to initialize global locks
    using_cpp = true
    add_languages('cpp', required: true)
    cpp = meson.get_compiler('cpp')
else
    using_cpp = false
endif

platform_cygwin = host_os.contains('cygwin')

# TODO: meson doesn't actually have WinCE support
platform_win32ce = host_os.contains('mingw32ce')

platform_unix = not platform_windows

config.set('DBUS_UNIX',   platform_unix)
config.set('DBUS_CYGWIN', platform_cygwin)
config.set('DBUS_WIN',    platform_windows)
config.set('DBUS_WINCE',  platform_win32ce)

if platform_unix
    platform = 'Unix'
elif platform_cygwin
    platform = 'cygwin'
elif platform_win32ce
    platform = 'win32ce'
elif platform_windows
    platform = 'windows'
else
    platform = 'unknown'
endif
message('Detected platform : @0@ (@1@)'.format(platform, host_os))

if platform_windows
    config.set('FD_SETSIZE', 8192,
        description: 'The maximum number of connections that can be handled at once'
    )
    if platform_win32ce
        config.set('_WIN32_WCE', '0x0502', description: 'Get newer W32 CE APIs')
    else
        config.set('_WIN32_WINNT', '0x0600',
            description: 'Define to the minimum supported Windows version (Vista)'
        )
    endif

    build_timestamp = run_command(
        find_program('tools/build-timestamp.py'),
        check: true,
    ).stdout().strip()

    data_config.set('BUILD_TIMESTAMP', build_timestamp)
    data_config.set('BUILD_FILEVERSION', ','.join(version.split('.')))

    data_config.set('DBUS_VER_FILE_TYPE', 'VFT_DLL')
    data_config.set('DBUS_VER_INTERNAL_NAME', 'libdbus-1-@0@'       .format(soversion))
    data_config.set('DBUS_VER_ORIGINAL_NAME', 'libdbus-1-@0@.dll'   .format(soversion))
endif

if platform_windows
    conf_maybe_external = '<!--<auth>EXTERNAL</auth>-->'
else
    conf_maybe_external = '<auth>EXTERNAL</auth>'
endif
data_config.set('DBUS_SESSION_CONF_MAYBE_AUTH_EXTERNAL', conf_maybe_external)

###############################################################################
# Functionally necessary compiler arguments
# Note that these must be set up before we do any checks like
# cc.has_header_symbol, otherwise we'll fail to find symbols that only exist
# when a particular feature-test macro like _GNU_SOURCE is defined.

compile_args = [
    '-D_GNU_SOURCE',
]

# dbus makes assumptions about aliasing that Standard C does not guarantee,
# particularly in DBusString.
# See https://gitlab.freedesktop.org/dbus/dbus/-/issues/4
compile_args += ['-fno-strict-aliasing']

if host_os.contains('solaris')
    compile_args += [
        # Solaris' C library apparently needs these runes to be threadsafe...
        '-D_POSIX_PTHREAD_SEMANTICS',
        # ... this opt-in to get sockaddr_in6 and sockaddr_storage...
        '-D__EXTENSIONS__',
        # ... and this opt-in to get file descriptor passing support
        ' -D_XOPEN_SOURCE=500',
    ]
endif

dbus_static_flags = ( get_option('default_library') == 'static'
    ? [ '-DDBUS_STATIC_BUILD' ]
    : []
)
compile_args += dbus_static_flags

if cc.get_id() != 'msvc'
    compile_args += [
          # On Windows, we expect to be using msvcrt.dll-compatible printf
          # (%I64u instead of %llu)
          '-D__USE_MINGW_ANSI_STDIO=0',
    ]
endif

compile_args_c = cc.get_supported_arguments(compile_args)
add_project_arguments(compile_args_c, language: 'c')

if using_cpp
    compile_args_cpp = cpp.get_supported_arguments(compile_args)
    add_project_arguments(compile_args_cpp, language: 'cpp')
endif

# Try to use hidden visibility on Unix. We don't do this on Windows to avoid
# interfering with use of __declspec(dllexport) and __declspec(dllimport),
# which have a similar effect.
if cc.has_argument('-fvisibility=hidden') and not platform_windows
    default_visibility = '__attribute__ ((__visibility__ ("default")))'
    test_visibility = '''
@0@ void public_func (void);
@0@ int public_var;
int main (void) { return 0; }
'''.format(default_visibility)

    if cc.compiles(test_visibility, args: compile_args_c + ['-fvisibility=hidden'])
        add_project_arguments(['-fvisibility=hidden'], language: 'c')
        config.set('DBUS_PRIVATE_EXPORT', default_visibility)
        config.set('DBUS_EXPORT', default_visibility)
        # If we were using C++ then we'd need to add -fvisibility=hidden
        # to the C++ arguments too, but that's not currently applicable here.
        assert(not using_cpp, 'C++ only used on Windows')
    endif
endif

if host_machine.endian() == 'big'
  config.set('WORDS_BIGENDIAN', 1)
endif

# Integer sizes
int_types = [
    {
        'type': 'short',
        'size': cc.sizeof('short'),
    }, {
        'type': 'int',
        'size': cc.sizeof('int'),
        'type_c': '(val)',
        'type_u_c': '(val)',
        'type_m': '',
    }, {
        'type': 'long',
        'size': cc.sizeof('long'),
        'type_c': '(val##L)',
        'type_u_c': '(val##UL)',
        'type_m': 'l',
    }, {
        'type': 'long long',
        'size': cc.sizeof('long long'),
        'type_c': '(val##LL)',
        'type_u_c': '(val##ULL)',
        'type_m': 'll',
    }, {
        'type': '__int64',
        'size': cc.sizeof('__int64'),
        'type_c': '(val##i64)',
        'type_u_c': '(val##ui64)',
        'type_m': 'I64',
    }, {
        'type': 'err'
    },
]

foreach type : int_types
    if type.get('type') == 'err'
        error('Could not find a 64-bit integer type.')
    endif
    if type.get('size') == 8
        arch_config.set('DBUS_INT64_TYPE', type.get('type'))
        arch_config.set('DBUS_INT64_CONSTANT', type.get('type_c'))
        arch_config.set('DBUS_UINT64_CONSTANT', type.get('type_u_c'))
        if platform_windows
          # MSVCRT.dll printf() doesn't support %lld
          arch_config.set('DBUS_INT64_MODIFIER', 'I64')
        else
          arch_config.set('DBUS_INT64_MODIFIER', type.get('type_m'))
        endif
        break
    endif
endforeach

foreach type : int_types
    if type.get('type') == 'err'
        error('Could not find a 32-bit integer type.')
    endif
    if type.get('size') == 4
        arch_config.set('DBUS_INT32_TYPE', type.get('type'))
        break
    endif
endforeach

foreach type : int_types
    if type.get('type') == 'err'
        error('Could not find a 16-bit integer type.')
    endif
    if type.get('size') == 2
        arch_config.set('DBUS_INT16_TYPE', type.get('type'))
        break
    endif
endforeach

arch_config.set('DBUS_SIZEOF_VOID_P', cc.sizeof('void *'))

###############################################################################
# Dependencies

xsltproc = find_program('xsltproc', required: get_option('xml_docs'))
build_xml_docs = false

if xsltproc.found()
    build_xml_docs = true
    foreach fmt: [ 'html', 'manpages' ]
        xsl = 'http://docbook.sourceforge.net/release/xsl/current/@0@/docbook.xsl'.format(fmt)
        if run_command([xsltproc, '--nonet', xsl], check : false).returncode() == 0
            continue
        endif

        build_xml_docs = false
        if get_option('xml_docs').enabled()
            error('Docbook XSL "@0@" not found'.format(fmt))
        else
            message('Docbook XSL "@0@" not found, disabled automatically'.format(fmt))
        endif
    endforeach
endif

# For doxygen
doxygen = find_program('doxygen', required: get_option('doxygen_docs'))
ducktype = find_program('ducktype', required: get_option('ducktype_docs'))
yelpbuild = find_program('yelp-build', required: get_option('ducktype_docs'))

can_upload_docs = doxygen.found() and xsltproc.found() and ducktype.found()

qhelpgen = find_program('qhelpgenerator', 'qhelpgenerator-qt5', required: get_option('qt_help'))
qt_help_generate = doxygen.found() and qhelpgen.found()

data_config.set('DBUS_GENERATE_MAN', platform_windows ? 'NO' : 'YES')
data_config.set('DOXYGEN_QCH_FILE', meson.current_build_dir()
    / 'doc' / 'api' / 'qch' / 'dbus-@0@.qch'.format(version))
if qhelpgen.found()
    data_config.set('DOXYGEN_QHG_LOCATION', qhelpgen.full_path())
    data_config.set('DOXYGEN_GENERATE_QHP', 'YES')
else
    data_config.set('DOXYGEN_QHG_LOCATION', '')
    data_config.set('DOXYGEN_GENERATE_QHP', 'NO')
endif
data_config.set('top_srcdir', meson.project_source_root())
data_config.set('top_builddir', meson.project_build_root())


threads = dependency('threads')
config.set(
    'HAVE_MONOTONIC_CLOCK',
    cc.has_header_symbol('pthread.h', 'CLOCK_MONOTONIC', args: compile_args_c)
    and cc.has_header_symbol('pthread.h', 'pthread_condattr_setclock', args: compile_args_c)
    and cc.has_header_symbol('time.h', 'clock_getres', args: compile_args_c),
)

# Controls whether message bus daemon is built. Tests which depend on
# a running dbus-daemon will be disabled if message_bus is not set.
message_bus = get_option('message_bus')

if get_option('modular_tests').disabled()
    glib = dependency('', required: false)
else
    glib = dependency(
        'glib-2.0', version: '>=2.40',
        required: get_option('modular_tests'),
        fallback: ['glib', 'libglib_dep'],
        default_options: [
           'tests=false',
        ],
    )
endif

if glib.found()
    if platform_windows
        gio = dependency('gio-windows-2.0', required: glib.found())
        have_gio_unix = false
    else
        gio = dependency('gio-unix-2.0',    required: glib.found())
        have_gio_unix = gio.found()
    endif
else
    gio = dependency('', required: false)
    have_gio_unix = false
endif
use_glib = glib.found() and gio.found()
config.set('DBUS_WITH_GLIB', use_glib)

if message_bus
    expat = dependency('expat')
else
    expat = dependency('', required: false)
endif

if expat.type_name() == 'internal'
    # Configure-time checks can't act on subprojects that haven't been
    # built yet, but we know that subprojects/expat.wrap is a new enough
    # version to have this
    config.set('HAVE_XML_SETHASHSALT', true)
else
    config.set('HAVE_XML_SETHASHSALT', cc.has_function('XML_SetHashSalt', dependencies: expat))
endif

selinux = dependency('libselinux', version: '>=2.0.86', required: get_option('selinux'))

# the selinux code creates threads which requires libpthread even on linux
# TODO: smcv: actually we've stopped doing that. We still include <pthread.h> in
# selinux.c (but probably shouldn't), and we don't actually create the thread;
# so this can probably be simplified.
config.set('HAVE_SELINUX', selinux.found() and threads.found())

apparmor = dependency('libapparmor', version: '>=2.8.95', required: get_option('apparmor'))
config.set('HAVE_APPARMOR', apparmor.found())
config.set('HAVE_APPARMOR_2_10', apparmor.version().version_compare('>=2.10'))

if get_option('inotify').disabled()
    use_inotify = false
else
    use_inotify = cc.has_header('sys/inotify.h', args: compile_args_c)
    if get_option('inotify').enabled() and not use_inotify
        error('inotify support requested but not found')
    endif
endif

if get_option('epoll').disabled()
    use_linux_epoll = false
else
    use_linux_epoll = (
      cc.has_header('sys/epoll.h', args: compile_args_c) and
      cc.has_function(
          'epoll_create1',
          prefix: '#include <sys/epoll.h>',
          args: compile_args_c,
      )
    )
    if get_option('epoll').enabled() and not use_linux_epoll
        error('epoll support requested but not found')
    endif
endif
config.set('DBUS_HAVE_LINUX_EPOLL', use_linux_epoll)

if get_option('kqueue').disabled()
    use_kqueue = false
else
    use_kqueue = (
      cc.has_header('sys/event.h', args: compile_args_c) and
      cc.has_function(
          'kqueue',
          prefix: '#include <sys/event.h>',
          args: compile_args_c,
      )
    )
    if get_option('kqueue').enabled() and not use_kqueue
        error('kqueue support requested but not found')
    endif
endif
config.set('DBUS_BUS_ENABLE_KQUEUE', use_kqueue)

if get_option('launchd').disabled()
    use_launchd = false
else
    launchctl = find_program('launchctl', required: get_option('launchd'))
    use_launchd = cc.has_header('launch.h', args: compile_args_c) and launchctl.found()
    if get_option('launchd').enabled() and not use_launchd
        error('launchd support requested but not found')
    endif
endif
config.set('DBUS_BUS_ENABLE_LAUNCHD', use_launchd)

if use_launchd
    launchd_agent_dir = get_option('launchd_agent_dir')
    if launchd_agent_dir == ''
        launchd_agent_dir = '/Library/LaunchAgents'
    endif
endif

systemd = dependency('libsystemd', version: '>=209', required: get_option('systemd'))
use_systemd = systemd.found()
config.set('HAVE_SYSTEMD', use_systemd)

if use_systemd
    # If not found in $PATH, we might still have systemd and systemctl at runtime
    # (perhaps dbus is being compiled in a minimal chroot with no systemd).
    # Assume the upstream-recommended location. Distributors with split /usr
    # can override this with --native-file (see https://mesonbuild.com/Machine-files.html)
    systemctl = find_program('systemctl', required: false)
    if systemctl.found()
        systemctl = systemctl.full_path()
    else
        systemctl = '/usr/bin/systemctl'
    endif

    systemd_system_unitdir  = get_option('systemd_system_unitdir')
    systemd_user_unitdir    = get_option('systemd_user_unitdir')

    systemd_dirs = dependency('systemd', required: false)
    if systemd_system_unitdir == ''
        systemd_system_unitdir = (systemd_dirs.found()
            ? systemd_dirs.get_variable(pkgconfig: 'systemdsystemunitdir')
            : '/lib/systemd/system'
        )
    endif
    if systemd_user_unitdir == ''
        systemd_user_unitdir = (systemd_dirs.found()
            ? systemd_dirs.get_variable(pkgconfig: 'systemduserunitdir')
            : '/usr/lib/systemd/user'
        )
    endif
else
    systemctl = ''
endif
data_config.set('SYSTEMCTL', systemctl)



use_traditional_activation = message_bus and get_option('traditional_activation')
config.set('ENABLE_TRADITIONAL_ACTIVATION', use_traditional_activation)

if not (use_systemd or use_traditional_activation)
    warning('Traditional activation and systemd activation are both disabled, '
        +   'so service activation (automatically starting services that '
        +   'receive messages) will not work')
endif


have_console_owner_file = false
console_owner_file = get_option('solaris_console_owner_file')
if console_owner_file != ''
  if not host_os.contains('solaris')
    error('solaris_console_owner_file is only supported on Solaris)')
  endif
  have_console_owner_file = true
  if console_owner_file == 'auto'
    console_owner_file = '/dev/console'
  else
  endif
endif
config.set('HAVE_CONSOLE_OWNER_FILE', have_console_owner_file)
config.set_quoted('DBUS_CONSOLE_OWNER_FILE', console_owner_file)

if get_option('libaudit').disabled()
    have_libaudit = false
else
    libaudit = cc.find_library('audit', required: false)
    libaudit_ok = cc.has_function('audit_log_user_avc_message', dependencies: libaudit)
    cap_ng   = cc.find_library('cap-ng', required: false)
    cap_ng_ok   = cc.has_function('capng_clear', dependencies: cap_ng)
    have_libaudit = libaudit_ok and cap_ng_ok
    if get_option('libaudit').enabled() and not have_libaudit
        error('libaudit support requested but not found')
    endif
endif
config.set('HAVE_LIBAUDIT', have_libaudit)

if have_libaudit
    selinux = [ selinux, libaudit, cap_ng ]
endif

# Check for ADT API (Solaris Basic Security Mode auditing)
adt_api_check = cc.compiles('''
#include <bsm/adt.h>
int main() {
    adt_user_context = ADT_USER;
    return 0;
}
''', args: compile_args_c)
config.set('HAVE_ADT', adt_api_check)
if adt_api_check
    adt_libs = cc.find_library('bsm')
else
    adt_libs = dependency('', required: false)
endif

# Check for SCM_RIGHTS
has_scm_rights = cc.compiles('''
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
int main() {
    static int x = SCM_RIGHTS;
    return 0;
}
''', args: compile_args_c)
config.set('HAVE_UNIX_FD_PASSING', has_scm_rights)

valgrind = dependency(
    'valgrind',
    version: '>=3.6',
    required: get_option('valgrind'),
)
config.set('WITH_VALGRIND', valgrind.found())

if platform_win32ce
    network_libs = [
        cc.find_library('ws2'),
    ]
elif platform_windows
    network_libs = [
        cc.find_library('ws2_32'),
        cc.find_library('iphlpapi'),
        cc.find_library('dbghelp'),
    ]
else
    network_libs = []
endif

if get_option('x11_autolaunch').disabled()
    use_x11_autolaunch = false
    x11 = not_found
else
    if get_option('x11_autolaunch').enabled() and platform_windows
        error('X11 autolaunch is not supported on Windows')
    endif

    x11 = dependency('x11', required: false)
    use_x11_autolaunch = x11.found()

    if get_option('x11_autolaunch').enabled() and not use_x11_autolaunch
        error('X11 autolaunch support requested but not found')
    endif
endif
config.set('DBUS_BUILD_X11', use_x11_autolaunch)
config.set('DBUS_ENABLE_X11_AUTOLAUNCH', use_x11_autolaunch)


check_functions = [
    'accept4',
    'clearenv',
    'closefrom',
    'close_range',
    'fpathconf',
    'getgrouplist',
    'getpeereid',
    'getpeerucred',
    'getpwnam_r',
    'getrandom',
    'getresuid',
    'getrlimit',
    'inotify_init1',
    'issetugid',
    'localeconv',
    'nanosleep',
    'pipe2',
    'poll',
    'prctl',
    'prlimit',
    'raise',
    'setenv',
    'setlocale',
    'setresuid',
    'setrlimit',
    'socketpair',
    'unsetenv',
    'usleep',
]

foreach function : check_functions
    macro = 'HAVE_' + function.underscorify().to_upper()
    config.set(macro, cc.has_function(function, args: compile_args_c) ? 1 : false)
endforeach

check_headers = [
    'afunix.h',
    'alloca.h',
    'byteswap.h',
    'crt_externs.h',
    'dirent.h',
    'errno.h',
    'inttypes.h',
    'io.h',
    'locale.h',
    'linux/close_range.h',
    'linux/magic.h',
    'locale.h',
    'signal.h',
    'syslog.h',
    'sys/prctl.h',
    'sys/random.h',
    'sys/resource.h',
    'sys/syscall.h',
    'sys/time.h',
    'sys/vfs.h',
    'unistd.h',
    'ws2tcpip.h',
]

foreach header : check_headers
    macro = 'HAVE_' + header.underscorify().to_upper()
    config.set(macro, cc.has_header(header, args: compile_args_c) ? 1 : false)
endforeach

execinfo = cc.find_library('execinfo', required: false)
have_backtrace = (cc.has_header('execinfo.h', args: compile_args_c)
    and cc.has_function('backtrace', dependencies: execinfo, args: compile_args_c)
)
config.set('HAVE_BACKTRACE', have_backtrace)

# Can't use cc.has_function here because atomic operations are not
# exactly functions
config.set10(
    'DBUS_USE_SYNC',
    cc.links('''
    int main(void)
    {
      int a = 4;
      int b = __sync_sub_and_fetch (&a, 4);
      return b;
    }
    ''', args: compile_args_c)
)

config.set10(
    'HAVE_DECL_ENVIRON',
    cc.has_header_symbol('unistd.h', 'environ', args: compile_args_c),
)
config.set10('HAVE_DECL_LOG_PERROR',
    cc.has_header_symbol('syslog.h', 'LOG_PERROR', args: compile_args_c),
)
config.set10('HAVE_DECL_MSG_NOSIGNAL',
    cc.has_header_symbol(
        'sys/socket.h',
        'MSG_NOSIGNAL',
        args: compile_args_c,
    ),
)
config.set('HAVE_SOCKLEN_T',
    cc.has_type(
        'socklen_t',
        prefix: '#include <sys/socket.h>',
        args: compile_args_c,
    )
)
config.set('HAVE_CMSGCRED',
    cc.has_type(
        'struct cmsgcred',
        prefix: '#include <sys/socket.h>',
        args: compile_args_c,
    )
)

config.set('HAVE_WRITEV',
    cc.has_header_symbol(
        'sys/uio.h',
        'writev',
        args: compile_args_c,
    )
)

config.set('HAVE_UNPCBID',
    cc.has_member(
        'struct unpcbid',
        'unp_pid',
        prefix: '#include <sys/un.h>',
        args: compile_args_c,
    )
)

config.set('HAVE_FSTATFS',
    cc.has_function(
        'fstatfs',
        prefix : '#include <sys/vfs.h>',
        args: compile_args_c,
    )
)

###############################################################################
# Project options

warning_args = []
link_args = []

# Verbose mode
verbose_mode = get_option('verbose_mode')
config.set('DBUS_ENABLE_VERBOSE_MODE', verbose_mode)

# Asserts defaults to developer mode
asserts = get_option('asserts')
config.set('DBUS_ENABLE_ASSERT', asserts ? 1 : false)
config.set('DBUS_DISABLE_ASSERT', not asserts ? 1 : false)

# -rdynamic is needed for glibc's backtrace_symbols to work.
# No clue how much overhead this adds, but it's useful
# to do this on any assertion failure,
# so for now it's enabled anytime asserts are (currently not
# in production builds).
# To get -rdynamic you pass -export-dynamic to libtool.
config.set('DBUS_BUILT_R_DYNAMIC', asserts ? 1 : false)
if asserts
    link_args += '-export-dynamic'
endif

if not asserts
    warning_args += [
        '-Wno-unused-but-set-variable',
        '-Wno-unused-variable',
        '-Wno-unused-function',
    ]
endif


checks = get_option('checks')
config.set('DBUS_ENABLE_CHECKS', checks ? 1 : false)
config.set('DBUS_DISABLE_CHECKS', not checks ? 1 : false)
config.set('G_DISABLE_CHECKS', not checks ? 1 : false)
config.set('HAVE_GIO_UNIX', have_gio_unix ? 1 : false)
# Ignore post-2.38 deprecations, prevent use of post-2.44 APIs.
config.set('GLIB_VERSION_MIN_REQUIRED', 'GLIB_VERSION_2_38')
config.set('GLIB_VERSION_MAX_ALLOWED', 'G_ENCODE_VERSION(2,44)')

windows_output_debug = get_option('windows_output_debug_string')
config.set('DBUS_USE_OUTPUT_DEBUG_STRING', windows_output_debug)

# Controls whether the tools are built.
tools = get_option('tools')

# DBUS_ENABLE_EMBEDDED_TESTS controls unit tests built in to .c files
# and some stuff in the test/ subdir.
embedded_tests = get_option('embedded_tests')
config.set('DBUS_ENABLE_EMBEDDED_TESTS', embedded_tests)


# DBUS_ENABLE_MODULAR_TESTS controls tests that work based on public API.
# These use GTest, from GLib, because life's too short. They're enabled by
# default (unless you don't have GLib), because they don't bloat the library
# or binaries.
dbus_enable_modular_tests = (
    not get_option('modular_tests').disabled()
    and glib.version().version_compare('>=2.40')
    and gio.found()
)

###############################################################################
# Paths and directories

docs_dir = get_option('datadir') / 'doc' / 'dbus'

# TODO: If a future Meson version gets a runstatedir option, try both.
# https://github.com/mesonbuild/meson/issues/4141
runstatedir = get_option('runtime_dir')

if runstatedir == ''
    if get_option('prefix') == '/usr'
        runstatedir = '/run'
    else
        runstatedir = get_option('localstatedir') / 'run'
    endif
endif

data_config.set('EXPANDED_LOCALSTATEDIR', get_option('prefix') / get_option('localstatedir'))
data_config.set('EXPANDED_SYSCONFDIR', get_option('prefix') / get_option('sysconfdir'))
data_config.set('EXPANDED_RUNSTATEDIR', get_option('prefix') / runstatedir)
data_config.set('EXPANDED_BINDIR',  get_option('prefix') / get_option('bindir'))
data_config.set('EXPANDED_DATADIR', get_option('prefix') / get_option('datadir'))

data_config.set('DBUS_PREFIX', get_option('prefix'))
config.set_quoted('DBUS_PREFIX', get_option('prefix'))
config.set_quoted('DBUS_BINDIR', get_option('prefix') / get_option('bindir'))
config.set_quoted('DBUS_DATADIR',get_option('prefix') / get_option('datadir'))
data_config.set('DBUS_LIBEXECDIR', get_option('prefix') / get_option('libexecdir'))

config.set_quoted('DBUS_RUNSTATEDIR',
    get_option('prefix') / runstatedir
)
config.set_quoted('DBUS_MACHINE_UUID_FILE',
    get_option('prefix') / get_option('localstatedir') / 'lib'/'dbus'/'machine-id'
)
config.set_quoted('DBUS_SYSTEM_CONFIG_FILE',
    get_option('prefix') / get_option('datadir') / 'dbus-1' / 'system.conf'
)
config.set_quoted('DBUS_SESSION_CONFIG_FILE',
    get_option('prefix') / get_option('datadir') / 'dbus-1' / 'session.conf'
)


system_socket = get_option('system_socket')
if system_socket == ''
    system_socket = (
        get_option('prefix') / runstatedir / 'dbus' / 'system_bus_socket'
    )
endif

# This check assumes that the disposition of /run and /var/run on the
# system where we're building is the same as on the system we're building
# for, so we can't usefully do this check if we're building for Windows,
# or if we're cross-building for Unix on a Windows machine.
#
# The check is shared between Autotools, CMake and Meson.
# Because we only run it on Unix, it's fine to make it a shell script.
if platform_unix and build_machine.system() != 'windows'
    msg = run_command(
        find_program('tools/check-runstatedir.sh'),
        system_socket,
        check: false,
    ).stdout()
    if msg != ''
        warning(msg)
    endif
endif

data_config.set('DBUS_SYSTEM_SOCKET', system_socket)

## System bus only listens on local domain sockets, and never
## on an abstract socket (so only root can create the socket).
##
## This won't work on Windows. It's not meant to - the system bus is
## meaningless on Windows anyway.
##
## This has to be suitable for hard-coding in client libraries as well as
## in the dbus-daemon's configuration, so it has to be valid to listen on
## and also to connect to. If this ever changes, it'll need to be split into
## two variables, one for the listening address and one for the connecting
## address.
system_bus_default_address = 'unix:path=@0@'.format(system_socket)
data_config.set('DBUS_SYSTEM_BUS_DEFAULT_ADDRESS', system_bus_default_address)
config.set_quoted('DBUS_SYSTEM_BUS_DEFAULT_ADDRESS', system_bus_default_address)


system_pid_file = get_option('system_pid_file')
if system_pid_file == ''
    system_pid_file = get_option('prefix') / runstatedir / 'dbus'/'pid'
endif
data_config.set('DBUS_SYSTEM_PID_FILE', system_pid_file)


dbus_user = get_option('dbus_user')
data_config.set('DBUS_USER', dbus_user)
config.set_quoted('DBUS_USER', dbus_user)

test_user = get_option('test_user')
config.set_quoted('DBUS_TEST_USER', test_user)

daemon_dir = get_option('dbus_daemondir')
if daemon_dir == ''
    daemon_dir = get_option('prefix') / get_option('bindir')
endif
data_config.set('DBUS_DAEMONDIR', daemon_dir)
config.set_quoted('DBUS_DAEMONDIR', daemon_dir)



# Relocation is disabled by default, let's check if we need to enable it
relocation = false
if get_option('relocation').enabled()
    # Manually forced at true
    relocation = true
endif
if get_option('relocation').auto() and platform_windows
    # By default, on Windows we are relocatable if possible
    relocation = true
endif

# Now check if it's not possible
# Meson does not separate exec_prefix and prefix (hopefully)
if relocation and not (get_option('libdir') in [ 'lib', 'lib64', ])
    message = (
        'Relocatable pkg-config metadata requires default libdir, '
        + 'not @0@'.format(get_option('libdir'))
    )
    if get_option('relocation').enabled()
        error(message)
    else
        warning(message)
    endif
endif

#### Directory to source sysconfdir configuration from

# On Windows this is relative to where we put the bus setup, in
# ${datadir}/dbus-1. For simplicity, we only do this if
# ${sysconfdir} = ${prefix}/etc and ${datadir} = ${prefix}/share.
#
# On Unix, or on Windows with weird install layouts, it's the absolute path.
if (platform_windows
    and data_config.get('EXPANDED_SYSCONFDIR') == get_option('prefix') / 'etc'
    and data_config.get('EXPANDED_DATADIR')    == get_option('prefix') / 'share'
)
    sysconfdir_from_pkgdatadir = '../../etc'
    datadir_from_pkgsysconfdir = '../../share'
else
    sysconfdir_from_pkgdatadir = data_config.get('EXPANDED_SYSCONFDIR')
    datadir_from_pkgsysconfdir = data_config.get('EXPANDED_DATADIR')
endif
data_config.set('SYSCONFDIR_FROM_PKGDATADIR', sysconfdir_from_pkgdatadir)
data_config.set('DATADIR_FROM_PKGSYSCONFDIR', datadir_from_pkgsysconfdir)

#### Find socket directories
values = run_command(python, '-c',
    'import os; [print(os.getenv(e, "")) for e in ["TMPDIR", "TEMP", "TMP"]]',
    check: true,
).stdout()
values += '/tmp'
default_socket_dir = values.strip().split('\n')[0]

test_socket_dir = get_option('test_socket_dir')
if test_socket_dir == ''
    test_socket_dir = default_socket_dir
endif
test_listen = platform_unix ? 'unix:tmpdir=' + test_socket_dir : 'tcp:host=localhost'
config.set_quoted('TEST_LISTEN', test_listen)
config.set_quoted('DBUS_TEST_SOCKET_DIR', test_socket_dir)
data_config.set('TEST_LISTEN', test_listen)

session_socket_dir = get_option('session_socket_dir')
if session_socket_dir == ''
    session_socket_dir = default_socket_dir
endif
config.set_quoted('DBUS_SESSION_SOCKET_DIR', session_socket_dir)

# This must be a listening address. It doesn't necessarily need to be an
# address you can connect to - it can be something vague like
# "nonce-tcp:".
session_bus_listen_address = get_option('dbus_session_bus_listen_address')
if session_bus_listen_address == ''
    if platform_windows
        # On Windows, you can (and should) listen on autolaunch addresses,
        # because autolaunching is not the same as X11 autolaunching.
        session_bus_listen_address = 'autolaunch:'
    elif use_launchd
        # macOS default is to use launchd
        session_bus_listen_address = 'launchd:env=DBUS_LAUNCHD_SESSION_BUS_SOCKET'
    else
        # The default on all other Unix platforms (notably Linux)
        # is to create a randomly named socket in /tmp or similar
        session_bus_listen_address = 'unix:tmpdir=@0@'.format(session_socket_dir)
    endif
endif
data_config.set('DBUS_SESSION_BUS_LISTEN_ADDRESS', session_bus_listen_address)


# This must be an address you can connect to. It doesn't necessarily
# need to be an address you can listen on - it can be "autolaunch:",
# even on Unix.
session_bus_connect_address = get_option('dbus_session_bus_connect_address')
if session_bus_connect_address == ''
    session_bus_connect_address = 'autolaunch:'
endif
config.set_quoted('DBUS_SESSION_BUS_CONNECT_ADDRESS', session_bus_connect_address)


config.set('DBUS_ENABLE_STATS', get_option('stats'))
config.set('DBUS_ENABLE_CONTAINERS', get_option('containers'))
enable_user_session = get_option('user_session')


config.set('DBUS_COMPILATION', true)

exe_ext = platform_windows ? '.exe' : ''
config.set_quoted('DBUS_EXEEXT', exe_ext)

compile_warnings = []
compile_warnings_c = []

# -fno-common makes the linker more strict: on some systems the linker
# is *always* this strict, so we want to behave like that everywhere.
# We treat this like a warning, since that's basically how we're using it.
compile_warnings += ['-fno-common']

if cc.get_id() == 'msvc'
  compile_warnings += [
      # once
      '/wo4018', # 'expression' : signed/unsigned mismatch
      # disabled
      '/wd4090', # 'operation' : different 'modifier' qualifiers
      '/wd4101', # 'identifier' : unreferenced local variable
      '/wd4127', # conditional expression is constant
      '/wd4244', # 'argument' : conversion from 'type1' to 'type2', possible loss of data
      # error
      '/we4002', # too many actual parameters for macro 'identifier'
      '/we4003', # not enough actual parameters for macro 'identifier'
      '/we4013', # 'function' undefined; assuming extern returning int
      '/we4028', # formal parameter 'number' different from declaration
      '/we4031', # second formal parameter list longer than the first list
      '/we4047', # operator' : 'identifier1' differs in levels of indirection from 'identifier2'
      '/we4114', # same type qualifier used more than once
      '/we4133', # 'type' : incompatible types - from 'type1' to 'type2'
  ]
else
  compile_warnings += [
      # These warnings are intentionally disabled:
      # - missing field initializers being implicitly 0 is a feature,
      #   not a bug
      # - -Wunused-parameter is annoying when writing callbacks that follow
      #   a fixed signature but do not necessarily need all of its parameters
      '-Wno-missing-field-initializers',
      '-Wno-unused-parameter',

      # General warnings for both C and C++
      # TODO: Some of these are probably redundant with Meson's warning_level
      '-Warray-bounds',
      '-Wcast-align',
      '-Wchar-subscripts',
      '-Wdouble-promotion',
      '-Wduplicated-branches',
      '-Wduplicated-cond',
      '-Wfloat-equal',
      '-Wformat-nonliteral',
      '-Wformat-security',
      '-Wformat=2',
      '-Winit-self',
      '-Winline',
      '-Wlogical-op',
      '-Wmissing-declarations',
      '-Wmissing-format-attribute',
      '-Wmissing-include-dirs',
      '-Wmissing-noreturn',
      '-Wnull-dereference',
      '-Wpacked',
      '-Wpointer-arith',
      '-Wredundant-decls',
      '-Wrestrict',
      '-Wreturn-type',
      '-Wshadow',
      '-Wsign-compare',
      '-Wstrict-aliasing',
      '-Wswitch-default',
      '-Wswitch-enum',
      '-Wundef',
      '-Wunused-but-set-variable',
      '-Wwrite-strings',
  ]

  compile_warnings_c += [
      # Extra warnings just for C
      '-Wdeclaration-after-statement',
      '-Wimplicit-function-declaration',
      '-Wjump-misses-init',
      '-Wmissing-prototypes',
      '-Wnested-externs',
      '-Wold-style-definition',
      '-Wpointer-sign',
      '-Wstrict-prototypes',
  ]
endif

compile_warnings_c = cc.get_supported_arguments(compile_warnings + compile_warnings_c)
add_project_arguments(compile_warnings_c, language: 'c')

link_args = cc.get_supported_link_arguments(link_args)
add_project_link_arguments(link_args, language: ['c'])

if using_cpp
    compile_warnings_cpp = cpp.get_supported_arguments(compile_warnings)
    add_project_arguments(compile_warnings_cpp, language: 'cpp')
    add_project_link_arguments(link_args, language: ['cpp'])
endif

root_include = include_directories('.')

configure_file(
    output: 'config.h',
    configuration: config,
)

bonus_files = files(
    'AUTHORS',
    'CONTRIBUTING.md',
    'COPYING',
    'NEWS',
    'README',
)

if platform_unix
    run_target(
        'maintainer-update-authors',
        command: 'maint/update-authors.sh',
    )
endif

subdir('dbus')
if message_bus
    subdir('bus')
endif
if tools
    subdir('tools')
endif
subdir('test')
subdir('doc')
subdir('cmake')

meson.add_install_script('meson_post_install.py',
    '@0@'.format(relocation),
)

pkgconfig.generate(
    libdbus,
    name: 'dbus',
    filebase: 'dbus-1',
    description: 'Free desktop message bus',
    subdirs: [ 'dbus-1.0' ],
    extra_cflags: [
        '-I${libdir}/dbus-1.0/include',
    ] + dbus_static_flags,
    variables: {
         'original_prefix': get_option('prefix'),

         'exec_prefix': '${prefix}',
         'bindir': '${prefix}' / get_option('bindir'),
         'datadir': '${prefix}' / get_option('datadir'),
         'datarootdir': '${prefix}' / get_option('datadir'),
         'sysconfdir': '${prefix}' / get_option('sysconfdir'),

         'daemondir': '${bindir}',
         'system_bus_default_address': system_bus_default_address,
         'session_bus_services_dir': '${datadir}/dbus-1/services',
         'system_bus_services_dir': '${datadir}/dbus-1/system-services',
         'interfaces_dir': '${datadir}/dbus-1/interfaces',
    }
)

if meson.version().version_compare('>=0.60.0')
    foreach dir : install_emptydirs
        install_emptydir(dir)
    endforeach
else
    meson.add_install_script(
        'tools/meson-compat-install-emptydirs.py',
        ':'.join(install_emptydirs),
    )
endif

foreach symlink : install_symlinks
    if not platform_unix
        warning(
            'Not creating symbolic link @0@/@1@ -> @2@'.format(
                symlink['install_dir'],
                symlink['link_name'],
                symlink['pointing_to'],
            )
        )
    elif meson.version().version_compare('>=0.61.0')
        install_symlink(
            symlink['link_name'],
            install_dir : symlink['install_dir'],
            pointing_to : symlink['pointing_to'],
        )
    else
        meson.add_install_script(
            'tools/meson-compat-install-symlink.py',
            symlink['link_name'],
            symlink['install_dir'],
            symlink['pointing_to'],
        )
    endif
endforeach

summary_dict = {
    'prefix':                   get_option('prefix'),
    'exec_prefix':              get_option('prefix'),
    'libdir':                   get_option('prefix') / get_option('libdir'),
    'libexecdir':               get_option('prefix') / get_option('libexecdir'),
    'bindir':                   get_option('prefix') / get_option('bindir'),
    'sysconfdir':               data_config.get('EXPANDED_SYSCONFDIR'),
    'localstatedir':            data_config.get('EXPANDED_LOCALSTATEDIR'),
    'runstatedir':              data_config.get('EXPANDED_RUNSTATEDIR'),
    'datadir':                  data_config.get('EXPANDED_DATADIR'),
    'source code location':     meson.project_source_root(),
    'compiler':                 cc.get_id(),
    'cflags':                   compile_args_c + compile_warnings_c,
}

if using_cpp
    summary_dict += {
        'cxxflags':             compile_args_cpp + compile_warnings_cpp,
    }
endif

summary_dict += {
    'ldflags':                  (link_args.length() == 0) ? '[]' : link_args,
    '64-bit int':               arch_config.get('DBUS_INT64_TYPE'),
    '32-bit int':               arch_config.get('DBUS_INT32_TYPE'),
    '16-bit int':               arch_config.get('DBUS_INT16_TYPE'),
    'pointer size':             arch_config.get('DBUS_SIZEOF_VOID_P'),
    'xsltproc':                 xsltproc.found() ? xsltproc.full_path() : '',
    'Doxygen':                  doxygen.found() ? doxygen.full_path() : '',
    'ducktype':                 ducktype.found() ? ducktype.full_path() : '',
    'yelp-build':               yelpbuild.found() ? yelpbuild.full_path() : '',

    'gcc coverage':             get_option('b_coverage'),
    'gcc profiling':            get_option('b_pgo'),
    'Building embedded tests':  embedded_tests,
    'Building modular tests':   dbus_enable_modular_tests,
    '- with GLib':              use_glib,
    'Installing tests':         get_option('installed_tests'),
    'Building verbose mode':    verbose_mode,
    'Building assertions':      asserts,
    'Building checks':          checks,
    'Building bus stats API':   get_option('stats'),
    'Building container API':   get_option('containers'),
    'Building SELinux support': config.get('HAVE_SELINUX'),
    'Building AppArmor support': apparmor.found(),
    'Building inotify support': use_inotify,
    'Building kqueue support':  use_kqueue,
    'Building systemd support': use_systemd,
    'Traditional activation':   use_traditional_activation,
    'Building X11 code':        config.get('DBUS_BUILD_X11'),
    'Building Doxygen docs':    doxygen.found(),
    'Building Qt help file':    qt_help_generate,
    'Building Ducktype docs':   ducktype.found(),
    'Building XML docs':        build_xml_docs,
    'Building launchd support': use_launchd,
    'Building dbus-daemon':     message_bus,
    'Building tools':           tools,
    'System bus socket':        data_config.get('DBUS_SYSTEM_SOCKET'),
    'System bus address':       config.get('DBUS_SYSTEM_BUS_DEFAULT_ADDRESS'),
    'System bus PID file':      data_config.get('DBUS_SYSTEM_PID_FILE'),
    'Session bus listens on':   data_config.get('DBUS_SESSION_BUS_LISTEN_ADDRESS'),
    'Session clients connect to': config.get('DBUS_SESSION_BUS_CONNECT_ADDRESS'),
    'System bus user':          dbus_user,
    'Session bus services dir':
        get_option('prefix') / get_option('datadir') / 'dbus-1' / 'services',
    'Tests socket dir':         test_socket_dir,
}

if host_os.contains('solaris')
  summary_dict += {
      'Console owner file': console_owner_file,
  }
endif

summary(summary_dict, bool_yn: true)

if embedded_tests
  warning('building with unit tests increases the size of the installed library and renders it insecure.')
  if not asserts
    warning('building with embedded tests but without assertions means tests may not properly report failures (this configuration is only useful when doing something like profiling the tests)')
  endif
endif

if get_option('b_coverage')
  warning('Building with coverage profiling is definitely for developers only.')
endif

if verbose_mode
  warning('building with verbose mode increases library size, may slightly increase security risk, and decreases performance.')
endif

if asserts
  warning('building with assertions increases library size and decreases performance.')
endif

if not checks
  warning('building without checks for arguments passed to public API makes it harder to debug apps using D-Bus, but will slightly decrease D-Bus library size and _very_ slightly improve performance.')
endif
