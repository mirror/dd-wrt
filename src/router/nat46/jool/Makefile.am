AUTOMAKE_OPTIONS = foreign

# It's taken quite a bit of effort to convince myself that this is probably for
# the best.
#
# Here's the starting point: I'd like to include src/mod in SUBDIRS, because I
# want users to be able to compile the whole suite with a single command (make).
# (As a bonus, it would also allow me to obviate DIST_SUBDIRS on account of
# being identical to SUBDIRS.)
# 
# Here's the problem: dpkg-buildpackage uses SUBDIRS during the package build.
# We don't want dpkg-buildpackage to build src/mod, because any kernel update
# will render the resulting binaries obsolete and useless. What we want is to
# ship the source code. Once installed, DKMS will automatically recompile it
# every time the kernel is updated.
#
# So the solution is to include src/mod in the distribution, but omit its
# compilation by default. Hence the SUBDIRS and DIST_SUBDIRS below.
#
# (I'm aware that I can just modify SUBDIRS using Debian's patch thingamajig,
# but as someone whose first ever package is a bunch of freaking kernel modules
# coupled with their corresponding userspace tools, libraries and systemd unit
# files, the idea of adding even more shit on top makes me sick.)
#
# So, going back to our starting point, we can't include src/mod in SUBDIRS.
#
# Here are a couple arguments that help sustain this solution a little further:
#
# 1. Most users are probably going to install the packages, instead of using the
#    full-suite make anyway.
# 2. The thing about the full-suite make is that it would install the kernel
#    modules via Kbuild, which we don't recommend in the first place. DKMS is
#    way better for users.
#
# Perhaps I've been misunderstandig Kbuild all this time, and is really only
# intended for development. As a matter of fact, I'm going to remove it from the
# documentation shortly. Screw it.
SUBDIRS = src/common src/usr
DIST_SUBDIRS = src/common src/mod src/usr

# Needed by libtoolize. (Othewise autoreconf complains.)
ACLOCAL_AMFLAGS=-I m4

EXTRA_DIST=dkms.conf README.md
