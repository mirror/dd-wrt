#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([Jool], [4.1.11], [ydahhrk@gmail.com])
AC_CONFIG_SRCDIR([src/common/xlat.h])
AM_INIT_AUTOMAKE([subdir-objects])
LT_PREREQ([2.4.6])

# Checks for programs.
AC_PROG_CC

# Note: src/usr/iptables/Makefile does not depend on configure.ac.
# If you need it to inherit more warnings, you will have to add them manually
# for now.
AS_IF([test "$GCC" = "yes"],
	[WARNINGCFLAGS="-Wall -pedantic -std=gnu11"],
	[WARNINGCFLAGS=""])
AC_SUBST([WARNINGCFLAGS])

# It's been added because automake requires it, since automake 1.14 it is no longer required.
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([inet_ntoa memset strcasecmp strtol pow])
AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_CHECK_LIB([argp], [argp_parse])

# Dependency: libnlgenl (mandatory)
PKG_CHECK_MODULES(LIBNLGENL3, libnl-genl-3.0 >= 3.2.22)

# Dependency: xtables (optional)
AC_ARG_WITH(
	[xtables],
	AS_HELP_STRING(
		[--with-xtables@<:@=yes|no@:>@],
		[Include xtables dependency? @<:@default=yes@:>@]
	)
)
AS_IF([test "x$with_xtables" != "xno"], [
	PKG_CHECK_MODULES(XTABLES, xtables)
])
AM_CONDITIONAL([XTABLES_ENABLED], [test "x$with_xtables" != "xno"])

# Bash autocompletion option (https://www.swansontec.com/bash-completion.html):
# 1. Offer the user the `--with-bash-completion-dir` configure option,
#    which can be set to a directory, "yes" (default; means autodetect
#    directory) or "no" (disable autocompletion).
AC_ARG_WITH([bash-completion-dir],
	AS_HELP_STRING([--with-bash-completion-dir@<:@=yes|no|PATH@:>@],
		[Directory where the bash auto-completion script will be installed.
		("yes" attempts to guess.) @<:@default=yes@:>@]),
	[],
	[with_bash_completion_dir=yes])

# 2. If the user chose "yes", find a reasonable default directory.
if test "x$with_bash_completion_dir" = "xyes"; then
	PKG_CHECK_MODULES([BASH_COMPLETION], [bash-completion >= 2.0],
		[BASH_COMPLETION_DIR="`pkg-config --variable=completionsdir bash-completion`"],
		[BASH_COMPLETION_DIR="$datadir/bash-completion/completions"])
else
	BASH_COMPLETION_DIR="$with_bash_completion_dir"
fi

# 3. Export ENABLE_BASH_COMPLETION and BASH_COMPLETION_DIR so the Makefile can
#    use them.
AC_SUBST([BASH_COMPLETION_DIR])
AM_CONDITIONAL([ENABLE_BASH_COMPLETION],
	[test "x$with_bash_completion_dir" != "xno"])

# Needed by libtool. (Othewise autoreconf complains.)
LT_INIT
# Needed by libtoolize. (Othewise autoreconf complains.)
AC_CONFIG_MACRO_DIRS([m4])

# Spit out the makefiles.
AC_OUTPUT(Makefile
	src/common/Makefile
	src/usr/Makefile
	src/usr/util/Makefile
	src/usr/nl/Makefile
	src/usr/argp/Makefile
	src/usr/siit/Makefile
	src/usr/nat64/Makefile
	src/usr/joold/Makefile)
