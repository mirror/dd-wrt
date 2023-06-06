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

session_conf = configure_file(
    input: 'session.conf.in',
    output: 'session.conf',
    configuration: data_config,
    install_dir: get_option('datadir') / 'dbus-1',
)

if platform_unix
    configure_file(
        input: 'system.conf.in',
        output: 'system.conf',
        configuration: data_config,
        install_dir: get_option('datadir') / 'dbus-1',
    )
endif

configure_file(
    input: 'example-system-enable-stats.conf.in',
    output: 'example-system-enable-stats.conf',
    configuration: data_config,
    install_dir: get_option('datadir') / 'doc' / 'dbus' / 'examples',
)

configure_file(
    input: 'example-session-disable-stats.conf.in',
    output: 'example-session-disable-stats.conf',
    configuration: data_config,
    install_dir: get_option('datadir') / 'doc' / 'dbus' / 'examples',
)

install_data(
    'example-system-hardening-without-traditional-activation.conf',
    install_dir: get_option('datadir') / 'doc' / 'dbus' / 'examples',
)

if use_launchd
    configure_file(
        input: 'org.freedesktop.dbus-session.plist.in',
        output: 'org.freedesktop.dbus-session.plist',
        configuration: data_config,
        install_dir: launchd_agent_dir,
    )
endif

if use_systemd
    configure_file(
        input: 'dbus.service.in',
        output: 'dbus.service',
        configuration: data_config,
        install_dir: systemd_system_unitdir,
    )
    configure_file(
        input: 'dbus.socket.in',
        output: 'dbus.socket',
        configuration: data_config,
        install_dir: systemd_system_unitdir,
    )
    subdir('sysusers.d')
    subdir('tmpfiles.d')

    if enable_user_session
        subdir('systemd-user')
    endif
endif

subdir('legacy-config')


libdbus_daemon_internal_sources = [
    'activation.c',
    'apparmor.c',
    'audit.c',
    'bus.c',
    'config-loader-expat.c',
    'config-parser-common.c',
    'config-parser.c',
    'connection.c',
    'containers.c',
    'desktop-file.c',
    'dispatch.c',
    'driver.c',
    'expirelist.c',
    'policy.c',
    'selinux.c',
    'services.c',
    'signals.c',
    'stats.c',
    'test.c',
    'utils.c',
]

if use_kqueue
    libdbus_daemon_internal_sources += 'dir-watch-kqueue.c'
elif use_inotify
    libdbus_daemon_internal_sources += 'dir-watch-inotify.c'
else
    libdbus_daemon_internal_sources += 'dir-watch-default.c'
endif

libdbus_daemon_internal = static_library('dbus-daemon-internal',
    libdbus_daemon_internal_sources,
    include_directories: root_include,
    dependencies: [
        adt_libs,
        apparmor,
        expat,
        network_libs,
        selinux,
        threads,
    ],
    link_with: [
        libdbus,
        libdbus_internal,
    ],
)

dbus_daemon = executable('dbus-daemon',
    'main.c',
    include_directories: root_include,
    link_with: libdbus_daemon_internal,
    install: true,
)


if platform_unix and use_traditional_activation
    liblaunch_helper_internal_sources = [
        'config-loader-expat.c',
        'config-parser-common.c',
        'config-parser-trivial.c',
        'desktop-file.c',
        'utils.c',
    ]
    liblaunch_helper_internal = static_library('launch-helper-internal',
        liblaunch_helper_internal_sources,
        include_directories: root_include,
        link_with: [
            libdbus,
            libdbus_internal,
        ],
        dependencies: [
            expat,
            network_libs,
            threads,
        ],
        install: false,
    )

    dbus_daemon_launch_helper_sources = [
        'activation-helper.c',
        'activation-helper-bin.c',
    ]

    # This is the installed launch helper with the setuid checks
    # All files that have special cases #ifdef ACTIVATION_LAUNCHER_TEST must
    # be listed here and included in test/bus/launch-helper-for-tests.c,
    # not in liblaunch_helper_internal.
    dbus_daemon_launch_helper = executable('dbus-daemon-launch-helper',
        dbus_daemon_launch_helper_sources,
        include_directories: root_include,
        link_with: liblaunch_helper_internal,
        install: true,
        install_dir: get_option('libexecdir'),
    )
endif

install_emptydirs += [
    get_option('datadir') / 'dbus-1' / 'session.d',
    get_option('datadir') / 'dbus-1' / 'services',
]

if platform_unix
    install_emptydirs += [
        runstatedir / 'dbus',
        get_option('datadir') / 'dbus-1' / 'system.d',
        get_option('datadir') / 'dbus-1' / 'system-services',
    ]
endif

if use_systemd
    install_symlinks += [
        {
            'link_name': 'dbus.service',
            'install_dir': systemd_system_unitdir / 'multi-user.target.wants',
            'pointing_to': '../dbus.service',
        },
        {
            'link_name': 'dbus.socket',
            'install_dir': systemd_system_unitdir / 'sockets.target.wants',
            'pointing_to': '../dbus.socket',
        },
    ]
endif

if use_systemd and get_option('user_session')
    install_symlinks += [
        {
            'link_name': 'dbus.socket',
            'install_dir': systemd_user_unitdir / 'sockets.target.wants',
            'pointing_to': '../dbus.socket',
        },
    ]
endif
