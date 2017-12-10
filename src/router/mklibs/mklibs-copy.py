#!/usr/bin/python

# mklibs.py: An automated way to create a minimal /lib/ directory.
#
# Copyright 2001 by Falk Hueffner <falk@debian.org>
#                 & Goswin Brederlow <goswin.brederlow@student.uni-tuebingen.de>
#
# mklibs.sh by Marcus Brinkmann <Marcus.Brinkmann@ruhr-uni-bochum.de>
# used as template
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import re
import sys
import os
import glob
import logging
from stat import *
import subprocess

try:
    import mklibs
except ImportError:
    sys.path.append('/usr/lib/mklibs/python')

from mklibs.utils.main import MainBase

class Main(MainBase):
    pass

main = Main()

logger = logging.getLogger(__name__)

# return a list of lines of output of the command
def command(command, *args):
    logger.debug("calling %s %s", command, ' '.join(args))
    pipe = os.popen(command + ' ' + ' '.join(args), 'r')
    output = pipe.read().strip()
    status = pipe.close()
    if status is not None and os.WEXITSTATUS(status) != 0:
        print "Command failed with status", os.WEXITSTATUS(status),  ":", \
               command, ' '.join(args)
	print "With output:", output
        sys.exit(1)
    return output.split('\n')

# Filter a list according to a regexp containing a () group. Return
# a set.
def regexpfilter(list, regexp, groupnr = 1):
    pattern = re.compile(regexp)
    result = set()
    for x in list:
        match = pattern.match(x)
        if match:
            result.add(match.group(groupnr))

    return result

def elf_header(obj):
    if not os.access(obj, os.F_OK):
        raise "Cannot find lib: " + obj
    output = command("mklibs-readelf", "--print-elf-header", obj)
    s = [int(i) for i in output[0].split()]
    return {'class': s[0], 'data': s[1], 'machine': s[2], 'flags': s[3]}

# Return a set of rpath strings for the passed object
def rpath(obj):
    if not os.access(obj, os.F_OK):
        raise "Cannot find lib: " + obj
    output = command("mklibs-readelf", "-R", obj)
    return [root + "/" + x for x in output if x]

# Return a set of libraries the passed objects depend on.
def library_depends(obj):
    if not os.access(obj, os.F_OK):
        raise "Cannot find lib: " + obj
    return [x for x in command("mklibs-readelf", "-n", obj) if x]

# Return real target of a symlink
def resolve_link(file):
    logger.debug("resolving %s", file)
    while S_ISLNK(os.lstat(file)[ST_MODE]):
        new_file = os.readlink(file)
        if new_file[0] != "/":
            file = os.path.join(os.path.dirname(file), new_file)
        else:
            file = new_file
    logger.debug("resolved to %s", file)
    return file

# Find complete path of a library, by searching in lib_path
def find_lib(lib):
    for path in lib_path:
        if os.access(path + "/" + lib, os.F_OK):
            return path + "/" + lib

    return ""

def extract_soname(so_file):
    soname_data = command("mklibs-readelf", "-s", so_file)
    if soname_data:
        return soname_data.pop()
    return ""

def multiarch(paths):
    devnull = open('/dev/null', 'w')
    dpkg_architecture = subprocess.Popen(
        ['dpkg-architecture', '-qDEB_HOST_MULTIARCH'],
        stdout=subprocess.PIPE, stderr=devnull)
    devnull.close()
    deb_host_multiarch, _ = dpkg_architecture.communicate()
    if dpkg_architecture.returncode == 0:
        deb_host_multiarch = deb_host_multiarch.rstrip('\n')
        new_paths = []
        for path in paths:
            new_paths.append(
                path.replace('/lib', '/lib/%s' % deb_host_multiarch, 1))
            new_paths.append(path)
        return new_paths
    else:
        return paths

def version(vers):
    print "mklibs: version ",vers
    print ""

# Clean the environment
vers="0.12"
os.environ['LC_ALL'] = "C"

# some global variables
lib_rpath = []
lib_path = main.options.library_dir
dest_path = main.options.dest_dir
ldlib = main.options.ldlib
default_lib_path = multiarch(["/lib/", "/usr/lib/", "/usr/X11R6/lib/"])
target = ""
root = main.options.root
include_default_lib_path = not main.options.omit_library_dir
proglist = main.files
so_pattern = re.compile("((lib|ld).*)\.so(\..+)*")

if include_default_lib_path:
    lib_path.extend(default_lib_path)

objects = {}  # map from inode to filename
for prog in proglist:
    inode = os.stat(prog)[ST_INO]
    if objects.has_key(inode):
        logger.debug("%s is a hardlink to %s", prog, objects[inode])
    elif so_pattern.match(prog):
        logger.debug("%s is a library", prog)
    elif open(prog).read(4)[1:] == 'ELF':
        objects[inode] = prog
    else:
        logger.debug("%s is no ELF", prog)

if not ldlib:
    for obj in objects.values():
        output = command("mklibs-readelf", "-i", obj)
	for x in output:
            ldlib = x
	if ldlib:
	    break

if not ldlib:
    sys.exit("E: Dynamic linker not found, aborting.")

logger.info('Using %s as dynamic linker', ldlib)

# Check for rpaths
for obj in objects.values():
    rpath_val = rpath(obj)
    if rpath_val:
        if root:
            for rpath_elem in rpath_val:
                if not rpath_elem in lib_rpath:
                    logger.verbose('Adding rpath %s for %s', rpath_elem, obj)
                    lib_rpath.append(rpath_elem)
        else:
            logger.warning('%s may need rpath, but --root not specified', obj)

lib_path.extend(lib_rpath)

passnr = 1
available_libs = []
previous_pass_libraries = set()
while 1:
    logger.info("library reduction pass %d", passnr)

    passnr = passnr + 1
    # Gather all already reduced libraries and treat them as objects as well
    small_libs = []
    for lib in regexpfilter(os.listdir(dest_path), "(.*-so-stripped)$"):
        obj = dest_path + "/" + lib
        small_libs.append(obj)
        inode = os.stat(obj)[ST_INO]
        if objects.has_key(inode):
            logger.debug("%s is hardlink to %s", obj, objects[inode])
        else:
            objects[inode] = obj

    for obj in objects.values():
        small_libs.append(obj)

    logger.verbose('Objects: %r', ' '.join([i[i.rfind('/') + 1:] for i in objects.itervalues()]))

    libraries = set()
    for obj in objects.values():
        libraries.update(library_depends(obj))

    if libraries == previous_pass_libraries:
        # No progress in last pass.
        break

    previous_pass_libraries = libraries

    # WORKAROUND: Always add libgcc on old-abi arm
    header = elf_header(find_lib(libraries.copy().pop()))
    if header['machine'] == 40 and header['flags'] & 0xff000000 == 0:
        libraries.add('libgcc_s.so.1')  

    # reduce libraries
    for library in libraries:
        so_file = find_lib(library)
        if root and (re.compile("^" + root).search(so_file)):
            logger.verbose("no action required for " + so_file)
            if not so_file in available_libs:
                logger.verbose("adding %s to available libs", so_file)
                available_libs.append(so_file)
            continue
        so_file_name = os.path.basename(so_file)
        if not so_file:
            sys.exit("File not found:" + library)
        command("cp", so_file, dest_path + "/" + so_file_name + "-so-stripped")

# Finalising libs and cleaning up
for lib in regexpfilter(os.listdir(dest_path), "(.*)-so-stripped$"):
    os.rename(dest_path + "/" + lib + "-so-stripped", dest_path + "/" + lib)
for lib in regexpfilter(os.listdir(dest_path), "(.*-so)$"):
    os.remove(dest_path + "/" + lib)

# Make sure the dynamic linker is present and is executable
ld_file = find_lib(ldlib)
ld_file_name = os.path.basename(ld_file)

if not os.access(dest_path + "/" + ld_file_name, os.F_OK):
    logger.info("stripping and copying dynamic linker.")
    command(target + "objcopy", "--strip-unneeded -R .note -R .comment",
            ld_file, dest_path + "/" + ld_file_name)

os.chmod(dest_path + "/" + ld_file_name, 0755)
