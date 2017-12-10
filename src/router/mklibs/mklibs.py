#! /usr/bin/env python

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

# HOW IT WORKS
#
# - Gather all unresolved symbols and libraries needed by the programs
#   and reduced libraries
# - Gather all symbols provided by the already reduced libraries
#   (none on the first pass)
# - If all symbols are provided we are done
# - go through all libraries and remember what symbols they provide
# - go through all unresolved/needed symbols and mark them as used
# - for each library:
#   - find pic file (if not present copy and strip the so)
#   - compile in only used symbols
#   - strip
# - back to the top

# TODO
# * complete argument parsing as given as comment in main

import string
import re
import sys
import os
import glob
import getopt
from stat import *
import subprocess

DEBUG_NORMAL  = 1
DEBUG_VERBOSE = 2
DEBUG_SPAM    = 3

debuglevel = DEBUG_NORMAL

def debug(level, *msg):
    if debuglevel >= level:
        print string.join(msg)

# return a list of lines of output of the command
def command(command, *args):
    debug(DEBUG_SPAM, "calling", command, string.join(args))
    pipe = os.popen(command + ' ' + ' '.join(args), 'r')
    output = pipe.read().strip()
    status = pipe.close() 
    if status is not None and os.WEXITSTATUS(status) != 0:
        print "Command failed with status", os.WEXITSTATUS(status),  ":", \
               command, string.join(args)
	print "With output:", output
        sys.exit(1)
    return [i for i in output.split('\n') if i]

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
        raise Exception("Cannot find lib: " + obj)
    output = command("mklibs-readelf", "--print-elf-header", obj)
    s = [int(i) for i in output[0].split()]
    return {'class': s[0], 'data': s[1], 'machine': s[2], 'flags': s[3]}

# Return a set of rpath strings for the passed object
def rpath(obj):
    if not os.access(obj, os.F_OK):
        raise Exception("Cannot find lib: " + obj)
    output = command("mklibs-readelf", "--print-rpath", obj)
    return [root + "/" + x for x in output]

# Return a set of libraries the passed objects depend on.
def library_depends(obj):
    if not os.access(obj, os.F_OK):
        raise Exception("Cannot find lib: " + obj)
    return command("mklibs-readelf", "--print-needed", obj)

# Return a list of libraries the passed objects depend on. The
# libraries are in "-lfoo" format suitable for passing to gcc.
def library_depends_gcc_libnames(obj, soname):
    if not os.access(obj, os.F_OK):
        raise Exception("Cannot find lib: " + obj)
    libs = library_depends(obj)
    ret = []
    for i in libs:
        match = re.match("^(((?P<ld>ld\S*)|(lib(?P<lib>\S+))))\.so.*$", i)
        if match:
            ret.append(find_lib(match.group(0)))
    return ' '.join(ret)

class Symbol(object):
    def __init__(self, name, version, library):
        self.name, self.version, self.library = name, version, library

    def __str__(self):
        ret = [self.name]
        if self.version:
            ret.append(self.version)
            if self.library:
                ret.append(self.library)
        return '@'.join(ret)

class UndefinedSymbol(Symbol):
    def __init__(self, name, weak, version, library):
        super(UndefinedSymbol, self).__init__(name, version, library)
        self.weak, self.library = weak, library

# Return undefined symbols in an object as a set of tuples (name, weakness)
def undefined_symbols(obj):
    if not os.access(obj, os.F_OK):
        raise Exception("Cannot find lib" + obj)

    output = command("mklibs-readelf", "--print-symbols-undefined", obj)

    result = []
    for line in output:
        name, weak_string, version_string, library_string = line.split()[:4]

        weak = False
        if weak_string.lower() == 'true':
            weak = True

        version = None
        if version_string.lower() not in ('base', 'none'):
            version = version_string

        library = None
        if library_string.lower() != 'none':
            library = library_string

        result.append(UndefinedSymbol(name, weak, version, library))

    return result

class ProvidedSymbol(Symbol):
    def __init__(self, name, version, library, default_version, weak):
        super(ProvidedSymbol, self).__init__(name, version, library)
        self.default_version = default_version
        self.weak = weak

    def base_names(self):
        ret = []

        if self.version:
            if self.library:
                ret.append('@'.join((self.name, self.version, self.library)))

            ret.append('@'.join((self.name, self.version)))

            if self.default_version:
                ret.append(self.name)

        else:
            ret.append(self.name)

        return ret

    def linker_name(self):
        if self.default_version or not self.version:
            return self.name

        return '@'.join((self.name, self.version))

# Return a set of symbols provided by a library
def provided_symbols(obj):
    if not os.access(obj, os.F_OK):
        raise Exception("Cannot find lib" + obj)
    library = extract_soname(obj)

    output = command("mklibs-readelf", "--print-symbols-provided", obj)

    result = []
    for line in output:
        name, weak_string, version_string, default_version_string = line.split()[:4]

        version = None
        if version_string.lower() not in ('base', 'none'):
            version = version_string

        weak = False
        if weak_string.lower() == 'true':
            weak = True

        default_version = False
        if default_version_string.lower() == 'true':
            default_version = True

        result.append(ProvidedSymbol(name, version, library, default_version, weak))

    return result
    
# Return real target of a symlink
def resolve_link(file):
    debug(DEBUG_SPAM, "resolving", file)
    while S_ISLNK(os.lstat(file)[ST_MODE]):
        new_file = os.readlink(file)
        if new_file[0] != "/":
            file = os.path.join(os.path.dirname(file), new_file)
        else:
            file = new_file
    debug(DEBUG_SPAM, "resolved to", file)
    return file

# Find complete path of a library, by searching in lib_path
def find_lib(lib):
    for path in lib_path:
        if os.access(sysroot + path + "/" + lib, os.F_OK):
            return sysroot + path + "/" + lib

    return ""

# Find a PIC archive for the library
def find_pic(lib):
    base_name = so_pattern.match(lib).group(1)
    for path in lib_path:
        for file in glob.glob(sysroot + path + "/" + base_name + "_pic.a"):
            if os.access(file, os.F_OK):
                return resolve_link(file)
    return ""

# Find a PIC .map file for the library
def find_pic_map(lib):
    base_name = so_pattern.match(lib).group(1)
    for path in lib_path:
        for file in glob.glob(sysroot + path + "/" + base_name + "_pic.map"):
            if os.access(file, os.F_OK):
                return resolve_link(file)
    return ""

def extract_soname(so_file):
    soname_data = command("mklibs-readelf", "--print-soname", so_file)
    if soname_data:
        return soname_data.pop()
    return ""

def multiarch(paths):
    return paths
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

def usage(was_err):
    if was_err:
        outfd = sys.stderr
    else:
        outfd = sys.stdout
    print >> outfd, "Usage: mklibs [OPTION]... -d DEST FILE ..."
    print >> outfd, "Make a set of minimal libraries for FILE(s) in DEST."
    print >> outfd, "" 
    print >> outfd, "  -d, --dest-dir DIRECTORY     create libraries in DIRECTORY"
    print >> outfd, "  -D, --no-default-lib         omit default libpath (", ':'.join(default_lib_path), ")"
    print >> outfd, "  -L DIRECTORY[:DIRECTORY]...  add DIRECTORY(s) to the library search path"
    print >> outfd, "  -l LIBRARY                   add LIBRARY always"
    print >> outfd, "      --ldlib LDLIB            use LDLIB for the dynamic linker"
    print >> outfd, "      --libc-extras-dir DIRECTORY  look for libc extra files in DIRECTORY"
    print >> outfd, "      --target TARGET          prepend TARGET- to the gcc and binutils calls"
    print >> outfd, "      --root ROOT              search in ROOT for library rpaths"
    print >> outfd, "      --sysroot ROOT           prepend ROOT to all paths for libraries"
    print >> outfd, "      --gcc-options OPTIONS    pass OPTIONS to gcc"
    print >> outfd, "      --libdir DIR             use DIR (e.g. lib64) in place of lib in default paths"
    print >> outfd, "  -v, --verbose                explain what is being done"
    print >> outfd, "  -h, --help                   display this help and exit"
    sys.exit(was_err)

def version(vers):
    print "mklibs: version ",vers
    print ""

#################### main ####################
## Usage: ./mklibs.py [OPTION]... -d DEST FILE ...
## Make a set of minimal libraries for FILE ... in directory DEST.
## 
## Options:
##   -L DIRECTORY               Add DIRECTORY to library search path.
##   -D, --no-default-lib       Do not use default lib directories of /lib:/usr/lib
##   -n, --dry-run              Don't actually run any commands; just print them.
##   -v, --verbose              Print additional progress information.
##   -V, --version              Print the version number and exit.
##   -h, --help                 Print this help and exit.
##   --ldlib                    Name of dynamic linker (overwrites environment variable ldlib)
##   --libc-extras-dir          Directory for libc extra files
##   --target                   Use as prefix for gcc or binutils calls
## 
##   -d, --dest-dir DIRECTORY   Create libraries in DIRECTORY.
## 
## Required arguments for long options are also mandatory for the short options.

# Clean the environment
vers="0.12"
os.environ['LC_ALL'] = "C"

# Argument parsing
opts = "L:DnvVhd:r:l:"
longopts = ["no-default-lib", "dry-run", "verbose", "version", "help",
            "dest-dir=", "ldlib=", "libc-extras-dir=", "target=", "root=",
            "sysroot=", "gcc-options=", "libdir="]

# some global variables
lib_rpath = []
lib_path = []
dest_path = "DEST"
ldlib = "LDLIB"
include_default_lib_path = "yes"
default_lib_path = multiarch(["/lib/", "/usr/lib/", "/usr/X11R6/lib/"])
libc_extras_dir = "/usr/lib/libc_pic"
libc_extras_dir_default = True
libdir = "lib"
target = ""
root = ""
sysroot = ""
force_libs = []
gcc_options = []
so_pattern = re.compile("((lib|ld).*)\.so(\..+)*")
script_pattern = re.compile("^#!\s*/")

try:
    optlist, proglist = getopt.getopt(sys.argv[1:], opts, longopts)
except getopt.GetoptError, msg:
    print >> sys.stderr, msg
    usage(1)

for opt, arg in optlist:
    if opt in ("-v", "--verbose"):
        if debuglevel < DEBUG_SPAM:
            debuglevel = debuglevel + 1
    elif opt == "-L":
        lib_path.extend(string.split(arg, ":"))
    elif opt in ("-d", "--dest-dir"):
        dest_path = arg
    elif opt in ("-D", "--no-default-lib"):
        include_default_lib_path = "no"
    elif opt == "--ldlib":
        ldlib = arg
    elif opt == "--libc-extras-dir":
        libc_extras_dir = arg
        libc_extras_dir_default = False
    elif opt == "--target":
        target = arg + "-"
    elif opt in ("-r", "--root"):
        root = arg
    elif opt == "--sysroot":
        sysroot = arg
    elif opt in ("-l",):
        force_libs.append(arg)
    elif opt == "--gcc-options":
        gcc_options.extend(string.split(arg, " "))
    elif opt == "--libdir":
        libdir = arg
    elif opt in ("--help", "-h"):
	usage(0)
        sys.exit(0)
    elif opt in ("--version", "-V"):
        version(vers)
        sys.exit(0)
    else:
        print "WARNING: unknown option: " + opt + "\targ: " + arg

if include_default_lib_path == "yes":
    lib_path.extend([a.replace("/lib/", "/" + libdir + "/") for a in default_lib_path])

if libc_extras_dir_default:
    libc_extras_dir = libc_extras_dir.replace("/lib/", "/" + libdir + "/")
    for path in multiarch([libc_extras_dir]):
        if os.path.isdir(path):
            libc_extras_dir = path
            break

if ldlib == "LDLIB":
    ldlib = os.getenv("ldlib")

objects = {}  # map from inode to filename
for prog in proglist:
    inode = os.stat(prog)[ST_INO]
    if objects.has_key(inode):
        debug(DEBUG_SPAM, prog, "is a hardlink to", objects[inode])
    elif so_pattern.match(prog):
        debug(DEBUG_SPAM, prog, "is a library")
    elif script_pattern.match(open(prog).read(256)):
        debug(DEBUG_SPAM, prog, "is a script")
    else:
        objects[inode] = prog

if not ldlib:
    for obj in objects.values():
        output = command("mklibs-readelf", "--print-interp", obj)
        if output:
            ldlib = output.pop()
	if ldlib:
	    break

if not ldlib:
    sys.exit("E: Dynamic linker not found, aborting.")

ldlib = sysroot + ldlib

debug(DEBUG_NORMAL, "I: Using", ldlib, "as dynamic linker.")

# Check for rpaths
for obj in objects.values():
    rpath_val = rpath(obj)
    if rpath_val:
        if root:
            for rpath_elem in rpath_val:
                if not rpath_elem in lib_rpath:
                    if debuglevel >= DEBUG_VERBOSE:
                        print "Adding rpath " + rpath_elem + " for " + obj
                    lib_rpath.append(rpath_elem)
        else:
            print "warning: " + obj + " may need rpath, but --root not specified"

lib_path.extend(lib_rpath)

passnr = 1
available_libs = []
previous_pass_unresolved = set()
while 1:
    debug(DEBUG_NORMAL, "I: library reduction pass", `passnr`)
    if debuglevel >= DEBUG_VERBOSE:
        print "Objects:",
        for obj in objects.values():
            print obj[string.rfind(obj, '/') + 1:],
        print

    passnr = passnr + 1
    # Gather all already reduced libraries and treat them as objects as well
    small_libs = []
    for lib in regexpfilter(os.listdir(dest_path), "(.*-so)$"):
        obj = dest_path + "/" + lib
        small_libs.append(obj)
        inode = os.stat(obj)[ST_INO]
        if objects.has_key(inode):
            debug(DEBUG_SPAM, obj, "is hardlink to", objects[inode])
        else:
            objects[inode] = obj

    # DEBUG
    for obj in objects.values():
        small_libs.append(obj)
        debug(DEBUG_VERBOSE, "Object:", obj)

    # calculate what symbols and libraries are needed
    needed_symbols = {}
    libraries = set(force_libs)
    for obj in objects.values():
        for symbol in undefined_symbols(obj):
            # Some undefined symbols in libthread_db are defined in
            # the application that uses it.  __gnu_local_gp is defined
            # specially by the linker on MIPS.
            if (not (re.search("libthread_db\.so", obj)
                     and re.search("^ps_", str(symbol)))
                and not (re.search("ld-linux.so.3$", str(symbol)))
                and not (re.search("^__gnu_local_gp", str(symbol)))):
                debug(DEBUG_SPAM, "needed_symbols adding %s, weak: %s" % (symbol, symbol.weak))
                needed_symbols[str(symbol)] = symbol
        libraries.update(library_depends(obj))

    # calculate what symbols are present in small_libs and available_libs
    present_symbols = {}
    checked_libs = small_libs
    checked_libs.extend(available_libs)
    checked_libs.append(ldlib)
    for lib in checked_libs:
        for symbol in provided_symbols(lib):
            debug(DEBUG_SPAM, "present_symbols adding %s" % symbol)
            names = symbol.base_names()
            for name in names:
                if name in present_symbols:
                    if symbol.library != present_symbols[name].library:
                        needed_symbols[name] = UndefinedSymbol(name, True, symbol.version, symbol.library)
                present_symbols[name] = symbol

    # are we finished?
    num_unresolved = 0
    unresolved = set()
    for name in needed_symbols:
        if not name in present_symbols:
            debug(DEBUG_SPAM, "Still need: %s" % name)
            unresolved.add(name)
            num_unresolved = num_unresolved + 1

    debug (DEBUG_NORMAL, `len(needed_symbols)`, "symbols,",
           `num_unresolved`, "unresolved")

    if num_unresolved == 0:
        break

    if unresolved == previous_pass_unresolved:
        # No progress in last pass. Verify all remaining symbols are weak.
        for name in unresolved:
            if not needed_symbols[name].weak:
                print "WARNING: Unresolvable symbol %s" % name
        break

    previous_pass_unresolved = unresolved

    library_symbols = {}
    library_symbols_used = {}

    # WORKAROUND: Always add libgcc on old-abi arm
    header = elf_header(find_lib(libraries.copy().pop()))
    if header['machine'] == 40 and header['flags'] & 0xff000000 == 0:
        libraries.add('libgcc_s.so.1')

    # Calculate all symbols each library provides
    for library in libraries:
        path = find_lib(library)
        if not path:
            sys.exit("Library not found: " + library + " in path: "
                    + ':'.join(lib_path))
        symbols = provided_symbols(path)
        library_symbols[library] = {}
        library_symbols_used[library] = set()
        for symbol in symbols:
            for name in symbol.base_names():
                library_symbols[library][name] = symbol

    # which symbols are actually used from each lib
    for name in needed_symbols:
        for lib in libraries:
            if name in library_symbols[lib]:
                library_symbols_used[lib].add(library_symbols[lib][name])

    # reduce libraries
    for library in libraries:
        debug(DEBUG_VERBOSE, "reducing", library)
        debug(DEBUG_SPAM, "using: " + ' '.join([str(i) for i in library_symbols_used[library]]))
        so_file = find_lib(library)
        if root and (re.compile("^" + root).search(so_file)):
            debug(DEBUG_VERBOSE, "no action required for " + so_file)
            if not so_file in available_libs:
                debug(DEBUG_VERBOSE, "adding " + so_file + " to available libs")
                available_libs.append(so_file)
            continue
        so_file_name = os.path.basename(so_file)
        if not so_file:
            sys.exit("File not found:" + library)
        pic_file = find_pic(library)
        if pic_file:
            # we have a pic file, recompile
            debug(DEBUG_SPAM, "extracting from:", pic_file, "so_file:", so_file)
            soname = extract_soname(so_file)
            if soname == "":
                debug(DEBUG_VERBOSE, so_file, " has no soname, copying")
                continue
            debug(DEBUG_SPAM, "soname:", soname)

            symbols = set()
            extra_flags = []
            extra_pre_obj = []
            extra_post_obj = []
            libgcc_link = find_lib("libgcc_s.so.1")

            symbols.update(library_symbols_used[library])

            # libc.so.6 needs its soinit.o and sofini.o as well as the pic
            if soname in ("libc.so.6", "libc.so.6.1", "libc.so.0.1", "libc.so.0.3"):
                # force dso_handle.os to be included, otherwise reduced libc
                # may segfault in ptmalloc_init due to undefined weak reference
                extra_pre_obj.append(sysroot + libc_extras_dir + "/soinit.o")
                extra_post_obj.append(sysroot + libc_extras_dir + "/sofini.o")
                symbols.add(ProvidedSymbol('__dso_handle', None, None, True, True))

            if soname == "libc.so.0":
                symbols.add(ProvidedSymbol('__uClibc_init', None, None, True, True))
                symbols.add(ProvidedSymbol('__uClibc_fini', None, None, True, True))
                extra_pre_obj.append("-Wl,-init,__uClibc_init")

            if soname == "libpthread.so.0":
                symbols.add(ProvidedSymbol('__pthread_initialize_minimal_internal', None, None, True, True))
                extra_flags.append("-Wl,-z,nodelete,-z,initfirst,-init=__pthread_initialize_minimal_internal")

            map_file = find_pic_map(library)
            if map_file:
                extra_flags.append("-Wl,--version-script=" + map_file)

            # compile in only used symbols
            cmd = []
            cmd.extend(gcc_options)
            cmd.append("-nostdlib -nostartfiles -shared -Wl,--gc-sections -Wl,-soname=" + soname)
            cmd.extend(["-u%s" % a.linker_name() for a in symbols])
            cmd.extend(["-o", dest_path + "/" + so_file_name + "-so"])
            cmd.extend(extra_pre_obj)
            cmd.append(pic_file)
            cmd.extend(extra_post_obj)
            cmd.extend(extra_flags)
            cmd.extend(["-L%s" % a for a in [dest_path] + [sysroot + b for b in lib_path if sysroot == "" or b not in ("/" + libdir + "/", "/usr/" + libdir + "/")]])
            if soname != "libgcc_s.so.1":
                cmd.append(library_depends_gcc_libnames(so_file, soname))
                cmd.append(libgcc_link)
            command(target + "gcc", *cmd)

            ## DEBUG
            debug(DEBUG_VERBOSE, so_file, "\t", `os.stat(so_file)[ST_SIZE]`)
            debug(DEBUG_VERBOSE, dest_path + "/" + so_file_name + "-so", "\t",
                  `os.stat(dest_path + "/" + so_file_name + "-so")[ST_SIZE]`)

# Finalising libs and cleaning up
for lib in regexpfilter(os.listdir(dest_path), "(.*)-so$"):
    os.rename(dest_path + "/" + lib + "-so", dest_path + "/" + lib)

# Canonicalize library names.
for lib in regexpfilter(os.listdir(dest_path), "(.*so[.\d]*)$"):
    this_lib_path = dest_path + "/" + lib
    if os.path.islink(this_lib_path):
        debug(DEBUG_VERBOSE, "Unlinking %s." % lib)
        os.remove(this_lib_path)
        continue
    soname = extract_soname(this_lib_path)
    if soname:
        debug(DEBUG_VERBOSE, "Moving %s to %s." % (lib, soname))
        os.rename(dest_path + "/" + lib, dest_path + "/" + soname)

# Make sure the dynamic linker is present and is executable
ld_file_name = os.path.basename(ldlib)
ld_file = find_lib(ld_file_name)

if not os.access(dest_path + "/" + ld_file_name, os.F_OK):
    debug(DEBUG_NORMAL, "I: stripping and copying dynamic linker.")
    command(target + "objcopy", "--strip-unneeded -R .note -R .comment",
            ld_file, dest_path + "/" + ld_file_name)

os.chmod(dest_path + "/" + ld_file_name, 0755)
