#!/usr/bin/env python3
# compile_commands.json generator
# Copyright (C) 2024 Intel Corporation
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import os
import sys
import glob
import json
import argparse

parser = argparse.ArgumentParser(description='Read each of the build directories that are given and generate '
                                 'a compile_commands.json file. If a source file is found multiple times, '
                                 'then the compile flags from the last build directory will be used.')
parser.add_argument('-o', '--output', default='compile_commands.json', type=str,
                    help="Output file to generate")
parser.add_argument('builddirs', nargs='+', type=str, metavar="builddir",
                    help='Build directories to search')

args = parser.parse_args()

files = {}

for builddir in args.builddirs:
    for cmd_file in glob.glob('**/*.o.cmd', root_dir=builddir, recursive=True):
        with open(os.path.join(builddir, cmd_file), encoding='ascii') as f:
            base_dir, cmd = f.readline().split(':', 1)
            src_file = cmd.rsplit(maxsplit=1)[1]

        src_file = os.path.abspath(os.path.join(base_dir, src_file))
        files[src_file] = {
            'command': cmd.strip(),
            'directory': base_dir,
            'file': src_file,
        }

flist = json.dumps(sorted(list(files.values()), key=lambda k: k['file']), indent=2, sort_keys=True)

try:
   # Avoid writing the file if it did not change, first read original
   with open(args.output, 'rt', encoding='UTF-8') as f:
       orig = []
       while data := f.read():
           orig.append(data)
       orig = ''.join(orig)
except OSError:
    orig = ''

# And only write if something changed
if orig != flist:
    with open(args.output, 'wt', encoding='UTF-8') as f:
        f.write(flist)
