ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = \
	src \
	rules \
	test

if ENABLE_MANPAGES
SUBDIRS += \
	man
endif

if ENABLE_HWDB
SUBDIRS += \
	hwdb
endif

if ENABLE_RULE_GENERATOR
SUBDIRS += \
	rule_generator
endif
