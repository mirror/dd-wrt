#! /usr/bin/env python
# vim: tabstop=8 shiftwidth=8 expandtab
# $Id: setup.py,v 1.3 2012/03/05 04:54:01 nanard Exp $
#
# python script to build the libnatpmp module under unix

from setuptools import setup, Extension
from setuptools.command import build_ext
import subprocess
import os

EXT = ['libnatpmp.a']

class make_then_build_ext(build_ext.build_ext):
      def run(self):
            subprocess.check_call([os.environ.get('MAKE','make')] + EXT)
            build_ext.build_ext.run(self)

setup(name="libnatpmp",
      version="1.0",
      author='Thomas BERNARD',
      author_email='miniupnp@free.fr',
      license=open('LICENSE').read(),
      description='NAT-PMP library',
      cmdclass={'build_ext': make_then_build_ext},
      ext_modules=[
        Extension(name="libnatpmp", sources=["libnatpmpmodule.c"],
                  extra_objects=EXT,
                  define_macros=[('ENABLE_STRNATPMPERR', None)]
        )]
     )

