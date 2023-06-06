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


if embedded_tests

    tests = [
        'test-ids',
        'test-pending-call-disconnected',
        'test-shutdown',
    ]

    if platform_windows
        tests += ['test-autolaunch-win']
    else
        tests += ['test-autolaunch']
    endif

    if use_traditional_activation
        tests += [
            'test-pending-call-dispatch',
            'test-pending-call-timeout',
            'test-privserver-client',
            'test-threads-init',
        ]
    endif

    foreach test: tests
        test_exe = executable(test,
            test + '.c',
            include_directories: root_include,
            link_with: [
                libdbus,
                libdbus_internal,
                libdbus_testutils,
            ],
            dependencies: dbus_dependencies,
        )

        if test == 'test-autolaunch'
            # This one is run from run-test.sh, not directly
            continue
        endif

        if test == 'test-pending-call-timeout'
            timeout = 75
            suites = ['name-test', 'slow']
        else
            timeout = 30
            suites = ['name-test']
        endif

        test(test,
            dbus_run_session,
            args: [
                '--config-file=@0@'.format(
                    meson.project_build_root()/'test/data/valid-config-files/tmp-session.conf'),
                '--dbus-daemon=@0@'.format(dbus_daemon.full_path()),
                '--',
                test_exe,
            ],
            env: test_env,
            protocol: test_protocol,
            suite: suites,
            timeout: timeout,
        )
    endforeach


    if platform_unix
        test('run-test',
            find_program('run-test.sh'),
            env: test_env,
            protocol: test_protocol,
            suite: 'name-test',
        )

        test('run-test-systemserver',
            find_program('run-test-systemserver.sh'),
            env: test_env,
            protocol: test_protocol,
            suite: 'name-test',
        )
    endif
endif
