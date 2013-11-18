# $Id: setup.py 4076 2006-12-05 02:00:19Z esr $
# Creates build/lib.linux-${arch}-${pyvers}/gpspacket.so,
# where ${arch} is an architecture and ${pyvers} is a Python version.
from distutils.core import setup, Extension
setup(name="gpspacket", version="1.0",
      ext_modules=[Extension("gpspacket",
                             ["gpspacket.c", "packet.c",
                              "isgps.c", "rtcm.c", "strl.c", "hex.c"])])
