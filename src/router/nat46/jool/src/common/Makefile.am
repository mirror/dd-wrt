# This directory contains code that is shared between kernelspace and userspace.

# Given that the shared libraries are presently private, we can skip the
# installation of all of this. If you ever need to publish some libraries,
# notice that a good chunk of this API does not need to be exported, as it's
# only used internally by both the kernel modules and libjoolnl.

# The .c files are header-source hybrids.
# They are implementation code that needs to be identical in user and kernel.
# Each one of them should be included no more than once in kernelspace, and no
# more than once in userspace.

noinst_HEADERS = \
	config.c config.h \
	constants.h \
	global.c global.h \
	iptables.h \
	session.h \
	stats.h \
	types.c types.h \
	xlat.h
