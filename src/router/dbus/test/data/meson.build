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

data_to_install = [
    'auth/anonymous-client-successful.auth-script',
    'auth/anonymous-server-successful.auth-script',
    'auth/cancel.auth-script',
    'auth/client-out-of-mechanisms.auth-script',
    'auth/cookie-sha1-username.auth-script',
    'auth/cookie-sha1.auth-script',
    'auth/external-auto.auth-script',
    'auth/external-failed.auth-script',
    'auth/external-root.auth-script',
    'auth/external-silly.auth-script',
    'auth/external-successful.auth-script',
    'auth/external-username.auth-script',
    'auth/extra-bytes.auth-script',
    'auth/fail-after-n-attempts.auth-script',
    'auth/fallback.auth-script',
    'auth/invalid-command-client.auth-script',
    'auth/invalid-command.auth-script',
    'auth/invalid-hex-encoding.auth-script',
    'auth/mechanisms.auth-script',
    'equiv-config-files/basic/basic-1.conf',
    'equiv-config-files/basic/basic-2.conf',
    'equiv-config-files/basic/basic.d/basic.conf',
    'equiv-config-files/entities/basic.d/basic.conf',
    'equiv-config-files/entities/entities-1.conf',
    'equiv-config-files/entities/entities-2.conf',
    'invalid-config-files/apparmor-bad-attribute.conf',
    'invalid-config-files/apparmor-bad-mode.conf',
    'invalid-config-files/bad-attribute-2.conf',
    'invalid-config-files/bad-attribute.conf',
    'invalid-config-files/bad-element.conf',
    'invalid-config-files/bad-limit.conf',
    'invalid-config-files/badselinux-1.conf',
    'invalid-config-files/badselinux-2.conf',
    'invalid-config-files/circular-1.conf',
    'invalid-config-files/circular-2.conf',
    'invalid-config-files/circular-3.conf',
    'invalid-config-files/double-attribute.conf',
    'invalid-config-files/impossible-send.conf',
    'invalid-config-files/limit-no-name.conf',
    'invalid-config-files/ludicrous-limit.conf',
    'invalid-config-files/negative-limit.conf',
    'invalid-config-files/non-numeric-limit.conf',
    'invalid-config-files/not-well-formed.conf',
    'invalid-config-files/policy-bad-at-console.conf',
    'invalid-config-files/policy-bad-attribute.conf',
    'invalid-config-files/policy-bad-context.conf',
    'invalid-config-files/policy-bad-rule-attribute.conf',
    'invalid-config-files/policy-contradiction.conf',
    'invalid-config-files/policy-member-no-path.conf',
    'invalid-config-files/policy-mixed.conf',
    'invalid-config-files/policy-no-attributes.conf',
    'invalid-config-files/policy-no-rule-attribute.conf',
    'invalid-config-files/send-and-receive.conf',
    'invalid-config-files/truncated-file.conf',
    'invalid-config-files/unknown-limit.conf',
    'invalid-messages/boolean-has-no-value.message-raw',
    'invalid-messages/fixed-array-not-divisible.message-raw',
    'invalid-messages/fixed-array-not-divisible.message-raw.hex',
    'invalid-messages/issue418.message-raw',
    'invalid-messages/issue418.message-raw.hex',
    'invalid-messages/mis-nested-sig.message-raw',
    'invalid-messages/mis-nested-sig.message-raw.hex',
    'invalid-messages/truncated-variant-sig.message-raw',
    'invalid-messages/truncated-variant-sig.message-raw.hex',
    'invalid-messages/zero-length-variant-sig.message-raw',
    'invalid-messages/zero-length-variant-sig.message-raw.hex',
    'sha-1/bit-hashes.sha1',
    'sha-1/bit-messages.sha1',
    'sha-1/byte-hashes.sha1',
    'sha-1/byte-messages.sha1',
    'sha-1/Readme.txt',
    'systemd-activation/com.example.ReceiveDenied.service',
    'systemd-activation/com.example.SendDenied.service',
    'systemd-activation/com.example.SendDeniedByAppArmorName.service',
    'systemd-activation/com.example.SendPrefixDenied.internal.service',
    'systemd-activation/com.example.SendPrefixDenied.SendPrefixAllowed.internal.service',
    'systemd-activation/com.example.SendPrefixDenied.service',
    'systemd-activation/com.example.SystemdActivatable1.service',
    'systemd-activation/com.example.SystemdActivatable2.service',
    'systemd-activation/org.freedesktop.systemd1.service',
    'valid-config-files-system/many-rules.conf',
    'valid-config-files-system/system.d/test.conf',
    'valid-config-files/basic.conf',
    'valid-config-files/basic.d/basic.conf',
    'valid-config-files/check-own-rules.conf',
    'valid-config-files/entities.conf',
    'valid-config-files/listen-unix-runtime.conf',
    'valid-config-files/many-rules.conf',
    'valid-config-files/minimal.conf',
    'valid-config-files/standard-session-dirs.conf',
    'valid-messages/byteswap-fd-index.message-raw',
    'valid-messages/byteswap-fd-index.message-raw.hex',
    'valid-messages/minimal.message-raw',
    'valid-messages/minimal.message-raw.hex',
]

data_in_to_install = [
    'dbus-installed-tests.aaprofile',
    'invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoExec.service',
    'invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoService.service',
    'invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoUser.service',
    'systemd-activation/com.example.ReceiveDeniedByAppArmorLabel.service',
    'systemd-activation/com.example.SendDeniedByAppArmorLabel.service',
    'systemd-activation/com.example.SendDeniedByNonexistentAppArmorLabel.service',
    'systemd-activation/com.example.SystemdActivatable3.service',
    'valid-config-files-system/debug-allow-all-fail.conf',
    'valid-config-files-system/debug-allow-all-pass.conf',
    'valid-config-files-system/tmp-session-like-system.conf',
    'valid-config-files/as-another-user.conf',
    'valid-config-files/count-fds.conf',
    'valid-config-files/debug-allow-all-sha1.conf',
    'valid-config-files/debug-allow-all.conf',
    'valid-config-files/finite-timeout.conf',
    'valid-config-files/forbidding.conf',
    'valid-config-files/incoming-limit.conf',
    'valid-config-files/limit-containers.conf',
    'valid-config-files/listen-autolaunch-win.conf',
    'valid-config-files/max-completed-connections.conf',
    'valid-config-files/max-connections-per-user.conf',
    'valid-config-files/max-containers.conf',
    'valid-config-files/max-match-rules-per-connection.conf',
    'valid-config-files/max-names-per-connection.conf',
    'valid-config-files/max-replies-per-connection.conf',
    'valid-config-files/multi-user.conf',
    'valid-config-files/pending-fd-timeout.conf',
    'valid-config-files/send-destination-prefix-rules.conf',
    'valid-config-files/systemd-activation.conf',
    'valid-config-files/tmp-session.conf',
    'valid-service-files-system/org.freedesktop.DBus.TestSuiteEchoService.service',
    'valid-service-files-system/org.freedesktop.DBus.TestSuiteSegfaultService.service',
    'valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service',
    'valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service',
    'valid-service-files/org.freedesktop.DBus.TestSuite.PrivServer.service',
    'valid-service-files/org.freedesktop.DBus.TestSuiteEchoService.service',
    'valid-service-files/org.freedesktop.DBus.TestSuiteForkingEchoService.service',
    'valid-service-files/org.freedesktop.DBus.TestSuiteSegfaultService.service',
    'valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service',
    'valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service',
]

foreach file : data_to_install
    if install_tests
        install_data(file,
            rename: file,
            install_dir: test_exec_dir / 'data',
        )
    endif
endforeach

foreach file : data_in_to_install
    # Underscorify the output name because Meson doesn't allow subdir output files
    configured_file = configure_file(
        input : file + '.in',
        output: file.underscorify(),
        configuration: test_data_config,
    )
    # We don't install this version: see test/data/installable/ for the
    # version that we *do* install.
endforeach

###############################################################################
# Copy files into correct places in build directory for tests

files = []
foreach file : data_to_install
    src = meson.current_source_dir() / file
    dst = meson.current_build_dir() / file
    files += src
    files += dst
endforeach

foreach file : data_in_to_install
    src = meson.current_build_dir() / file.underscorify()
    dst = meson.current_build_dir() / file
    files += src
    files += dst
endforeach

if message_bus
    files += meson.project_build_root() / 'bus' / 'session.conf'
    files += meson.current_build_dir() / 'valid-config-files/session.conf'

    if platform_unix
        files += meson.project_build_root() / 'bus' / 'system.conf'
        files += meson.current_build_dir() / 'valid-config-files-system/system.conf'
    endif
endif

run_result = run_command(find_program('copy_data_for_tests.py'), files, check: true)

files_not_found = run_result.stdout().split()
if files_not_found.length() > 0
    error('Those files could not be copied for test : @0@'.format(files_not_found))
endif

if install_tests
    subdir('installable')

    if platform_unix
        install_symlinks += [
            {
                'link_name': 'session.conf',
                'install_dir': test_exec_dir / 'data' / 'valid-config-files',
                'pointing_to':
                    get_option('prefix') / get_option('datadir') /
                    'dbus-1' / 'session.conf',
            },
            {
                'link_name': 'system.conf',
                'install_dir': test_exec_dir / 'data' / 'valid-config-files-system',
                'pointing_to':
                    get_option('prefix') / get_option('datadir') /
                    'dbus-1' / 'system.conf',
            },
        ]
    endif
endif
