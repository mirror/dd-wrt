# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
# SPDX-License-Identifier: MIT
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

###############################################################################
# Tests installation

install_tests = get_option('installed_tests')

test_exec_dir = get_option('libexecdir') / 'installed-tests' / 'dbus'
test_meta_dir = get_option('datadir') / 'installed-tests' / 'dbus'

###############################################################################
# Test configuration needs some specific keys

test_data_config = configuration_data()
test_data_config.merge_from(data_config)

test_data_config.set('EXEEXT', exe_ext)
# / '' to convert \-separated dir to /-separated dir on win32
test_data_config.set('DBUS_TEST_EXEC', meson.current_build_dir() / '')
test_data_config.set('DBUS_TEST_DATA', meson.current_build_dir() / 'data')


test_env = environment()

test_env.set('DBUS_TOP_SRCDIR',     meson.project_source_root())
test_env.set('DBUS_TEST_SRCDIR',    meson.current_source_dir())
test_env.set('G_TEST_SRCDIR',       meson.current_source_dir())

test_env.set('DBUS_TOP_BUILDDIR',   meson.project_build_root())
test_env.set('DBUS_TEST_HOMEDIR',   meson.project_build_root() / 'dbus')
test_env.set('HOME',                meson.project_build_root() / 'dbus')
# Tests in bus/config-parser.c rely on this specific value
test_env.set('DBUS_TEST_BUILDDIR',  meson.current_build_dir())
test_env.set('G_TEST_BUILDDIR',     meson.current_build_dir())
test_env.set('DBUS_TEST_EXEC',      meson.current_build_dir())
test_env.set('DBUS_TEST_DATA',      meson.current_build_dir() / 'data')

if message_bus
    test_env.set('DBUS_TEST_DAEMON',        dbus_daemon.full_path())
endif

if tools
    test_env.set('DBUS_TEST_DBUS_MONITOR',  dbus_monitor.full_path())
    test_env.set('DBUS_TEST_DBUS_SEND',     dbus_send.full_path())
endif

if message_bus and tools
    test_env.set('DBUS_TEST_DBUS_LAUNCH',   dbus_launch.full_path())
endif

if platform_unix and tools
    test_env.set('DBUS_TEST_DBUS_UUIDGEN',  dbus_uuidgen.full_path())
endif

if platform_windows
    # test-autolaunch-win relies on being able to find the just-built
    # dbus-daemon in the PATH
    if build_machine.system().contains('windows')
        test_env.prepend('PATH', meson.project_build_root() / 'bus')
    else
        # Assume we'll use Wine to run tests while cross-compiling
        test_env.prepend(
            'WINEPATH',
            'Z:' + meson.project_build_root() / 'bus',
            separator: ';',
        )
    endif
endif

# Tests in bus/config-parser.c rely on these specific values for XDG_*
test_env.set('XDG_DATA_HOME',       meson.current_build_dir() / 'XDG_DATA_HOME')
test_env.set('XDG_RUNTIME_DIR',     meson.current_build_dir() / 'XDG_RUNTIME_DIR')
xdg_data_dirs = [
    meson.current_build_dir() / 'XDG_DATA_DIRS',
    meson.current_build_dir() / 'XDG_DATA_DIRS2'
]
test_env.set('XDG_DATA_DIRS',       xdg_data_dirs)

test_env.set('DBUS_SESSION_BUS_ADDRESS', 'do-not-use-real-session:')

test_env.set('DBUS_FATAL_WARNINGS', '1')
test_env.set('DBUS_TEST_UNINSTALLED', '1')

xdgdir = custom_target('gen-xdgdir',
    command: [
        python,
        files('mkdir-m700.py'),
        meson.current_build_dir() / 'XDG_RUNTIME_DIR',
    ],
    output: 'XDG_RUNTIME_DIR'
)

installed_tests = []

if meson.version().version_compare('>=0.63') and meson.version().version_compare('<0.63.1')
    # Work around https://github.com/mesonbuild/meson/issues/10577
    test_protocol = 'exitcode'
elif platform_windows
    # Work around https://gitlab.gnome.org/GNOME/glib/-/issues/2746
    test_protocol = 'exitcode'
else
    test_protocol = 'tap'
endif

###############################################################################
# Dbus testutils


libdbus_testutils_sources = [
    'disable-crash-handling.c',
    'test-utils.c',
]

if use_glib
    libdbus_testutils_sources += 'test-utils-glib.c'
endif

libdbus_testutils = static_library('dbus-testutils',
    libdbus_testutils_sources,

    include_directories: root_include,
    link_with: [
        libdbus,
        libdbus_internal,
    ],
    dependencies: [
        glib,
        gio,
        dbus_dependencies,
    ],
)


###############################################################################
# Test tools

# these binaries are used in tests but are not themselves tests
test_exit = executable('test-exit',
    'test-exit.c',
    include_directories: root_include,
    link_with: libdbus_testutils,
    dependencies: dbus_dependencies,
)
test_names = executable('test-names',
    'test-names.c',
    include_directories: root_include,
    link_with: libdbus_testutils,
    dependencies: dbus_dependencies,
)
test_privserver = executable('test-privserver',
    'test-privserver.c',
    include_directories: root_include,
    link_with: libdbus_testutils,
    dependencies: dbus_dependencies,
)
# This helper is meant to crash, so if we're compiling the rest with
# AddressSanitizer, we need to stop it from catching the SIGSEGV and
# turning it into _exit(1).
# We have to compile a separate copy of disable-crash-handling.c for
# test-segfault rather than using libdbus-testutils, because
# otherwise it would fail to link when using the AddressSanitizer.
test_segfault = executable('test-segfault',
    'test-segfault.c', 'disable-crash-handling.c',
    include_directories: root_include,
    dependencies: dbus_dependencies,
    override_options: ['b_sanitize=none'],
)
test_shell_service = executable('test-shell-service',
    'test-shell-service.c',
    include_directories: root_include,
    link_with: libdbus_testutils,
    dependencies: dbus_dependencies,
)

if use_traditional_activation
    test_spawn = executable('test-spawn',
        'spawn-test.c',
        include_directories: root_include,
        link_with: libdbus_testutils,
        dependencies: dbus_dependencies,
    )
endif
if use_traditional_activation and platform_unix
    launch_helper_for_tests = executable('launch-helper-for-tests',
        'bus/launch-helper-for-tests.c',
        include_directories: root_include,
        link_with: liblaunch_helper_internal,
        dependencies: dbus_dependencies,
    )
    test_data_config.set('TEST_LAUNCH_HELPER_BINARY', launch_helper_for_tests.full_path())
else
    # Dummy value, should not be used in practice
    test_data_config.set('TEST_LAUNCH_HELPER_BINARY', '/bin/false')
endif

if message_bus and tools and platform_unix and use_glib
    test_apparmor_activation = executable('test-apparmor-activation',
        'sd-activation.c',
        include_directories: root_include,
        c_args: '-DDBUS_TEST_APPARMOR_ACTIVATION',
        link_with: libdbus_testutils,
        dependencies: [
            glib, gio,
            apparmor,
        ],
        install: install_tests,
        install_dir: test_exec_dir,
    )
endif

###############################################################################
# Subdirectories need utilities above.

subdir('data')

# the "name-test" subdir in fact contains a bunch of tests now that need a
# temporary bus to be running to do stuff with. The directory should be renamed.
if message_bus and tools
    subdir('name-test')
endif

tests = []

if embedded_tests

    tests += [
        {
            'name': 'marshal-recursive',
            'srcs': [
                'internals/dbus-marshal-recursive-util.c',
                'internals/marshal-recursive.c',
            ],
            'link': [ libdbus_testutils, ],
            'install': false,
            'timeout': 60,
        },
        {
            'name': 'message-internals',
            'srcs': [
                'internals/dbus-marshal-recursive-util.c',
                'internals/dbus-message-factory.c',
                'internals/dbus-message-util.c',
                'internals/message-internals.c',
            ],
            'link': [ libdbus_testutils, ],
            'install': false,
            'suite': ['slow'],
        },
    ]

    if message_bus
       tests += [
           {
               'name': 'bus',
               'srcs': [ 'bus/main.c', 'bus/common.c' ],
               'link': [ libdbus_testutils, libdbus_daemon_internal, ],
               'install': false,
           },
           {
               'name': 'bus-dispatch-sha1',
               'srcs': [ 'bus/dispatch-sha1.c', 'bus/common.c' ],
               'link': [ libdbus_testutils, libdbus_daemon_internal, ],
               'install': false,
               'suite': ['slow'],
           },
       ]
    endif

    if use_traditional_activation
        tests += [
            {
                'name': 'bus-normal-activation',
                'srcs': [ 'bus/normal-activation.c', 'bus/common.c' ],
                'link': [ libdbus_testutils, libdbus_daemon_internal, ],
                'install': false,
                'suite': ['slow'],
                'timeout': 1000,
            },
        ]
    endif

    if platform_unix
        tests += [
            {
            'name': 'counter',
            'srcs': [ 'internals/counter.c' ],
            'link': [ libdbus_testutils, ],
            },
        ]
    endif

    if use_traditional_activation and platform_unix
        tests += [
            {
                'name': 'bus-failed-helper-activation',
                'srcs': [ 'bus/failed-helper-activation.c', 'bus/common.c' ],
                'link': [ libdbus_testutils, libdbus_daemon_internal, ],
                'install': false,
            },
            {
                'name': 'bus-helper-activation',
                'srcs': [ 'bus/helper-activation.c', 'bus/common.c' ],
                'link': [ libdbus_testutils, libdbus_daemon_internal, ],
                'install': false,
                'suite': ['slow'],
                'timeout': 1000,
            },
            {
                'name': 'bus-launch-helper-oom',
                'srcs': [ 'bus/launch-helper-oom.c' ],
                'link': [ libdbus_testutils, liblaunch_helper_internal, ],
                'install': false,
            },
            {
                'name': 'bus-system',
                'srcs': [ 'bus/system.c', ],
                'link': [ libdbus_testutils, liblaunch_helper_internal, ],
                'install': false,
            },
            {
                'name': 'spawn-oom',
                'srcs': [ 'internals/spawn-oom.c', ],
                'link': [ libdbus_testutils, ],
                'install': false,
            }
        ]
    endif
endif

tests += [
    {
        'name': 'test-service',
        'srcs': [ 'test-service.c' ],
        'link': [ libdbus_testutils, ],
        'install': true,
        'test': false,
    },
    {
        'name': 'test-sleep-forever',
        'srcs': [ 'test-sleep-forever.c' ],
        'link': [ libdbus_testutils, ],
        'install': true,
        'test': false,
    }
]

tests += [
    {
        'name': 'atomic',
        'srcs': [ 'internals/atomic.c' ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'hash',
        'srcs': [ 'internals/hash.c' ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'misc-internals',
        'srcs': [
            'internals/address.c',
            'internals/dbus-auth-script.c',
            'internals/dbus-auth-util.c',
            'internals/dbus-credentials-util.c',
            'internals/dbus-marshal-byteswap-util.c',
            'internals/dbus-marshal-recursive-util.c',
            'internals/dbus-marshal-validate-util.c',
            'internals/dbus-string-util.c',
            'internals/dbus-sysdeps-util.c',
            'internals/mempool.c',
            'internals/misc-internals.c',
            'internals/sha.c',
        ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'shell',
        'srcs': [ 'shell-test.c' ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'strings',
        'srcs': [ 'internals/strings.c' ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'printf',
        'srcs': [ 'internals/printf.c' ],
        'link': [ libdbus_testutils, ],
    },
    {
        'name': 'manual-backtrace',
        'srcs': [ 'manual-backtrace.c' ],
        'link': [ libdbus_testutils, ],
        'test': false,
    },
    {
        'name': 'manual-dir-iter',
        'srcs': [ 'manual-dir-iter.c' ],
        'link': [ libdbus_testutils, ],
        'test': false,
    },
    {
        'name': 'manual-tcp',
        'srcs': [ 'manual-tcp.c' ],
        'link': [ libdbus_testutils, ],
        'test': false,
    }
]

if platform_windows
    tests += [
        {
            'name': 'manual-paths',
            'srcs': [ 'manual-paths.c' ],
            'link': [ libdbus_testutils, ],
            'test': false,
        }
    ]
endif

if use_glib
    tests += [
        {
            'name': 'assertions',
            'srcs': [ 'internals/assertions.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'corrupt',
            'srcs': [ 'corrupt.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'dbus-daemon',
            'srcs': [ 'dbus-daemon.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['runs-dbus-daemon', 'slow'],
        },
        {
            'name': 'dbus-daemon-eavesdrop',
            'srcs': [ 'dbus-daemon-eavesdrop.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['runs-dbus-daemon'],
        },
        {
            'name': 'desktop-file',
            'srcs': [ 'internals/desktop-file.c' ],
            'link': [ libdbus_testutils, libdbus_internal, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'fdpass',
            'srcs': [ 'fdpass.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'header-fields',
            'srcs': [ 'header-fields.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['runs-dbus-daemon', 'slow'],
        },
        {
            'name': 'message',
            'srcs': [ 'message.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'monitor',
            'srcs': [ 'monitor.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['runs-dbus-daemon'],
            'timeout': 45,
        },
        {
            'name': 'loopback',
            'srcs': [ 'loopback.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'marshal',
            'srcs': [ 'marshal.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'refs',
            'srcs': [ 'internals/refs.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['slow'],
        },
        {
            'name': 'relay',
            'srcs': [ 'relay.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'timeout': 60,
        },
        {
            'name': 'server-oom',
            'srcs': [ 'internals/server-oom.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'syntax',
            'srcs': [ 'syntax.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'sysdeps',
            'srcs': [ 'internals/sysdeps.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'syslog',
            'srcs': [ 'internals/syslog.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'uid-permissions',
            'srcs': [ 'uid-permissions.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'suite': ['runs-dbus-daemon'],
        },
        {
            'name': 'variant',
            'srcs': [ 'internals/variant.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
        },
        {
            'name': 'manual-authz',
            'srcs': [ 'manual-authz.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'test': false,
        },
        {
            'name': 'manual-test-thread-blocking',
            'srcs': [ 'thread-blocking.c' ],
            'link': [ libdbus_testutils, ],
            'deps': [ glib, gio, ],
            'test': false,
        },
    ]

    if platform_unix
        tests += [
            {   'name': 'containers',
                'srcs': [ 'containers.c' ],
                'link': [ libdbus_testutils, ],
                'deps': [ glib, gio, ],
                'suite': ['runs-dbus-daemon'],
            },
            {   'name': 'sd-activation',
                'srcs': [ 'sd-activation.c' ],
                'link': [ libdbus_testutils, ],
                'deps': [ glib, gio, ],
                'suite': ['runs-dbus-daemon'],
            },
        ]

    endif
endif

foreach test: tests
    name = test.get('name')
    srcs = test.get('srcs')
    link = test.get('link', [])
    deps = test.get('deps', [])
    suites = test.get('suite', ['dbus'])
    install = test.get('install', true)

    if suites.contains('runs-dbus-daemon') and not (message_bus and tools)
        continue
    endif

    if test.get('test', true)
        exe_name = 'test-' + name
    else
        exe_name = name
    endif

    if 'slow' in suites
        timeout = 300
    else
        timeout = 30
    endif

    timeout = test.get('timeout', timeout)

    test_exe = executable(exe_name,
        srcs,
        link_with: link,
        dependencies: deps,
        include_directories: root_include,
        install: install_tests and install,
        install_dir: test_exec_dir,
    )

    # Some binaries are used in tests but are not themselves tests,
    # and some might be "as-installed" integration tests that aren't
    # guaranteed to work at build-time
    if test.get('build_time_test', true) and test.get('test', true)
        test(name,
            test_exe,
            args: ['--tap'],
            env: test_env,
            protocol: test_protocol,
            suite: suites,
            timeout: timeout,
        )
    endif

    if install and test.get('test', true)
        installed_tests += [{
            'name': exe_name,
            'exe': exe_name + exe_ext,
        }]
    endif
endforeach


###############################################################################
# Scripts

scripts = []

if message_bus and tools and platform_unix and use_glib
    scripts += [
        {   'name': 'test-dbus-daemon-fork.sh', },
        {   'name': 'transient-services.sh',
            'subdir': 'integration',
            'build_time_test': false },
        {   'name': 'test-apparmor-activation.sh',
            'build_time_test': false },
    ]

    # Testing dbus-launch relies on special code in that binary.
    if embedded_tests
        scripts += { 'name': 'test-dbus-launch-eval.sh' }
    endif
    if embedded_tests and use_x11_autolaunch
        scripts += { 'name': 'test-dbus-launch-x11.sh' }
    endif
endif

foreach script: scripts
    name = script.get('name')
    install = script.get('install', true)
    suites = script.get('suite', ['dbus'])
    test_subdir = script.get('subdir', '')

    if test_subdir == ''
        install_dir = test_exec_dir
    else
        install_dir = test_exec_dir / test_subdir
    endif

    if install_tests and install
        install_data(test_subdir / name,
            install_mode: 'rwxr-xr-x',
            install_dir: install_dir,
        )
        installed_tests += [{
            'name': name,
            'subdir': test_subdir,
        }]
    endif

    # Some scripts might be used in tests but not themselves tests,
    # and some are "as-installed" integration tests that aren't
    # guaranteed to work at build-time
    if script.get('build_time_test', true) and script.get('test', true)
        test(name,
            find_program(script.get('subdir', '.') / name),
            env: test_env,
            depends: xdgdir,
            suite: suites,
        )
    endif
endforeach


foreach test_case: installed_tests
    name = test_case.get('name')
    exe = test_case.get('exe', name)
    test_subdir = test_case.get('subdir', '')

    if test_subdir == ''
        exe = get_option('prefix') / test_exec_dir / exe
        install_dir = test_meta_dir
    else
        exe = get_option('prefix') / test_exec_dir / test_subdir / exe
        install_dir = test_meta_dir / test_subdir
    endif

    meta_config = configuration_data()
    meta_config.set('command',
        'env @0@ --tap'.format(exe),
    )
    configure_file(
        input : 'meta_template.test.in',
        output: name + '.test',
        configuration: meta_config,
        install: install_tests,
        install_dir: install_dir,
    )

    meta_config = configuration_data()
    meta_config.set('command',
        'env DBUS_TEST_EXEC=@0@ DBUS_TEST_DATA=@0@/data @1@ --tap'.format(
            get_option('prefix') / test_exec_dir, exe,
    ))
    configure_file(
        input : 'meta_template.test.in',
        output: name + '_with_config.test',
        configuration: meta_config,
        install: install_tests,
        install_dir: install_dir,
    )

endforeach
