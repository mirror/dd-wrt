#!/usr/bin/env python3
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

import os, sys, stat
from pathlib import Path
import shlex, subprocess, json

meson = shlex.split(os.environ.get('MESONINTROSPECT', ''))
introspection = json.loads(subprocess.check_output(meson + ['-a']).decode())
build_options = introspection['buildoptions']
targets = introspection['targets']

def get_option(name):
    for i in build_options:
        if i['name'] == name:
            return i['value']
    return None

def get_target(name):
    for i in targets:
        if i['name'] == name:
            return i
    return None

destdir = Path(os.getenv('DESTDIR')) if 'DESTDIR' in os.environ else None
prefix = Path(get_option('prefix'))
destdir_prefix = Path(os.getenv('MESON_INSTALL_DESTDIR_PREFIX'))

def to_destdir(path):
    path_abs = prefix / path
    if destdir:
        path_rel_root = path_abs.relative_to(path_abs.anchor)
        path_final = destdir / path_rel_root
        return path_final
    else:
        return path_abs

###############################################################################

# Define paths here
abs_libexecdir = destdir_prefix / get_option('libexecdir')

relocation = sys.argv[1].lower() == 'true'

def post_install_relocation():
    # Edit pkg-config file to replace the prefix
    #
    # TODO: Meson >=0.63 has a new feature, -Dpkgconfig.relocatable=true.

    if not relocation:
        return

    pc_filepath = next(
        v for (k,v) in introspection['installed'].items() if k.endswith('.pc')
    )
    # Find the really installed path
    pc_filepath = to_destdir(pc_filepath)

    with open(pc_filepath, 'r') as pcfile:
        lines = pcfile.readlines()
    with open(pc_filepath, 'w') as pcfile:
        for line in lines:
            if line.startswith('prefix='):
                line = 'prefix=${pcfiledir}/../..\n'
            pcfile.write(line)

def post_install_exe():
    # Setuid, chmod and chown for dbus-daemon-launch-helper
    daemon_launch_helper = get_target('dbus-daemon-launch-helper')
    if daemon_launch_helper:
        import grp
        exe_name = os.path.basename(daemon_launch_helper['install_filename'][0])
        exe_path = abs_libexecdir / exe_name
        dbus_user = get_option('dbus_user')
        if os.getuid() == 0:
            os.chown(exe_path, 0, grp.getgrnam(dbus_user).gr_gid)
            os.chmod(exe_path, stat.S_ISUID | stat.S_IXUSR | stat.S_IXGRP)
        else:
            print('Not installing {0} binary setuid!'.format(exe_path))
            print('You\'ll need to manually set permissions to root:{0} and permissions 4750'
                .format(dbus_user)
            )


if __name__ == "__main__":
    post_install_relocation()
    post_install_exe()
