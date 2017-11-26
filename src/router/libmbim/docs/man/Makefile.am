
dist_man_MANS = \
	mbimcli.1 \
	mbim-network.1

# List of all source files which affect the output of --help-all
MBIMCLI_SOURCES_WITH_HELP = \
	$(top_srcdir)/src/mbimcli/mbimcli-basic-connect.c \
	$(top_srcdir)/src/mbimcli/mbimcli.c

# Depend only in the source files, not in the actual program, so that the
# manpage doesn't get rebuilt when building from a tarball
# Also, make sure that the mbimcli.1 file is always generated, even when help2man
# is not available
mbimcli.1: $(MBIMCLI_SOURCES_WITH_HELP)
	$(AM_V_GEN) \
		$(HELP2MAN) \
			--output=$@ \
			--name='Control MBIM devices' \
			--help-option='--help-all' \
			--libtool \
			$(top_builddir)/src/mbimcli/mbimcli || \
		touch $@

# Depend only in the .in file, not in the actual script, so that the
# manpage doesn't get rebuilt when building from a tarball.
# Make sure that the mbim-network.1 file is always generated, even when help2man
# is not available
mbim-network.1: $(top_srcdir)/utils/mbim-network.in
	$(AM_V_GEN) \
		$(HELP2MAN) \
			--output=$@ \
			--name='Simple network management of MBIM devices' \
			$(top_builddir)/utils/mbim-network || \
		touch $@
