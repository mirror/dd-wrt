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

if not platform_windows
    dbus_cleanup_sockets = executable('dbus-cleanup-sockets',
        'dbus-cleanup-sockets.c',
        include_directories: root_include,
        install: true,
    )
endif

if platform_windows
    dbus_launch_sources = [
        'dbus-launch-win.c'
    ]
else
    dbus_launch_sources = [
        'dbus-launch.c',
        'dbus-launch-x11.c',
        'tool-common.c',
    ]
endif

if message_bus
    dbus_launch = executable('dbus-launch',
        dbus_launch_sources,
        include_directories: root_include,
        link_with: libdbus,
        dependencies: [ x11, ],
        install: true,
    )
endif

dbus_monitor = executable('dbus-monitor',
    'dbus-print-message.c',
    'dbus-monitor.c',
    'tool-common.c',
    include_directories: root_include,
    link_with: libdbus,
    install: true,
)

if message_bus
    dbus_run_session = executable('dbus-run-session',
        'dbus-run-session.c',
        'tool-common.c',
        include_directories: root_include,
        link_with: libdbus_internal,
        install: true,
    )
endif

dbus_send = executable('dbus-send',
    'dbus-print-message.c',
    'dbus-send.c',
    'tool-common.c',
    include_directories: root_include,
    link_with: libdbus,
    install: true,
)

dbus_test_tool = executable('dbus-test-tool',
    'dbus-echo.c',
    'dbus-spam.c',
    'test-tool.c',
    'tool-common.c',
    include_directories: root_include,
    link_with: libdbus,
    install: true,
)

dbus_update_activation_environment = executable('dbus-update-activation-environment',
    'dbus-update-activation-environment.c',
    'tool-common.c',
    platform_windows ? windows.compile_resources('disable-uac.rc') : [],
    include_directories: root_include,
    link_with: libdbus,
    install: true,
)

if not platform_windows
    dbus_uuidgen = executable('dbus-uuidgen',
        'dbus-uuidgen.c',
        include_directories: root_include,
        link_with: libdbus,
        install: true,
    )
endif


install_data('GetAllMatchRules.py',
    install_dir: docs_dir / 'examples',
)

# Create the empty directory to hold the machine ID
install_emptydirs += [
    get_option('localstatedir') / 'lib' / 'dbus',
]
