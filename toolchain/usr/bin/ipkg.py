#!/usr/bin/env python
#   Copyright (C) 2001 Alexander S. Guy <a7r@andern.org>
#                      Andern Research Labs
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330,
#   Boston, MA 02111-1307, USA.  */
#
#   Copyright 2001, Russell Nelson <ipkg.py@russnelson.com>
#   Added reading in of packages.
#   Added missing package information fields.
#   Changed render_control() to __repr__().
#
# Current Issues:
#    The API doesn't validate package information fields.  It should be
#        throwing exceptions in the right places.
#    Executions of tar could silently fail.
#    Executions of tar *do* fail, and loudly, because you have to specify a full filename,
#        and tar complains if any files are missing, and the ipkg spec doesn't require
#        people to say "./control.tar.gz" or "./control" when they package files.
#        It would be much better to require ./control or disallow ./control (either)
#        rather than letting people pick.  Some freedoms aren't worth their cost.

import tempfile
import os
import sys
import glob
import md5
import re
import string
import commands
from stat import ST_SIZE

class Package:
    """A class for creating objects to manipulate (e.g. create) ipkg
       packages."""
    def __init__(self, fn=None):
	self.package = None
	self.version = None
	self.architecture = None
	self.maintainer = None
	self.source = None
	self.description = None
	self.depends = None
	self.provides = None
	self.replaces = None
	self.conflicts = None
        self.recommends = None
	self.suggests = None
	self.section = None
        self.filename_header = None
	self.file_list = []
        self.md5 = None
        self.size = None
        self.installed_size = None
        self.filename = None
        self.isdeb = 0

	if fn:
            # see if it is deb format
            f = open(fn, "r")
            magic = f.read(4)
            f.close()
            if (magic == "!<ar"):
                self.isdeb = 1

            # compute the MD5.
            f = open(fn, "r")
            sum = md5.new()
            while 1:
                data = f.read(1024)
                if not data: break
                sum.update(data)
            f.close()
            if sys.version[:1] > '2':
                # when using Python 2.0 or newer
                self.md5 = sum.hexdigest() 
            else:
                self.md5 = string.join(map((lambda x:"%02x" % ord(x)),sum.digest()),'')
            stat = os.stat(fn)
            self.size = stat[ST_SIZE]
            self.filename = os.path.basename(fn)
	    ## sys.stderr.write("  extracting control.tar.gz from %s\n"% (fn,)) 
            if self.isdeb:
                control = os.popen("ar p "+fn+" control.tar.gz | tar xfzO - '*control'","r")
            else:
                control = os.popen("tar xfzO "+fn+" '*control.tar.gz' | tar xfzO - '*control'","r")
            line = control.readline()
            while 1:
                if not line: break
                line = string.rstrip(line)
                lineparts = re.match(r'([\w-]*?):\s*(.*)', line)
		if lineparts:
                    name = string.lower(lineparts.group(1))
		    value = lineparts.group(2)
		    while 1:
			line = control.readline()
			if not line: break
			if line[0] != ' ': break
                        line = string.rstrip(line)
			value = value + '\n' + line
                    # don't allow package to override its own filename
                    if name == "filename":
                        self.filename_header = value
                    else:
                        if self.__dict__.has_key(name):
                            self.__dict__[name] = value
                else:
                    line = control.readline()
            control.close()
            if self.isdeb:
                data = os.popen("ar p "+fn+" data.tar.gz | tar tfz -","r")
            else:
                data = os.popen("tar xfzO "+fn+" '*data.tar.gz' | tar tfz -","r")
            while 1:
                line = data.readline()
                if not line: break
                self.file_list.append(string.rstrip(line))
            data.close()

	self.scratch_dir = None
	self.file_dir = None
	self.meta_dir = None

    def read_control(self, control):
        import os

        line = control.readline()
        while 1:
            if not line: break
            line = string.rstrip(line)
            lineparts = re.match(r'([\w-]*?):\s*(.*)', line)
            if lineparts:
                name = string.lower(lineparts.group(1))
                value = lineparts.group(2)
                while 1:
                    line = control.readline()
                    if not line: break
                    if line[0] != ' ': break
                    value = value + '\n' + line
                if name == 'size':
                    self.size = int(value)
                elif self.__dict__.has_key(name):
                    self.__dict__[name] = value
                if line[0] == '\n':
                    return # consumes one blank line at end of package descriptoin
            else:
                line = control.readline()
                pass
        return    

    def _setup_scratch_area(self):
	self.scratch_dir = "%s/%sipkg" % (tempfile.gettempdir(),
					   tempfile.gettempprefix())
	self.file_dir = "%s/files" % (self.scratch_dir)
	self.meta_dir = "%s/meta" % (self.scratch_dir)

	os.mkdir(self.scratch_dir)
	os.mkdir(self.file_dir)
	os.mkdir(self.meta_dir)

    def set_package(self, package):
	self.package = package

    def get_package(self):
	return self.package
		
    def set_version(self, version):
	self.version = version

    def get_version(self):
	return self.version

    def set_architecture(self, architecture):
	self.architecture = architecture

    def get_architecture(self):
	return self.architecture

    def set_maintainer(self, maintainer):
	self.maintainer = maintainer

    def get_maintainer(self):
	return self.maintainer

    def set_source(self, source):
	self.source = source

    def get_source(self):
	return self.source

    def set_description(self, description):
	self.description = description

    def get_description(self):
	return self.description

    def set_depends(self, depends):
	self.depends = depends

    def get_depends(self, depends):
	return self.depends

    def set_provides(self, provides):
	self.provides = provides

    def get_provides(self, provides):
	return self.provides

    def set_replaces(self, replaces):
	self.replaces = replaces

    def get_replaces(self, replaces):
	return self.replaces

    def set_conflicts(self, conflicts):
	self.conflicts = conflicts

    def get_conflicts(self, conflicts):
	return self.conflicts

    def set_suggests(self, suggests):
	self.suggests = suggests

    def get_suggests(self, suggests):
	return self.suggests

    def set_section(self, section):
	self.section = section

    def get_section(self, section):
	return self.section

    def get_file_list(self):
	return self.file_list

    def write_package(self, dirname):
        buf = self.render_control()
	file = open("%s/control" % self.meta_dir, 'w')
	file.write(buf)

	self._setup_scratch_area()
	cmd = "cd %s ; tar cvfz %s/control.tar.gz control" % (self.meta_dir,
							      self.scratch_dir)

	cmd_out, cmd_in, cmd_err = os.popen3(cmd)
	
	while cmd_err.readline() != "":
	    pass

	cmd_out.close()
	cmd_in.close()
	cmd_err.close()

	bits = "control.tar.gz"

	if self.file_list:
		cmd = "cd %s ; tar cvfz %s/data.tar.gz" % (self.file_dir,
					   		   self.scratch_dir)

		cmd_out, cmd_in, cmd_err = os.popen3(cmd)

		while cmd_err.readline() != "":
		    pass

		cmd_out.close()
		cmd_in.close()
		cmd_err.close()

		bits = bits + " data.tar.gz"

	file = "%s_%s_%s.ipk" % (self.package, self.version, self.architecture)
	cmd = "cd %s ; tar cvfz %s/%s %s" % (self.scratch_dir,
					     dirname,
					     file,
					     bits)

	cmd_out, cmd_in, cmd_err = os.popen3(cmd)

	while cmd_err.readline() != "":
	    pass

	cmd_out.close()
	cmd_in.close()
	cmd_err.close()

    def __repr__(self):
	out = ""

	# XXX - Some checks need to be made, and some exceptions
	#       need to be thrown. -- a7r

        if self.package: out = out + "Package: %s\n" % (self.package)
        if self.version: out = out + "Version: %s\n" % (self.version)
        if self.depends: out = out + "Depends: %s\n" % (self.depends)
        if self.provides: out = out + "Provides: %s\n" % (self.provides)
        if self.replaces: out = out + "Replaces: %s\n" % (self.replaces)
        if self.conflicts: out = out + "Conflicts: %s\n" % (self.conflicts)
        if self.suggests: out = out + "Suggests: %s\n" % (self.suggests)
        if self.recommends: out = out + "Recommends: %s\n" % (self.recommends)
        if self.section: out = out + "Section: %s\n" % (self.section)
        if self.architecture: out = out + "Architecture: %s\n" % (self.architecture)
        if self.maintainer: out = out + "Maintainer: %s\n" % (self.maintainer)
        if self.md5: out = out + "MD5Sum: %s\n" % (self.md5)
        if self.size: out = out + "Size: %d\n" % int(self.size)
        if self.installed_size: out = out + "InstalledSize: %d\n" % int(self.installed_size)
        if self.filename: out = out + "Filename: %s\n" % (self.filename)
        if self.source: out = out + "Source: %s\n" % (self.source)
        if self.description: out = out + "Description: %s\n" % (self.description)
	out = out + "\n"

	return out

    def __del__(self):
	# XXX - Why is the `os' module being yanked out before Package objects
	#       are being destroyed?  -- a7r
        pass

class Packages:
    """A currently unimplemented wrapper around the ipkg utility."""
    def __init__(self):
        self.packages = {}
        return

    def add_package(self, pkg):
        package = pkg.package
        arch = pkg.architecture
        name = ("%s:%s" % (package, arch))
        if (not self.packages.has_key(name)):
            self.packages[name] = pkg
        (s, outtext) = commands.getstatusoutput("ipkg-compare-versions %s '>' %s" % (pkg.version, self.packages[name].version))    
        if (s == 0):
            self.packages[name] = pkg
            return 0
        else:
            return 1

    def read_packages_file(self, fn):
        f = open(fn, "r")
        while 1:
            pkg = Package()
            pkg.read_control(f)
            if pkg.get_package():
                self.add_package(pkg)
            else:
                break
        f.close()    
        return

    def write_packages_file(self, fn):
        f = open(fn, "w")
        names = self.packages.keys()
        names.sort()
        for name in names:
            f.write(self.packages[name].__repr__())
        return    

    def keys(self):
        return self.packages.keys()

    def __getitem__(self, key):
        return self.packages[key]

if __name__ == "__main__":
    package = Package()

    package.set_package("FooBar")
    package.set_version("0.1-fam1")
    package.set_architecture("arm")
    package.set_maintainer("Testing <testing@testing.testing>")
    package.set_depends("libc")
    package.set_description("A test of the APIs.")

    print "<"
    sys.stdout.write(package)
    print ">"

    package.write_package("/tmp")

