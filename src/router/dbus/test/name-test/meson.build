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


if intrusive_tests

    tests = [
        {'name': 'test-ids'},
        {'name': 'test-pending-call-disconnected'},
        {'name': 'test-shutdown'},
    ]

    if platform_windows
        autolaunch_name = 'test-autolaunch-win'
    else
        autolaunch_name = 'test-autolaunch'
    endif
    autolaunch = executable(autolaunch_name,
        autolaunch_name + '.c',
        include_directories: root_include,
        link_with: [libdbus, libdbus_internal, libdbus_testutils],
        dependencies: dbus_dependencies
    )

    if use_traditional_activation
        tests += [
            {'name': 'test-pending-call-dispatch', 'test_deps': [test_service]},
            {'name': 'test-pending-call-timeout', 'test_deps': [test_service]},
            {
                'name': 'test-privserver-client',
                'test_deps': [test_privserver, test_service],
            },
            {'name': 'test-threads-init', 'test_deps': [test_service]},
        ]
    endif

    foreach test: tests
        test_exe = executable(test['name'],
            test['name'] + '.c',
            include_directories: root_include,
            link_with: [
                libdbus,
                libdbus_internal,
                libdbus_testutils,
            ],
            dependencies: dbus_dependencies,
        )

        if test['name'] == 'test-autolaunch'
            # This one is run from run-test.sh, not directly
            continue
        endif

        if test['name'] == 'test-pending-call-timeout'
            timeout = 75
            suites = ['name-test', 'slow']
        else
            timeout = 30
            suites = ['name-test']
        endif

        test(test['name'],
            dbus_run_session,
            args: [
                '--config-file=@0@'.format(
                    meson.project_build_root()/'test/data/valid-config-files/tmp-session.conf'),
                '--dbus-daemon=@0@'.format(dbus_daemon.full_path()),
                '--',
                test_exe,
            ],
            depends: test.get('test_deps', []),
            env: test_env,
            protocol: test_protocol,
            suite: suites,
            timeout: timeout,
        )
    endforeach


    if platform_unix
        test('run-test',
            find_program('run-test.sh'),
            depends: [dbus_launch, autolaunch],
            env: test_env,
            protocol: test_protocol,
            suite: 'name-test',
        )

        test('run-test-systemserver',
            find_program('run-test-systemserver.sh'),
            depends: [dbus_send, test_service],
            env: test_env,
            protocol: test_protocol,
            suite: 'name-test',
        )
    endif
endif
