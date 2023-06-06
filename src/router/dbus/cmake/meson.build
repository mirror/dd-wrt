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

cmake_config = configuration_data()
cmake_config.set('CMAKE_INSTALL_INCLUDEDIR',get_option('prefix') / get_option('includedir'))
cmake_config.set('CMAKE_INSTALL_LIBDIR',    get_option('prefix') / get_option('libdir'))
cmake_config.set('DBUS_PREFIX',             get_option('prefix'))
cmake_config.set('DBUS_RELOCATABLE',        relocation)
cmake_config.set('DBUS_VERSION',            version)

cmake_files = [
    configure_file(
        input: 'DBus1Config.pkgconfig.in',
        output: 'DBus1Config.cmake',
        configuration: cmake_config,
    ),
    configure_file(
        input: 'DBus1ConfigVersion.cmake.in',
        output: 'DBus1ConfigVersion.cmake',
        configuration: cmake_config,
    )
]

install_data(cmake_files,
    install_dir: get_option('libdir') / 'cmake' / 'DBus1',
)
