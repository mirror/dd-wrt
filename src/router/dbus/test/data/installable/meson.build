# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
# Copyright 2022 Collabora Ltd.
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

# Some configuration parameters used in installed-tests are not the same
# as the configuration parameters used during build-time testing.
installed_tests_config = configuration_data()
installed_tests_config.merge_from(data_config)
installed_tests_config.set('DBUS_TEST_DATA', get_option('prefix') / test_exec_dir / 'data')
installed_tests_config.set('DBUS_TEST_EXEC', get_option('prefix') / test_exec_dir)
installed_tests_config.set('EXEEXT', exe_ext)
installed_tests_config.set('TEST_LAUNCH_HELPER_BINARY', '/bin/false')

foreach file : data_in_to_install
    # Underscorify the output name because Meson doesn't allow subdir output files
    configured_file = configure_file(
        input : files('../' + file + '.in'),
        output: file.underscorify(),
        configuration: installed_tests_config,
    )
    if install_tests
        install_data(configured_file,
            rename: file,
            install_dir: test_exec_dir / 'data',
        )
    endif
endforeach
