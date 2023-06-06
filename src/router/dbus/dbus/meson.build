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

dbus_dependencies = [
    threads,
    network_libs,
    systemd,
    valgrind.partial_dependency(compile_args: true),
]

# source code that goes in the installed client library
# and is specific to library functionality
dbus_lib_sources = [
    'dbus-address.c',
    'dbus-auth.c',
    'dbus-bus.c',
    'dbus-connection.c',
    'dbus-credentials.c',
    'dbus-errors.c',
    'dbus-keyring.c',
    'dbus-marshal-byteswap.c',
    'dbus-marshal-header.c',
    'dbus-marshal-recursive.c',
    'dbus-marshal-validate.c',
    'dbus-message.c',
    'dbus-misc.c',
    'dbus-nonce.c',
    'dbus-object-tree.c',
    'dbus-pending-call.c',
    'dbus-resources.c',
    'dbus-server-debug-pipe.c',
    'dbus-server-socket.c',
    'dbus-server.c',
    'dbus-sha.c',
    'dbus-signature.c',
    'dbus-syntax.c',
    'dbus-threads.c',
    'dbus-timeout.c',
    'dbus-transport-socket.c',
    'dbus-transport.c',
    'dbus-watch.c',
]

# source code that goes in the installed client library
# AND is generic utility functionality used by the
# daemon or test programs (all symbols in here should
# be underscore-prefixed)

dbus_shared_sources = [
    'dbus-dataslot.c',
    'dbus-file.c',
    'dbus-hash.c',
    'dbus-internals.c',
    'dbus-list.c',
    'dbus-marshal-basic.c',
    'dbus-memory.c',
    'dbus-mempool.c',
    'dbus-pipe.c',
    'dbus-string.c',
    'dbus-sysdeps.c',
]

if embedded_tests
    dbus_shared_sources += 'dbus-test-tap.c'
endif


# source code that is generic utility functionality used
# by the bus daemon or test apps, but is NOT included
# in the D-Bus client library (all symbols in here
# should be underscore-prefixed but don't really need
# to be unless they move to DBUS_SHARED_SOURCES later)

dbus_util_sources = [
    'dbus-asv-util.c',
    'dbus-mainloop.c',
    'dbus-message-util.c',
    'dbus-pollable-set-poll.c',
    'dbus-pollable-set.c',
    'dbus-shell.c',
    'dbus-string-util.c',
    'dbus-sysdeps-util.c',
]

if platform_windows
    # On Windows, we use C++ constructors to initialize global locks
    assert(using_cpp)

    dbus_lib_sources += [
        'dbus-init-win.cpp',
        'dbus-server-win.c',
    ]

    dbus_lib_sources += windows.compile_resources(configure_file(
        input: 'versioninfo.rc.in',
        output: 'versioninfo.rc',
        configuration: data_config,
    ))

    dbus_shared_sources += [
        'dbus-backtrace-win.c',
        'dbus-file-win.c',
        'dbus-pipe-win.c',
        'dbus-sysdeps-thread-win.c',
        'dbus-sysdeps-win.c',
        'dbus-transport-win.c',
    ]

    if platform_win32ce
        dbus_shared_sources += 'dbus-sysdeps-wince-glue.c'
    endif

    dbus_util_sources += 'dbus-sysdeps-util-win.c'

    if use_traditional_activation
        dbus_util_sources += 'dbus-spawn-win.c'
    endif

else # Unix

    dbus_lib_sources += [
        'dbus-uuidgen.c',
        'dbus-server-unix.c',
    ]

    dbus_shared_sources += [
        'dbus-file-unix.c',
        'dbus-pipe-unix.c',
        'dbus-sysdeps-pthread.c',
        'dbus-sysdeps-unix.c',
        'dbus-transport-unix.c',
        'dbus-userdb.c',
    ]

    if use_launchd
        dbus_shared_sources += 'dbus-server-launchd.c'
    endif

    dbus_util_sources += [
        'dbus-sysdeps-util-unix.c',
        'dbus-userdb-util.c',
    ]

    if use_traditional_activation
        dbus_util_sources += 'dbus-spawn-unix.c'
    endif

endif


if use_linux_epoll
    dbus_util_sources += 'dbus-pollable-set-epoll.c'
endif

version_script = configure_file(
    input: 'Version.in',
    output: 'version_script',
    configuration: data_config,
)

# We can't use version_script to check for support, because it hasn't been
# generated yet, so use a static source file instead.
if cc.has_link_argument(
    '-Wl,--version-script,@0@'.format(
        meson.current_source_dir() / 'test-version-script'
    )
)
    version_flags = ['-Wl,--version-script,@0@'.format(version_script)]
else
    version_flags = []
endif

libdbus = library('dbus-1',
    dbus_lib_sources,
    dbus_shared_sources,

    include_directories: root_include,
    c_args: '-Ddbus_1_EXPORTS',

    link_args: version_flags,
    soversion: soversion,
    version: version_info,

    dependencies: dbus_dependencies,
    install: true,
)

libdbus_dep = declare_dependency(
    include_directories: root_include,
    dependencies: dbus_dependencies,
    link_with: libdbus,
)

libdbus_internal = static_library('dbus-internal',
    dbus_util_sources,

    include_directories: root_include,
    link_with: libdbus,
    dependencies: dbus_dependencies,
)


install_headers(
    'dbus-address.h',
    'dbus-bus.h',
    'dbus-connection.h',
    'dbus-errors.h',
    'dbus-macros.h',
    'dbus-memory.h',
    'dbus-message.h',
    'dbus-misc.h',
    'dbus-pending-call.h',
    'dbus-protocol.h',
    'dbus-server.h',
    'dbus-shared.h',
    'dbus-signature.h',
    'dbus-syntax.h',
    'dbus-threads.h',
    'dbus-types.h',
    'dbus.h',
    subdir: 'dbus-1.0' / 'dbus',
)

dbus_arch_deps_h = configure_file(
  input: 'dbus-arch-deps.h.in',
  output: 'dbus-arch-deps.h',
  configuration: arch_config,
)
install_data(dbus_arch_deps_h,
  install_dir: get_option('libdir') / 'dbus-1.0' / 'include' / 'dbus',
)
